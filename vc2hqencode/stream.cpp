/*****************************************************************************
 * stream.cpp : VC2 Stream syntax
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

#include "stream.hpp"
#include "encode.hpp"
#include "logger.hpp"
#include <cstdio>

#define MAX_PARSE_INFO_SIZE 13
#define MAX_SEQ_HEADER_SIZE 1011
#define MAX_PIC_HEADER_SIZE 1011

#define MAX_SEQ_HEADER_VALUES 48


#ifdef DEBUG
#define DEBUG_PRINT_SEQHEADER
#define DEBUG_PRINT_PICHEADER
#endif

uint8_t encode_bool(bool x, uint32_t *out, uint8_t *len) {
  if (x)
    *out = 1;
  else
    *out = 0;
  *len = 1;
  return 1;
}

SequenceHeader::SequenceHeader(VC2EncoderParams &params) {
  data = (uint8_t*)malloc(MAX_PARSE_INFO_SIZE + MAX_SEQ_HEADER_SIZE);
  length = 0;


  // Now start encoding the data
  uint32_t codewords[MAX_SEQ_HEADER_VALUES];
  uint8_t  wordlengths[MAX_SEQ_HEADER_VALUES];
  int l = 0;
  int n = 0;
  uint32_t *cw = codewords;
  uint8_t *wl   = wordlengths;

  //parse_parameters
  l += encode_uint(2, cw++, wl++); // major version == 2
  l += encode_uint(0, cw++, wl++); // minor version == 0
  l += encode_uint(3, cw++, wl++); // profile       == 3 (HQ)

  int level = 0;
  if (params.transform_params.wavelet_index <= VC2ENCODER_WFT_HAAR_SINGLE_SHIFT &&
      params.transform_params.wavelet_depth <= 4) {
    if (!(params.video_format.custom_dimensions_flag ||
          params.video_format.custom_scan_format_flag ||
          params.video_format.custom_frame_rate_flag  ||
          params.video_format.custom_pixel_aspect_ratio_flag  ||
          params.video_format.custom_clean_area_flag  ||
          params.video_format.custom_signal_range_flag  ||
          params.video_format.custom_color_spec_flag)) {
      switch (params.video_format.base_video_format) {
      case VC2ENCODER_BVF_CUSTOM:
        level = 0;
        break;
      case VC2ENCODER_BVF_QSIF525:
      case VC2ENCODER_BVF_QCIF:
      case VC2ENCODER_BVF_SIF525:
      case VC2ENCODER_BVF_CIF:
      case VC2ENCODER_BVF_4SIF525:
      case VC2ENCODER_BVF_4CIF:
        level = 1;
        break;
      case VC2ENCODER_BVF_SD480I_60:
      case VC2ENCODER_BVF_SD576I_50:
      case VC2ENCODER_BVF_SDPRO486:
        level = 2;
        break;
      case VC2ENCODER_BVF_HD720P_60:
      case VC2ENCODER_BVF_HD720P_50:
      case VC2ENCODER_BVF_HD1080I_60:
      case VC2ENCODER_BVF_HD1080I_50:
      case VC2ENCODER_BVF_HD1080P_60:
      case VC2ENCODER_BVF_HD1080P_50:
      case VC2ENCODER_BVF_HD1080P_24:
        level = 3;
        break;
      case VC2ENCODER_BVF_DC2K:
        level = 4;
        break;
      case VC2ENCODER_BVF_DC4K:
        level = 5;
        break;
      case VC2ENCODER_BVF_UHDTV4K_60:
      case VC2ENCODER_BVF_UHDTV4K_50:
        level = 6;
        break;
      case VC2ENCODER_BVF_UHDTV8K_60:
      case VC2ENCODER_BVF_UHDTV8K_50:
        level = 7;
        break;
      }
    } else if (params.video_format.base_video_format == VC2ENCODER_BVF_SD480I_60 &&
               !(params.video_format.custom_frame_rate_flag  ||
                 params.video_format.custom_pixel_aspect_ratio_flag  ||
                 params.video_format.custom_clean_area_flag  ||
                 params.video_format.custom_signal_range_flag  ||
                 params.video_format.custom_color_spec_flag) &&
               params.video_format.custom_dimensions_flag &&
               params.video_format.frame_width == 720 &&
               params.video_format.frame_height >= 480 &&
               params.video_format.frame_height <= 486) {
      level = 2;
    } else if ((params.video_format.base_video_format == VC2ENCODER_BVF_SD480I_60 ||
                params.video_format.base_video_format == VC2ENCODER_BVF_SD576I_50 ||
                params.video_format.base_video_format == VC2ENCODER_BVF_SDPRO486) &&
               !(params.video_format.custom_dimensions_flag ||
                 params.video_format.custom_frame_rate_flag  ||
                 params.video_format.custom_pixel_aspect_ratio_flag  ||
                 params.video_format.custom_clean_area_flag  ||
                 params.video_format.custom_signal_range_flag  ||
                 params.video_format.custom_color_spec_flag)) {
      level = 2;
    } else if ((params.video_format.base_video_format == VC2ENCODER_BVF_HD1080I_60 ||
                params.video_format.base_video_format == VC2ENCODER_BVF_HD1080I_50) &&
               !(params.video_format.custom_dimensions_flag ||
                 params.video_format.custom_frame_rate_flag  ||
                 params.video_format.custom_pixel_aspect_ratio_flag  ||
                 params.video_format.custom_clean_area_flag  ||
                 params.video_format.custom_signal_range_flag  ||
                 params.video_format.custom_color_spec_flag)) {
      level = 3;
    } else if ((params.video_format.base_video_format == VC2ENCODER_BVF_DC2K) &&
               !(params.video_format.custom_dimensions_flag ||
                 params.video_format.custom_scan_format_flag ||
                 params.video_format.custom_pixel_aspect_ratio_flag  ||
                 params.video_format.custom_clean_area_flag  ||
                 params.video_format.custom_signal_range_flag  ||
                 params.video_format.custom_color_spec_flag) &&
               params.video_format.custom_frame_rate_flag &&
               params.video_format.frame_rate_index == VC2ENCODER_FR_48) {
      level = 4;
    }
  }
  l += encode_uint(level, cw++, wl++); // level
  l += encode_uint(params.video_format.base_video_format, cw++, wl++); // base_video_format

  // Source parameters
  // frame size
  if (params.video_format.custom_dimensions_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.video_format.frame_width,  cw++, wl++);
    l += encode_uint(params.video_format.frame_height, cw++, wl++);
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  // colour difference sampling format
  if (params.video_format.custom_color_diff_format_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.video_format.color_diff_format_index,  cw++, wl++);
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  // scan format
  if (params.video_format.custom_scan_format_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.video_format.source_sampling,  cw++, wl++);
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  // frame rate
  if (params.video_format.custom_frame_rate_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.video_format.frame_rate_index,  cw++, wl++);
    if (params.video_format.frame_rate_index == 0) {
      l += encode_uint(params.video_format.frame_rate_numer,  cw++, wl++);
      l += encode_uint(params.video_format.frame_rate_denom,  cw++, wl++);
    }
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  // pixel aspect ratio
  if (params.video_format.custom_pixel_aspect_ratio_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.video_format.pixel_aspect_ratio_index,  cw++, wl++);
    if (params.video_format.frame_rate_index == 0) {
      l += encode_uint(params.video_format.pixel_aspect_ratio_numer,  cw++, wl++);
      l += encode_uint(params.video_format.pixel_aspect_ratio_denom,  cw++, wl++);
    }
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  // clean area
  if (params.video_format.custom_clean_area_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.video_format.clean_width,  cw++, wl++);
    l += encode_uint(params.video_format.clean_height, cw++, wl++);
    l += encode_uint(params.video_format.left_offset,  cw++, wl++);
    l += encode_uint(params.video_format.top_offset,   cw++, wl++);
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  // signal range
  if (params.video_format.custom_signal_range_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.video_format.signal_range_index,  cw++, wl++);
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  // color spec
  if (params.video_format.custom_color_spec_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.video_format.color_spec_index,  cw++, wl++);
    if (params.video_format.color_spec_index == 0) {
      if (params.video_format.custom_color_primaries_flag) {
        l += encode_bool(1, cw++, wl++);
        l += encode_uint(params.video_format.color_primaries_index,  cw++, wl++);
      } else {
        l += encode_bool(0, cw++, wl++);
      }

      if (params.video_format.custom_color_matrix_flag) {
        l += encode_bool(1, cw++, wl++);
        l += encode_uint(params.video_format.color_matrix_index,  cw++, wl++);
      } else {
        l += encode_bool(0, cw++, wl++);
      }

      if (params.video_format.custom_transfer_function_flag) {
        l += encode_bool(1, cw++, wl++);
        l += encode_uint(params.video_format.transfer_function_index,  cw++, wl++);
      } else {
        l += encode_bool(0, cw++, wl++);
      }
    }
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  l += encode_uint(params.picture_coding_mode,  cw++, wl++);

  // Round up to whole bytes
  l = ((l + 7)/8) + 13;
  n = cw - codewords;

  // parse_info
  data[length++] = 0x42; // B
  data[length++] = 0x42; // B
  data[length++] = 0x43; // C
  data[length++] = 0x44; // D

  data[length++] = 0x00; // 0 == Sequence Header

  data[length++] = (l >> 24)&0xFF; // next_parse_offset,
  data[length++] = (l >> 16)&0xFF; //
  data[length++] = (l >>  8)&0xFF; //
  data[length++] = (l >>  0)&0xFF; //

  data[length++] = 0x00; // previous_parse_offset,
  data[length++] = 0x00; //   non-existant
  data[length++] = 0x00; //
  data[length++] = 0x00; //

  uint64_t accum = 0;
  int bits = 0;
  for (int x = 0; x < n; x++) {
    accum |= ((uint64_t)codewords[x] << (64 - wordlengths[x] - bits));
    bits += wordlengths[x];

    while(bits > 8) {
      data[length++] = (accum >> 56);
      accum <<= 8;
      bits -= 8;
    }
  }
  if (bits > 0) {
    data[length++] = (accum >> 56) | (0xFF >> bits);
  }


#ifdef DEBUG_PRINT_SEQHEADER
  {
    printf("--------------------------------------------------------------------\n");
    printf("   Sequence Header Coded Data\n");
    printf("--------------------------------------------------------------------\n");
    for (int y = 0; y*4 < n; y++) {
      printf("    ");
      for (int x = 0; x < 4 && y*4 + x < n; x++) {
        printf("  0x%08x/%2d", codewords[y*4 + x], wordlengths[y*4 + x]);
      }
      printf("\n");
    }
    printf("--------------------------------------------------------------------\n");
  }

  {
    printf("--------------------------------------------------------------------\n");
    printf("   Sequence Header Stream Data\n");
    printf("--------------------------------------------------------------------\n");
    for (int y = 0; y*16 < length; y++) {
      printf("    ");
      for (int x = 0; x < 16 && y*16 + x < length; x++) {
        printf("  %02x", data[y*16 + x]);
      }
      printf("\n");
    }
    printf("--------------------------------------------------------------------\n");
  }
#endif
}

PictureHeader::PictureHeader(VC2EncoderParams &params, int slices_x, int slices_y, int slice_size_scalar) {
  data = (uint8_t*)malloc(MAX_PARSE_INFO_SIZE + MAX_PIC_HEADER_SIZE);
  length = 0;


  // Now start encoding the data
  uint32_t codewords[MAX_SEQ_HEADER_VALUES];
  uint8_t  wordlengths[MAX_SEQ_HEADER_VALUES];
  int l = 0;
  int n = 0;
  uint32_t *cw = codewords;
  uint8_t *wl   = wordlengths;

  l += encode_uint(params.transform_params.wavelet_index,  cw++, wl++);
  l += encode_uint(params.transform_params.wavelet_depth,  cw++, wl++);
  l += encode_uint(slices_x,                               cw++, wl++);
  l += encode_uint(slices_y,                               cw++, wl++);
  l += encode_uint(0,                                      cw++, wl++);
  l += encode_uint(slice_size_scalar,                      cw++, wl++);

  if (params.transform_params.custom_quant_matrix_flag) {
    l += encode_bool(1, cw++, wl++);
    l += encode_uint(params.transform_params.quant_matrix_LL, cw++, wl++);
    for (int i = 0; i < (int)params.transform_params.wavelet_depth - 1; i++) {
      l += encode_uint(params.transform_params.quant_matrix_HL[i], cw++, wl++);
      l += encode_uint(params.transform_params.quant_matrix_LH[i], cw++, wl++);
      l += encode_uint(params.transform_params.quant_matrix_HH[i], cw++, wl++);
    }
  } else {
    l += encode_bool(0, cw++, wl++);
  }

  // Round up to whole bytes
  l = ((l + 7)/8) + 13 + 4;
  n = cw - codewords;

  // parse_info
  data[length++] = 0x42; // B
  data[length++] = 0x42; // B
  data[length++] = 0x43; // C
  data[length++] = 0x44; // D

  data[length++] = 0xE8; // 0 == HQ Picture

  data[length++] = 0x00; // next_parse_offset,
  data[length++] = 0x00; // FILLED IN LATER
  data[length++] = 0x00; //
  data[length++] = 0x00; //

  data[length++] = 0x00; // previous_parse_offset,
  data[length++] = 0x00; // FILLED IN LATER
  data[length++] = 0x00; //
  data[length++] = 0x00; //

  data[length++] = 0x00; // picture_number,
  data[length++] = 0x00; // FILLED IN LATER
  data[length++] = 0x00; //
  data[length++] = 0x00; //

  uint64_t accum = 0;
  int bits = 0;
  for (int x = 0; x < n; x++) {
    accum |= ((uint64_t)codewords[x] << (64 - wordlengths[x] - bits));
    bits += wordlengths[x];

    while(bits > 8) {
      data[length++] = (accum >> 56);
      accum <<= 8;
      bits -= 8;
    }
  }
  if (bits > 0) {
    data[length++] = (accum >> 56) | (0xFF >> bits);
  }

#ifdef DEBUG_PRINT_PICHEADER
  {
    printf("--------------------------------------------------------------------\n");
    printf("   Picture Header Coded Data\n");
    printf("--------------------------------------------------------------------\n");
    for (int y = 0; y*4 < n; y++) {
      printf("    ");
      for (int x = 0; x < 4 && y*4 + x < n; x++) {
        printf("  0x%08x/%2d", codewords[y*4 + x], wordlengths[y*4 + x]);
      }
      printf("\n");
    }
    printf("--------------------------------------------------------------------\n");
  }

  {
    printf("--------------------------------------------------------------------\n");
    printf("   Picture Header Stream Data\n");
    printf("--------------------------------------------------------------------\n");
    for (int y = 0; y*16 < length; y++) {
      printf("    ");
      for (int x = 0; x < 16 && y*16 + x < length; x++) {
        printf("  %02x", data[y*16 + x]);
      }
      printf("\n");
    }
    printf("--------------------------------------------------------------------\n");
  }
#endif
}
