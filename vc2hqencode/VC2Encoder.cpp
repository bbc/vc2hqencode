/*****************************************************************************
 * VC2Encoder.cpp : Main implementation file
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

#include "VC2Encoder.hpp"
#include "logger.hpp"

#include <functional>

#include <stdio.h>

#include "logger.hpp"

#include "debug.hpp"

#include "stats.hpp"

#if !defined _WIN32 && !defined __CYGWIN__
  #include "config.h"
#endif

#ifdef VC2_USE_BOOST
#include <boost/bind.hpp>
using boost::bind;
#else
using std::bind;
#endif

#include <numa.h>

#include "vc2transform_c/transform_c.hpp"
#ifndef NO_SIMD
#include "vc2transform_sse4_2/transform_sse4_2.hpp"
#include "vc2transform_avx/transform_avx.hpp"
#include "vc2transform_avx2/transform_avx2.hpp"
#endif

#ifdef DEBUG_P_BLOCK
static int DEBUG_P_JOB;
static int DEBUG_P_SLICE_Y;
static int DEBUG_P_SLICE_W;
static int DEBUG_P_SLICE_H;
#endif

#ifdef DEBUG_SINGLE_ENCODE
  const int NFACTOR = 1;
#else
  const int NFACTOR = 540;
#endif

static bool HAS_SSE4_2 = false;
static bool HAS_AVX    = false;
static bool HAS_AVX2   = false;

GetHTransformInitial get_htransforminitial;
GetHTransform get_htransform;
GetVTransform get_vtransform;

void detect_cpu_features() {
  __builtin_cpu_init();

  HAS_SSE4_2 = __builtin_cpu_supports("sse4.2");
  HAS_AVX    = __builtin_cpu_supports("avx");
  HAS_AVX2   = __builtin_cpu_supports("avx2");

  writelog(LOG_INFO, "Processor Features:");
  if (HAS_SSE4_2)
    writelog(LOG_INFO, "  SSE4.2 [X]");
  else
    writelog(LOG_INFO, "  SSE4.2 [ ]");

  if (HAS_AVX)
    writelog(LOG_INFO, "  AVX    [X]");
  else
    writelog(LOG_INFO, "  AVX    [ ]");

  if (HAS_AVX2)
    writelog(LOG_INFO, "  AVX2   [X]");
  else
    writelog(LOG_INFO, "  AVX2   [ ]");

  get_htransforminitial = get_htransforminitial_c;
  get_htransform = get_htransform_c;
  get_vtransform = get_vtransform_c;

#ifndef NO_SIMD
  if (HAS_SSE4_2) {
    get_htransforminitial = get_htransforminitial_sse4_2;
    get_htransform = get_htransform_sse4_2;
    get_vtransform = get_vtransform_sse4_2;
  }

#ifndef NO_AVX
  if (HAS_AVX) {
    get_htransforminitial = get_htransforminitial_avx;
    get_htransform = get_htransform_avx;
    get_vtransform = get_vtransform_avx;
  }

  if (HAS_AVX2) {
    get_htransforminitial = get_htransforminitial_avx2;
    get_htransform = get_htransform_avx2;
    get_vtransform = get_vtransform_avx2;
  }
#endif
#endif
}

void VC2Encoder::setParams(VC2EncoderParams &params) throw(VC2EncoderResult){
  mParams = params;

  if (mJobsInFlight != 0) {
    writelog(LOG_ERROR, "%s:%d:  Params change whilst working", __FILE__, __LINE__);
    throw VC2ENCODER_BADTHREAD;
  }

  if (mPool) {
    mPool->stop();
    delete mPool;
  }

  mCoefSize = 2;
  if (params.transform_params.wavelet_depth > 4)
    mCoefSize = 4;

  mVideoFormat = vc2::preset_formats[params.video_format.base_video_format];

  if(params.video_format.custom_dimensions_flag) {
    mVideoFormat.frame_width  = params.video_format.frame_width;
    mVideoFormat.frame_height = params.video_format.frame_height;
  }

  if (params.video_format.custom_color_diff_format_flag) {
    mVideoFormat.color_diff_format_index = params.video_format.color_diff_format_index;
  }

  if (params.video_format.custom_scan_format_flag) {
    mVideoFormat.source_sampling = params.video_format.source_sampling;
  }

  if (params.video_format.custom_frame_rate_flag) {
    if (params.video_format.frame_rate_index == 0) {
      mVideoFormat.frame_rate_numer = params.video_format.frame_rate_numer;
      mVideoFormat.frame_rate_denom = params.video_format.frame_rate_denom;
    } else {
      mVideoFormat.frame_rate_numer = vc2::preset_framerates[params.video_format.frame_rate_index].numer;
      mVideoFormat.frame_rate_denom = vc2::preset_framerates[params.video_format.frame_rate_index].denom;
    }
  }

  if (params.video_format.custom_pixel_aspect_ratio_flag) {
    if (params.video_format.pixel_aspect_ratio_index == 0) {
      mVideoFormat.pixel_aspect_ratio_numer = params.video_format.pixel_aspect_ratio_numer;
      mVideoFormat.pixel_aspect_ratio_denom = params.video_format.pixel_aspect_ratio_denom;
    } else {
      mVideoFormat.pixel_aspect_ratio_numer = vc2::preset_pixel_aspect_ratios[params.video_format.pixel_aspect_ratio_index][0];
      mVideoFormat.pixel_aspect_ratio_denom = vc2::preset_pixel_aspect_ratios[params.video_format.pixel_aspect_ratio_index][1];
    }
  }

  if (params.video_format.custom_clean_area_flag) {
    mVideoFormat.clean_width  = params.video_format.clean_width;
    mVideoFormat.clean_height = params.video_format.clean_height;
    mVideoFormat.left_offset  = params.video_format.left_offset;
    mVideoFormat.top_offset   = params.video_format.top_offset;
  }

  if (params.video_format.custom_signal_range_flag) {
    mVideoFormat.luma_offset                 = vc2::preset_signal_ranges[params.video_format.signal_range_index].luma_offset;
    mVideoFormat.luma_excursion              = vc2::preset_signal_ranges[params.video_format.signal_range_index].luma_excursion;
    mVideoFormat.luma_bytes_per_sample       = vc2::preset_signal_ranges[params.video_format.signal_range_index].luma_bytes_per_sample;
    mVideoFormat.luma_active_bits            = vc2::preset_signal_ranges[params.video_format.signal_range_index].luma_active_bits;
    mVideoFormat.color_diff_offset           = vc2::preset_signal_ranges[params.video_format.signal_range_index].color_diff_offset;
    mVideoFormat.color_diff_excursion        = vc2::preset_signal_ranges[params.video_format.signal_range_index].color_diff_excursion;
    mVideoFormat.color_diff_bytes_per_sample = vc2::preset_signal_ranges[params.video_format.signal_range_index].color_diff_bytes_per_sample;
    mVideoFormat.color_diff_active_bits      = vc2::preset_signal_ranges[params.video_format.signal_range_index].color_diff_active_bits;
  }

  if (params.video_format.custom_color_spec_flag) {
    if (params.video_format.color_spec_index == 0) {
      if (params.video_format.custom_color_primaries_flag) {
        mVideoFormat.color_primaries = params.video_format.color_primaries_index;
      }

      if (params.video_format.custom_color_matrix_flag) {
        mVideoFormat.color_matrix = params.video_format.color_matrix_index;
      }

      if (params.video_format.custom_transfer_function_flag) {
        mVideoFormat.transfer_function = params.video_format.transfer_function_index;
      }
    } else {
      mVideoFormat.color_primaries   = vc2::preset_color_specs[params.video_format.color_spec_index].color_primaries;
      mVideoFormat.color_matrix      = vc2::preset_color_specs[params.video_format.color_spec_index].color_matrix;
      mVideoFormat.transfer_function = vc2::preset_color_specs[params.video_format.color_spec_index].transfer_function;
    }
  }

  mLumaWidth       = mVideoFormat.frame_width;
  mLumaHeight      = mVideoFormat.frame_height;
  mColorDiffWidth  = mVideoFormat.frame_width;
  mColorDiffHeight = mVideoFormat.frame_height;

  if (params.picture_coding_mode == VC2ENCODER_PCM_FRAME)
    mInterlaced = false;
  else
    mInterlaced = true;

  int color_diff_height_factor = 1;
  int color_diff_width_factor  = 1;

  switch (mVideoFormat.color_diff_format_index) {
  case VC2ENCODER_CDS_420:
    color_diff_height_factor = 2;
  case VC2ENCODER_CDS_422:
    color_diff_width_factor = 2;
    break;
  }

  if (mJobData) {
    for (int i = 0; i < mJobs; i++)
      delete mJobData[i];
    delete[] mJobData;
  }

  mColorDiffWidth  /= color_diff_width_factor;
  mColorDiffHeight /= color_diff_height_factor;

  if (mInterlaced) {
    mColorDiffHeight /= 2;
    mLumaHeight      /= 2;
  }

  mPaddedWidth  = (mLumaWidth  + params.transform_params.slice_width  - 1)/params.transform_params.slice_width*params.transform_params.slice_width;
  mPaddedHeight = (mLumaHeight + params.transform_params.slice_height - 1)/params.transform_params.slice_height*params.transform_params.slice_height;

  if (((params.transform_params.slice_width/color_diff_width_factor)%(1 << params.transform_params.wavelet_depth) != 0) ||
      ((params.transform_params.slice_height/color_diff_height_factor)%(1 << params.transform_params.wavelet_depth) != 0)) {
    writelog(LOG_ERROR, "%s:%d: Transform depth inavlid for this slice size\n", __FILE__, __LINE__);
    throw VC2ENCODER_BADPARAMS;
  }


#ifndef DEBUG_SINGLE_JOB
  mThreads = params.n_threads;
  for (mJobs = 1; mJobs < params.n_threads*4; mJobs <<= 1);
#else
  mThreads = 1;
  mJobs = 1;
#endif

  writelog(LOG_INFO, "%s:%d: Configuring for %d threads, %d jobs\n", __FILE__, __LINE__, mThreads, mJobs);

#ifndef DEBUG
  const int numa_first_node = 0;
  mPool = new ThreadPool(mThreads, numa_first_node);
#endif

  mSlicesPerLine  = mPaddedWidth/params.transform_params.slice_width;
  mSlicesPerPicture = (mPaddedHeight/params.transform_params.slice_height)*mSlicesPerLine;

  mDepth = params.transform_params.wavelet_depth;
  mBlockOverlap = vc2::transform_block_overlap[params.transform_params.wavelet_index] * (1 << (mDepth - 1));


  if (mSlices16)
    delete mSlices16;

  if (mSlices32)
    delete mSlices32;
  if (mCoefSize == 2) {
    mSlices16 = new CodedSlices<int16_t>(mSlicesPerLine, mSlicesPerPicture/mSlicesPerLine,
                                         params.transform_params.slice_width, params.transform_params.slice_height, mDepth);
  } else if (mCoefSize == 4) {
    mSlices32 = new CodedSlices<int32_t>(mSlicesPerLine, mSlicesPerPicture/mSlicesPerLine,
                                         params.transform_params.slice_width, params.transform_params.slice_height, mDepth);
  }

  mJobData = new JobBase*[mJobs];


  /*
    The constructor for JobData takes a *lot* of parameters:
      int j     == job number
      int w     == width of picture portion represented
      int h     == height (not including padding)
      int pt    == pad top (in pixels)    -- This padding will contain data from the picture but is not part of the coded output, and must be a whole number of slices
      int pb    == pad bottom (in pixels)
      int d     == depth
      int sx    == slices_x
      int sy    == slices_y (not including padding)
      int off_x == not used
      int off_y == offset in lines to the input data for the first slice in this job INCLUDING padding

      CodedSlices<T> *coded_slices, == coded slices
      int slice_offset_x,           == not used
      int slice_offset_y,           == the offset in slice-lines to the first OUTPUT slice of this job, apparently
      int slice_stride              == how many slices accross the picture
   */
  const int slice_overlap = (TRANSFORM_OVERLAP[mParams.transform_params.wavelet_index][mParams.transform_params.wavelet_depth - 1] + params.transform_params.slice_height - 1)/params.transform_params.slice_height;
  if (mCoefSize == 2) {
    if (mJobs == 1) {
      mJobData[0] = new JobData<int16_t>(0,
                                         mLumaWidth, mLumaHeight,
                                         0, 0,
                                         mDepth,
                                         mSlicesPerLine, mSlicesPerPicture/mSlicesPerLine,
                                         0, 0,
                                         mSlices16,
                                         0, 0,
                                         mSlicesPerLine);
    } else {
      int slices_y = ((mSlicesPerPicture/mSlicesPerLine) + mJobs - 1)/mJobs;
      if (slices_y < slice_overlap)
        slices_y = slice_overlap;
      int slices_already = 0;
      int i = 0;
      {
        mJobData[i] = new JobData<int16_t>(i,
                                           mLumaWidth, slices_y*params.transform_params.slice_height,
                                           0, slice_overlap*params.transform_params.slice_height,
                                           mDepth,
                                           mSlicesPerLine, slices_y,
                                           0, 0,
                                           mSlices16,
                                           0, 0,
                                           mSlicesPerLine);
        slices_already += slices_y;
        slices_y = ((mSlicesPerPicture/mSlicesPerLine - slices_already) + mJobs - i - 1)/(mJobs - i);
      }
      i++;
      for (; i < mJobs - 1 && slices_already + slices_y  + slice_overlap < mSlicesPerPicture/mSlicesPerLine; i++) {
        mJobData[i] = new JobData<int16_t>(i,
                                           mLumaWidth, slices_y*params.transform_params.slice_height,
                                           slice_overlap*params.transform_params.slice_height, slice_overlap*params.transform_params.slice_height,
                                           mDepth,
                                           mSlicesPerLine, slices_y,
                                           0, (slices_already - slice_overlap)*params.transform_params.slice_height,
                                           mSlices16,
                                           0, slices_already,
                                           mSlicesPerLine);
        slices_already += slices_y;
        slices_y = ((mSlicesPerPicture/mSlicesPerLine - slices_already) + mJobs - i - 1)/(mJobs - i);
      }
      {
        mJobData[i] = new JobData<int16_t>(i,
                                           mLumaWidth, mLumaHeight - slices_already*params.transform_params.slice_height,
                                           slice_overlap*params.transform_params.slice_height, 0,
                                           mDepth,
                                           mSlicesPerLine, mSlicesPerPicture/mSlicesPerLine - slices_already,
                                           0, (slices_already - slice_overlap)*params.transform_params.slice_height,
                                           mSlices16,
                                           0, slices_already,
                                           mSlicesPerLine);
      }
      i++;
      mJobs = i;
    }
  } else if (mCoefSize == 4) {
    if (mJobs == 1) {
      mJobData[0] = new JobData<int32_t>(0,
                                         mLumaWidth, mLumaHeight,
                                         0, 0,
                                         mDepth,
                                         mSlicesPerLine, mSlicesPerPicture/mSlicesPerLine,
                                         0, 0,
                                         mSlices32,
                                         0, 0,
                                         mSlicesPerLine);
    } else {
      int slices_y = ((mSlicesPerPicture/mSlicesPerLine) + mJobs - 1)/mJobs;
      if (slices_y < slice_overlap)
        slices_y = slice_overlap;
      int slices_already = 0;
      int i = 0;
      {
        mJobData[0] = new JobData<int32_t>(i,
                                           mLumaWidth, slices_y*params.transform_params.slice_height,
                                           0, slice_overlap*params.transform_params.slice_height,
                                           mDepth,
                                           mSlicesPerLine, slices_y,
                                           0, 0,
                                           mSlices32,
                                           0, 0,
                                           mSlicesPerLine);
        slices_already += slices_y;
        slices_y = ((mSlicesPerPicture/mSlicesPerLine - slices_already) + mJobs - i - 1)/(mJobs - i);
      }
      i++;
      for (int i = 1; i < mJobs - 1 && slices_already + slices_y  + slice_overlap < mSlicesPerPicture/mSlicesPerLine; i++) {
        mJobData[i] = new JobData<int32_t>(i,
                                           mLumaWidth, slices_y*params.transform_params.slice_height,
                                           slice_overlap*params.transform_params.slice_height, slice_overlap*params.transform_params.slice_height,
                                           mDepth,
                                           mSlicesPerLine, slices_y,
                                           0, (slices_already - slice_overlap)*params.transform_params.slice_height,
                                           mSlices32,
                                           0, slices_already,
                                           mSlicesPerLine);
        slices_already += slices_y;
        slices_y = ((mSlicesPerPicture/mSlicesPerLine - slices_already) + mJobs - i - 1)/(mJobs - i);
      }
      {
        mJobData[i] = new JobData<int32_t>(i,
                                           mLumaWidth, mLumaHeight - slices_already*params.transform_params.slice_height,
                                           slice_overlap*params.transform_params.slice_height, 0,
                                           mDepth,
                                           mSlicesPerLine, mSlicesPerPicture/mSlicesPerLine - slices_already,
                                           0, (slices_already - slice_overlap)*params.transform_params.slice_height,
                                           mSlices32,
                                           0, slices_already,
                                           mSlicesPerLine);
      }
      i++;
      mJobs = i;
    }
  }

