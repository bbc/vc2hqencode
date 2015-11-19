/*****************************************************************************
 * transform_avx2.cpp : Transform Selection: AVX2 version
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

#include "vc2hqencode/debug.hpp"

#ifndef NO_SIMD
#include "logger.hpp"
#include "vc2transform_avx/transform_avx.hpp"
#include "transform_avx2.hpp"
#include "transform_kernels.hpp"

InplaceTransformInitial get_htransforminitial_avx2(int wavelet_index, int active_bits, int coef_size, int c, VC2EncoderInputFormat fmt) {
  (void) c;
  if (fmt == VC2ENCODER_INPUT_10P2) {
    if (active_bits != 10) {
      writelog(LOG_ERROR, "%s:%d:  invalid bit depth", __FILE__, __LINE__);
      throw VC2ENCODER_NOTIMPLEMENTED;
    }

    if (coef_size == 2) {
      switch (wavelet_index) {
      case VC2ENCODER_WFT_LEGALL_5_3:
        return LeGall_5_3_transform_H_inplace_10P2_sse4_2_avx2;
      case VC2ENCODER_WFT_HAAR_NO_SHIFT:
        return Haar_transform_H_inplace_10P2_avx2<0, int16_t>;
      case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
        return Haar_transform_H_inplace_10P2_sse4_2_avx2<1, int16_t>;
      }
    } else if (coef_size == 4) {
      switch (wavelet_index) {
      case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
        return Haar_transform_H_inplace_10P2_sse4_2_avx2<1, int32_t>;
      }
    }
  } else if (fmt == VC2ENCODER_INPUT_V210) {
    if (c != 0)
      return NULL;

    if (coef_size == 2) {
      switch(wavelet_index) {
      case VC2ENCODER_WFT_LEGALL_5_3:
        return LeGall_5_3_transform_H_inplace_V210_sse4_2_avx2;
      case VC2ENCODER_WFT_HAAR_NO_SHIFT:
        return Haar_transform_H_inplace_V210_sse4_2_avx2<0, int16_t>;
      case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
        return Haar_transform_H_inplace_V210_sse4_2_avx2<1, int16_t>;
      }
    }
  }

  return get_htransforminitial_avx(wavelet_index, active_bits, coef_size, c, fmt);
}

InplaceTransform get_htransform_avx2(int wavelet_index, int level, int coef_size) {
  if (coef_size == 2) {
    switch (wavelet_index) {
    case VC2ENCODER_WFT_LEGALL_5_3:
      switch (level) {
      case 1:
        return LeGall_5_3_transform_H_inplace_sse4_2_avx2<2>;
      }
      break;
    case VC2ENCODER_WFT_HAAR_NO_SHIFT:
      switch (level) {
      case 1:
        return Haar_transform_H_inplace_sse4_2_avx2<2,0>;
      }
      break;
    case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
      switch (level) {
      case 1:
        return Haar_transform_H_inplace_sse4_2_avx2<2,1>;
      }
      break;
    }
  }

  return get_htransform_avx(wavelet_index, level, coef_size);
}

InplaceTransform get_vtransform_avx2(int wavelet_index, int level, int coef_size, VC2EncoderInputFormat fmt) {
  if (coef_size == 2) {
    switch (wavelet_index) {
    case VC2ENCODER_WFT_LEGALL_5_3:
      switch (level) {
      case 0:
        return LeGall_5_3_transform_V_inplace_sse4_2_avx2<1>;
      case 1:
        return LeGall_5_3_transform_V_inplace_sse4_2_avx2<2>;
      }
      break;
    case VC2ENCODER_WFT_HAAR_NO_SHIFT:
    case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
      switch (level) {
      case 0:
        if (fmt == VC2ENCODER_INPUT_10P2)
          return NULL;
        else
          return Haar_transform_V_inplace_sse4_2_avx2<1, int16_t>;
      case 1:
        return Haar_transform_V_inplace_sse4_2_avx2<2, int16_t>;
      case 2:
        return Haar_transform_V_inplace_sse4_2_avx2<4, int16_t>;
      case 3:
        return Haar_transform_V_inplace_sse4_2_avx2<8, int16_t>;
      }
      break;
    }
  } else if (coef_size == 4) {
    switch (wavelet_index) {
    case VC2ENCODER_WFT_HAAR_NO_SHIFT:
    case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
      switch (level) {
      case 0:
        return NULL;
      }
      break;
    }
  }

  return get_vtransform_avx(wavelet_index, level, coef_size, fmt);
}
#endif
