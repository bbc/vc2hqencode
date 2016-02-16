/*****************************************************************************
 * encode.cpp : Variable Length Encoding
 *****************************************************************************
 * Copyright (C) 2014-2015 BBC
 *
 * Authors: James P. Weaver <james.barrett@bbc.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at ipstudio@bbc.co.uk.
 *****************************************************************************/

#include "vc2hqencode.h"
#include "encode.hpp"
#include "lut.hpp"
#include "logger.hpp"
#include "debug.hpp"

#include <x86intrin.h>

#include "encode_slice_component_optimised.hpp"

//#define FIXED_QI 30
#define MIN_QI 0
#define MAX_QI 64

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

#define max(A,B) (((A)>(B))?(A):(B))
#define min(A,B) (((A)<(B))?(A):(B))

#include "quantiserselection.hpp"


// This is a very noddy implementation used for coding the header parameters, it is *never* used for coded coefficients
//   it returns the length of the coded string, and places the coded string itself at the location pointed to by output
uint8_t encode_uint(uint16_t x, uint32_t *output, uint8_t *lengthout) {
  if (x == 0) {
    *output = 1;
    *lengthout = 1;
    return 1;
  }

  uint32_t y = x+1;
  int l = ((32 - __builtin_clz(y)) - 1)*2 + 1;
  y = (y | (y << 8)) & 0x00FF00FF;
  y = (y | (y << 4)) & 0x0F0F0F0F;
  y = (y | (y << 2)) & 0x33333333;
  y = (y | (y << 1)) & 0x55555555;
  y <<= 1;
  y |= 1;
  y &= ((1 << l) - 1);
  *output    = y;
  *lengthout = l;
  return l;
}

template<class T> inline void zero_slice(CodedSlice<T> *slice) {
  slice->length[0]  = 0;
  slice->samples[0] = 0;
  slice->length[1]  = 0;
  slice->samples[1] = 0;
  slice->length[2]  = 0;
  slice->samples[2] = 0;
}