#ifdef DEBUG_P_BLOCK
  DEBUG_P_JOB = 0;
  DEBUG_P_SLICE_Y = DEBUG_P_BLOCK_Y;
  for (int i = 0; i < mJobs; i++) {
    if ((int)(DEBUG_P_SLICE_Y) < (int)((mJobData[i]->height + params.transform_params.slice_height - 1)/params.transform_params.slice_height))
      break;
    DEBUG_P_SLICE_Y -= (mJobData[i]->height + params.transform_params.slice_height - 1)/params.transform_params.slice_height;
    DEBUG_P_JOB++;
  }

  DEBUG_P_SLICE_W = (DEBUG_P_COMP == 0)?params.transform_params.slice_width:params.transform_params.slice_width/2;
  DEBUG_P_SLICE_H = params.transform_params.slice_height;
#endif

  if (mQuantisationMatrices)
    delete mQuantisationMatrices;

  mQuantisationMatrices = new QuantisationMatrices(params.transform_params.slice_width, params.transform_params.slice_height,
                                                   mDepth, params.transform_params.wavelet_index, 0, 32);

  if (mSequenceHeader)
    delete mSequenceHeader;
  mSequenceHeader = new SequenceHeader(mParams);
  if (mPictureHeader)
    delete mPictureHeader;
  mPictureHeader  = new PictureHeader(mParams, mSlicesPerLine, mSlicesPerPicture/mSlicesPerLine, mSliceSizeScalar);

  transform_initial[0] = get_htransforminitial(mParams.transform_params.wavelet_index, 10, mCoefSize, 0, mParams.input_format);
  transform_initial[1] = get_htransforminitial(mParams.transform_params.wavelet_index, 10, mCoefSize, 1, mParams.input_format);
  transform_initial[2] = get_htransforminitial(mParams.transform_params.wavelet_index, 10, mCoefSize, 2, mParams.input_format);

  if (transforms_h)
    delete[] transforms_h;
  transforms_h = new InplaceTransform[mDepth];
  for (int l = 1; l < mDepth; l++)
    transforms_h[l] = get_htransform(mParams.transform_params.wavelet_index, l, mCoefSize);

  if (transforms_v)
    delete[] transforms_v;
  transforms_v = new InplaceTransform[mDepth];
  for (int l = 0; l < mDepth; l++)
    transforms_v[l] = get_vtransform(mParams.transform_params.wavelet_index, l, mCoefSize, mParams.input_format);

  int QUANTISER_SELECTION_QUALITY = QUANTISER_SELECTION_FULLSEARCH;
  int passes = 1;
  switch (mParams.speed) {
  case VC2ENCODER_SPEED_SLOWEST:
    QUANTISER_SELECTION_QUALITY = QUANTISER_SELECTION_FULLSEARCH;
    passes = 3;
    break;
  case VC2ENCODER_SPEED_SLOWER:
    QUANTISER_SELECTION_QUALITY = QUANTISER_SELECTION_FULLSEARCH;
    passes = 2;
    break;
  case VC2ENCODER_SPEED_SLOW:
    QUANTISER_SELECTION_QUALITY = QUANTISER_SELECTION_FULLSEARCH;
    passes = 1;
    break;
  case VC2ENCODER_SPEED_MEDIUM:
    QUANTISER_SELECTION_QUALITY = QUANTISER_SELECTION_HALFSEARCH;
    passes = 1;
    break;
  case VC2ENCODER_SPEED_FAST:
  case VC2ENCODER_SPEED_FASTER:
    QUANTISER_SELECTION_QUALITY = QUANTISER_SELECTION_QUARTERSEARCH;
    passes = 1;
    break;
  case VC2ENCODER_SPEED_FASTEST:
    QUANTISER_SELECTION_QUALITY = QUANTISER_SELECTION_EIGHTHSEARCH;
    passes = 1;
    break;
  }

  slice_encoder_func16 = get_slice_encoder16(params.transform_params.slice_width, params.transform_params.slice_height, mDepth, QUANTISER_SELECTION_QUALITY, passes);
  slice_encoder_func32 = get_slice_encoder32(params.transform_params.slice_width, params.transform_params.slice_height, mDepth, QUANTISER_SELECTION_QUALITY, passes);
}

