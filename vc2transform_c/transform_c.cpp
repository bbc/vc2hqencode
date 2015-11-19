/*****************************************************************************
 * transform_c.cpp : Transform Selection: Plain C++ version
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

#include "logger.hpp"
#include "transform.hpp"
#include "transform_kernels.hpp"

InplaceTransformInitial get_htransforminitial_c(int wavelet_index, int active_bits, int coef_size, int c, VC2EncoderInputFormat fmt) {
  (void)c;
  if (fmt == VC2ENCODER_INPUT_10P2) {
    if (active_bits != 10) {
      writelog(LOG_ERROR, "%s:%d:  invalid bit depth", __FILE__, __LINE__);
      throw VC2ENCODER_NOTIMPLEMENTED;
    }

    if (coef_size == 2) {
      switch (wavelet_index) {
      case VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7:
        return Deslauriers_Dubuc_9_7_transform_H_inplace_10P2<int16_t>;
      case VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7:
        return Deslauriers_Dubuc_13_7_transform_H_inplace_10P2<int16_t>;
      case VC2ENCODER_WFT_LEGALL_5_3:
        return LeGall_5_3_transform_H_inplace_10P2<int16_t>;
      case VC2ENCODER_WFT_HAAR_NO_SHIFT:
        return Haar_transform_H_inplace_10P2<0, int16_t>;
      case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
        return Haar_transform_H_inplace_10P2<1, int16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    } else if (coef_size == 4) {
      switch (wavelet_index) {
        /*  case VC2ENCODER_WFT_FIDELITY:
            return Fidelity_transform_H_inplace_10P2;*/
      case VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7:
        return Deslauriers_Dubuc_9_7_transform_H_inplace_10P2<int32_t>;
      case VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7:
        return Deslauriers_Dubuc_13_7_transform_H_inplace_10P2<int32_t>;
      case VC2ENCODER_WFT_LEGALL_5_3:
        return LeGall_5_3_transform_H_inplace_10P2<int32_t>;
      case VC2ENCODER_WFT_HAAR_NO_SHIFT:
        return Haar_transform_H_inplace_10P2<0, int32_t>;
      case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
        return Haar_transform_H_inplace_10P2<1, int32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    }
  } else if (fmt == VC2ENCODER_INPUT_V210) {
    if (c != 0)
      return NULL;

    if (coef_size == 2) {
      switch(wavelet_index) {
      case VC2ENCODER_WFT_LEGALL_5_3:
        return LeGall_5_3_transform_H_inplace_V210<int16_t>;
      case VC2ENCODER_WFT_HAAR_NO_SHIFT:
        return Haar_transform_H_inplace_V210<0, uint16_t>;
      case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
        return Haar_transform_H_inplace_V210<1, uint16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  V210 input is only supported for LeGall and Haar wavelet", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    } else if (coef_size == 4) {
      switch(wavelet_index) {
      case VC2ENCODER_WFT_LEGALL_5_3:
        return LeGall_5_3_transform_H_inplace_V210<int32_t>;
      case VC2ENCODER_WFT_HAAR_NO_SHIFT:
        return Haar_transform_H_inplace_V210<0, uint32_t>;
      case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
        return Haar_transform_H_inplace_V210<1, uint32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  V210 input is only supported for LeGall and Haar wavelet", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    }
  } else {
    writelog(LOG_ERROR, "%s:%d:  invalid input format", __FILE__, __LINE__);
    throw VC2ENCODER_NOTIMPLEMENTED;
  }

  writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
  throw VC2ENCODER_NOTIMPLEMENTED;
}

InplaceTransform get_htransform_c(int wavelet_index, int level, int coef_size) {
  if (coef_size == 2) {
    switch (wavelet_index) {
      /*  case VC2ENCODER_WFT_FIDELITY:
          switch (level) {
          case 0:
          return Fidelity_transform_H_inplace<1>;
          case 1:
          return Fidelity_transform_H_inplace<2>;
          case 2:
          return Fidelity_transform_H_inplace<4>;
          case 3:
          return Fidelity_transform_H_inplace<8>;
          default:
          writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
          throw VC2ENCODER_NOTIMPLEMENTED;
          }*/
    case VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7:
      switch (level) {
      case 0:
        return Deslauriers_Dubuc_9_7_transform_H_inplace<1, int16_t>;
      case 1:
        return Deslauriers_Dubuc_9_7_transform_H_inplace<2, int16_t>;
      case 2:
        return Deslauriers_Dubuc_9_7_transform_H_inplace<4, int16_t>;
      case 3:
        return Deslauriers_Dubuc_9_7_transform_H_inplace<8, int16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7:
      switch (level) {
      case 0:
        return Deslauriers_Dubuc_13_7_transform_H_inplace<1, int16_t>;
      case 1:
        return Deslauriers_Dubuc_13_7_transform_H_inplace<2, int16_t>;
      case 2:
        return Deslauriers_Dubuc_13_7_transform_H_inplace<4, int16_t>;
      case 3:
        return Deslauriers_Dubuc_13_7_transform_H_inplace<8, int16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_LEGALL_5_3:
      switch (level) {
      case 0:
        return LeGall_5_3_transform_H_inplace<1, int16_t>;
      case 1:
        return LeGall_5_3_transform_H_inplace<2, int16_t>;
      case 2:
        return LeGall_5_3_transform_H_inplace<4, int16_t>;
      case 3:
        return LeGall_5_3_transform_H_inplace<8, int16_t>;
      default:
        writelog(LOG_WARN, "%s:%d:  Falling back to slow path for non-optimised transform depth", __FILE__, __LINE__);
        return LeGall_5_3_transform_H_inplace_dynamic<int16_t>;
      }
    case VC2ENCODER_WFT_HAAR_NO_SHIFT:
      switch (level) {
      case 0:
        return Haar_transform_H_inplace<1,0, int16_t>;
      case 1:
        return Haar_transform_H_inplace<2,0, int16_t>;
      case 2:
        return Haar_transform_H_inplace<4,0, int16_t>;
      case 3:
        return Haar_transform_H_inplace<8,0, int16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
      switch (level) {
      case 0:
        return Haar_transform_H_inplace<1,1, int16_t>;
      case 1:
        return Haar_transform_H_inplace<2,1, int16_t>;
      case 2:
        return Haar_transform_H_inplace<4,1, int16_t>;
      case 3:
        return Haar_transform_H_inplace<8,1, int16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    default:
      writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
      throw VC2ENCODER_NOTIMPLEMENTED;
    }
  } else if (coef_size == 4) {
        switch (wavelet_index) {
      /*  case VC2ENCODER_WFT_FIDELITY:
          switch (level) {
          case 0:
          return Fidelity_transform_H_inplace<1>;
          case 1:
          return Fidelity_transform_H_inplace<2>;
          case 2:
          return Fidelity_transform_H_inplace<4>;
          case 3:
          return Fidelity_transform_H_inplace<8>;
          default:
          writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
          throw VC2ENCODER_NOTIMPLEMENTED;
          }*/
    case VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7:
      switch (level) {
      case 0:
        return Deslauriers_Dubuc_9_7_transform_H_inplace<1, int32_t>;
      case 1:
        return Deslauriers_Dubuc_9_7_transform_H_inplace<2, int32_t>;
      case 2:
        return Deslauriers_Dubuc_9_7_transform_H_inplace<4, int32_t>;
      case 3:
        return Deslauriers_Dubuc_9_7_transform_H_inplace<8, int32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7:
      switch (level) {
      case 0:
        return Deslauriers_Dubuc_13_7_transform_H_inplace<1, int32_t>;
      case 1:
        return Deslauriers_Dubuc_13_7_transform_H_inplace<2, int32_t>;
      case 2:
        return Deslauriers_Dubuc_13_7_transform_H_inplace<4, int32_t>;
      case 3:
        return Deslauriers_Dubuc_13_7_transform_H_inplace<8, int32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_LEGALL_5_3:
      switch (level) {
      case 0:
        return LeGall_5_3_transform_H_inplace<1, int32_t>;
      case 1:
        return LeGall_5_3_transform_H_inplace<2, int32_t>;
      case 2:
        return LeGall_5_3_transform_H_inplace<4, int32_t>;
      case 3:
        return LeGall_5_3_transform_H_inplace<8, int32_t>;
      default:
        writelog(LOG_WARN, "%s:%d:  Falling back to slow path for non-optimised transform depth", __FILE__, __LINE__);
        return LeGall_5_3_transform_H_inplace_dynamic<int32_t>;
      }
    case VC2ENCODER_WFT_HAAR_NO_SHIFT:
      switch (level) {
      case 0:
        return Haar_transform_H_inplace<1,0, int32_t>;
      case 1:
        return Haar_transform_H_inplace<2,0, int32_t>;
      case 2:
        return Haar_transform_H_inplace<4,0, int32_t>;
      case 3:
        return Haar_transform_H_inplace<8,0, int32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
      switch (level) {
      case 0:
        return Haar_transform_H_inplace<1,1, int32_t>;
      case 1:
        return Haar_transform_H_inplace<2,1, int32_t>;
      case 2:
        return Haar_transform_H_inplace<4,1, int32_t>;
      case 3:
        return Haar_transform_H_inplace<8,1, int32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    default:
      writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
      throw VC2ENCODER_NOTIMPLEMENTED;
    }
  }
  writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
  throw VC2ENCODER_NOTIMPLEMENTED;
}

InplaceTransform get_vtransform_c(int wavelet_index, int level, int coef_size, VC2EncoderInputFormat) {
  if (coef_size == 2) {
    switch (wavelet_index) {
      /*  case VC2ENCODER_WFT_FIDELITY:
          switch (level) {
          case 0:
          return Fidelity_transform_V_inplace<1>;
          case 1:
          return Fidelity_transform_V_inplace<2>;
          case 2:
          return Fidelity_transform_V_inplace<4>;
          case 3:
          return Fidelity_transform_V_inplace<8>;
          default:
          writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
          throw VC2ENCODER_NOTIMPLEMENTED;
          }*/
    case VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7:
      switch (level) {
      case 0:
        return Deslauriers_Dubuc_9_7_transform_V_inplace<1, int16_t>;
      case 1:
        return Deslauriers_Dubuc_9_7_transform_V_inplace<2, int16_t>;
      case 2:
        return Deslauriers_Dubuc_9_7_transform_V_inplace<4, int16_t>;
      case 3:
        return Deslauriers_Dubuc_9_7_transform_V_inplace<8, int16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7:
      switch (level) {
      case 0:
        return Deslauriers_Dubuc_13_7_transform_V_inplace<1, int16_t>;
      case 1:
        return Deslauriers_Dubuc_13_7_transform_V_inplace<2, int16_t>;
      case 2:
        return Deslauriers_Dubuc_13_7_transform_V_inplace<4, int16_t>;
      case 3:
        return Deslauriers_Dubuc_13_7_transform_V_inplace<8, int16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_LEGALL_5_3:
      switch (level) {
      case 0:
        return LeGall_5_3_transform_V_inplace<1, int16_t>;
      case 1:
        return LeGall_5_3_transform_V_inplace<2, int16_t>;
      case 2:
        return LeGall_5_3_transform_V_inplace<4, int16_t>;
      case 3:
        return LeGall_5_3_transform_V_inplace<8, int16_t>;
      default:
        writelog(LOG_WARN, "%s:%d:  Falling back to slow path for non-optimised transform depth", __FILE__, __LINE__);
        return LeGall_5_3_transform_V_inplace_dynamic<int16_t>;
      }
    case VC2ENCODER_WFT_HAAR_NO_SHIFT:
    case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
      switch (level) {
      case 0:
        return Haar_transform_V_inplace<1, int16_t>;
      case 1:
        return Haar_transform_V_inplace<2, int16_t>;
      case 2:
        return Haar_transform_V_inplace<4, int16_t>;
      case 3:
        return Haar_transform_V_inplace<8, int16_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    default:
      writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
      throw VC2ENCODER_NOTIMPLEMENTED;
    }
  } else if (coef_size == 4) {
    switch (wavelet_index) {
      /*  case VC2ENCODER_WFT_FIDELITY:
          switch (level) {
          case 0:
          return Fidelity_transform_V_inplace<1>;
          case 1:
          return Fidelity_transform_V_inplace<2>;
          case 2:
          return Fidelity_transform_V_inplace<4>;
          case 3:
          return Fidelity_transform_V_inplace<8>;
          default:
          writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
          throw VC2ENCODER_NOTIMPLEMENTED;
          }*/
    case VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7:
      switch (level) {
      case 0:
        return Deslauriers_Dubuc_9_7_transform_V_inplace<1, int32_t>;
      case 1:
        return Deslauriers_Dubuc_9_7_transform_V_inplace<2, int32_t>;
      case 2:
        return Deslauriers_Dubuc_9_7_transform_V_inplace<4, int32_t>;
      case 3:
        return Deslauriers_Dubuc_9_7_transform_V_inplace<8, int32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7:
      switch (level) {
      case 0:
        return Deslauriers_Dubuc_13_7_transform_V_inplace<1, int32_t>;
      case 1:
        return Deslauriers_Dubuc_13_7_transform_V_inplace<2, int32_t>;
      case 2:
        return Deslauriers_Dubuc_13_7_transform_V_inplace<4, int32_t>;
      case 3:
        return Deslauriers_Dubuc_13_7_transform_V_inplace<8, int32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    case VC2ENCODER_WFT_LEGALL_5_3:
      switch (level) {
      case 0:
        return LeGall_5_3_transform_V_inplace<1, int32_t>;
      case 1:
        return LeGall_5_3_transform_V_inplace<2, int32_t>;
      case 2:
        return LeGall_5_3_transform_V_inplace<4, int32_t>;
      case 3:
        return LeGall_5_3_transform_V_inplace<8, int32_t>;
      default:
        writelog(LOG_WARN, "%s:%d:  Falling back to slow path for non-optimised transform depth", __FILE__, __LINE__);
        return LeGall_5_3_transform_V_inplace_dynamic<int32_t>;
      }
    case VC2ENCODER_WFT_HAAR_NO_SHIFT:
    case VC2ENCODER_WFT_HAAR_SINGLE_SHIFT:
      switch (level) {
      case 0:
        return Haar_transform_V_inplace<1, int32_t>;
      case 1:
        return Haar_transform_V_inplace<2, int32_t>;
      case 2:
        return Haar_transform_V_inplace<4, int32_t>;
      case 3:
        return Haar_transform_V_inplace<8, int32_t>;
      default:
        writelog(LOG_ERROR, "%s:%d:  invalid depth", __FILE__, __LINE__);
        throw VC2ENCODER_NOTIMPLEMENTED;
      }
    default:
      writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
      throw VC2ENCODER_NOTIMPLEMENTED;
    }
  }
  writelog(LOG_ERROR, "%s:%d:  invalid wavelet kernel", __FILE__, __LINE__);
  throw VC2ENCODER_NOTIMPLEMENTED;
}
