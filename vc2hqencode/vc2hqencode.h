/*****************************************************************************
 * vc2hqencode.h : C-compatible interface
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

#ifndef __VC2HQENCODE_H__
#define __VC2HQENCODE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DWT_DEPTH 8

#define VC2ENCODER_API_VERSION 1

/*
 This forces a link error if trying to link to an incompatible version of the code,
 the actual call to vc2encode_init becomes a call to vc2encode_init_VC2ENCODER_API_VERSION
 */
#define vc2encoder_combine_internal(x,y) x##y
#define vc2encoder_combine(x,y) vc2encoder_combine_internal(x,y)
#define vc2encode_init vc2encoder_combine(vc2encode_init_,VC2ENCODER_API_VERSION)
#define vc2encode_create vc2encoder_combine(vc2encode_create_,VC2ENCODER_API_VERSION)

typedef void * VC2EncoderHandle;

typedef enum _VC2EncoderInputFormat {
  VC2ENCODER_INPUT_10P2 = 0,
  VC2ENCODER_INPUT_V210 = 1
} VC2EncoderInputFormat;

enum _VC2EncoderBaseVideoFormat {
  VC2ENCODER_BVF_CUSTOM     = 0,
  VC2ENCODER_BVF_QSIF525    = 1,
  VC2ENCODER_BVF_QCIF       = 2,
  VC2ENCODER_BVF_SIF525     = 3,
  VC2ENCODER_BVF_CIF        = 4,
  VC2ENCODER_BVF_4SIF525    = 5,
  VC2ENCODER_BVF_4CIF       = 6,
  VC2ENCODER_BVF_SD480I_60  = 7,
  VC2ENCODER_BVF_SD576I_50  = 8,
  VC2ENCODER_BVF_HD720P_60  = 9,
  VC2ENCODER_BVF_HD720P_50  = 10,
  VC2ENCODER_BVF_HD1080I_60 = 11,
  VC2ENCODER_BVF_HD1080I_50 = 12,
  VC2ENCODER_BVF_HD1080P_60 = 13,
  VC2ENCODER_BVF_HD1080P_50 = 14,
  VC2ENCODER_BVF_DC2K       = 15,
  VC2ENCODER_BVF_DC4K       = 16,
  VC2ENCODER_BVF_UHDTV4K_60 = 17,
  VC2ENCODER_BVF_UHDTV4K_50 = 18,
  VC2ENCODER_BVF_UHDTV8K_60 = 19,
  VC2ENCODER_BVF_UHDTV8K_50 = 20,
  VC2ENCODER_BVF_HD1080P_24 = 21,
  VC2ENCODER_BVF_SDPRO486   = 22,

  VC2ENCODER_BVF_NUM
};

enum _VC2EncoderPictureCodingMode {
  VC2ENCODER_PCM_FRAME = 0,
  VC2ENCODER_PCM_FIELD = 1,
};

enum _VC2EncoderWaveletFilterType {
  VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7  = 0,
  VC2ENCODER_WFT_LEGALL_5_3             = 1,
  VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7 = 2,
  VC2ENCODER_WFT_HAAR_NO_SHIFT          = 3,
  VC2ENCODER_WFT_HAAR_SINGLE_SHIFT      = 4,
  VC2ENCODER_WFT_FIDELITY               = 5,
  VC2ENCODER_WFT_DAUBECHIES_9_7         = 6,

  VC2ENCODER_WFT_NUM
};

enum _VC2EncoderSubsamplingFormat {
  VC2ENCODER_CDS_420 = 0,
  VC2ENCODER_CDS_422 = 1,
  VC2ENCODER_CDS_444 = 2
};

enum _VC2EncoderFrameRate {
  VC2ENCODER_FR_CUSTOM = 0,
  VC2ENCODER_FR_24000_1001 = 1,
  VC2ENCODER_FR_24 = 2,
  VC2ENCODER_FR_25 = 3,
  VC2ENCODER_FR_30000_1001 = 4,
  VC2ENCODER_FR_30 = 5,
  VC2ENCODER_FR_50 = 6,
  VC2ENCODER_FR_60000_1001 = 7,
  VC2ENCODER_FR_60 = 8,
  VC2ENCODER_FR_15000_1001 = 9,
  VC2ENCODER_FR_25_2 = 10,
  VC2ENCODER_FR_48 = 11
};