uint32_t VC2Encoder::startSequence(char **data) {
  mSequenceHeader->setPrevParseOffset(0);
  memcpy(*data, mSequenceHeader->data, mSequenceHeader->length);
  *data += mSequenceHeader->length;
  return mSequenceHeader->length;
}

uint32_t VC2Encoder::repeatSequenceStart(char **data, uint32_t prev_parse_offset) {
  mSequenceHeader->setPrevParseOffset(prev_parse_offset);
  memcpy(*data, mSequenceHeader->data, mSequenceHeader->length);
  *data += mSequenceHeader->length;
  return mSequenceHeader->length;
}


uint32_t VC2Encoder::startPicture(char **data, uint32_t prev_parse_offset, uint32_t picture_number, int data_length) {
  mPictureHeader->setNextParseOffset(data_length + mPictureHeader->length);
  mPictureHeader->setPrevParseOffset(prev_parse_offset);
  mPictureHeader->setPictureNumber(picture_number);

  memcpy(*data, mPictureHeader->data, mPictureHeader->length);
  *data += mPictureHeader->length;
  return data_length + mPictureHeader->length;
}

uint32_t VC2Encoder::startAuxiliaryData(char **_data, uint32_t prev_parse_offset, int data_length) {
  int length = 0;
  uint8_t *data = (uint8_t *)(*_data);
  uint32_t next_parse_offset = 13 + data_length;
  data[length++] = 0x42; // B
  data[length++] = 0x42; // B
  data[length++] = 0x43; // C
  data[length++] = 0x44; // D

  data[length++] = 0x20; // Auxiliary Data

  data[length++] = (next_parse_offset >> 24)&0xFF; // next_parse_offset,
  data[length++] = (next_parse_offset >> 16)&0xFF; //
  data[length++] = (next_parse_offset >>  8)&0xFF; //
  data[length++] = (next_parse_offset >>  0)&0xFF; //

  data[length++] = (prev_parse_offset >> 24)&0xFF; // previous_parse_offset,
  data[length++] = (prev_parse_offset >> 16)&0xFF; //
  data[length++] = (prev_parse_offset >>  8)&0xFF; //
  data[length++] = (prev_parse_offset >>  0)&0xFF; //

  *_data += 13;

  return next_parse_offset;
}

