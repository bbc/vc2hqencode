/*****************************************************************************
 * test_transforms.cpp : Test wavelet transform functions
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

#include <stdint.h>
#include <cstdio>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <vc2hqencode/vc2hqencode.h>
#include "../vc2transform_c/transform_c.hpp"
#include "../vc2transform_sse4_2/transform_sse4_2.hpp"
#include "../vc2transform_avx/transform_avx.hpp"
#include "../vc2transform_avx2/transform_avx2.hpp"

const char *VC2EncoderWaveletFilterTypeString[] = { "Deslauriers-Dubuc 9,7",
                                                    "LeGall 5,3",
                                                    "Deslauriers-Dubuc 13,7",
                                                    "Haar (0 shift)",
                                                    "Haar (1 shift)",
                                                    "Fidelity",
                                                    "Daubechies 9,7" };

const char *VC2EncoderInputFormatString[] = {
  "10P2",
  "V210",
};

const char *ComponentString[] = {
  "Y",
  "U",
  "V",
};

struct transforminitial_test {
  int wavelet_index;
  int active_bits;
  int coef_size;
  VC2EncoderInputFormat fmt;
};

const transforminitial_test TRANSFORMINITIAL_TEST[] = {
  { VC2ENCODER_WFT_HAAR_NO_SHIFT,     10, 2, VC2ENCODER_INPUT_10P2 },
  { VC2ENCODER_WFT_HAAR_SINGLE_SHIFT, 10, 2, VC2ENCODER_INPUT_10P2 },
  { VC2ENCODER_WFT_LEGALL_5_3,        10, 2, VC2ENCODER_INPUT_10P2 },
  { VC2ENCODER_WFT_HAAR_NO_SHIFT,     10, 4, VC2ENCODER_INPUT_10P2 },
  { VC2ENCODER_WFT_HAAR_SINGLE_SHIFT, 10, 4, VC2ENCODER_INPUT_10P2 },
  { VC2ENCODER_WFT_LEGALL_5_3,        10, 4, VC2ENCODER_INPUT_10P2 },

  { VC2ENCODER_WFT_HAAR_NO_SHIFT,     10, 2, VC2ENCODER_INPUT_V210 },
  { VC2ENCODER_WFT_HAAR_SINGLE_SHIFT, 10, 2, VC2ENCODER_INPUT_V210 },
  { VC2ENCODER_WFT_LEGALL_5_3,        10, 2, VC2ENCODER_INPUT_V210 },
  { VC2ENCODER_WFT_HAAR_NO_SHIFT,     10, 4, VC2ENCODER_INPUT_V210 },
  { VC2ENCODER_WFT_HAAR_SINGLE_SHIFT, 10, 4, VC2ENCODER_INPUT_V210 },
  { VC2ENCODER_WFT_LEGALL_5_3,        10, 4, VC2ENCODER_INPUT_V210 },
};

struct transform_test {
  int wavelet_index;
  int coef_size;
};

const transform_test TRANSFORM_TEST[] = {
  { VC2ENCODER_WFT_HAAR_NO_SHIFT,     2 },
  { VC2ENCODER_WFT_HAAR_SINGLE_SHIFT, 2 },
  { VC2ENCODER_WFT_LEGALL_5_3,        2 },
  { VC2ENCODER_WFT_HAAR_NO_SHIFT,     4 },
  { VC2ENCODER_WFT_HAAR_SINGLE_SHIFT, 4 },
  { VC2ENCODER_WFT_LEGALL_5_3,        4 },
};

int perform_transforminitialtest(const transforminitial_test &data, void *idata_pre, bool HAS_SSE4_2, bool HAS_AVX, bool HAS_AVX2) {
  int r = 0;
  (void)HAS_SSE4_2;(void)HAS_AVX;(void)HAS_AVX2;
  (void)idata_pre;

  const int width   = 480;
  const int height  = 270;
  int istride;
  const int ostride = ((width + 8 + 15)/16)*16;
  int idata_length;
  int comps;

  void *idata;
  if (data.fmt == VC2ENCODER_INPUT_10P2) {
    comps = 3;
    istride = 1024;
    idata_length = istride*height*sizeof(uint16_t);
    idata = malloc(idata_length);
    for (int i = 0; i < istride*height; i++) {
      ((uint16_t*)idata)[i] = ((uint16_t*)idata_pre)[i]&0x3FF;
    }
  } else if (data.fmt == VC2ENCODER_INPUT_V210) {
    comps = 1;
    istride = (width + 47)/48*128;
    idata_length = istride*height;
    idata = malloc(idata_length);
    memset(idata, 0, idata_length);
    for (int y = 0; y < height; y++) {
      int x = 0;
      for (; x < width; x+=6) {
        ((uint32_t*)idata)[y*istride/4 + x/6*4 + 0] = ((uint32_t*)idata_pre)[y*istride/4 + x/6*4 + 0]&0x3FFFFFFF;
        ((uint32_t*)idata)[y*istride/4 + x/6*4 + 1] = ((uint32_t*)idata_pre)[y*istride/4 + x/6*4 + 1]&0x3FFFFFFF;
        ((uint32_t*)idata)[y*istride/4 + x/6*4 + 2] = ((uint32_t*)idata_pre)[y*istride/4 + x/6*4 + 2]&0x3FFFFFFF;
        ((uint32_t*)idata)[y*istride/4 + x/6*4 + 3] = ((uint32_t*)idata_pre)[y*istride/4 + x/6*4 + 3]&0x3FFFFFFF;
      }
    }
  } else {
    printf("INVALID INPUT FORMAT\n\n");
    r = 1;
    return 1;
  }

  for (int c = 0; c < comps; c++) {
    printf("%-20s: H 0/*  %-4s (%s) ", VC2EncoderWaveletFilterTypeString[data.wavelet_index], VC2EncoderInputFormatString[data.fmt], ComponentString[c]);
    if (data.coef_size == 2)
      printf("16-bit ");
    else
      printf("32-bit ");

    printf(" C [ ");
    InplaceTransformInitial chtrans = NULL;
    InplaceTransform        cvtrans = NULL;
    try {
      chtrans = get_htransforminitial_c(data.wavelet_index, data.active_bits, data.coef_size, c, data.fmt);
      cvtrans = get_vtransform_c(data.wavelet_index, 0, data.coef_size, data.fmt);
    } catch (...) {
      printf(" ????  ]\n:");
      r = 1;
      break;
    }
    printf("EXISTS ] ");

    void *cdata = memalign(32, ostride*height*data.coef_size*2);
    memset(cdata, 0, ostride*height*data.coef_size*2);
    {
      void *odata[] = { (uint8_t*)cdata, (uint8_t*)cdata + ostride*height*data.coef_size, (uint8_t*)cdata + ostride*height*data.coef_size + ostride*height*data.coef_size/2 };
      chtrans((const char *)idata, istride, odata, ostride, width, height, width, height);
      if (cvtrans)
        cvtrans(cdata, ostride, width, height, 1);
    }



    InplaceTransformInitial sse42htrans = NULL;
    InplaceTransform        sse42vtrans = NULL;
    if (HAS_SSE4_2) {
      printf(" SSE4.2 [ ");
      try {
        sse42htrans = get_htransforminitial_sse4_2(data.wavelet_index, data.active_bits, data.coef_size, c, data.fmt);
        sse42vtrans = get_vtransform_sse4_2(data.wavelet_index, 0, data.coef_size, data.fmt);
      } catch (...) {
        printf(" ????  ]\n");
        r = 1;
        free(cdata);
        break;
      }

      if (sse42htrans == chtrans && sse42vtrans == cvtrans) {
        printf(" NONE  ] ");
      } else {
        void *tdata = memalign(32, ostride*height*data.coef_size*2);
        memset(tdata, 0, ostride*height*data.coef_size*2);
        {
          void *odata[] = { (uint8_t*)tdata, (uint8_t*)tdata + ostride*height*data.coef_size, (uint8_t*)tdata + ostride*height*data.coef_size + ostride*height*data.coef_size/2 };
          sse42htrans((const char *)idata, istride, odata, ostride, width, height, width, height);
          if (sse42vtrans)
            sse42vtrans(tdata, ostride, width, height, 1);
        }
        int cmp = 0;
        if (data.fmt == VC2ENCODER_INPUT_10P2)
          cmp = memcmp(cdata, tdata, ostride*height*data.coef_size*2);
        else {
          for (int y = 0; y < height; y++) {
            cmp |= memcmp(&((int8_t*)cdata)[y*ostride*data.coef_size], &((int8_t*)tdata)[y*ostride*data.coef_size], width*data.coef_size);
          }
        }
        if (cmp) {
          printf(" FAIL  ]\n");
          for (int i = 0; i < ostride*height*data.coef_size*2; i++) {
            if (((uint8_t*)cdata)[i] != ((uint8_t*)tdata)[i]) {
              printf("Differs first at byte %d: 0x%02x != 0x%02x\n", i, ((uint8_t*)cdata)[i], ((uint8_t*)tdata)[i]);
              break;
            }
          }
          r = 1;
          free(tdata);
          free(cdata);
          break;
        }

        printf("  OK   ] ");
        free(tdata);
      }
    }

    InplaceTransformInitial avxhtrans = NULL;
    InplaceTransform        avxvtrans = NULL;
    if (HAS_AVX) {
      printf(" AVX [ ");
      try {
        avxhtrans = get_htransforminitial_avx(data.wavelet_index, data.active_bits, data.coef_size, c, data.fmt);
        avxvtrans = get_vtransform_avx(data.wavelet_index, 0, data.coef_size, data.fmt);
      } catch (...) {
        printf(" ????  ]\n");
        r = 1;
        free(cdata);
        break;
      }

      if (avxhtrans == sse42htrans && avxvtrans == sse42vtrans) {
        printf(" NONE  ] ");
      } else {
        void *tdata = memalign(32, ostride*height*data.coef_size*2);
        memset(tdata, 0, ostride*height*data.coef_size*2);
        {
          void *odata[] = { (uint8_t*)tdata, (uint8_t*)tdata + ostride*height*data.coef_size, (uint8_t*)tdata + ostride*height*data.coef_size + ostride*height*data.coef_size/2 };
          avxhtrans((const char *)idata, istride, odata, ostride, width, height, width, height);
          if (avxvtrans)
            avxvtrans(tdata, ostride, width, height, 1);
        }
        int cmp = 0;
        if (data.fmt == VC2ENCODER_INPUT_10P2)
          cmp = memcmp(cdata, tdata, ostride*height*data.coef_size*2);
        else {
          for (int y = 0; y < height; y++) {
            cmp |= memcmp(&((int8_t*)cdata)[y*ostride*data.coef_size], &((int8_t*)tdata)[y*ostride*data.coef_size], width*data.coef_size);
          }
        }
        if (cmp) {
          printf(" FAIL  ]\n");
          for (int i = 0; i < ostride*height*data.coef_size*2; i++) {
            if (((uint8_t*)cdata)[i] != ((uint8_t*)tdata)[i]) {
              printf("Differs first at byte %d: 0x%02x != 0x%02x\n", i, ((uint8_t*)cdata)[i], ((uint8_t*)tdata)[i]);
              break;
            }
          }
          r = 1;
          free(tdata);
          free(cdata);
          break;
        }

        printf("  OK   ] ");
        free(tdata);
      }
    }

    InplaceTransformInitial avx2htrans = NULL;
    InplaceTransform        avx2vtrans = NULL;
    if (HAS_AVX2) {
      printf(" AVX2 [ ");
      try {
        avx2htrans = get_htransforminitial_avx2(data.wavelet_index, data.active_bits, data.coef_size, c, data.fmt);
        avx2vtrans = get_vtransform_avx2(data.wavelet_index, 0, data.coef_size, data.fmt);
      } catch (...) {
        printf(" ????  ]\n");
        r = 1;
        free(cdata);
        break;
      }

      if (avxhtrans == avx2htrans && avxvtrans == avx2vtrans) {
        printf(" NONE  ] ");
      } else {
        void *tdata = memalign(32, ostride*height*data.coef_size*2);
        memset(tdata, 0, ostride*height*data.coef_size);
        {
          void *odata[] = { (uint8_t*)tdata, (uint8_t*)tdata + ostride*height*data.coef_size, (uint8_t*)tdata + ostride*height*data.coef_size + ostride*height*data.coef_size/2 };
          avx2htrans((const char *)idata, istride, odata, ostride, width, height, width, height);
          if (avx2vtrans)
            avx2vtrans(tdata, ostride, width, height, 1);
        }
        int cmp = 0;
        if (data.fmt == VC2ENCODER_INPUT_10P2)
          cmp = memcmp(cdata, tdata, ostride*height*data.coef_size*2);
        else {
          for (int y = 0; y < height; y++) {
            cmp |= memcmp(&((int8_t*)cdata)[y*ostride*data.coef_size], &((int8_t*)tdata)[y*ostride*data.coef_size], width*data.coef_size);
          }
        }
        if (cmp) {
          printf(" FAIL  ]\n");
          for (int i = 0; i < ostride*height*data.coef_size*2; i++) {
            if (((uint8_t*)cdata)[i] != ((uint8_t*)tdata)[i]) {
              printf("Differs first at byte %d: 0x%02x != 0x%02x\n", i, ((uint8_t*)cdata)[i], ((uint8_t*)tdata)[i]);
              break;
            }
          }
          r = 1;
          free(tdata);
          free(cdata);
          break;
        }

        printf("  OK   ] ");
        free(tdata);
      }
    }

    free(cdata);

    printf("\n");
  }

  free(idata);
  return r;
}

int perform_transformtest(const transform_test &data, void *idata_pre, bool HAS_SSE4_2, bool HAS_AVX, bool HAS_AVX2) {
  (void)HAS_AVX;(void)HAS_AVX2;
  int r = 0;

  const int width   = 480;
  const int height  = 272;
  int stride = 1024;
  void *idata;
  int idata_length = stride*height*data.coef_size;
  idata = memalign(32, idata_length);
  if (data.coef_size == 2) {
    for (int i = 0; i < stride*height; i++) {
      ((uint16_t*)idata)[i] = ((uint16_t*)idata_pre)[i]&0x3FF;
    }
  } else {
    for (int i = 0; i < stride*height; i++) {
      ((uint32_t*)idata)[i] = ((uint32_t*)idata_pre)[i]&0x3FF;
    }
  }

  for (int level = 1; level < 4; level++) {
    printf("%-20s: H %d/* ", VC2EncoderWaveletFilterTypeString[data.wavelet_index], level);
    if (data.coef_size == 2)
      printf("16-bit ");
    else
      printf("32-bit ");

    printf(" C [ ");
    InplaceTransform chtrans = NULL;
    InplaceTransform cvtrans = NULL;
    try {
      chtrans = get_htransform_c(data.wavelet_index, level, data.coef_size);
      cvtrans = get_vtransform_c(data.wavelet_index, level, data.coef_size, VC2ENCODER_INPUT_10P2);
    } catch (...) {
      printf(" ????  ]\n:");
      r = 1;
      break;
    }
    printf("EXISTS ] ");

    void *cdata = memalign(32, idata_length);
    memcpy(cdata, idata, idata_length);
    {
      if (chtrans)
        chtrans(cdata, stride, width, height, 1 << level);
      if (cvtrans)
        cvtrans(cdata, stride, width, height, 1 << level);
    }



    InplaceTransform sse42htrans = NULL;
    InplaceTransform sse42vtrans = NULL;
    if (HAS_SSE4_2) {
      printf(" SSE4.2 [ ");
      try {
      sse42htrans = get_htransform_sse4_2(data.wavelet_index, level, data.coef_size);
      sse42vtrans = get_vtransform_sse4_2(data.wavelet_index, level, data.coef_size, VC2ENCODER_INPUT_10P2);
      } catch (...) {
        printf(" ????  ]\n");
        r = 1;
        free(cdata);
        break;
      }

      if (sse42htrans == chtrans && sse42vtrans == cvtrans) {
        printf(" NONE  ] ");
      } else {
        void *tdata = memalign(32, idata_length);
        memcpy(tdata, idata, idata_length);
        {
          if (sse42htrans)
            sse42htrans(tdata, stride, width, height, 1 << level);
          if (sse42vtrans)
            sse42vtrans(tdata, stride, width, height, 1 << level);
        }
        int cmp = memcmp(cdata, tdata, idata_length);
        if (cmp) {
          printf(" FAIL  ]\n");
          for (int i = 0; i < idata_length; i++) {
            if (((uint8_t*)cdata)[i] != ((uint8_t*)tdata)[i]) {
              printf("Differs first at byte %d: 0x%02x != 0x%02x\n", i, ((uint8_t*)cdata)[i], ((uint8_t*)tdata)[i]);
              break;
            }
          }
          r = 1;
          free(tdata);
          free(cdata);
          break;
        }

        printf("  OK   ] ");
        free(tdata);
      }
    }

    InplaceTransform avxhtrans = NULL;
    InplaceTransform avxvtrans = NULL;
    if (HAS_AVX) {
      printf(" AVX [ ");
      try {
        avxhtrans = get_htransform_avx(data.wavelet_index, level, data.coef_size);
        avxvtrans = get_vtransform_avx(data.wavelet_index, level, data.coef_size, VC2ENCODER_INPUT_10P2);
      } catch (...) {
        printf(" ????  ]\n");
        r = 1;
        free(cdata);
        break;
      }

      if (sse42htrans == avxhtrans && sse42vtrans == avxvtrans) {
        printf(" NONE  ] ");
      } else {
        void *tdata = memalign(32, idata_length);
        memcpy(tdata, idata, idata_length);
        {
          if (avxhtrans)
            avxhtrans(tdata, stride, width, height, 1 << level);
          if (avxvtrans)
            avxvtrans(tdata, stride, width, height, 1 << level);
        }
        int cmp = memcmp(cdata, tdata, idata_length);
        if (cmp) {
          printf(" FAIL  ]\n");
          for (int i = 0; i < idata_length; i++) {
            if (((uint8_t*)cdata)[i] != ((uint8_t*)tdata)[i]) {
              printf("Differs first at byte %d: 0x%02x != 0x%02x\n", i, ((uint8_t*)cdata)[i], ((uint8_t*)tdata)[i]);
              break;
            }
          }
          r = 1;
          free(tdata);
          free(cdata);
          break;
        }

        printf("  OK   ] ");
        free(tdata);
      }
    }

    InplaceTransform avx2htrans = NULL;
    InplaceTransform avx2vtrans = NULL;
    if (HAS_AVX2) {
      printf(" AVX2 [ ");
      try {
        avx2htrans = get_htransform_avx2(data.wavelet_index, level, data.coef_size);
        avx2vtrans = get_vtransform_avx2(data.wavelet_index, level, data.coef_size, VC2ENCODER_INPUT_10P2);
      } catch (...) {
        printf(" ????  ]\n");
        r = 1;
        free(cdata);
        break;
      }

      if (avx2htrans == avxhtrans && avx2vtrans == avxvtrans) {
        printf(" NONE  ] ");
      } else {
        void *tdata = memalign(32, idata_length);
        memcpy(tdata, idata, idata_length);
        {
          if (avx2htrans)
            avx2htrans(tdata, stride, width, height, 1 << level);
          if (avx2vtrans)
            avx2vtrans(tdata, stride, width, height, 1 << level);
        }
        int cmp = memcmp(cdata, tdata, idata_length);
        if (cmp) {
          printf(" FAIL  ]\n");
          for (int i = 0; i < idata_length; i++) {
            if (((uint8_t*)cdata)[i] != ((uint8_t*)tdata)[i]) {
              printf("Differs first at byte %d: 0x%02x != 0x%02x\n", i, ((uint8_t*)cdata)[i], ((uint8_t*)tdata)[i]);
              break;
            }
          }
          r = 1;
          free(tdata);
          free(cdata);
          break;
        }

        printf("  OK   ] ");
        free(tdata);
      }
    }

    free(cdata);

    printf("\n");
  }

  free(idata);
  return r;
}

int test_transforms(bool HAS_SSE4_2, bool HAS_AVX, bool HAS_AVX2) {
  int r = 0;

  printf("--------------------------------------------------------------------------------\n");
  printf("  Testing Transforms for Consistency\n\n");

  /* Load some input data for the tests */
  const int ilength = 2048*1080*4*2;
  void *idata = memalign(32, ilength);
  {
    int f = open("/dev/urandom", O_RDONLY);
    ssize_t length = ilength;
    int offs = 0;
    while (length > 0) {
      ssize_t s = read(f, ((uint8_t *)idata) + offs, length);
      if (s <= 0) {
        printf("Reading Error Getting Random Data\n");
        return 1;
      }
      offs += s;
      length -= s;
    }
  }

  /* Perform some tests */
  for (int i = 0; !r && i < (int)(sizeof(TRANSFORMINITIAL_TEST)/sizeof(TRANSFORMINITIAL_TEST[0])); i++) {
    r = perform_transforminitialtest(TRANSFORMINITIAL_TEST[i], idata, HAS_SSE4_2, HAS_AVX, HAS_AVX2);
  }

  for (int i = 0; !r && i < (int)(sizeof(TRANSFORM_TEST)/sizeof(TRANSFORM_TEST[0])); i++) {
    r = perform_transformtest(TRANSFORM_TEST[i], idata, HAS_SSE4_2, HAS_AVX, HAS_AVX2);
  }

  printf("--------------------------------------------------------------------------------\n");

  return r;
}