enum _VC2EncoderPresetSignalRange {
  VC2ENCODER_PSR_CUSTOM   = 0,
  VC2ENCODER_PSR_8BITFULL = 1,
  VC2ENCODER_PSR_8BITVID  = 2,
  VC2ENCODER_PSR_10BITVID = 3,
  VC2ENCODER_PSR_12BITVID = 4
};

enum _VC2EncoderColorSpec {
  VC2ENCODER_CSP_CUSTOM  = 0,
  VC2ENCODER_CSP_SDTV525 = 1,
  VC2ENCODER_CSP_SDTV625 = 2,
  VC2ENCODER_CSP_HDTV    = 3,
  VC2ENCODER_CSP_DCINE   = 4
};

enum _VC2EncoderColorPrimaries {
  VC2ENCODER_CPR_HDTV    = 0,
  VC2ENCODER_CPR_SDTV525 = 1,
  VC2ENCODER_CPR_SDTV625 = 2,
  VC2ENCODER_CPR_DCINE   = 3
};

enum _VC2EncoderColorMatrix {
  VC2ENCODER_CMA_HDTV       = 0,
  VC2ENCODER_CMA_SDTV       = 1,
  VC2ENCODER_CMA_REVERSIBLE = 2,
  VC2ENCODER_CMA_RGB        = 3
};

enum _VC2EncoderTransferFunction {
  VC2ENCODER_TRF_TVGAMMA  = 0,
  VC2ENCODER_TRF_EXTGAMUT = 1,
  VC2ENCODER_TRF_LINEAR   = 2,
  VC2ENCODER_TRF_DCINE    = 3
};

typedef struct _VC2EncoderVideoFormat {
  uint32_t base_video_format;


  int custom_dimensions_flag;
  uint32_t frame_width;
  uint32_t frame_height;


  int custom_color_diff_format_flag;
  uint32_t color_diff_format_index;


  int custom_scan_format_flag;
  uint32_t source_sampling;


  int custom_frame_rate_flag;
  uint32_t frame_rate_index;
  uint32_t frame_rate_numer;
  uint32_t frame_rate_denom;


  int custom_pixel_aspect_ratio_flag;
  uint32_t pixel_aspect_ratio_index;
  uint32_t pixel_aspect_ratio_numer;
  uint32_t pixel_aspect_ratio_denom;


  int custom_clean_area_flag;
  uint32_t clean_width;
  uint32_t clean_height;
  uint32_t left_offset;
  uint32_t top_offset;


  int custom_signal_range_flag;
  uint32_t signal_range_index;

  int custom_color_spec_flag;
  uint32_t color_spec_index;

  int custom_color_primaries_flag;
  uint32_t color_primaries_index;

  int custom_color_matrix_flag;
  uint32_t color_matrix_index;

  int custom_transfer_function_flag;
  uint32_t transfer_function_index;
} VC2EncoderVideoFormat;

typedef struct _VC2EncoderTransformParams {
  uint32_t wavelet_index;
  uint32_t wavelet_depth;
  uint32_t slice_width;
  uint32_t slice_height;

  int custom_quant_matrix_flag;
  uint32_t quant_matrix_LL;
  uint32_t quant_matrix_HL[MAX_DWT_DEPTH];
  uint32_t quant_matrix_LH[MAX_DWT_DEPTH];
  uint32_t quant_matrix_HH[MAX_DWT_DEPTH];
} VC2EncoderTransformParams;

enum _VC2EncoderSpeed {
  VC2ENCODER_SPEED_SLOWEST = 0,
  VC2ENCODER_SPEED_SLOWER  = 1,
  VC2ENCODER_SPEED_SLOW    = 2,
  VC2ENCODER_SPEED_MEDIUM  = 3,
  VC2ENCODER_SPEED_FAST    = 4,
  VC2ENCODER_SPEED_FASTER  = 5,
  VC2ENCODER_SPEED_FASTEST = 6
};

