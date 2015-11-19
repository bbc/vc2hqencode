/*****************************************************************************
 * encode_slice_component_optimised.hpp : Optimised versions of encoding
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

template<int w, int h, int d, class T> inline void encode_slice_component(CodedSlice<T> *slice, int c, QuantisationMatrices *matrices);

// This is a super-optimised version used only for transformed coefficients. It combines quantisation and encoding
template<class T> inline uint8_t encode_sample(T *input, uint16_t *output, uint8_t *lengthout, int n, const uint16_t m, const uint8_t sh, uint8_t shift, int &samples) {
  int32_t x = (*input);
  uint32_t d = udiv<uint16_t>(abs(x), m, sh) >> shift;

  int s = (x >> 31)&0x1;
  int l = WLLUT[d];

  *output    = (CWLUT[d] | s);
  *lengthout = l;
  if (x != 0)
    samples    = (samples < n)?n:samples;

  return l;
}

template<class T> inline void encode_slice_component_fallback(CodedSlice<T> *slice, int c, QuantisationMatrices *matrices, int w, int h, int d) {
  const int SAMPLES_PER_SLICE = w*h;
  int length = 0;
  int samples = -1;

  const int istride = slice->istride[c];

  const int qindex = (slice->qindex <= 31)?(slice->qindex):(28 + (slice->qindex%4));
  const uint8_t qshift = (slice->qindex <= 31)?(0):((slice->qindex/4) - 7);

  const uint16_t *m = matrices->m(qindex,  c);
  const uint8_t *sh = matrices->sh(qindex, c);

  int skip = 1 << d;
  int n = 0;
  for (int y = 0; y < h; y += skip) {
    for (int x = 0; x < w; x += skip) {
      length += encode_sample<T>(&slice->idata[c][(y + 0*skip/2)*istride + (x + 0*skip/2)], &slice->codewords[c][n], &slice->wordlengths[c][n], n, m[0], sh[0], qshift, samples);
      n++;
    }
  }

  for (int l = 0; l < d; l++) {
    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < w; x += skip) {
        length += encode_sample<T>(&slice->idata[c][(y + 0*skip/2)*istride + (x + 1*skip/2)], &slice->codewords[c][n], &slice->wordlengths[c][n], n, m[(0*skip/2)*w + (1*skip/2)], sh[(0*skip/2)*w + (1*skip/2)], qshift, samples);
        n++;
      }
    }

    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < w; x += skip) {
        length += encode_sample<T>(&slice->idata[c][(y + 1*skip/2)*istride + (x + 0*skip/2)], &slice->codewords[c][n], &slice->wordlengths[c][n], n, m[(1*skip/2)*w + (0*skip/2)], sh[(1*skip/2)*w + (0*skip/2)], qshift, samples);
        n++;
      }
    }

    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < w; x += skip) {
        length += encode_sample<T>(&slice->idata[c][(y + 1*skip/2)*istride + (x + 1*skip/2)], &slice->codewords[c][n], &slice->wordlengths[c][n], n, m[(1*skip/2)*w + (1*skip/2)], sh[(1*skip/2)*w + (1*skip/2)], qshift, samples);
        n++;
      }
    }
    skip /= 2;
  }

  length -= (SAMPLES_PER_SLICE - 1) - samples;
  slice->length[c] = (length + 7)/8;
  slice->samples[c] = samples + 1;
}

template<int w, int h, int d, class T> inline void encode_slice_component(CodedSlice<T> *slice, int c, QuantisationMatrices *matrices) {
  const int SAMPLES_PER_SLICE = w*h;
  int length = 0;
  int samples = -1;

  const int istride = slice->istride[c];

  const int qindex = (slice->qindex <= 31)?(slice->qindex):(28 + (slice->qindex%4));
  const uint8_t qshift = (slice->qindex <= 31)?(0):((slice->qindex/4) - 7);

  const uint16_t *m = matrices->m(qindex,  c);
  const uint8_t *sh = matrices->sh(qindex, c);

  int skip = 1 << d;
  int n = 0;
  for (int y = 0; y < h; y += skip) {
    for (int x = 0; x < w; x += skip) {
      length += encode_sample<T>(&slice->idata[c][(y + 0*skip/2)*istride + (x + 0*skip/2)], &slice->codewords[c][n], &slice->wordlengths[c][n], n, m[0], sh[0], qshift, samples);
      n++;
    }
  }

  for (int l = 0; l < d; l++) {
    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < w; x += skip) {
        length += encode_sample<T>(&slice->idata[c][(y + 0*skip/2)*istride + (x + 1*skip/2)], &slice->codewords[c][n], &slice->wordlengths[c][n], n, m[(0*skip/2)*w + (1*skip/2)], sh[(0*skip/2)*w + (1*skip/2)], qshift, samples);
        n++;
      }
    }

    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < w; x += skip) {
        length += encode_sample<T>(&slice->idata[c][(y + 1*skip/2)*istride + (x + 0*skip/2)], &slice->codewords[c][n], &slice->wordlengths[c][n], n, m[(1*skip/2)*w + (0*skip/2)], sh[(1*skip/2)*w + (0*skip/2)], qshift, samples);
        n++;
      }
    }

    for (int y = 0; y < h; y += skip) {
      for (int x = 0; x < w; x += skip) {
        length += encode_sample<T>(&slice->idata[c][(y + 1*skip/2)*istride + (x + 1*skip/2)], &slice->codewords[c][n], &slice->wordlengths[c][n], n, m[(1*skip/2)*w + (1*skip/2)], sh[(1*skip/2)*w + (1*skip/2)], qshift, samples);
        n++;
      }
    }
    skip /= 2;
  }

  length -= (SAMPLES_PER_SLICE - 1) - samples;
  slice->length[c] = (length + 7)/8;
  slice->samples[c] = samples + 1;
}

template<> inline void encode_slice_component<4,8,2, int16_t>(CodedSlice<int16_t> *slice, int c, QuantisationMatrices *matrices) {
  (void)matrices;
  const int w = 4;
  const int h = 8;
  const int SAMPLES_PER_SLICE = w*h;
  int length = 0;
  int samples = -1;

  const int istride = slice->istride[c];

  const int qindex = (slice->qindex <= 31)?(slice->qindex):(28 + (slice->qindex%4));
  const uint8_t qshift = (slice->qindex <= 31)?(0):((slice->qindex/4) - 7);

  const uint16_t *m = matrices->m(qindex,  c);
  const uint8_t *sh = matrices->sh(qindex, c);

  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  0], &slice->codewords[c][ 0], &slice->wordlengths[c][ 0],  0, m[ 0], sh[ 0], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  1], &slice->codewords[c][ 8], &slice->wordlengths[c][ 8],  8, m[ 1], sh[ 1], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  2], &slice->codewords[c][ 2], &slice->wordlengths[c][ 2],  2, m[ 2], sh[ 2], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  3], &slice->codewords[c][ 9], &slice->wordlengths[c][ 9],  9, m[ 3], sh[ 3], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  0], &slice->codewords[c][16], &slice->wordlengths[c][16], 16, m[ 4], sh[ 4], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  1], &slice->codewords[c][24], &slice->wordlengths[c][24], 24, m[ 5], sh[ 5], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  2], &slice->codewords[c][17], &slice->wordlengths[c][17], 17, m[ 6], sh[ 6], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  3], &slice->codewords[c][25], &slice->wordlengths[c][25], 25, m[ 7], sh[ 7], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  0], &slice->codewords[c][ 4], &slice->wordlengths[c][ 4],  4, m[ 8], sh[ 8], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  1], &slice->codewords[c][10], &slice->wordlengths[c][10], 10, m[ 9], sh[ 9], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  2], &slice->codewords[c][ 6], &slice->wordlengths[c][ 6],  6, m[10], sh[10], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  3], &slice->codewords[c][11], &slice->wordlengths[c][11], 11, m[11], sh[11], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  0], &slice->codewords[c][18], &slice->wordlengths[c][18], 18, m[12], sh[12], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  1], &slice->codewords[c][26], &slice->wordlengths[c][26], 26, m[13], sh[13], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  2], &slice->codewords[c][19], &slice->wordlengths[c][19], 19, m[14], sh[14], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  3], &slice->codewords[c][27], &slice->wordlengths[c][27], 27, m[15], sh[15], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  0], &slice->codewords[c][ 1], &slice->wordlengths[c][ 1],  1, m[16], sh[16], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  1], &slice->codewords[c][12], &slice->wordlengths[c][12], 12, m[17], sh[17], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  2], &slice->codewords[c][ 3], &slice->wordlengths[c][ 3],  3, m[18], sh[18], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  3], &slice->codewords[c][13], &slice->wordlengths[c][13], 13, m[19], sh[19], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  0], &slice->codewords[c][20], &slice->wordlengths[c][20], 20, m[20], sh[20], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  1], &slice->codewords[c][28], &slice->wordlengths[c][28], 28, m[21], sh[21], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  2], &slice->codewords[c][21], &slice->wordlengths[c][21], 21, m[22], sh[22], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  3], &slice->codewords[c][29], &slice->wordlengths[c][29], 29, m[23], sh[23], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  0], &slice->codewords[c][ 5], &slice->wordlengths[c][ 5],  5, m[24], sh[24], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  1], &slice->codewords[c][14], &slice->wordlengths[c][14], 14, m[25], sh[25], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  2], &slice->codewords[c][ 7], &slice->wordlengths[c][ 7],  7, m[26], sh[26], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  3], &slice->codewords[c][15], &slice->wordlengths[c][15], 15, m[27], sh[27], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  0], &slice->codewords[c][22], &slice->wordlengths[c][22], 22, m[28], sh[28], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  1], &slice->codewords[c][30], &slice->wordlengths[c][30], 30, m[29], sh[29], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  2], &slice->codewords[c][23], &slice->wordlengths[c][23], 23, m[30], sh[30], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  3], &slice->codewords[c][31], &slice->wordlengths[c][31], 31, m[31], sh[31], qshift, samples);

  length -= (SAMPLES_PER_SLICE - 1) - samples;
  slice->length[c] = (length + 7)/8;
  slice->samples[c] = samples + 1;
}

template<> inline void encode_slice_component<8,8,2, int16_t>(CodedSlice<int16_t> *slice, int c, QuantisationMatrices *matrices) {
  (void)matrices;
  const int w = 8;
  const int h = 8;
  const int SAMPLES_PER_SLICE = w*h;
  int length = 0;
  int samples = -1;

  const int istride = slice->istride[c];

  int qindex = (slice->qindex <= 31)?(slice->qindex):(28 + (slice->qindex%4));
  int qshift = (slice->qindex <= 31)?(0):((slice->qindex/4) - 7);

  const uint16_t *m = matrices->m(qindex,  c);
  const uint8_t *sh = matrices->sh(qindex, c);

  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  0], &slice->codewords[c][ 0], &slice->wordlengths[c][ 0],  0, m[ 0], sh[ 0], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  1], &slice->codewords[c][16], &slice->wordlengths[c][16], 16, m[ 1], sh[ 1], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  2], &slice->codewords[c][ 4], &slice->wordlengths[c][ 4],  4, m[ 2], sh[ 2], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  3], &slice->codewords[c][17], &slice->wordlengths[c][17], 17, m[ 3], sh[ 3], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  4], &slice->codewords[c][ 1], &slice->wordlengths[c][ 1],  1, m[ 4], sh[ 4], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  5], &slice->codewords[c][18], &slice->wordlengths[c][18], 18, m[ 5], sh[ 5], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  6], &slice->codewords[c][ 5], &slice->wordlengths[c][ 5],  5, m[ 6], sh[ 6], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  7], &slice->codewords[c][19], &slice->wordlengths[c][19], 19, m[ 7], sh[ 7], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  0], &slice->codewords[c][32], &slice->wordlengths[c][32], 32, m[ 8], sh[ 8], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  1], &slice->codewords[c][48], &slice->wordlengths[c][48], 48, m[ 9], sh[ 9], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  2], &slice->codewords[c][33], &slice->wordlengths[c][33], 33, m[10], sh[10], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  3], &slice->codewords[c][49], &slice->wordlengths[c][49], 49, m[11], sh[11], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  4], &slice->codewords[c][34], &slice->wordlengths[c][34], 34, m[12], sh[12], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  5], &slice->codewords[c][50], &slice->wordlengths[c][50], 50, m[13], sh[13], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  6], &slice->codewords[c][35], &slice->wordlengths[c][35], 35, m[14], sh[14], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  7], &slice->codewords[c][51], &slice->wordlengths[c][51], 51, m[15], sh[15], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  0], &slice->codewords[c][ 8], &slice->wordlengths[c][ 8],  8, m[16], sh[16], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  1], &slice->codewords[c][20], &slice->wordlengths[c][20], 20, m[17], sh[17], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  2], &slice->codewords[c][12], &slice->wordlengths[c][12], 12, m[18], sh[18], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  3], &slice->codewords[c][21], &slice->wordlengths[c][21], 21, m[19], sh[19], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  4], &slice->codewords[c][ 9], &slice->wordlengths[c][ 9],  9, m[20], sh[20], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  5], &slice->codewords[c][22], &slice->wordlengths[c][22], 22, m[21], sh[21], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  6], &slice->codewords[c][13], &slice->wordlengths[c][13], 13, m[22], sh[22], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  7], &slice->codewords[c][23], &slice->wordlengths[c][23], 23, m[23], sh[23], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  0], &slice->codewords[c][36], &slice->wordlengths[c][36], 36, m[24], sh[24], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  1], &slice->codewords[c][52], &slice->wordlengths[c][52], 52, m[25], sh[25], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  2], &slice->codewords[c][37], &slice->wordlengths[c][37], 37, m[26], sh[26], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  3], &slice->codewords[c][53], &slice->wordlengths[c][53], 53, m[27], sh[27], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  4], &slice->codewords[c][38], &slice->wordlengths[c][38], 38, m[28], sh[28], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  5], &slice->codewords[c][54], &slice->wordlengths[c][54], 54, m[29], sh[29], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  6], &slice->codewords[c][39], &slice->wordlengths[c][39], 39, m[30], sh[30], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  7], &slice->codewords[c][55], &slice->wordlengths[c][55], 55, m[31], sh[31], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  0], &slice->codewords[c][ 2], &slice->wordlengths[c][ 2],  2, m[32], sh[32], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  1], &slice->codewords[c][24], &slice->wordlengths[c][24], 24, m[33], sh[33], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  2], &slice->codewords[c][ 6], &slice->wordlengths[c][ 6],  6, m[34], sh[34], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  3], &slice->codewords[c][25], &slice->wordlengths[c][25], 25, m[35], sh[35], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  4], &slice->codewords[c][ 3], &slice->wordlengths[c][ 3],  3, m[36], sh[36], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  5], &slice->codewords[c][26], &slice->wordlengths[c][26], 26, m[37], sh[37], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  6], &slice->codewords[c][ 7], &slice->wordlengths[c][ 7],  7, m[38], sh[38], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  7], &slice->codewords[c][27], &slice->wordlengths[c][27], 27, m[39], sh[39], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  0], &slice->codewords[c][40], &slice->wordlengths[c][40], 40, m[40], sh[40], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  1], &slice->codewords[c][56], &slice->wordlengths[c][56], 56, m[41], sh[41], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  2], &slice->codewords[c][41], &slice->wordlengths[c][41], 41, m[42], sh[42], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  3], &slice->codewords[c][57], &slice->wordlengths[c][57], 57, m[43], sh[43], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  4], &slice->codewords[c][42], &slice->wordlengths[c][42], 42, m[44], sh[44], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  5], &slice->codewords[c][58], &slice->wordlengths[c][58], 58, m[45], sh[45], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  6], &slice->codewords[c][43], &slice->wordlengths[c][43], 43, m[46], sh[46], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  7], &slice->codewords[c][59], &slice->wordlengths[c][59], 59, m[47], sh[47], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  0], &slice->codewords[c][10], &slice->wordlengths[c][10], 10, m[48], sh[48], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  1], &slice->codewords[c][28], &slice->wordlengths[c][28], 28, m[49], sh[49], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  2], &slice->codewords[c][14], &slice->wordlengths[c][14], 14, m[50], sh[50], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  3], &slice->codewords[c][29], &slice->wordlengths[c][29], 29, m[51], sh[51], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  4], &slice->codewords[c][11], &slice->wordlengths[c][11], 11, m[52], sh[52], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  5], &slice->codewords[c][30], &slice->wordlengths[c][30], 30, m[53], sh[53], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  6], &slice->codewords[c][15], &slice->wordlengths[c][15], 15, m[54], sh[54], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  7], &slice->codewords[c][31], &slice->wordlengths[c][31], 31, m[55], sh[55], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  0], &slice->codewords[c][44], &slice->wordlengths[c][44], 44, m[56], sh[56], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  1], &slice->codewords[c][60], &slice->wordlengths[c][60], 60, m[57], sh[57], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  2], &slice->codewords[c][45], &slice->wordlengths[c][45], 45, m[58], sh[58], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  3], &slice->codewords[c][61], &slice->wordlengths[c][61], 61, m[59], sh[59], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  4], &slice->codewords[c][46], &slice->wordlengths[c][46], 46, m[60], sh[60], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  5], &slice->codewords[c][62], &slice->wordlengths[c][62], 62, m[61], sh[61], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  6], &slice->codewords[c][47], &slice->wordlengths[c][47], 47, m[62], sh[62], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  7], &slice->codewords[c][63], &slice->wordlengths[c][63], 63, m[63], sh[63], qshift, samples);

  length -= (SAMPLES_PER_SLICE - 1) - samples;
  slice->length[c] = (length + 7)/8;
  slice->samples[c] = samples + 1;
}


template<> inline void encode_slice_component<32,8,3, int16_t>(CodedSlice<int16_t> *slice, int c, QuantisationMatrices *matrices) {
  (void)matrices;
  const int w = 32;
  const int h = 8;
  const int SAMPLES_PER_SLICE = w*h;
  int length = 0;
  int samples = -1;

  const int istride = slice->istride[c];

  const int qindex = (slice->qindex <= 31)?(slice->qindex):(28 + (slice->qindex%4));
  const uint8_t qshift = (slice->qindex <= 31)?(0):((slice->qindex/4) - 7);

  const uint16_t *m = matrices->m(qindex,  c);
  const uint8_t *sh = matrices->sh(qindex, c);

  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  0], &slice->codewords[c][  0], &slice->wordlengths[c][  0],   0, m[  0], sh[  0], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  1], &slice->codewords[c][ 64], &slice->wordlengths[c][ 64],  64, m[  1], sh[  1], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  2], &slice->codewords[c][ 16], &slice->wordlengths[c][ 16],  16, m[  2], sh[  2], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  3], &slice->codewords[c][ 65], &slice->wordlengths[c][ 65],  65, m[  3], sh[  3], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  4], &slice->codewords[c][  4], &slice->wordlengths[c][  4],   4, m[  4], sh[  4], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  5], &slice->codewords[c][ 66], &slice->wordlengths[c][ 66],  66, m[  5], sh[  5], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  6], &slice->codewords[c][ 17], &slice->wordlengths[c][ 17],  17, m[  6], sh[  6], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  7], &slice->codewords[c][ 67], &slice->wordlengths[c][ 67],  67, m[  7], sh[  7], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  8], &slice->codewords[c][  1], &slice->wordlengths[c][  1],   1, m[  8], sh[  8], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride +  9], &slice->codewords[c][ 68], &slice->wordlengths[c][ 68],  68, m[  9], sh[  9], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 10], &slice->codewords[c][ 18], &slice->wordlengths[c][ 18],  18, m[ 10], sh[ 10], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 11], &slice->codewords[c][ 69], &slice->wordlengths[c][ 69],  69, m[ 11], sh[ 11], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 12], &slice->codewords[c][  5], &slice->wordlengths[c][  5],   5, m[ 12], sh[ 12], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 13], &slice->codewords[c][ 70], &slice->wordlengths[c][ 70],  70, m[ 13], sh[ 13], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 14], &slice->codewords[c][ 19], &slice->wordlengths[c][ 19],  19, m[ 14], sh[ 14], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 15], &slice->codewords[c][ 71], &slice->wordlengths[c][ 71],  71, m[ 15], sh[ 15], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 16], &slice->codewords[c][  2], &slice->wordlengths[c][  2],   2, m[ 16], sh[ 16], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 17], &slice->codewords[c][ 72], &slice->wordlengths[c][ 72],  72, m[ 17], sh[ 17], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 18], &slice->codewords[c][ 20], &slice->wordlengths[c][ 20],  20, m[ 18], sh[ 18], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 19], &slice->codewords[c][ 73], &slice->wordlengths[c][ 73],  73, m[ 19], sh[ 19], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 20], &slice->codewords[c][  6], &slice->wordlengths[c][  6],   6, m[ 20], sh[ 20], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 21], &slice->codewords[c][ 74], &slice->wordlengths[c][ 74],  74, m[ 21], sh[ 21], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 22], &slice->codewords[c][ 21], &slice->wordlengths[c][ 21],  21, m[ 22], sh[ 22], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 23], &slice->codewords[c][ 75], &slice->wordlengths[c][ 75],  75, m[ 23], sh[ 23], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 24], &slice->codewords[c][  3], &slice->wordlengths[c][  3],   3, m[ 24], sh[ 24], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 25], &slice->codewords[c][ 76], &slice->wordlengths[c][ 76],  76, m[ 25], sh[ 25], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 26], &slice->codewords[c][ 22], &slice->wordlengths[c][ 22],  22, m[ 26], sh[ 26], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 27], &slice->codewords[c][ 77], &slice->wordlengths[c][ 77],  77, m[ 27], sh[ 27], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 28], &slice->codewords[c][  7], &slice->wordlengths[c][  7],   7, m[ 28], sh[ 28], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 29], &slice->codewords[c][ 78], &slice->wordlengths[c][ 78],  78, m[ 29], sh[ 29], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 30], &slice->codewords[c][ 23], &slice->wordlengths[c][ 23],  23, m[ 30], sh[ 30], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 0*istride + 31], &slice->codewords[c][ 79], &slice->wordlengths[c][ 79],  79, m[ 31], sh[ 31], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  0], &slice->codewords[c][128], &slice->wordlengths[c][128], 128, m[ 32], sh[ 32], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  1], &slice->codewords[c][192], &slice->wordlengths[c][192], 192, m[ 33], sh[ 33], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  2], &slice->codewords[c][129], &slice->wordlengths[c][129], 129, m[ 34], sh[ 34], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  3], &slice->codewords[c][193], &slice->wordlengths[c][193], 193, m[ 35], sh[ 35], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  4], &slice->codewords[c][130], &slice->wordlengths[c][130], 130, m[ 36], sh[ 36], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  5], &slice->codewords[c][194], &slice->wordlengths[c][194], 194, m[ 37], sh[ 37], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  6], &slice->codewords[c][131], &slice->wordlengths[c][131], 131, m[ 38], sh[ 38], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  7], &slice->codewords[c][195], &slice->wordlengths[c][195], 195, m[ 39], sh[ 39], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  8], &slice->codewords[c][132], &slice->wordlengths[c][132], 132, m[ 40], sh[ 40], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride +  9], &slice->codewords[c][196], &slice->wordlengths[c][196], 196, m[ 41], sh[ 41], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 10], &slice->codewords[c][133], &slice->wordlengths[c][133], 133, m[ 42], sh[ 42], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 11], &slice->codewords[c][197], &slice->wordlengths[c][197], 197, m[ 43], sh[ 43], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 12], &slice->codewords[c][134], &slice->wordlengths[c][134], 134, m[ 44], sh[ 44], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 13], &slice->codewords[c][198], &slice->wordlengths[c][198], 198, m[ 45], sh[ 45], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 14], &slice->codewords[c][135], &slice->wordlengths[c][135], 135, m[ 46], sh[ 46], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 15], &slice->codewords[c][199], &slice->wordlengths[c][199], 199, m[ 47], sh[ 47], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 16], &slice->codewords[c][136], &slice->wordlengths[c][136], 136, m[ 48], sh[ 48], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 17], &slice->codewords[c][200], &slice->wordlengths[c][200], 200, m[ 49], sh[ 49], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 18], &slice->codewords[c][137], &slice->wordlengths[c][137], 137, m[ 50], sh[ 50], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 19], &slice->codewords[c][201], &slice->wordlengths[c][201], 201, m[ 51], sh[ 51], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 20], &slice->codewords[c][138], &slice->wordlengths[c][138], 138, m[ 52], sh[ 52], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 21], &slice->codewords[c][202], &slice->wordlengths[c][202], 202, m[ 53], sh[ 53], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 22], &slice->codewords[c][139], &slice->wordlengths[c][139], 139, m[ 54], sh[ 54], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 23], &slice->codewords[c][203], &slice->wordlengths[c][203], 203, m[ 55], sh[ 55], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 24], &slice->codewords[c][140], &slice->wordlengths[c][140], 140, m[ 56], sh[ 56], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 25], &slice->codewords[c][204], &slice->wordlengths[c][204], 204, m[ 57], sh[ 57], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 26], &slice->codewords[c][141], &slice->wordlengths[c][141], 141, m[ 58], sh[ 58], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 27], &slice->codewords[c][205], &slice->wordlengths[c][205], 205, m[ 59], sh[ 59], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 28], &slice->codewords[c][142], &slice->wordlengths[c][142], 142, m[ 60], sh[ 60], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 29], &slice->codewords[c][206], &slice->wordlengths[c][206], 206, m[ 61], sh[ 61], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 30], &slice->codewords[c][143], &slice->wordlengths[c][143], 143, m[ 62], sh[ 62], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 1*istride + 31], &slice->codewords[c][207], &slice->wordlengths[c][207], 207, m[ 63], sh[ 63], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  0], &slice->codewords[c][ 32], &slice->wordlengths[c][ 32],  32, m[ 64], sh[ 64], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  1], &slice->codewords[c][ 80], &slice->wordlengths[c][ 80],  80, m[ 65], sh[ 65], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  2], &slice->codewords[c][ 48], &slice->wordlengths[c][ 48],  48, m[ 66], sh[ 66], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  3], &slice->codewords[c][ 81], &slice->wordlengths[c][ 81],  81, m[ 67], sh[ 67], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  4], &slice->codewords[c][ 33], &slice->wordlengths[c][ 33],  33, m[ 68], sh[ 68], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  5], &slice->codewords[c][ 82], &slice->wordlengths[c][ 82],  82, m[ 69], sh[ 69], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  6], &slice->codewords[c][ 49], &slice->wordlengths[c][ 49],  49, m[ 70], sh[ 70], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  7], &slice->codewords[c][ 83], &slice->wordlengths[c][ 83],  83, m[ 71], sh[ 71], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  8], &slice->codewords[c][ 34], &slice->wordlengths[c][ 34],  34, m[ 72], sh[ 72], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride +  9], &slice->codewords[c][ 84], &slice->wordlengths[c][ 84],  84, m[ 73], sh[ 73], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 10], &slice->codewords[c][ 50], &slice->wordlengths[c][ 50],  50, m[ 74], sh[ 74], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 11], &slice->codewords[c][ 85], &slice->wordlengths[c][ 85],  85, m[ 75], sh[ 75], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 12], &slice->codewords[c][ 35], &slice->wordlengths[c][ 35],  35, m[ 76], sh[ 76], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 13], &slice->codewords[c][ 86], &slice->wordlengths[c][ 86],  86, m[ 77], sh[ 77], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 14], &slice->codewords[c][ 51], &slice->wordlengths[c][ 51],  51, m[ 78], sh[ 78], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 15], &slice->codewords[c][ 87], &slice->wordlengths[c][ 87],  87, m[ 79], sh[ 79], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 16], &slice->codewords[c][ 36], &slice->wordlengths[c][ 36],  36, m[ 80], sh[ 80], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 17], &slice->codewords[c][ 88], &slice->wordlengths[c][ 88],  88, m[ 81], sh[ 81], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 18], &slice->codewords[c][ 52], &slice->wordlengths[c][ 52],  52, m[ 82], sh[ 82], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 19], &slice->codewords[c][ 89], &slice->wordlengths[c][ 89],  89, m[ 83], sh[ 83], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 20], &slice->codewords[c][ 37], &slice->wordlengths[c][ 37],  37, m[ 84], sh[ 84], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 21], &slice->codewords[c][ 90], &slice->wordlengths[c][ 90],  90, m[ 85], sh[ 85], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 22], &slice->codewords[c][ 53], &slice->wordlengths[c][ 53],  53, m[ 86], sh[ 86], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 23], &slice->codewords[c][ 91], &slice->wordlengths[c][ 91],  91, m[ 87], sh[ 87], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 24], &slice->codewords[c][ 38], &slice->wordlengths[c][ 38],  38, m[ 88], sh[ 88], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 25], &slice->codewords[c][ 92], &slice->wordlengths[c][ 92],  92, m[ 89], sh[ 89], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 26], &slice->codewords[c][ 54], &slice->wordlengths[c][ 54],  54, m[ 90], sh[ 90], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 27], &slice->codewords[c][ 93], &slice->wordlengths[c][ 93],  93, m[ 91], sh[ 91], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 28], &slice->codewords[c][ 39], &slice->wordlengths[c][ 39],  39, m[ 92], sh[ 92], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 29], &slice->codewords[c][ 94], &slice->wordlengths[c][ 94],  94, m[ 93], sh[ 93], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 30], &slice->codewords[c][ 55], &slice->wordlengths[c][ 55],  55, m[ 94], sh[ 94], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 2*istride + 31], &slice->codewords[c][ 95], &slice->wordlengths[c][ 95],  95, m[ 95], sh[ 95], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  0], &slice->codewords[c][144], &slice->wordlengths[c][144], 144, m[ 96], sh[ 96], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  1], &slice->codewords[c][208], &slice->wordlengths[c][208], 208, m[ 97], sh[ 97], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  2], &slice->codewords[c][145], &slice->wordlengths[c][145], 145, m[ 98], sh[ 98], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  3], &slice->codewords[c][209], &slice->wordlengths[c][209], 209, m[ 99], sh[ 99], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  4], &slice->codewords[c][146], &slice->wordlengths[c][146], 146, m[100], sh[100], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  5], &slice->codewords[c][210], &slice->wordlengths[c][210], 210, m[101], sh[101], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  6], &slice->codewords[c][147], &slice->wordlengths[c][147], 147, m[102], sh[102], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  7], &slice->codewords[c][211], &slice->wordlengths[c][211], 211, m[103], sh[103], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  8], &slice->codewords[c][148], &slice->wordlengths[c][148], 148, m[104], sh[104], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride +  9], &slice->codewords[c][212], &slice->wordlengths[c][212], 212, m[105], sh[105], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 10], &slice->codewords[c][149], &slice->wordlengths[c][149], 149, m[106], sh[106], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 11], &slice->codewords[c][213], &slice->wordlengths[c][213], 213, m[107], sh[107], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 12], &slice->codewords[c][150], &slice->wordlengths[c][150], 150, m[108], sh[108], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 13], &slice->codewords[c][214], &slice->wordlengths[c][214], 214, m[109], sh[109], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 14], &slice->codewords[c][151], &slice->wordlengths[c][151], 151, m[110], sh[110], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 15], &slice->codewords[c][215], &slice->wordlengths[c][215], 215, m[111], sh[111], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 16], &slice->codewords[c][152], &slice->wordlengths[c][152], 152, m[112], sh[112], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 17], &slice->codewords[c][216], &slice->wordlengths[c][216], 216, m[113], sh[113], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 18], &slice->codewords[c][153], &slice->wordlengths[c][153], 153, m[114], sh[114], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 19], &slice->codewords[c][217], &slice->wordlengths[c][217], 217, m[115], sh[115], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 20], &slice->codewords[c][154], &slice->wordlengths[c][154], 154, m[116], sh[116], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 21], &slice->codewords[c][218], &slice->wordlengths[c][218], 218, m[117], sh[117], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 22], &slice->codewords[c][155], &slice->wordlengths[c][155], 155, m[118], sh[118], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 23], &slice->codewords[c][219], &slice->wordlengths[c][219], 219, m[119], sh[119], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 24], &slice->codewords[c][156], &slice->wordlengths[c][156], 156, m[120], sh[120], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 25], &slice->codewords[c][220], &slice->wordlengths[c][220], 220, m[121], sh[121], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 26], &slice->codewords[c][157], &slice->wordlengths[c][157], 157, m[122], sh[122], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 27], &slice->codewords[c][221], &slice->wordlengths[c][221], 221, m[123], sh[123], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 28], &slice->codewords[c][158], &slice->wordlengths[c][158], 158, m[124], sh[124], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 29], &slice->codewords[c][222], &slice->wordlengths[c][222], 222, m[125], sh[125], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 30], &slice->codewords[c][159], &slice->wordlengths[c][159], 159, m[126], sh[126], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 3*istride + 31], &slice->codewords[c][223], &slice->wordlengths[c][223], 223, m[127], sh[127], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  0], &slice->codewords[c][  8], &slice->wordlengths[c][  8],   8, m[128], sh[128], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  1], &slice->codewords[c][ 96], &slice->wordlengths[c][ 96],  96, m[129], sh[129], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  2], &slice->codewords[c][ 24], &slice->wordlengths[c][ 24],  24, m[130], sh[130], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  3], &slice->codewords[c][ 97], &slice->wordlengths[c][ 97],  97, m[131], sh[131], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  4], &slice->codewords[c][ 12], &slice->wordlengths[c][ 12],  12, m[132], sh[132], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  5], &slice->codewords[c][ 98], &slice->wordlengths[c][ 98],  98, m[133], sh[133], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  6], &slice->codewords[c][ 25], &slice->wordlengths[c][ 25],  25, m[134], sh[134], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  7], &slice->codewords[c][ 99], &slice->wordlengths[c][ 99],  99, m[135], sh[135], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  8], &slice->codewords[c][  9], &slice->wordlengths[c][  9],   9, m[136], sh[136], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride +  9], &slice->codewords[c][100], &slice->wordlengths[c][100], 100, m[137], sh[137], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 10], &slice->codewords[c][ 26], &slice->wordlengths[c][ 26],  26, m[138], sh[138], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 11], &slice->codewords[c][101], &slice->wordlengths[c][101], 101, m[139], sh[139], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 12], &slice->codewords[c][ 13], &slice->wordlengths[c][ 13],  13, m[140], sh[140], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 13], &slice->codewords[c][102], &slice->wordlengths[c][102], 102, m[141], sh[141], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 14], &slice->codewords[c][ 27], &slice->wordlengths[c][ 27],  27, m[142], sh[142], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 15], &slice->codewords[c][103], &slice->wordlengths[c][103], 103, m[143], sh[143], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 16], &slice->codewords[c][ 10], &slice->wordlengths[c][ 10],  10, m[144], sh[144], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 17], &slice->codewords[c][104], &slice->wordlengths[c][104], 104, m[145], sh[145], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 18], &slice->codewords[c][ 28], &slice->wordlengths[c][ 28],  28, m[146], sh[146], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 19], &slice->codewords[c][105], &slice->wordlengths[c][105], 105, m[147], sh[147], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 20], &slice->codewords[c][ 14], &slice->wordlengths[c][ 14],  14, m[148], sh[148], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 21], &slice->codewords[c][106], &slice->wordlengths[c][106], 106, m[149], sh[149], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 22], &slice->codewords[c][ 29], &slice->wordlengths[c][ 29],  29, m[150], sh[150], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 23], &slice->codewords[c][107], &slice->wordlengths[c][107], 107, m[151], sh[151], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 24], &slice->codewords[c][ 11], &slice->wordlengths[c][ 11],  11, m[152], sh[152], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 25], &slice->codewords[c][108], &slice->wordlengths[c][108], 108, m[153], sh[153], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 26], &slice->codewords[c][ 30], &slice->wordlengths[c][ 30],  30, m[154], sh[154], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 27], &slice->codewords[c][109], &slice->wordlengths[c][109], 109, m[155], sh[155], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 28], &slice->codewords[c][ 15], &slice->wordlengths[c][ 15],  15, m[156], sh[156], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 29], &slice->codewords[c][110], &slice->wordlengths[c][110], 110, m[157], sh[157], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 30], &slice->codewords[c][ 31], &slice->wordlengths[c][ 31],  31, m[158], sh[158], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 4*istride + 31], &slice->codewords[c][111], &slice->wordlengths[c][111], 111, m[159], sh[159], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  0], &slice->codewords[c][160], &slice->wordlengths[c][160], 160, m[160], sh[160], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  1], &slice->codewords[c][224], &slice->wordlengths[c][224], 224, m[161], sh[161], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  2], &slice->codewords[c][161], &slice->wordlengths[c][161], 161, m[162], sh[162], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  3], &slice->codewords[c][225], &slice->wordlengths[c][225], 225, m[163], sh[163], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  4], &slice->codewords[c][162], &slice->wordlengths[c][162], 162, m[164], sh[164], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  5], &slice->codewords[c][226], &slice->wordlengths[c][226], 226, m[165], sh[165], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  6], &slice->codewords[c][163], &slice->wordlengths[c][163], 163, m[166], sh[166], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  7], &slice->codewords[c][227], &slice->wordlengths[c][227], 227, m[167], sh[167], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  8], &slice->codewords[c][164], &slice->wordlengths[c][164], 164, m[168], sh[168], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride +  9], &slice->codewords[c][228], &slice->wordlengths[c][228], 228, m[169], sh[169], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 10], &slice->codewords[c][165], &slice->wordlengths[c][165], 165, m[170], sh[170], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 11], &slice->codewords[c][229], &slice->wordlengths[c][229], 229, m[171], sh[171], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 12], &slice->codewords[c][166], &slice->wordlengths[c][166], 166, m[172], sh[172], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 13], &slice->codewords[c][230], &slice->wordlengths[c][230], 230, m[173], sh[173], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 14], &slice->codewords[c][167], &slice->wordlengths[c][167], 167, m[174], sh[174], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 15], &slice->codewords[c][231], &slice->wordlengths[c][231], 231, m[175], sh[175], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 16], &slice->codewords[c][168], &slice->wordlengths[c][168], 168, m[176], sh[176], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 17], &slice->codewords[c][232], &slice->wordlengths[c][232], 232, m[177], sh[177], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 18], &slice->codewords[c][169], &slice->wordlengths[c][169], 169, m[178], sh[178], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 19], &slice->codewords[c][233], &slice->wordlengths[c][233], 233, m[179], sh[179], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 20], &slice->codewords[c][170], &slice->wordlengths[c][170], 170, m[180], sh[180], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 21], &slice->codewords[c][234], &slice->wordlengths[c][234], 234, m[181], sh[181], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 22], &slice->codewords[c][171], &slice->wordlengths[c][171], 171, m[182], sh[182], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 23], &slice->codewords[c][235], &slice->wordlengths[c][235], 235, m[183], sh[183], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 24], &slice->codewords[c][172], &slice->wordlengths[c][172], 172, m[184], sh[184], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 25], &slice->codewords[c][236], &slice->wordlengths[c][236], 236, m[185], sh[185], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 26], &slice->codewords[c][173], &slice->wordlengths[c][173], 173, m[186], sh[186], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 27], &slice->codewords[c][237], &slice->wordlengths[c][237], 237, m[187], sh[187], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 28], &slice->codewords[c][174], &slice->wordlengths[c][174], 174, m[188], sh[188], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 29], &slice->codewords[c][238], &slice->wordlengths[c][238], 238, m[189], sh[189], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 30], &slice->codewords[c][175], &slice->wordlengths[c][175], 175, m[190], sh[190], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 5*istride + 31], &slice->codewords[c][239], &slice->wordlengths[c][239], 239, m[191], sh[191], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  0], &slice->codewords[c][ 40], &slice->wordlengths[c][ 40],  40, m[192], sh[192], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  1], &slice->codewords[c][112], &slice->wordlengths[c][112], 112, m[193], sh[193], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  2], &slice->codewords[c][ 56], &slice->wordlengths[c][ 56],  56, m[194], sh[194], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  3], &slice->codewords[c][113], &slice->wordlengths[c][113], 113, m[195], sh[195], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  4], &slice->codewords[c][ 41], &slice->wordlengths[c][ 41],  41, m[196], sh[196], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  5], &slice->codewords[c][114], &slice->wordlengths[c][114], 114, m[197], sh[197], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  6], &slice->codewords[c][ 57], &slice->wordlengths[c][ 57],  57, m[198], sh[198], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  7], &slice->codewords[c][115], &slice->wordlengths[c][115], 115, m[199], sh[199], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  8], &slice->codewords[c][ 42], &slice->wordlengths[c][ 42],  42, m[200], sh[200], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride +  9], &slice->codewords[c][116], &slice->wordlengths[c][116], 116, m[201], sh[201], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 10], &slice->codewords[c][ 58], &slice->wordlengths[c][ 58],  58, m[202], sh[202], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 11], &slice->codewords[c][117], &slice->wordlengths[c][117], 117, m[203], sh[203], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 12], &slice->codewords[c][ 43], &slice->wordlengths[c][ 43],  43, m[204], sh[204], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 13], &slice->codewords[c][118], &slice->wordlengths[c][118], 118, m[205], sh[205], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 14], &slice->codewords[c][ 59], &slice->wordlengths[c][ 59],  59, m[206], sh[206], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 15], &slice->codewords[c][119], &slice->wordlengths[c][119], 119, m[207], sh[207], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 16], &slice->codewords[c][ 44], &slice->wordlengths[c][ 44],  44, m[208], sh[208], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 17], &slice->codewords[c][120], &slice->wordlengths[c][120], 120, m[209], sh[209], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 18], &slice->codewords[c][ 60], &slice->wordlengths[c][ 60],  60, m[210], sh[210], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 19], &slice->codewords[c][121], &slice->wordlengths[c][121], 121, m[211], sh[211], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 20], &slice->codewords[c][ 45], &slice->wordlengths[c][ 45],  45, m[212], sh[212], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 21], &slice->codewords[c][122], &slice->wordlengths[c][122], 122, m[213], sh[213], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 22], &slice->codewords[c][ 61], &slice->wordlengths[c][ 61],  61, m[214], sh[214], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 23], &slice->codewords[c][123], &slice->wordlengths[c][123], 123, m[215], sh[215], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 24], &slice->codewords[c][ 46], &slice->wordlengths[c][ 46],  46, m[216], sh[216], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 25], &slice->codewords[c][124], &slice->wordlengths[c][124], 124, m[217], sh[217], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 26], &slice->codewords[c][ 62], &slice->wordlengths[c][ 62],  62, m[218], sh[218], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 27], &slice->codewords[c][125], &slice->wordlengths[c][125], 125, m[219], sh[219], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 28], &slice->codewords[c][ 47], &slice->wordlengths[c][ 47],  47, m[220], sh[220], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 29], &slice->codewords[c][126], &slice->wordlengths[c][126], 126, m[221], sh[221], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 30], &slice->codewords[c][ 63], &slice->wordlengths[c][ 63],  63, m[222], sh[222], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 6*istride + 31], &slice->codewords[c][127], &slice->wordlengths[c][127], 127, m[223], sh[223], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  0], &slice->codewords[c][176], &slice->wordlengths[c][176], 176, m[224], sh[224], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  1], &slice->codewords[c][240], &slice->wordlengths[c][240], 240, m[225], sh[225], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  2], &slice->codewords[c][177], &slice->wordlengths[c][177], 177, m[226], sh[226], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  3], &slice->codewords[c][241], &slice->wordlengths[c][241], 241, m[227], sh[227], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  4], &slice->codewords[c][178], &slice->wordlengths[c][178], 178, m[228], sh[228], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  5], &slice->codewords[c][242], &slice->wordlengths[c][242], 242, m[229], sh[229], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  6], &slice->codewords[c][179], &slice->wordlengths[c][179], 179, m[230], sh[230], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  7], &slice->codewords[c][243], &slice->wordlengths[c][243], 243, m[231], sh[231], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  8], &slice->codewords[c][180], &slice->wordlengths[c][180], 180, m[232], sh[232], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride +  9], &slice->codewords[c][244], &slice->wordlengths[c][244], 244, m[233], sh[233], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 10], &slice->codewords[c][181], &slice->wordlengths[c][181], 181, m[234], sh[234], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 11], &slice->codewords[c][245], &slice->wordlengths[c][245], 245, m[235], sh[235], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 12], &slice->codewords[c][182], &slice->wordlengths[c][182], 182, m[236], sh[236], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 13], &slice->codewords[c][246], &slice->wordlengths[c][246], 246, m[237], sh[237], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 14], &slice->codewords[c][183], &slice->wordlengths[c][183], 183, m[238], sh[238], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 15], &slice->codewords[c][247], &slice->wordlengths[c][247], 247, m[239], sh[239], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 16], &slice->codewords[c][184], &slice->wordlengths[c][184], 184, m[240], sh[240], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 17], &slice->codewords[c][248], &slice->wordlengths[c][248], 248, m[241], sh[241], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 18], &slice->codewords[c][185], &slice->wordlengths[c][185], 185, m[242], sh[242], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 19], &slice->codewords[c][249], &slice->wordlengths[c][249], 249, m[243], sh[243], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 20], &slice->codewords[c][186], &slice->wordlengths[c][186], 186, m[244], sh[244], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 21], &slice->codewords[c][250], &slice->wordlengths[c][250], 250, m[245], sh[245], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 22], &slice->codewords[c][187], &slice->wordlengths[c][187], 187, m[246], sh[246], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 23], &slice->codewords[c][251], &slice->wordlengths[c][251], 251, m[247], sh[247], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 24], &slice->codewords[c][188], &slice->wordlengths[c][188], 188, m[248], sh[248], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 25], &slice->codewords[c][252], &slice->wordlengths[c][252], 252, m[249], sh[249], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 26], &slice->codewords[c][189], &slice->wordlengths[c][189], 189, m[250], sh[250], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 27], &slice->codewords[c][253], &slice->wordlengths[c][253], 253, m[251], sh[251], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 28], &slice->codewords[c][190], &slice->wordlengths[c][190], 190, m[252], sh[252], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 29], &slice->codewords[c][254], &slice->wordlengths[c][254], 254, m[253], sh[253], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 30], &slice->codewords[c][191], &slice->wordlengths[c][191], 191, m[254], sh[254], qshift, samples);
  length += encode_sample<int16_t>(&slice->idata[c][ 7*istride + 31], &slice->codewords[c][255], &slice->wordlengths[c][255], 255, m[255], sh[255], qshift, samples);

  length -= (SAMPLES_PER_SLICE - 1) - samples;
  slice->length[c] = (length + 7)/8;
  slice->samples[c] = samples + 1;
}