#ifdef DEBUG
#define INIT_MT
#define MT_JOB(X) (X())
#define EXEC_MT 0
#else
#define INIT_MT mPool->ready()
#define MT_JOB(X) mPool->post(X)
#define EXEC_MT mPool->execute()
#endif


bool VC2Encoder::encodeData(char **idata, int *istride, char **_odata, int length) {
  (void)idata;
  (void)length;
  char *odata = *_odata;

  ++mJobsInFlight;

  int l = ((length + mJobs - 1)/mJobs)/mSliceSizeScalar*mSliceSizeScalar;
  if (mParams.input_format == VC2ENCODER_INPUT_10P2) {
    for (int i = 0; i < mJobs; i++) {
      if (mJobData[i]) {
        mJobData[i]->idata[0] = &idata[0][2*(mJobData[i]->offset_y*istride[0] + mJobData[i]->offset_x)];
        mJobData[i]->idata[1] = &idata[1][2*(mJobData[i]->offset_y*istride[1] + mJobData[i]->offset_x/2)];
        mJobData[i]->idata[2] = &idata[2][2*(mJobData[i]->offset_y*istride[2] + mJobData[i]->offset_x/2)];

        mJobData[i]->istride[0] = istride[0];
        mJobData[i]->istride[1] = istride[1];
        mJobData[i]->istride[2] = istride[2];

        mJobData[i]->odata = &odata[i*l];
        mJobData[i]->olength = l;
      }
    }
    if (mJobData[mJobs - 1]) {
      mJobData[mJobs - 1]->olength = (length/mSliceSizeScalar*mSliceSizeScalar) - (mJobs - 1)*l;
    }
  } else if (mParams.input_format == VC2ENCODER_INPUT_V210) {
    for (int i = 0; i < mJobs; i++) {
      if (mJobData[i]) {
        mJobData[i]->idata[0] = &idata[0][mJobData[i]->offset_y*istride[0]];
        mJobData[i]->istride[0] = istride[0];
      }
    }
  } else {
    writelog(LOG_ERROR, "%s:%d:  Unknown input format", __FILE__, __LINE__);
    throw VC2ENCODER_BADPARAMS;
  }

  if (mCoefSize == 2) {
    INIT_MT;
    for (int i = 0; i < mJobs; i++) {
      if (mJobData[i])
        MT_JOB(bind(&VC2Encoder::Transform<int16_t>, this, mJobData[i]));
    }
    if (EXEC_MT)
      throw VC2ENCODER_ENCODE_FAILED;
  } else if (mCoefSize == 4) {
    INIT_MT;
    for (int i = 0; i < mJobs; i++) {
      if (mJobData[i])
        MT_JOB(bind(&VC2Encoder::Transform<int32_t>, this, mJobData[i]));
    }
    if (EXEC_MT)
      throw VC2ENCODER_ENCODE_FAILED;
  }

#ifdef DEBUG_PRINT_STATS
  for (int i = 0; i < mJobs; i++) {
    if (mJobData[i])
        accumulate_transformed_statistics(mJobData[i]);
  }
#endif

  if (mCoefSize == 2) {
    INIT_MT;
    CodedSlice<int16_t> *slices = mSlices16->slices;
    int n_slices = (mSlicesPerPicture + NFACTOR - 1)/NFACTOR;
    int k = 0;
    int remaining_length = (length - 4*mSlicesPerPicture)/mSliceSizeScalar;
    int remaining_slices = mSlicesPerPicture;
    for (; remaining_slices > n_slices; k++) {
      int olength = n_slices*remaining_length/remaining_slices;
      MT_JOB(bind(&VC2Encoder::EncodePartial<int16_t>, this,
                         slices,
                         n_slices,
                         odata,
                         olength*mSliceSizeScalar + 4*n_slices,
                         k,
                         mSlices16->width[0]*mSlices16->height[0]));
      slices += n_slices;
      remaining_slices -= n_slices;
      odata += olength*mSliceSizeScalar + 4*n_slices;
      remaining_length -= olength;
    }
    MT_JOB(bind(&VC2Encoder::EncodePartial<int16_t>, this,
                       slices,
                       remaining_slices,
                       odata,
                       remaining_length*mSliceSizeScalar + 4*remaining_slices,
                       k,
                       mSlices16->width[0]*mSlices16->height[0]));
    if (EXEC_MT)
      throw VC2ENCODER_ENCODE_FAILED;
  } else if (mCoefSize == 4) {
    INIT_MT;
    CodedSlice<int32_t> *slices = mSlices32->slices;
    int n_slices = (mSlicesPerPicture + NFACTOR - 1)/NFACTOR;
    int k = 0;
    int remaining_length = (length - 4*mSlicesPerPicture)/mSliceSizeScalar;
    int remaining_slices = mSlicesPerPicture;
    for (; remaining_slices > n_slices; k++) {
      int olength = n_slices*remaining_length/remaining_slices;
      MT_JOB(bind(&VC2Encoder::EncodePartial<int32_t>, this,
                         slices,
                         n_slices,
                         odata,
                         olength*mSliceSizeScalar + 4*n_slices,
                         k,
                         mSlices16->width[0]*mSlices16->height[0]));
      slices += n_slices;
      remaining_slices -= n_slices;
      odata += olength*mSliceSizeScalar + 4*n_slices;
      remaining_length -= olength;
    }
    MT_JOB(bind(&VC2Encoder::EncodePartial<int32_t>, this,
                       slices,
                       remaining_slices,
                       odata,
                       remaining_length*mSliceSizeScalar + 4*remaining_slices,
                       k,
                       mSlices16->width[0]*mSlices16->height[0]));
    if (EXEC_MT)
      throw VC2ENCODER_ENCODE_FAILED;
  }

#ifdef DEBUG_PRINT_STATS
  accumulate_encoded_statistics(mSlices16->slices, mSlicesPerPicture);
#endif

  /* Clean Up */
  --mJobsInFlight;

#ifdef DEBUG_P_BLOCK
  {
    printf("-----------------------------------------------------------------\n");
    printf("  Quantised: qi=%d\n", mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex);
    printf("-----------------------------------------------------------------\n");
    const int qindex = (mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex <= 31)?(mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex):(28 + (mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex%4));
    const uint8_t qshift = (mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex <= 31)?(0):((mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex/4) - 7);
    const uint16_t *qf = mQuantisationMatrices->qf(qindex, DEBUG_P_COMP);
    for (int y = 0; y < DEBUG_P_SLICE_H; y++) {
      int16_t *D = &mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].idata[DEBUG_P_COMP][y*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].istride[DEBUG_P_COMP]];
      printf("  ");
      for (int x = 0; x < DEBUG_P_SLICE_W; x ++)
        printf("%+6d  ", 4*(D[x] >> qshift)/qf[x]);
      printf("\n");
      qf += DEBUG_P_SLICE_W;
    }
    printf("-----------------------------------------------------------------\n");
  }

  {
    int16_t D[DEBUG_P_SLICE_H*DEBUG_P_SLICE_W];
    const int qindex = (mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex <= 31)?(mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex):(28 + (mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex%4));
    const uint8_t qshift = (mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex <= 31)?(0):((mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].qindex/4) - 7);
    const uint16_t *qf = mQuantisationMatrices->qf(qindex, DEBUG_P_COMP);
    int skip = 1 << mDepth;
    int n = 0;
    for (int y = 0; y < DEBUG_P_SLICE_H; y += skip)
      for (int x = 0; x < DEBUG_P_SLICE_W; x += skip)
        D[n++] = (4*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].idata[DEBUG_P_COMP][y*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].istride[DEBUG_P_COMP] + x] >> qshift)/qf[y*DEBUG_P_SLICE_W + x];

    for (int l = 0; l < mDepth; l++) {
      for (int y = 0; y < DEBUG_P_SLICE_H; y += skip)
        for (int x = skip/2; x < DEBUG_P_SLICE_W; x += skip)
          D[n++] = (4*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].idata[DEBUG_P_COMP][y*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].istride[DEBUG_P_COMP] + x] >> qshift)/qf[y*DEBUG_P_SLICE_W + x];

      for (int y = skip/2; y < DEBUG_P_SLICE_H; y += skip)
        for (int x = 0; x < DEBUG_P_SLICE_W; x += skip)
          D[n++] = (4*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].idata[DEBUG_P_COMP][y*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].istride[DEBUG_P_COMP] + x] >> qshift)/qf[y*DEBUG_P_SLICE_W + x];

      for (int y = skip/2; y < DEBUG_P_SLICE_H; y += skip)
        for (int x = skip/2; x < DEBUG_P_SLICE_W; x += skip)
          D[n++] = (4*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].idata[DEBUG_P_COMP][y*mSlices16->slices[DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X].istride[DEBUG_P_COMP] + x] >> qshift)/qf[y*DEBUG_P_SLICE_W + x];

      skip /= 2;
    }

    printf("-----------------------------------------------------------------\n");
    printf("  Reordered:\n");
    printf("-----------------------------------------------------------------\n");
    for (int y = 0; y < DEBUG_P_SLICE_H; y++) {
      printf("  ");
      for (int x = 0; x < DEBUG_P_SLICE_W; x ++)
        printf("%+6d  ", D[y*DEBUG_P_SLICE_W + x]);
      printf("\n");
    }
    printf("-----------------------------------------------------------------\n");
  }

  {
    int BN = DEBUG_P_BLOCK_Y*mSlicesPerLine + DEBUG_P_BLOCK_X;
    CodedSlice<int16_t> *slice = &mSlices16->slices[BN];
    {
      printf("-----------------------------------------------------------------\n");
      printf("  Encoded\n");
      printf("-----------------------------------------------------------------\n");
      uint16_t *D = (uint16_t *)(slice->codewords[DEBUG_P_COMP]);
      uint8_t  *L = (uint8_t  *)(slice->wordlengths[DEBUG_P_COMP]);
#define CODEWORD(N) (D[(N)])
      for (int y = 0; y < DEBUG_P_SLICE_H; y++) {
        for (int x = 0; x < DEBUG_P_SLICE_W; x++)
          printf(" %04x/%-2d", CODEWORD(x), L[x]);
        printf("\n");
        D += DEBUG_P_SLICE_W;
        L += DEBUG_P_SLICE_W;
      }
      printf("-----------------------------------------------------------------\n");
    }
  }
