/*****************************************************************************
 * quantiserselection.hpp : Quantiser Selection
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

#ifndef __QUANTISER_SELECTION_HPP__
#define __QUANTISER_SELECTION_HPP__

#include "logger.hpp"
#include <string.h>

template<class T> inline uint8_t coded_length_for_sample(T *input, int n, const uint16_t m, const uint8_t sh, int shift, int &samples) {
  int32_t x = (*input);
  uint32_t d = udiv<uint16_t>(abs(x), m, sh) >> shift;
  int l = WLLUT[d];
  if (x != 0)
    samples    = (samples < n)?n:samples;

  return l;
}

template<class T> inline void coded_length_for_slice_fallback(CodedSlice<T> *slice, int qi, QuantisationMatrices *matrices, int slice_size_scalar, int *lengths, int W, int h, int d) {
  const int qindex = (qi <= 31)?(qi):(28 + (qi%4));
  const uint8_t qshift = (qi <= 31)?(0):((qi/4) - 7);

  for (int c = 0; c < 3; c++) {
    const int w = (c==0)?W:W/2;
    const int istride = slice->istride[c];
    const int SAMPLES_PER_SLICE = w*h;

    const uint16_t *m = matrices->m(qindex,  c);
    const uint8_t *sh = matrices->sh(qindex, c);

    int length = 0;
    int samples = -1;

    int skip = 1 << d;
    int n = 0;
    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < w; x += skip) {
        length += coded_length_for_sample<T>(&slice->idata[c][(y + 0*skip/2)*istride + (x + 0*skip/2)], n, m[0], sh[0], qshift, samples);
        n++;
      }
    }

    for (int l = 0; l < d; l++) {
      for (int y = 0; y < h; y += skip) {
        for (int x = 0; x < w; x += skip) {
          length += coded_length_for_sample<T>(&slice->idata[c][(y + 0*skip/2)*istride + (x + 1*skip/2)], n, m[(0*skip/2)*w + (1*skip/2)], sh[(0*skip/2)*w + (1*skip/2)], qshift, samples);
          n++;
        }
      }

      for (int y = 0; y < h; y += skip) {
        for (int x = 0; x < w; x += skip) {
          length += coded_length_for_sample<T>(&slice->idata[c][(y + 1*skip/2)*istride + (x + 0*skip/2)], n, m[(1*skip/2)*w + (0*skip/2)], sh[(1*skip/2)*w + (0*skip/2)], qshift, samples);
          n++;
        }
      }

      for (int y = 0; y < h; y += skip) {
        for (int x = 0; x < w; x += skip) {
          length += coded_length_for_sample<T>(&slice->idata[c][(y + 1*skip/2)*istride + (x + 1*skip/2)], n, m[(1*skip/2)*w + (1*skip/2)], sh[(1*skip/2)*w + (1*skip/2)], qshift, samples);
          n++;
        }
      }
      skip /= 2;
    }

    length -= (SAMPLES_PER_SLICE - 1) - samples;
    lengths[c] = ((length + 7)/8 + slice_size_scalar - 1)/slice_size_scalar*slice_size_scalar;
  }
}

template<int W, int h, int d, class T> inline void coded_length_for_slice(CodedSlice<T> *slice, int qi, QuantisationMatrices *matrices, int slice_size_scalar, int *lengths);

template<int W, int h, int d, class T> inline void coded_length_for_slice(CodedSlice<T> *slice, int qi, QuantisationMatrices *matrices, int slice_size_scalar, int *lengths) {

  const int qindex = (qi <= 31)?(qi):(28 + (qi%4));
  const uint8_t qshift = (qi <= 31)?(0):((qi/4) - 7);

  for (int c = 0; c < 3; c++) {
    const int w = (c==0)?W:W/2;
    const int istride = slice->istride[c];
    const int SAMPLES_PER_SLICE = w*h;

    const uint16_t *m = matrices->m(qindex,  c);
    const uint8_t *sh = matrices->sh(qindex, c);

    int length = 0;
    int samples = -1;

    int skip = 1 << d;
    int n = 0;
    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < w; x += skip) {
        length += coded_length_for_sample<T>(&slice->idata[c][(y + 0*skip/2)*istride + (x + 0*skip/2)], n, m[0], sh[0], qshift, samples);
        n++;
      }
    }

    for (int l = 0; l < d; l++) {
      for (int y = 0; y < h; y += skip) {
        for (int x = 0; x < w; x += skip) {
          length += coded_length_for_sample<T>(&slice->idata[c][(y + 0*skip/2)*istride + (x + 1*skip/2)], n, m[(0*skip/2)*w + (1*skip/2)], sh[(0*skip/2)*w + (1*skip/2)], qshift, samples);
          n++;
        }
      }

      for (int y = 0; y < h; y += skip) {
        for (int x = 0; x < w; x += skip) {
          length += coded_length_for_sample<T>(&slice->idata[c][(y + 1*skip/2)*istride + (x + 0*skip/2)], n, m[(1*skip/2)*w + (0*skip/2)], sh[(1*skip/2)*w + (0*skip/2)], qshift, samples);
          n++;
        }
      }

      for (int y = 0; y < h; y += skip) {
        for (int x = 0; x < w; x += skip) {
          length += coded_length_for_sample<T>(&slice->idata[c][(y + 1*skip/2)*istride + (x + 1*skip/2)], n, m[(1*skip/2)*w + (1*skip/2)], sh[(1*skip/2)*w + (1*skip/2)], qshift, samples);
          n++;
        }
      }
      skip /= 2;
    }

    length -= (SAMPLES_PER_SLICE - 1) - samples;
    lengths[c] = ((length + 7)/8 + slice_size_scalar - 1)/slice_size_scalar*slice_size_scalar;
  }
}

template<> inline void coded_length_for_slice<32,8,3, int16_t>(CodedSlice<int16_t> *slice, int qi, QuantisationMatrices *matrices, int slice_size_scalar, int *lengths) {

  const int qindex = (qi <= 31)?(qi):(28 + (qi%4));
  const uint8_t qshift = (qi <= 31)?(0):((qi/4) - 7);

  {
    const int c = 0;
    const int w = 32;
    const int h = 8;
    const int istride = slice->istride[c];
    const int SAMPLES_PER_SLICE = w*h;

    const uint16_t *m = matrices->m(qindex,  c);
    const uint8_t *sh = matrices->sh(qindex, c);

    int length = 0;
    int samples = -1;

    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  0],   0, m[  0], sh[  0], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  1],  64, m[  1], sh[  1], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  2],  16, m[  2], sh[  2], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  3],  65, m[  3], sh[  3], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  4],   4, m[  4], sh[  4], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  5],  66, m[  5], sh[  5], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  6],  17, m[  6], sh[  6], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  7],  67, m[  7], sh[  7], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  8],   1, m[  8], sh[  8], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  9],  68, m[  9], sh[  9], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 10],  18, m[ 10], sh[ 10], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 11],  69, m[ 11], sh[ 11], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 12],   5, m[ 12], sh[ 12], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 13],  70, m[ 13], sh[ 13], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 14],  19, m[ 14], sh[ 14], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 15],  71, m[ 15], sh[ 15], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 16],   2, m[ 16], sh[ 16], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 17],  72, m[ 17], sh[ 17], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 18],  20, m[ 18], sh[ 18], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 19],  73, m[ 19], sh[ 19], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 20],   6, m[ 20], sh[ 20], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 21],  74, m[ 21], sh[ 21], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 22],  21, m[ 22], sh[ 22], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 23],  75, m[ 23], sh[ 23], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 24],   3, m[ 24], sh[ 24], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 25],  76, m[ 25], sh[ 25], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 26],  22, m[ 26], sh[ 26], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 27],  77, m[ 27], sh[ 27], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 28],   7, m[ 28], sh[ 28], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 29],  78, m[ 29], sh[ 29], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 30],  23, m[ 30], sh[ 30], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 31],  79, m[ 31], sh[ 31], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  0], 128, m[ 32], sh[ 32], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  1], 192, m[ 33], sh[ 33], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  2], 129, m[ 34], sh[ 34], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  3], 193, m[ 35], sh[ 35], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  4], 130, m[ 36], sh[ 36], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  5], 194, m[ 37], sh[ 37], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  6], 131, m[ 38], sh[ 38], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  7], 195, m[ 39], sh[ 39], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  8], 132, m[ 40], sh[ 40], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  9], 196, m[ 41], sh[ 41], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 10], 133, m[ 42], sh[ 42], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 11], 197, m[ 43], sh[ 43], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 12], 134, m[ 44], sh[ 44], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 13], 198, m[ 45], sh[ 45], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 14], 135, m[ 46], sh[ 46], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 15], 199, m[ 47], sh[ 47], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 16], 136, m[ 48], sh[ 48], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 17], 200, m[ 49], sh[ 49], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 18], 137, m[ 50], sh[ 50], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 19], 201, m[ 51], sh[ 51], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 20], 138, m[ 52], sh[ 52], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 21], 202, m[ 53], sh[ 53], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 22], 139, m[ 54], sh[ 54], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 23], 203, m[ 55], sh[ 55], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 24], 140, m[ 56], sh[ 56], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 25], 204, m[ 57], sh[ 57], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 26], 141, m[ 58], sh[ 58], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 27], 205, m[ 59], sh[ 59], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 28], 142, m[ 60], sh[ 60], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 29], 206, m[ 61], sh[ 61], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 30], 143, m[ 62], sh[ 62], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 31], 207, m[ 63], sh[ 63], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  0],  32, m[ 64], sh[ 64], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  1],  80, m[ 65], sh[ 65], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  2],  48, m[ 66], sh[ 66], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  3],  81, m[ 67], sh[ 67], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  4],  33, m[ 68], sh[ 68], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  5],  82, m[ 69], sh[ 69], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  6],  49, m[ 70], sh[ 70], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  7],  83, m[ 71], sh[ 71], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  8],  34, m[ 72], sh[ 72], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  9],  84, m[ 73], sh[ 73], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 10],  50, m[ 74], sh[ 74], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 11],  85, m[ 75], sh[ 75], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 12],  35, m[ 76], sh[ 76], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 13],  86, m[ 77], sh[ 77], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 14],  51, m[ 78], sh[ 78], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 15],  87, m[ 79], sh[ 79], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 16],  36, m[ 80], sh[ 80], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 17],  88, m[ 81], sh[ 81], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 18],  52, m[ 82], sh[ 82], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 19],  89, m[ 83], sh[ 83], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 20],  37, m[ 84], sh[ 84], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 21],  90, m[ 85], sh[ 85], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 22],  53, m[ 86], sh[ 86], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 23],  91, m[ 87], sh[ 87], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 24],  38, m[ 88], sh[ 88], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 25],  92, m[ 89], sh[ 89], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 26],  54, m[ 90], sh[ 90], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 27],  93, m[ 91], sh[ 91], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 28],  39, m[ 92], sh[ 92], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 29],  94, m[ 93], sh[ 93], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 30],  55, m[ 94], sh[ 94], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 31],  95, m[ 95], sh[ 95], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  0], 144, m[ 96], sh[ 96], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  1], 208, m[ 97], sh[ 97], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  2], 145, m[ 98], sh[ 98], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  3], 209, m[ 99], sh[ 99], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  4], 146, m[100], sh[100], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  5], 210, m[101], sh[101], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  6], 147, m[102], sh[102], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  7], 211, m[103], sh[103], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  8], 148, m[104], sh[104], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  9], 212, m[105], sh[105], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 10], 149, m[106], sh[106], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 11], 213, m[107], sh[107], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 12], 150, m[108], sh[108], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 13], 214, m[109], sh[109], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 14], 151, m[110], sh[110], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 15], 215, m[111], sh[111], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 16], 152, m[112], sh[112], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 17], 216, m[113], sh[113], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 18], 153, m[114], sh[114], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 19], 217, m[115], sh[115], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 20], 154, m[116], sh[116], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 21], 218, m[117], sh[117], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 22], 155, m[118], sh[118], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 23], 219, m[119], sh[119], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 24], 156, m[120], sh[120], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 25], 220, m[121], sh[121], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 26], 157, m[122], sh[122], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 27], 221, m[123], sh[123], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 28], 158, m[124], sh[124], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 29], 222, m[125], sh[125], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 30], 159, m[126], sh[126], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 31], 223, m[127], sh[127], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  0],   8, m[128], sh[128], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  1],  96, m[129], sh[129], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  2],  24, m[130], sh[130], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  3],  97, m[131], sh[131], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  4],  12, m[132], sh[132], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  5],  98, m[133], sh[133], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  6],  25, m[134], sh[134], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  7],  99, m[135], sh[135], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  8],   9, m[136], sh[136], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  9], 100, m[137], sh[137], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 10],  26, m[138], sh[138], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 11], 101, m[139], sh[139], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 12],  13, m[140], sh[140], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 13], 102, m[141], sh[141], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 14],  27, m[142], sh[142], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 15], 103, m[143], sh[143], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 16],  10, m[144], sh[144], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 17], 104, m[145], sh[145], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 18],  28, m[146], sh[146], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 19], 105, m[147], sh[147], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 20],  14, m[148], sh[148], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 21], 106, m[149], sh[149], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 22],  29, m[150], sh[150], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 23], 107, m[151], sh[151], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 24],  11, m[152], sh[152], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 25], 108, m[153], sh[153], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 26],  30, m[154], sh[154], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 27], 109, m[155], sh[155], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 28],  15, m[156], sh[156], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 29], 110, m[157], sh[157], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 30],  31, m[158], sh[158], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 31], 111, m[159], sh[159], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  0], 160, m[160], sh[160], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  1], 224, m[161], sh[161], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  2], 161, m[162], sh[162], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  3], 225, m[163], sh[163], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  4], 162, m[164], sh[164], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  5], 226, m[165], sh[165], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  6], 163, m[166], sh[166], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  7], 227, m[167], sh[167], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  8], 164, m[168], sh[168], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  9], 228, m[169], sh[169], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 10], 165, m[170], sh[170], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 11], 229, m[171], sh[171], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 12], 166, m[172], sh[172], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 13], 230, m[173], sh[173], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 14], 167, m[174], sh[174], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 15], 231, m[175], sh[175], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 16], 168, m[176], sh[176], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 17], 232, m[177], sh[177], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 18], 169, m[178], sh[178], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 19], 233, m[179], sh[179], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 20], 170, m[180], sh[180], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 21], 234, m[181], sh[181], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 22], 171, m[182], sh[182], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 23], 235, m[183], sh[183], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 24], 172, m[184], sh[184], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 25], 236, m[185], sh[185], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 26], 173, m[186], sh[186], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 27], 237, m[187], sh[187], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 28], 174, m[188], sh[188], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 29], 238, m[189], sh[189], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 30], 175, m[190], sh[190], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 31], 239, m[191], sh[191], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  0],  40, m[192], sh[192], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  1], 112, m[193], sh[193], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  2],  56, m[194], sh[194], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  3], 113, m[195], sh[195], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  4],  41, m[196], sh[196], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  5], 114, m[197], sh[197], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  6],  57, m[198], sh[198], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  7], 115, m[199], sh[199], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  8],  42, m[200], sh[200], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  9], 116, m[201], sh[201], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 10],  58, m[202], sh[202], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 11], 117, m[203], sh[203], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 12],  43, m[204], sh[204], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 13], 118, m[205], sh[205], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 14],  59, m[206], sh[206], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 15], 119, m[207], sh[207], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 16],  44, m[208], sh[208], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 17], 120, m[209], sh[209], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 18],  60, m[210], sh[210], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 19], 121, m[211], sh[211], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 20],  45, m[212], sh[212], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 21], 122, m[213], sh[213], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 22],  61, m[214], sh[214], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 23], 123, m[215], sh[215], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 24],  46, m[216], sh[216], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 25], 124, m[217], sh[217], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 26],  62, m[218], sh[218], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 27], 125, m[219], sh[219], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 28],  47, m[220], sh[220], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 29], 126, m[221], sh[221], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 30],  63, m[222], sh[222], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 31], 127, m[223], sh[223], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  0], 176, m[224], sh[224], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  1], 240, m[225], sh[225], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  2], 177, m[226], sh[226], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  3], 241, m[227], sh[227], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  4], 178, m[228], sh[228], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  5], 242, m[229], sh[229], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  6], 179, m[230], sh[230], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  7], 243, m[231], sh[231], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  8], 180, m[232], sh[232], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  9], 244, m[233], sh[233], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 10], 181, m[234], sh[234], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 11], 245, m[235], sh[235], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 12], 182, m[236], sh[236], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 13], 246, m[237], sh[237], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 14], 183, m[238], sh[238], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 15], 247, m[239], sh[239], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 16], 184, m[240], sh[240], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 17], 248, m[241], sh[241], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 18], 185, m[242], sh[242], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 19], 249, m[243], sh[243], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 20], 186, m[244], sh[244], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 21], 250, m[245], sh[245], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 22], 187, m[246], sh[246], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 23], 251, m[247], sh[247], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 24], 188, m[248], sh[248], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 25], 252, m[249], sh[249], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 26], 189, m[250], sh[250], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 27], 253, m[251], sh[251], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 28], 190, m[252], sh[252], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 29], 254, m[253], sh[253], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 30], 191, m[254], sh[254], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 31], 255, m[255], sh[255], qshift, samples);

    length -= (SAMPLES_PER_SLICE - 1) - samples;
    lengths[c] = ((length + 7)/8 + slice_size_scalar - 1)/slice_size_scalar*slice_size_scalar;
  }

  for (int c = 1; c < 3; c++) {
    const int w = 32/2;
    const int h = 8;
    const int istride = slice->istride[c];
    const int SAMPLES_PER_SLICE = w*h;

    const uint16_t *m = matrices->m(qindex,  c);
    const uint8_t *sh = matrices->sh(qindex, c);

    int length = 0;
    int samples = -1;

    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  0],   0, m[  0], sh[  0], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  1],  32, m[  1], sh[  1], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  2],   8, m[  2], sh[  2], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  3],  33, m[  3], sh[  3], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  4],   2, m[  4], sh[  4], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  5],  34, m[  5], sh[  5], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  6],   9, m[  6], sh[  6], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  7],  35, m[  7], sh[  7], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  8],   1, m[  8], sh[  8], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride +  9],  36, m[  9], sh[  9], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 10],  10, m[ 10], sh[ 10], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 11],  37, m[ 11], sh[ 11], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 12],   3, m[ 12], sh[ 12], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 13],  38, m[ 13], sh[ 13], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 14],  11, m[ 14], sh[ 14], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 0*istride + 15],  39, m[ 15], sh[ 15], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  0],  64, m[ 16], sh[ 16], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  1],  96, m[ 17], sh[ 17], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  2],  65, m[ 18], sh[ 18], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  3],  97, m[ 19], sh[ 19], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  4],  66, m[ 20], sh[ 20], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  5],  98, m[ 21], sh[ 21], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  6],  67, m[ 22], sh[ 22], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  7],  99, m[ 23], sh[ 23], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  8],  68, m[ 24], sh[ 24], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride +  9], 100, m[ 25], sh[ 25], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 10],  69, m[ 26], sh[ 26], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 11], 101, m[ 27], sh[ 27], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 12],  70, m[ 28], sh[ 28], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 13], 102, m[ 29], sh[ 29], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 14],  71, m[ 30], sh[ 30], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 1*istride + 15], 103, m[ 31], sh[ 31], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  0],  16, m[ 32], sh[ 32], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  1],  40, m[ 33], sh[ 33], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  2],  24, m[ 34], sh[ 34], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  3],  41, m[ 35], sh[ 35], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  4],  17, m[ 36], sh[ 36], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  5],  42, m[ 37], sh[ 37], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  6],  25, m[ 38], sh[ 38], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  7],  43, m[ 39], sh[ 39], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  8],  18, m[ 40], sh[ 40], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride +  9],  44, m[ 41], sh[ 41], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 10],  26, m[ 42], sh[ 42], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 11],  45, m[ 43], sh[ 43], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 12],  19, m[ 44], sh[ 44], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 13],  46, m[ 45], sh[ 45], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 14],  27, m[ 46], sh[ 46], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 2*istride + 15],  47, m[ 47], sh[ 47], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  0],  72, m[ 48], sh[ 48], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  1], 104, m[ 49], sh[ 49], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  2],  73, m[ 50], sh[ 50], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  3], 105, m[ 51], sh[ 51], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  4],  74, m[ 52], sh[ 52], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  5], 106, m[ 53], sh[ 53], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  6],  75, m[ 54], sh[ 54], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  7], 107, m[ 55], sh[ 55], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  8],  76, m[ 56], sh[ 56], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride +  9], 108, m[ 57], sh[ 57], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 10],  77, m[ 58], sh[ 58], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 11], 109, m[ 59], sh[ 59], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 12],  78, m[ 60], sh[ 60], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 13], 110, m[ 61], sh[ 61], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 14],  79, m[ 62], sh[ 62], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 3*istride + 15], 111, m[ 63], sh[ 63], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  0],   4, m[ 64], sh[ 64], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  1],  48, m[ 65], sh[ 65], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  2],  12, m[ 66], sh[ 66], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  3],  49, m[ 67], sh[ 67], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  4],   6, m[ 68], sh[ 68], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  5],  50, m[ 69], sh[ 69], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  6],  13, m[ 70], sh[ 70], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  7],  51, m[ 71], sh[ 71], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  8],   5, m[ 72], sh[ 72], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride +  9],  52, m[ 73], sh[ 73], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 10],  14, m[ 74], sh[ 74], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 11],  53, m[ 75], sh[ 75], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 12],   7, m[ 76], sh[ 76], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 13],  54, m[ 77], sh[ 77], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 14],  15, m[ 78], sh[ 78], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 4*istride + 15],  55, m[ 79], sh[ 79], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  0],  80, m[ 80], sh[ 80], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  1], 112, m[ 81], sh[ 81], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  2],  81, m[ 82], sh[ 82], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  3], 113, m[ 83], sh[ 83], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  4],  82, m[ 84], sh[ 84], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  5], 114, m[ 85], sh[ 85], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  6],  83, m[ 86], sh[ 86], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  7], 115, m[ 87], sh[ 87], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  8],  84, m[ 88], sh[ 88], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride +  9], 116, m[ 89], sh[ 89], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 10],  85, m[ 90], sh[ 90], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 11], 117, m[ 91], sh[ 91], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 12],  86, m[ 92], sh[ 92], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 13], 118, m[ 93], sh[ 93], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 14],  87, m[ 94], sh[ 94], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 5*istride + 15], 119, m[ 95], sh[ 95], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  0],  20, m[ 96], sh[ 96], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  1],  56, m[ 97], sh[ 97], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  2],  28, m[ 98], sh[ 98], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  3],  57, m[ 99], sh[ 99], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  4],  21, m[100], sh[100], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  5],  58, m[101], sh[101], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  6],  29, m[102], sh[102], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  7],  59, m[103], sh[103], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  8],  22, m[104], sh[104], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride +  9],  60, m[105], sh[105], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 10],  30, m[106], sh[106], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 11],  61, m[107], sh[107], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 12],  23, m[108], sh[108], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 13],  62, m[109], sh[109], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 14],  31, m[110], sh[110], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 6*istride + 15],  63, m[111], sh[111], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  0],  88, m[112], sh[112], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  1], 120, m[113], sh[113], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  2],  89, m[114], sh[114], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  3], 121, m[115], sh[115], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  4],  90, m[116], sh[116], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  5], 122, m[117], sh[117], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  6],  91, m[118], sh[118], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  7], 123, m[119], sh[119], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  8],  92, m[120], sh[120], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride +  9], 124, m[121], sh[121], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 10],  93, m[122], sh[122], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 11], 125, m[123], sh[123], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 12],  94, m[124], sh[124], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 13], 126, m[125], sh[125], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 14],  95, m[126], sh[126], qshift, samples);
    length += coded_length_for_sample<int16_t>(&slice->idata[c][ 7*istride + 15], 127, m[127], sh[127], qshift, samples);

    length -= (SAMPLES_PER_SLICE - 1) - samples;
    lengths[c] = ((length + 7)/8 + slice_size_scalar - 1)/slice_size_scalar*slice_size_scalar;
  }
}

template<int w, int h, int depth, int QUAL, class T> inline void choose_quantiser(CodedSlice<T> *slice, int max_size, int wavelet_index, int slice_size_scalar, QuantisationMatrices *matrices);

template<int w, int h, int depth, int QUAL, class T> inline void choose_quantiser(CodedSlice<T> *slice, int max_size, int wavelet_index, int slice_size_scalar, QuantisationMatrices *matrices) {
#ifdef FIXED_QI
  (void)slice;
  (void)max_size;
  (void)wavelet_index;
  (void)slice_size_scalar;
  (void)matrices;
  slice->qindex = FIXED_QI;
#else

  const int QI_FIRST_INC = (QUAL == QUANTISER_SELECTION_EIGHTHSEARCH)?8:4;
  const int DESIRED_PROXIMITY = ((QUAL == QUANTISER_SELECTION_EIGHTHSEARCH)?8:
                                 ((QUAL == QUANTISER_SELECTION_QUARTERSEARCH)?4:
                                  ((QUAL == QUANTISER_SELECTION_HALFSEARCH)?2:1)));


  QuantisationWeightingMatrix matrix = preset_quantisation_matrices[wavelet_index][depth];
  uint32_t L[3][depth][4], A[3][depth][4];
  int qi_bl = 0;

  memset((char *)L, 0, sizeof(L));

  for (int c = 0; c < 3; c++) {
    int skip = 1 << depth;
    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < ((c==0)?w:w/2); x += skip) {
        L[c][0][0] |= abs(slice->idata[c][y*slice->istride[c] + x]);
      }
    }

    for (int l = 0; l < depth; l++) {
      for (int y = 0; y < h; y += skip) {
        for (int x = skip/2; x < ((c==0)?w:w/2); x += skip) {
          L[c][l][1] |= abs(slice->idata[c][y*slice->istride[c] + x]);
        }
      }

      for (int y = skip/2; y < h; y += skip) {
        for (int x = 0; x < ((c==0)?w:w/2); x += skip) {
          L[c][l][2] |= abs(slice->idata[c][y*slice->istride[c] + x]);
        }
      }
      A[c][l][2] /= (((c==0)?w:w/2)/skip)*(h/skip);

      for (int y = skip/2; y < h; y += skip) {
        for (int x = skip/2; x < ((c==0)?w:w/2); x += skip) {
          L[c][l][3] |= abs(slice->idata[c][y*slice->istride[c] + x]);
        }
      }

      skip /= 2;
    }

    qi_bl = max(qi_bl, 4*(24 - __builtin_clz(L[c][0][0] + 1)) + matrix.LL);
    for (int l = 0; l < depth; l++) {
      qi_bl = max(qi_bl, 4*(24 - __builtin_clz(L[c][l][1] + 1)) + matrix.HL[l]);
      qi_bl = max(qi_bl, 4*(24 - __builtin_clz(L[c][l][2] + 1)) + matrix.LH[l]);
      qi_bl = max(qi_bl, 4*(24 - __builtin_clz(L[c][l][3] + 1)) + matrix.HH[l]);
    }
  }

  int qi_cur  = max(qi_bl, slice->qindex) - QI_FIRST_INC;
  int length;
  int count = 0;
  {
    //qi_cur += QI_FIRST_INC;
    int qi_floor = qi_bl;
    int qi_ceil  = MAX_QI;
    int lengths[3];
    int inc = QI_FIRST_INC;

    do {
      qi_floor = qi_cur;
      qi_cur += inc;
      //      inc *= 2;
      qi_ceil = qi_cur;
      coded_length_for_slice<w,h,depth, T>(slice, qi_cur, matrices, slice_size_scalar, lengths);
      length = 4 + lengths[0] + lengths[1] + lengths[2];
      count++;
    } while(qi_cur < MAX_QI &&
            (lengths[0]/slice_size_scalar > 255 ||
             lengths[1]/slice_size_scalar > 255 ||
             lengths[2]/slice_size_scalar > 255 ||
             length > max_size));

    qi_cur = (qi_ceil + qi_floor + 1)/2;

    qi_floor = max(qi_floor, qi_bl);

    if (QUAL != QUANTISER_SELECTION_EIGHTHSEARCH && QUAL != QUANTISER_SELECTION_QUARTERSEARCH) {
      while(qi_ceil - qi_floor > DESIRED_PROXIMITY) {
        coded_length_for_slice<w,h,depth, T>(slice, qi_cur, matrices, slice_size_scalar, lengths);
        length = 4 + lengths[0] + lengths[1] + lengths[2];
        count++;

        if (lengths[0]/slice_size_scalar > 255 ||
            lengths[1]/slice_size_scalar > 255 ||
            lengths[2]/slice_size_scalar > 255 ||
            length > max_size) {
          qi_floor = qi_cur;
        } else {
          qi_ceil = qi_cur;
        }
        qi_cur = (qi_ceil + qi_floor + 1)/2;
      }
    }

    qi_cur = max(qi_bl, qi_ceil);
  }

  (void)count;

  slice->qindex = min(max(qi_cur, MIN_QI), MAX_QI);
#endif
}

template<class T> inline void choose_quantiser_fallback(CodedSlice<T> *slice, int max_size, int wavelet_index, int slice_size_scalar, QuantisationMatrices *matrices, int w, int h, int depth) {
#ifdef FIXED_QI
  (void)slice;
  (void)max_size;
  (void)wavelet_index;
  (void)slice_size_scalar;
  (void)matrices;
  (void)w;
  (void)h;
  (void)depth;
  slice->qindex = FIXED_QI;
#else

  const int QI_FIRST_INC = 4;
  const int DESIRED_PROXIMITY = 1;

  QuantisationWeightingMatrix matrix = preset_quantisation_matrices[wavelet_index][depth];
  uint32_t L[3][depth][4], A[3][depth][4];
  int qi_bl = 0;

  memset((char *)L, 0, sizeof(L));

  for (int c = 0; c < 3; c++) {
    int skip = 1 << depth;
    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < ((c==0)?w:w/2); x += skip) {
        L[c][0][0] |= abs(slice->idata[c][y*slice->istride[c] + x]);
      }
    }

    for (int l = 0; l < depth; l++) {
      for (int y = 0; y < h; y += skip) {
        for (int x = skip/2; x < ((c==0)?w:w/2); x += skip) {
          L[c][l][1] |= abs(slice->idata[c][y*slice->istride[c] + x]);
        }
      }

      for (int y = skip/2; y < h; y += skip) {
        for (int x = 0; x < ((c==0)?w:w/2); x += skip) {
          L[c][l][2] |= abs(slice->idata[c][y*slice->istride[c] + x]);
        }
      }
      A[c][l][2] /= (((c==0)?w:w/2)/skip)*(h/skip);

      for (int y = skip/2; y < h; y += skip) {
        for (int x = skip/2; x < ((c==0)?w:w/2); x += skip) {
          L[c][l][3] |= abs(slice->idata[c][y*slice->istride[c] + x]);
        }
      }

      skip /= 2;
    }

    qi_bl = max(qi_bl, 4*(24 - __builtin_clz(L[c][0][0] + 1)) + matrix.LL);
    for (int l = 0; l < depth; l++) {
      qi_bl = max(qi_bl, 4*(24 - __builtin_clz(L[c][l][1] + 1)) + matrix.HL[l]);
      qi_bl = max(qi_bl, 4*(24 - __builtin_clz(L[c][l][2] + 1)) + matrix.LH[l]);
      qi_bl = max(qi_bl, 4*(24 - __builtin_clz(L[c][l][3] + 1)) + matrix.HH[l]);
    }
  }

  int qi_cur  = max(qi_bl, slice->qindex) - QI_FIRST_INC;
  int length;
  int count = 0;
  {
    //qi_cur += QI_FIRST_INC;
    int qi_floor = qi_bl;
    int qi_ceil  = MAX_QI;
    int lengths[3];
    int inc = QI_FIRST_INC;

    do {
      qi_floor = qi_cur;
      qi_cur += inc;
      //      inc *= 2;
      qi_ceil = qi_cur;
      coded_length_for_slice_fallback<T>(slice, qi_cur, matrices, slice_size_scalar, lengths, w,h,depth);
      length = 4 + lengths[0] + lengths[1] + lengths[2];
      count++;
    } while(qi_cur < MAX_QI &&
            (lengths[0]/slice_size_scalar > 255 ||
             lengths[1]/slice_size_scalar > 255 ||
             lengths[2]/slice_size_scalar > 255 ||
             length > max_size));

    qi_cur = (qi_ceil + qi_floor + 1)/2;

    qi_floor = max(qi_floor, qi_bl);

    while(qi_ceil - qi_floor > DESIRED_PROXIMITY) {
      coded_length_for_slice_fallback<T>(slice, qi_cur, matrices, slice_size_scalar, lengths, w,h,depth);
      length = 4 + lengths[0] + lengths[1] + lengths[2];
      count++;

      if (lengths[0]/slice_size_scalar > 255 ||
          lengths[1]/slice_size_scalar > 255 ||
          lengths[2]/slice_size_scalar > 255 ||
          length > max_size) {
        qi_floor = qi_cur;
      } else {
        qi_ceil = qi_cur;
      }
      qi_cur = (qi_ceil + qi_floor + 1)/2;
    }

    qi_cur = max(qi_bl, qi_ceil);
  }

  (void)count;

  slice->qindex = min(max(qi_cur, MIN_QI), MAX_QI);
#endif
}

#endif /* __QUANTISER_SELECTION_HPP__ */
