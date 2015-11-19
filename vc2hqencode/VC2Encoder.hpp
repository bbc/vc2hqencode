/*****************************************************************************
 * VC2Encoder.hpp : Main header file for VC2Encoder class
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

#ifndef __VC2ENCODER_HPP__
#define __VC2ENCODER_HPP__

#include <atomic>

#include "vc2hqencode.h"
#include "VideoFormat.hpp"
#include "encode.hpp"
#include "serialise.hpp"
#include "quantise.hpp"
#include "transform.hpp"

#include "ThreadPool.hpp"

#include "datastructures.hpp"
#include "stream.hpp"

void detect_cpu_features();

class VC2Encoder {
public:
  VC2Encoder()
    : mJobsInFlight(0)
    , mThreads(1)
    , mCoefSize (2) {
    mVideoFormat = vc2::preset_formats[VC2ENCODER_BVF_HD1080P_50];
    mPool = NULL;
    mJobData = NULL;
    mSlices16 = NULL;
    mSlices32 = NULL;
    mQuantisationMatrices = NULL;
    mSequenceHeader = NULL;
    mPictureHeader = NULL;

    transform_initial[0] = NULL;
    transform_initial[1] = NULL;
    transform_initial[2] = NULL;
    transforms_h = NULL;
    transforms_v = NULL;
    slice_encoder_func16 = NULL;
    slice_encoder_func32 = NULL;

    mSliceSizeScalar = 2;
    mInterlaced = false;
  }
  virtual ~VC2Encoder() {
    if (mPool) {
      mPool->stop();
      delete mPool;
    }
    if (mJobData) {
      for (int i = 0; i < mJobs; i++)
        if (mJobData[i])
          delete mJobData[i];
      delete[] mJobData;
    }
    if (mSlices16)
      delete mSlices16;
    if (mSlices32)
      delete mSlices32;
    if (mQuantisationMatrices)
      delete mQuantisationMatrices;
  }

  void setParams(VC2EncoderParams &params) throw(VC2EncoderResult);

  VC2EncoderParams getParams() {return mParams;}
  uint32_t getParseInfoSize() { return 13; }
  uint32_t getSequenceHeaderSize() { return mSequenceHeader->length - 13; }
  uint32_t getPictureHeaderSize() { return 4; }
  uint32_t getTransformParametersSize() { return mPictureHeader->length - 17; }

  uint32_t startSequence(char **data);
  uint32_t repeatSequenceStart(char **data, uint32_t prev_parse_offset);
  uint32_t startPicture(char **data, uint32_t prev_parse_offset, uint32_t picture_number, int data_length);
  bool encodeData(char **idata, int *istride, char **odata, int length);
  uint32_t startAuxiliaryData(char **data, uint32_t prev_parse_offset, int data_length);
  void endSequence(char **data, uint32_t prev_parse_offset);

  int getCallCount() { return 0; }

protected:
  template<class T> void EncodePartial(CodedSlice<T> *slices, int n_slices, char *odata, int olength, int N, int n_samples);

  template<class T> void Transform(JobBase *);
  template<class T> void Encode(CodedSlice<T> *slices, int n_slices, int olength);
  template<class T> void Serialise(CodedSlice<T> *slices, int n_slices, char *odata, int length, int n_samples);

  VC2EncoderParams mParams;
  vc2::VideoFormat mVideoFormat;

  int mLumaWidth;
  int mLumaHeight;
  int mPaddedHeight;
  int mPaddedWidth;
  int mColorDiffWidth;
  int mColorDiffHeight;

  bool mInterlaced;

  int mSlicesPerLine;
  int mSlicesPerPicture;

  int mBlockOverlap;

  int mDepth;

  std::atomic<uint32_t> mJobsInFlight;

  ThreadPool *mPool;
  int mThreads;
  int mJobs;
  
  JobBase **mJobData;
  CodedSlices<int16_t> *mSlices16;
  CodedSlices<int32_t> *mSlices32;
  QuantisationMatrices *mQuantisationMatrices;

  SequenceHeader *mSequenceHeader;
  PictureHeader *mPictureHeader;

  InplaceTransformInitial transform_initial[3];
  InplaceTransform *transforms_h;
  InplaceTransform *transforms_v;

  SliceEncoderFunc16 slice_encoder_func16;
  SliceEncoderFunc32 slice_encoder_func32;

  int mSliceSizeScalar;

  int mBytesPerSlice;
  uint32_t mPictureNumber;

  int mCoefSize;
};

#endif /* __VC2ENCODER_HPP__ */