#endif /*DEBUG_P_BLOCK*/

  *_odata += length;

  return true;
}

void VC2Encoder::endSequence(char **_data, uint32_t prev_parse_offset) {
  int length = 0;
  char *data = *_data;
  data[length++] = 0x42; // B
  data[length++] = 0x42; // B
  data[length++] = 0x43; // C
  data[length++] = 0x44; // D

  data[length++] = 0x10; // Sequence End

  data[length++] = 0x00; // next_parse_offset,
  data[length++] = 0x00; //
  data[length++] = 0x00; //
  data[length++] = 0x00; //

  data[length++] = (prev_parse_offset >> 24)&0xFF; // previous_parse_offset,
  data[length++] = (prev_parse_offset >> 16)&0xFF; //
  data[length++] = (prev_parse_offset >>  8)&0xFF; //
  data[length++] = (prev_parse_offset >>  0)&0xFF; //

  *_data += 13;
}



template<class T> void VC2Encoder::EncodePartial(CodedSlice<T> *slices, int n_slices, char *odata, int olength, int N, int n_samples) {
  (void)N;
  /* First Encode Pass */
  Encode<T>(slices, n_slices, olength);

#ifdef DEBUG_OP_QI
  {
    std::stringstream ss;
    ss << "qi_" << N << ".raw.out";
    int of = open(ss.str().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00777);
    for (int n = 0; n < n_slices; n++) {
      write(of, &slices[n].qindex, 1);
    }
    close(of);
  }
#endif


  /* Serialise */
  Serialise<T>(slices, n_slices, odata, olength, n_samples);
}