template<int w, int h, int d, int QUAL, int passes, class T> void encode_slices(CodedSlice<T> *slices, int n, QuantisationMatrices *matrices, int encode_length, int wavelet_index, int slice_size_scalar, int, int, int) {
#ifdef DEBUG
  uint32_t samples[64];
  for (int i = 0; i < 64; i++)
    samples[i] = 0;
#endif

  int n_tgt_slices[passes];
  CodedSlice<T> *tgt_slices[passes][n];

  int remaining_length = encode_length;
  n_tgt_slices[0] = 0;
  for (int i = 0; i < n; i++) {
    int coded_size = ( ( remaining_length / slice_size_scalar )/( n - i ) ) * slice_size_scalar;

    choose_quantiser<w,h,d,QUAL,T>(&slices[i], coded_size, wavelet_index, slice_size_scalar, matrices);
    encode_slice_component<w,h,d,T>(&slices[i], 0, matrices);
    encode_slice_component<w/2,h,d,T>(&slices[i], 1, matrices);
    encode_slice_component<w/2,h,d,T>(&slices[i], 2, matrices);
    int length[3];
    length[0] = (slices[i].length[0] + slice_size_scalar - 1)/slice_size_scalar;
    length[1] = (slices[i].length[1] + slice_size_scalar - 1)/slice_size_scalar;
    length[2] = (slices[i].length[2] + slice_size_scalar - 1)/slice_size_scalar;
    int cl = 4 + length[0]*slice_size_scalar + length[1]*slice_size_scalar + length[2]*slice_size_scalar;
    if (cl > coded_size || length[0] > 255 || length[1] > 255 || length[2] > 255) {
      writelog(LOG_ERROR, "%s:%d:  Slice Length cannot be brought sane via quantisation, blanking slice\n", __FILE__, __LINE__);
      zero_slice<T>(&slices[i]);
      cl = 4;
    }
    if (passes == 1) {
      remaining_length -= coded_size;
      slices[i].padding = coded_size - cl;
    } else {
      remaining_length -= cl;
      slices[i].padding = 0;

      if (coded_size == cl) {
        tgt_slices[0][n_tgt_slices[0]++] = &slices[i];
      }
    }

#ifdef DEBUG
    samples[slices[i].qindex]++;
#endif
  }

  for (int pass = 1; pass < passes; pass++) {
    n_tgt_slices[pass] = 0;
    for (int i = 0; i < n_tgt_slices[pass - 1]; i++) {
      int length[3];
      length[0] = (tgt_slices[pass - 1][i]->length[0] + slice_size_scalar - 1)/slice_size_scalar;
      length[1] = (tgt_slices[pass - 1][i]->length[1] + slice_size_scalar - 1)/slice_size_scalar;
      length[2] = (tgt_slices[pass - 1][i]->length[2] + slice_size_scalar - 1)/slice_size_scalar;
      int old_cl = 4 + length[0]*slice_size_scalar + length[1]*slice_size_scalar + length[2]*slice_size_scalar;
      int coded_size = old_cl + ((remaining_length/slice_size_scalar)/(n_tgt_slices[pass - 1] - i)*slice_size_scalar);

      choose_quantiser<w,h,d,QUAL,T>(tgt_slices[pass - 1][i], coded_size, wavelet_index, slice_size_scalar, matrices);
      encode_slice_component<w,h,d,T>(tgt_slices[pass - 1][i], 0, matrices);
      encode_slice_component<w/2,h,d,T>(tgt_slices[pass - 1][i], 1, matrices);
      encode_slice_component<w/2,h,d,T>(tgt_slices[pass - 1][i], 2, matrices);
      length[0] = (tgt_slices[pass - 1][i]->length[0] + slice_size_scalar - 1)/slice_size_scalar;
      length[1] = (tgt_slices[pass - 1][i]->length[1] + slice_size_scalar - 1)/slice_size_scalar;
      length[2] = (tgt_slices[pass - 1][i]->length[2] + slice_size_scalar - 1)/slice_size_scalar;
      int cl = 4 + length[0]*slice_size_scalar + length[1]*slice_size_scalar + length[2]*slice_size_scalar;
      if (cl > coded_size || length[0] > 255 || length[1] > 255 || length[2] > 255) {
        writelog(LOG_ERROR, "%s:%d:  Slice Length cannot be brought sane via quantisation, blanking slice\n", __FILE__, __LINE__);
        zero_slice<T>(tgt_slices[pass - 1][i]);
        cl = 4;
      }
      remaining_length -= cl - old_cl;
      tgt_slices[pass - 1][i]->padding = 0;

      if (coded_size == cl) {
        tgt_slices[pass][n_tgt_slices[pass]++] = tgt_slices[pass - 1][i];
      }
    }
  }

  if (passes > 1) {
    for (int i = 0; i < n && remaining_length > 0; i++) {
      int length[3];
      length[0] = (slices[i].length[0] + slice_size_scalar - 1)/slice_size_scalar;
      length[1] = (slices[i].length[1] + slice_size_scalar - 1)/slice_size_scalar;
      length[2] = (slices[i].length[2] + slice_size_scalar - 1)/slice_size_scalar;
      int cl = 4 + length[0]*slice_size_scalar + length[1]*slice_size_scalar + length[2]*slice_size_scalar;
      int p = (4 + 3*255*slice_size_scalar) - cl;
      if (p > remaining_length) {
        p = remaining_length;
      }
      slices[i].padding = p;
      remaining_length -= p;
    }
  }

  if (remaining_length > 0) {
    writelog(LOG_ERROR, "%s:%d: Cannot reassign available padding!\n", __FILE__, __LINE__);
    throw VC2ENCODER_CODEROVERRUN;
  }

#ifdef DEBUG_PRINT_QUANTISERS
  printf("------------------------------------\n");
  printf("    Slice Quantiser Frequencies\n");
  printf("------------------------------------\n");
  for (int i = 0; i < 64; i++)
    printf("  %2d : %4d\n", i, samples[i]);
  printf("------------------------------------\n");
#endif
}

template<class T> void encode_slices_fallback(CodedSlice<T> *slices, int n, QuantisationMatrices *matrices, int encode_length, int wavelet_index, int slice_size_scalar, int w, int h, int d) {
#ifdef DEBUG
  uint32_t samples[64];
  for (int i = 0; i < 64; i++)
    samples[i] = 0;
#endif

  int remaining_length = encode_length;
  for (int i = 0; i < n; i++) {
    int coded_size = ( ( remaining_length / slice_size_scalar )/( n - i ) ) * slice_size_scalar;

    choose_quantiser_fallback<T>(&slices[i], coded_size, wavelet_index, slice_size_scalar, matrices, w, h, d);
    encode_slice_component_fallback<T>(&slices[i], 0, matrices, w,  h,d);
    encode_slice_component_fallback<T>(&slices[i], 1, matrices, w/2,h,d);
    encode_slice_component_fallback<T>(&slices[i], 2, matrices, w/2,h,d);
    int length[3];
    length[0] = (slices[i].length[0] + slice_size_scalar - 1)/slice_size_scalar;
    length[1] = (slices[i].length[1] + slice_size_scalar - 1)/slice_size_scalar;
    length[2] = (slices[i].length[2] + slice_size_scalar - 1)/slice_size_scalar;
    int cl = 4 + length[0]*slice_size_scalar + length[1]*slice_size_scalar + length[2]*slice_size_scalar;
    if (cl > coded_size || length[0] > 255 || length[1] > 255 || length[2] > 255) {
      writelog(LOG_ERROR, "%s:%d:  Slice Length cannot be brought sane via quantisation, blanking slice\n", __FILE__, __LINE__);
      zero_slice<T>(&slices[i]);
      cl = 4;
    }
    remaining_length -= coded_size;
    slices[i].padding = coded_size - cl;

#ifdef DEBUG
    samples[slices[i].qindex]++;
#endif
  }

  if (remaining_length > 0) {
    writelog(LOG_ERROR, "%s:%d: Cannot reassign available padding!\n", __FILE__, __LINE__);
    throw VC2ENCODER_CODEROVERRUN;
  }

#ifdef DEBUG_PRINT_QUANTISERS
  printf("------------------------------------\n");
  printf("    Slice Quantiser Frequencies\n");
  printf("------------------------------------\n");
  for (int i = 0; i < 64; i++)
    printf("  %2d : %4d\n", i, samples[i]);
  printf("------------------------------------\n");
#endif
}

