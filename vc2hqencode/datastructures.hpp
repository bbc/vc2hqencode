/*****************************************************************************
 * datastructures.hpp : data structures used throughout
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

#ifndef __DATASTRUCTURES_HPP__
#define __DATASTRUCTURES_HPP__

#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

#include <stdio.h>

template<class T> struct CodedSlice {
public:
  uint32_t length[3];
  uint8_t qindex;
  int samples[3];
  T *idata[3];
  int istride[3];
  uint16_t *codewords[3];
  uint8_t  *wordlengths[3];
  int padding;
  int y;
  int x;
};

template<class T> class CodedSlices {
public:
  CodedSlices(int slices_x, int slices_y, int w, int h, int d) {
    int n = slices_x * slices_y;
    n_slices = n;
    slices = new CodedSlice<T>[n];

    mCodeWords   = (uint16_t *)memalign(64, w*h*sizeof(uint16_t)*n*3);
    mWordLengths = (uint8_t  *)memalign(64, w*h*sizeof(uint8_t)*n*3);

    width[0]  = w;
    height[0] = h;

    width[1]  = w/2;
    height[1] = h;

    width[2]  = w/2;
    height[2] = h;
    depth  = d;

    for (int y = 0; y < slices_y; y++) {
      for (int x = 0; x < slices_x; x++) {
        const int i = y*slices_x + x;
        
        slices[i].length[0]      = -1;
        slices[i].length[1]      = -1;
        slices[i].length[2]      = -1;
        slices[i].samples[0]     = -1;
        slices[i].samples[1]     = -1;
        slices[i].samples[2]     = -1;
        slices[i].qindex         = 0;
        slices[i].codewords[0]   = &mCodeWords[w*h*(3*i + 0)];
        slices[i].wordlengths[0] = &mWordLengths[w*h*(3*i + 0)];
        slices[i].codewords[1]   = &mCodeWords[w*h*(3*i + 1)];
        slices[i].wordlengths[1] = &mWordLengths[w*h*(3*i + 1)];
        slices[i].codewords[2]   = &mCodeWords[w*h*(3*i + 2)];
        slices[i].wordlengths[2] = &mWordLengths[w*h*(3*i + 2)];
        slices[i].y = y;
        slices[i].x = x;
      }
    }
  }

  ~CodedSlices() {
    delete[] slices;
    free(mCodeWords);
    free(mWordLengths);
  }

  int width[3];
  int height[3];
  int depth;
  
  CodedSlice<T> *slices;
  int n_slices;

private:
  uint16_t *mCodeWords;
  uint8_t  *mWordLengths;
};

template<class T> struct VideoPlane {
  VideoPlane(int w, int h) {
    width  = w;
    height = h;
    stride = (((w + 15)/16)*16); // Integer number of cache lines
    data = (T *)memalign(64, stride*(height + 1)*sizeof(T)); // there's some padding here to allow overspill in V210 unpack
  }

  ~VideoPlane() {
    free(data);
  }

  T *data;
  int stride;
  int height;
  int width;
};

class JobBase {
public:
  char *idata[3];
  int   istride[3];
  int   width;
  int   height;
  int   depth;
  int   offset_x;
  int   offset_y;

  int   pad_t;
  int   pad_b;

  int   iwidth[3];
  int   iheight[3];

  char *odata;
  int   olength;

  int opadding;

  int job;

  JobBase(int j,
          int w, int h, int pt, int pb, int d, int off_x, int off_y)
    : width(w)
    , height(h)
    , depth(d)
    , offset_x(off_x)
    , offset_y(off_y)
    , pad_t(pt)
    , pad_b(pb)
    , odata (NULL)
    , olength (0)
    , opadding (0)
    , job(j) {
    idata[0] = NULL;
    idata[1] = NULL;
    idata[2] = NULL;

    istride[0] = 0;
    istride[1] = 0;
    istride[2] = 0;

    iwidth[0] = w;
    iwidth[1] = iwidth[2] = iwidth[0]/2;
    iheight[0] = iheight[1] = iheight[2] = pad_t + h + pad_b;
  }

  virtual ~JobBase() {};
};

template<class T> class JobData : public JobBase {
public:
  JobData(int j,
          int w, int h, int pt, int pb, int d, int sx, int sy, int off_x, int off_y,
          CodedSlices<T> *coded_slices, int slice_offset_x, int slice_offset_y, int slice_stride)
    : JobBase(j, w, h, pt, pb, d, off_x, off_y) {

    int W = sx*coded_slices->width[0];
    int H = sy*coded_slices->height[0];
    video_data[0] = new VideoPlane<T>(W,     pt + H + pb);
    video_data[1] = new VideoPlane<T>((W)/2, pt + H + pb);
    video_data[2] = new VideoPlane<T>((W)/2, pt + H + pb);

    int is[3] = { video_data[0]->stride, video_data[1]->stride, video_data[2]->stride };
    T *id[3] = { &video_data[0]->data[pt*is[0]],
                 &video_data[1]->data[pt*is[1]],
                 &video_data[2]->data[pt*is[2]] };
    for (int Y = 0; Y < sy; Y++) {
      for (int X = 0; X < sx; X++) {
        CodedSlice<T> *slice = &coded_slices->slices[(slice_offset_y + Y)*slice_stride + (slice_offset_x + X)];
        slice->istride[0]     = is[0];
        slice->istride[1]     = is[1];
        slice->istride[2]     = is[2];
        slice->idata[0]       = &id[0][Y*(H/sy)*is[0] + X*(W/sx)];
        slice->idata[1]       = &id[1][Y*(H/sy)*is[1] + X*(W/sx/2)];
        slice->idata[2]       = &id[2][Y*(H/sy)*is[2] + X*(W/sx/2)];
      }
    }
  }

  virtual ~JobData() {
    delete video_data[0];
    delete video_data[1];
    delete video_data[2];
  }

  VideoPlane<T> *video_data[3];
};

#endif /* __DATASTRUCTURES_HPP__ */
