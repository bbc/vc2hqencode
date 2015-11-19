/*****************************************************************************
 * quantise.cpp : Quantise
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

#include "quantise.hpp"
#include "vc2hqencode.h"
#include "logger.hpp"
#include <stdlib.h>

#include <stdio.h>
#include <malloc.h>

#ifdef DEBUG
//#define DEBUG_PRINT_QUANTISATION_MATRICES
#endif

#define max(A,B) (((A) > (B))?(A):(B))

uint32_t quant_factor(const uint8_t index) {
  int base = 1 << (index >> 2);
  switch (index%4) {
  case 0:
    return 4*base;
  case 1:
    return (((503829*base) + 52958) / 105917);
  case 2:
    return (((665857*base) + 58854) / 117708);
  case 3:
    return (((440253*base) + 32722) / 65444);
  }
  throw;
}

QuantisationMatrices::QuantisationMatrices(int bw, int bh, int d, int t, int q_min, int q_max) {
  mBW        = bw;
  mBH        = bh;
  mDepth     = d;
  mTransform = t;
  mQmin      = q_min;
  mQmax      = q_max;

  mMatrixYLen = bw*bh;
  mMatrixCLen = bw*bh/2;

  mQF = (uint16_t *)memalign(64, (q_max - q_min)*(mMatrixYLen + 2*mMatrixCLen)*sizeof(uint16_t));
  mM  = (uint16_t *)memalign(64, (q_max - q_min)*(mMatrixYLen + 2*mMatrixCLen)*sizeof(uint16_t));
  mSh = (uint8_t  *)memalign(64, (q_max - q_min)*(mMatrixYLen + 2*mMatrixCLen)*sizeof(uint8_t));

  for (int qi = q_min; qi < q_max; qi++) {
    uint16_t *Y = &mQF[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen)];
    const QuantisationWeightingMatrix *QWM = &preset_quantisation_matrices[t][d];

    {
      int skip = 1 << d;
      for (int y = 0; y < bh; y += skip)
        for (int x = 0; x < bw; x += skip)
          Y[y*bw + x] = quant_factor(max(qi - QWM->LL, 0));

      for (int l = 0; l < d; l++) {
        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw; x += skip)
            Y[y*bw + x + skip/2] = quant_factor(max(qi - QWM->HL[l], 0));

        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw; x += skip)
            Y[(y + skip/2)*bw + x] = quant_factor(max(qi - QWM->LH[l], 0));

        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw; x += skip)
            Y[(y + skip/2)*bw + x + skip/2] = quant_factor(max(qi - QWM->HH[l], 0));

        skip /= 2;
      }
    }
    Y += mMatrixYLen;

    {
      int skip = 1 << d;
      for (int y = 0; y < bh; y += skip)
        for (int x = 0; x < bw/2; x += skip)
          Y[y*bw/2 + x] = quant_factor(max(qi - QWM->LL, 0));

      for (int l = 0; l < d; l++) {
        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw/2; x += skip)
            Y[y*bw/2 + x + skip/2] = quant_factor(max(qi - QWM->HL[l], 0));

        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw/2; x += skip)
            Y[(y + skip/2)*bw/2 + x] = quant_factor(max(qi - QWM->LH[l], 0));

        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw/2; x += skip)
            Y[(y + skip/2)*bw/2 + x + skip/2] = quant_factor(max(qi - QWM->HH[l], 0));

        skip /= 2;
      }
    }
    Y += mMatrixCLen;

    {
      int skip = 1 << d;
      for (int y = 0; y < bh; y += skip)
        for (int x = 0; x < bw/2; x += skip)
          Y[y*bw/2 + x] = quant_factor(max(qi - QWM->LL, 0));

      for (int l = 0; l < d; l++) {
        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw/2; x += skip)
            Y[y*bw/2 + x + skip/2] = quant_factor(max(qi - QWM->HL[l], 0));

        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw/2; x += skip)
            Y[(y + skip/2)*bw/2 + x] = quant_factor(max(qi - QWM->LH[l], 0));

        for (int y = 0; y < bh; y += skip)
          for (int x = 0; x < bw/2; x += skip)
            Y[(y + skip/2)*bw/2 + x + skip/2] = quant_factor(max(qi - QWM->HH[l], 0));

        skip /= 2;
      }
    }
  }

  for (int n = 0; n < (mQmax-mQmin)*(mMatrixYLen + 2*mMatrixCLen); n++) {
    Divisor<uint16_t> D(mQF[n]);
    mM[n]  = D.m;
    mSh[n] = D.l - 2;
  }

#ifdef DEBUG_PRINT_QUANTISATION_MATRICES
  for (int qi = q_min; qi < q_max; qi++) {
    printf("----------------------------------------------\n");
    printf("qi = %d\n", qi);
    printf("----------------------------------------------\n");
    for (int y = 0; y < bh; y++) {
      printf("    ");
      for (int x = 0; x < bw; x++) {
        printf("%2d  ", qf(qi, 0)[y*bw + x]);
      }
      printf("\n");
    }
    printf("\n");
    for (int y = 0; y < bh; y++) {
      printf("    ");
      for (int x = 0; x < bw/2; x++) {
        printf("%2d  ", qf(qi, 1)[y*bw/2 + x]);
      }
      printf("\n");
    }
    printf("\n");
    for (int y = 0; y < bh; y++) {
      printf("    ");
      for (int x = 0; x < bw/2; x++) {
        printf("%2d  ", qf(qi, 2)[y*bw/2 + x]);
      }
      printf("\n");
    }
    printf("----------------------------------------------\n");
  }
#endif
}

QuantisationMatrices::~QuantisationMatrices() {
  free(mQF);
  free(mM);
  free(mSh);
}