typedef struct _VC2EncoderParams {
  VC2EncoderVideoFormat video_format;
  uint32_t picture_coding_mode;
  VC2EncoderTransformParams transform_params;

  int qindex_min;
  int n_threads;
  int speed;
  int numa_first_node;

  VC2EncoderInputFormat input_format;
} VC2EncoderParams;

enum _VC2EncoderEndianness {
  VC2ENCODER_ENDIANNESS_LE = 0,
  VC2ENCODER_ENDIANNESS_BE = 1
};

typedef enum _VC2EncoderResult {
  VC2ENCODER_OK = 0,
  VC2ENCODER_BADPARAMS = 1,
  VC2ENCODER_ENCODE_FAILED = 2,
  VC2ENCODER_BADTHREAD = 3,
  VC2ENCODER_NOTIMPLEMENTED = 4,
  VC2ENCODER_NOQUANTISER = 5,
  VC2ENCODER_CODEROVERRUN = 6,

  VC2ENCODER_UNKNOWN_ERROR = 99
} VC2EncoderResult;


enum _VC2EncoderComponent {
  VC2ENCODER_COMP_Y  = 0,
  VC2ENCODER_COMP_Cb = 1,
  VC2ENCODER_COMP_Cr = 2
};

typedef struct _VC2EncoderLoggers {
  void (*error)(char *, void *);
  void (*warn)(char *, void *);
  void (*info)(char *, void *);
  void (*debug)(char *, void *);
  void *opaque;
} VC2EncoderLoggers;


/* Methods in interface of library */

/*
   This interface allows a lot of flexibility from the calling process, but there are some restrictions. The calling process needs to allocate and manage the memory for
   the output data stream, and is responsible for choosing how much space to allocate.

   And in addition each sequence also needs to set asside enough space for the sequence_start_size and sequence_end_size. How many frames you put between these is up to you.
 */

void vc2encode_init();
void vc2encoder_init_logging(VC2EncoderLoggers);
VC2EncoderHandle vc2encode_create();
VC2EncoderResult vc2encode_set_parameters(VC2EncoderHandle, VC2EncoderParams);
VC2EncoderResult vc2encode_get_parameters(VC2EncoderHandle, VC2EncoderParams *);

VC2EncoderResult vc2encode_get_sequence_start_size(VC2EncoderHandle, uint32_t *size);
VC2EncoderResult vc2encode_start_sequence(VC2EncoderHandle, char **data, uint32_t *next_parse_offset);
VC2EncoderResult vc2encode_repeat_sequence_start(VC2EncoderHandle, char **data, uint32_t prev_parse_offset, uint32_t *next_parse_offset);
VC2EncoderResult vc2encode_get_coded_picture_start_size(VC2EncoderHandle, uint32_t *size);
VC2EncoderResult vc2encode_start_picture(VC2EncoderHandle handle, char **data, uint32_t prev_parse_offset, uint32_t picture_number, int data_length, uint32_t *next_parse_offset);
VC2EncoderResult vc2encode_encode_data(VC2EncoderHandle, char **idata, int *istride, char **odata, int length);
VC2EncoderResult vc2encode_get_auxiliary_data_start_size(VC2EncoderHandle, uint32_t *size);
VC2EncoderResult vc2encode_start_auxiliary_data(VC2EncoderHandle handle, char **data, uint32_t prev_parse_offset, int data_length, uint32_t *next_parse_offset);
VC2EncoderResult vc2encode_get_sequence_end_size(VC2EncoderHandle, uint32_t *size);
VC2EncoderResult vc2encode_end_sequence(VC2EncoderHandle, char **data, uint32_t prev_parse_offset);

void vc2encode_destroy(VC2EncoderHandle);

#ifdef __cplusplus
};
#endif

#endif /* __VC2HQENCODE_H__ */