SliceEncoderFunc32 get_slice_encoder32(int w, int h, int d, int QUAL, int passes) {
  if (d > 4 || w % (1 << d) || h % (1 << d)) {
    throw VC2ENCODER_BADPARAMS;
  }

  switch (passes) {
  case 1:
    switch(d) {
    case 2:
      if (w == 8 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_FULLSEARCH, 1, int32_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_HALFSEARCH, 1, int32_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_QUARTERSEARCH, 1, int32_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_EIGHTHSEARCH, 1, int32_t>;
        }
      }
      break;
    case 3:
      if (w == 32 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_FULLSEARCH, 1, int32_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_HALFSEARCH, 1, int32_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_QUARTERSEARCH, 1, int32_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_EIGHTHSEARCH, 1, int32_t>;
        }
      }
      break;
    }
    break;
  case 2:
    switch(d) {
    case 2:
      if (w == 8 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_FULLSEARCH, 2, int32_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_HALFSEARCH, 2, int32_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_QUARTERSEARCH, 2, int32_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_EIGHTHSEARCH, 2, int32_t>;
        }
      }
      break;
    case 3:
      if (w == 32 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_FULLSEARCH, 2, int32_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_HALFSEARCH, 2, int32_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_QUARTERSEARCH, 2, int32_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_EIGHTHSEARCH, 2, int32_t>;
        }
      }
      break;
    }
    break;
  case 3:
    switch(d) {
    case 2:
      if (w == 8 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_FULLSEARCH, 3, int32_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_HALFSEARCH, 3, int32_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_QUARTERSEARCH, 3, int32_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_EIGHTHSEARCH, 3, int32_t>;
        }
      }
      break;
    case 3:
      if (w == 32 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_FULLSEARCH, 3, int32_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_HALFSEARCH, 3, int32_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_QUARTERSEARCH, 3, int32_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_EIGHTHSEARCH, 3, int32_t>;
        }
      }
      break;
    }
    break;
  }

  return encode_slices_fallback;
}

SliceEncoderFunc16 get_slice_encoder16(int w, int h, int d, int QUAL, int passes) {
  if (d > 4 || w % (1 << d) || h % (1 << d)) {
    throw VC2ENCODER_BADPARAMS;
  }

  switch (passes) {
  case 1:
    switch(d) {
    case 2:
      if (w == 8 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_FULLSEARCH, 1, int16_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_HALFSEARCH, 1, int16_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_QUARTERSEARCH, 1, int16_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_EIGHTHSEARCH, 1, int16_t>;
        }
      }
      break;
    case 3:
      if (w == 32 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_FULLSEARCH, 1, int16_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_HALFSEARCH, 1, int16_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_QUARTERSEARCH, 1, int16_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_EIGHTHSEARCH, 1, int16_t>;
        }
      }
      break;
    }
    break;
  case 2:
    switch(d) {
    case 2:
      if (w == 8 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_FULLSEARCH, 2, int16_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_HALFSEARCH, 2, int16_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_QUARTERSEARCH, 2, int16_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_EIGHTHSEARCH, 2, int16_t>;
        }
      }
      break;
    case 3:
      if (w == 32 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_FULLSEARCH, 2, int16_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_HALFSEARCH, 2, int16_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_QUARTERSEARCH, 2, int16_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_EIGHTHSEARCH, 2, int16_t>;
        }
      }
      break;
    }
    break;
  case 3:
    switch(d) {
    case 2:
      if (w == 8 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_FULLSEARCH, 3, int16_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_HALFSEARCH, 3, int16_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_QUARTERSEARCH, 3, int16_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<8,8,2, QUANTISER_SELECTION_EIGHTHSEARCH, 3, int16_t>;
        }
      }
      break;
    case 3:
      if (w == 32 && h == 8) {
        switch(QUAL) {
        case QUANTISER_SELECTION_FULLSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_FULLSEARCH, 3, int16_t>;
        case QUANTISER_SELECTION_HALFSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_HALFSEARCH, 3, int16_t>;
        case QUANTISER_SELECTION_QUARTERSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_QUARTERSEARCH, 3, int16_t>;
        case QUANTISER_SELECTION_EIGHTHSEARCH:
          return encode_slices<32,8,3, QUANTISER_SELECTION_EIGHTHSEARCH, 3, int16_t>;
        }
      }
      break;
    }
    break;
  }

  return encode_slices_fallback;
}