template <class T> void VC2Encoder::Transform(JobBase *_job) {
  JobData<T> *job = dynamic_cast<JobData<T> *>(_job);
  char **idata = job->idata;
  int *istride = job->istride;
  void *odata[3] = { (void *)job->video_data[0]->data,
                     (void *)job->video_data[1]->data,
                     (void *)job->video_data[2]->data };
  for (int c = 0; c < 3; c++) {
    int skip = 1;
#ifdef DEBUG_P_BLOCK
    if (mParams.input_format == VC2ENCODER_INPUT_10P2) {
      if (c == DEBUG_P_COMP && job->job == DEBUG_P_JOB) {
        printf("-----------------------------------------------------------------\n");
        printf("  Input\n");
        printf("-----------------------------------------------------------------\n");
        for (int y = 0; y < DEBUG_P_SLICE_H; y++) {
          uint16_t *D = (uint16_t *)(&idata[c][2*((DEBUG_P_SLICE_Y*DEBUG_P_SLICE_H + y + job->pad_t)*istride[c] + DEBUG_P_BLOCK_X*DEBUG_P_SLICE_W)]);
          for (int x = 0; x < DEBUG_P_SLICE_W; x++)
            printf("  %6u", D[x]);
          printf("\n");
        }
        printf("-----------------------------------------------------------------\n");
      }
    }
#endif /*DEBUG_P_BLOCK*/

    // First level H transform
    if (transform_initial[c])
      transform_initial[c](idata[c], istride[c], odata + c, job->video_data[c]->stride,
                           job->iwidth[c],
                           job->iheight[c],
                           job->video_data[c]->width, job->video_data[c]->height);

#ifdef DEBUG_P_BLOCK
    if (c == DEBUG_P_COMP && job->job == DEBUG_P_JOB) {
      printf("-----------------------------------------------------------------\n");
      printf("  Post H-transform 0\n");
      printf("-----------------------------------------------------------------\n");
      for (int y = 0; y < DEBUG_P_SLICE_H; y++) {
        T *D = &job->video_data[c]->data[((DEBUG_P_SLICE_Y*DEBUG_P_SLICE_H + y + job->pad_t)*job->video_data[c]->stride + DEBUG_P_BLOCK_X*DEBUG_P_SLICE_W)];
        for (int x = 0; x < DEBUG_P_SLICE_W; x++)
          printf("  %+6d", D[x]);
        printf("\n");
      }
      printf("-----------------------------------------------------------------\n");
    }
#endif /*DEBUG_P_BLOCK*/

    if (transforms_v[0])
      transforms_v[0](job->video_data[c]->data, job->video_data[c]->stride, job->video_data[c]->width, job->video_data[c]->height, skip);

#ifdef DEBUG_P_BLOCK
    if (c == DEBUG_P_COMP && job->job == DEBUG_P_JOB) {
      printf("-----------------------------------------------------------------\n");
      printf("  Post V-transform 0\n");
      printf("-----------------------------------------------------------------\n");
      for (int y = 0; y < DEBUG_P_SLICE_H; y++) {
        T *D = &job->video_data[c]->data[((DEBUG_P_SLICE_Y*DEBUG_P_SLICE_H + y + job->pad_t)*job->video_data[c]->stride + DEBUG_P_BLOCK_X*DEBUG_P_SLICE_W)];
        for (int x = 0; x < DEBUG_P_SLICE_W; x++)
          printf("  %+6d", D[x]);
        printf("\n");
      }
      printf("-----------------------------------------------------------------\n");
    }
#endif /*DEBUG_P_BLOCK*/

    for (int l = 1; l < mDepth; l++) {
      skip <<= 1;
      transforms_h[l](job->video_data[c]->data, job->video_data[c]->stride, job->video_data[c]->width, job->video_data[c]->height, skip);

#ifdef DEBUG_P_BLOCK
      if (c == DEBUG_P_COMP && job->job == DEBUG_P_JOB) {
        printf("-----------------------------------------------------------------\n");
        printf("  Post H-transform %d\n", l);
        printf("-----------------------------------------------------------------\n");
        for (int y = 0; y < DEBUG_P_SLICE_H; y++) {
          T *D = &job->video_data[c]->data[((DEBUG_P_SLICE_Y*DEBUG_P_SLICE_H + y + job->pad_t)*job->video_data[c]->stride + DEBUG_P_BLOCK_X*DEBUG_P_SLICE_W)];
          for (int x = 0; x < DEBUG_P_SLICE_W; x++)
            printf("  %+6d", D[x]);
          printf("\n");
        }
        printf("-----------------------------------------------------------------\n");
      }
#endif /*DEBUG_P_BLOCK*/

      if (transforms_v[l])
        transforms_v[l](job->video_data[c]->data, job->video_data[c]->stride, job->video_data[c]->width, job->video_data[c]->height, skip);

#ifdef DEBUG_P_BLOCK
      if (c == DEBUG_P_COMP && job->job == DEBUG_P_JOB) {
        printf("-----------------------------------------------------------------\n");
        printf("  Post V-transform %d\n", l);
        printf("-----------------------------------------------------------------\n");
        for (int y = 0; y < DEBUG_P_SLICE_H; y++) {
          T *D = &job->video_data[c]->data[((DEBUG_P_SLICE_Y*DEBUG_P_SLICE_H + y + job->pad_t)*job->video_data[c]->stride + DEBUG_P_BLOCK_X*DEBUG_P_SLICE_W)];
          for (int x = 0; x < DEBUG_P_SLICE_W; x++)
            printf("  %+6d", D[x]);
          printf("\n");
        }
        printf("-----------------------------------------------------------------\n");
      }
#endif /*DEBUG_P_BLOCK*/
    }

#ifdef DEBUG_OP_TRANSFORMED
    {
      std::stringstream ss;
      ss << "transformed_dump_" << job->job << ".raw";
      int f = open(ss.str().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00777);
      for (int c = 0; c < 3; c++) {
        for (int y = 0; y < job->video_data[c]->height; y++) {
          ssize_t s = write(f, (char *)&job->video_data[c]->data[y*job->video_data[c]->stride], job->video_data[c]->width*sizeof(int16_t));
          if (s < 0) {
            throw std::runtime_error("File Writing Error");
          }
        }
      }
    }
#endif /* DEBUG_OP_TRANSFORMED */
  }
}

template<> void VC2Encoder::Encode(CodedSlice<int16_t> *slices, int n_slices, int olength) {
  slice_encoder_func16(slices, n_slices, mQuantisationMatrices, olength, mParams.transform_params.wavelet_index, mSliceSizeScalar, mParams.transform_params.slice_width, mParams.transform_params.slice_height, mDepth);
}

template<> void VC2Encoder::Encode(CodedSlice<int32_t> *slices, int n_slices, int olength) {
  slice_encoder_func32(slices, n_slices, mQuantisationMatrices, olength, mParams.transform_params.wavelet_index, mSliceSizeScalar, mParams.transform_params.slice_width, mParams.transform_params.slice_height, mDepth);
}

template<class T> void VC2Encoder::Serialise(CodedSlice<T> *slices, int n_slices, char *odata, int length, int n_samples) {
  serialise_slices<T>(slices, n_slices, odata, length, n_samples, mSliceSizeScalar);
}
