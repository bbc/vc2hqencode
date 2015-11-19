/*****************************************************************************
 * quantise.hpp : Quantise
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

#ifndef __QUANTISE_HPP__
#define __QUANTISE_HPP__

#include <stdint.h>
#include "intdivide.hpp"

struct QuantisationWeightingMatrix {
  uint8_t LL;
  uint8_t HL[4];
  uint8_t LH[4];
  uint8_t HH[4];
};

const QuantisationWeightingMatrix preset_quantisation_matrices[][5] = {
  //  VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7
  {
    // 0
    {
      .LL = 0,
      .HL = { 0, 0, 0, 0 },
      .LH = { 0, 0, 0, 0 },
      .HH = { 0, 0, 0, 0 }
    },
    // 1
    {
      .LL = 5,
      .HL = { 3, 4, 5, 6 },
      .LH = { 3, 4, 5, 6 },
      .HH = { 0, 1, 2, 3 }
    },
    // 2
    {
      .LL = 5,
      .HL = { 3, 4, 5, 6 },
      .LH = { 3, 4, 5, 6 },
      .HH = { 0, 1, 2, 3 }
    },
    // 3
    {
      .LL = 5,
      .HL = { 3, 4, 5, 6 },
      .LH = { 3, 4, 5, 6 },
      .HH = { 0, 1, 2, 3 }
    },
    // 4
    {
      .LL = 5,
      .HL = { 3, 4, 5, 6 },
      .LH = { 3, 4, 5, 6 },
      .HH = { 0, 1, 2, 3 }
    },
  },
  //  VC2ENCODER_WFT_LEGALL_5_3
  {
    // 0
    {
      .LL = 0,
      .HL = { 0, 0, 0, 0 },
      .LH = { 0, 0, 0, 0 },
      .HH = { 0, 0, 0, 0 }
    },
    // 1
    {
      .LL = 4,
      .HL = { 2, 4, 5, 7 },
      .LH = { 2, 4, 5, 7 },
      .HH = { 0, 2, 3, 5 }
    },
    // 2
    {
      .LL = 4,
      .HL = { 2, 4, 5, 7 },
      .LH = { 2, 4, 5, 7 },
      .HH = { 0, 2, 3, 5 }
    },
    // 3
    {
      .LL = 4,
      .HL = { 2, 4, 5, 7 },
      .LH = { 2, 4, 5, 7 },
      .HH = { 0, 2, 3, 5 }
    },
    // 4
    {
      .LL = 4,
      .HL = { 2, 4, 5, 7 },
      .LH = { 2, 4, 5, 7 },
      .HH = { 0, 2, 3, 5 }
    },
  },
  //  VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7
  {
    // 0
    {
      .LL = 0,
      .HL = { 0, 0, 0, 0 },
      .LH = { 0, 0, 0, 0 },
      .HH = { 0, 0, 0, 0 }
    },
    // 1
    {
      .LL = 5,
      .HL = { 3, 4, 5, 6 },
      .LH = { 3, 4, 5, 6 },
      .HH = { 0, 1, 2, 3 }
    },
    // 2
    {
      .LL = 5,
      .HL = { 3, 4, 5, 6 },
      .LH = { 3, 4, 5, 6 },
      .HH = { 0, 1, 2, 3 }
    },
    // 3
    {
      .LL = 5,
      .HL = { 3, 4, 5, 6 },
      .LH = { 3, 4, 5, 6 },
      .HH = { 0, 1, 2, 3 }
    },
    // 4
    {
      .LL = 5,
      .HL = { 3, 4, 5, 6 },
      .LH = { 3, 4, 5, 6 },
      .HH = { 0, 1, 2, 3 }
    },
  },
  //  VC2ENCODER_WFT_HAAR_NO_SHIFT
  {
    // 0
    {
      .LL = 0,
      .HL = { 0, 0, 0, 0 },
      .LH = { 0, 0, 0, 0 },
      .HH = { 0, 0, 0, 0 }
    },
    // 1
    {
      .LL = 8,
      .HL = { 4, 0, 0, 0 },
      .LH = { 4, 0, 0, 0 },
      .HH = { 0, 0, 0, 0 }
    },
    // 2
    {
      .LL = 12,
      .HL = { 8, 4, 0, 0 },
      .LH = { 8, 4, 0, 0 },
      .HH = { 4, 0, 0, 0 }
    },
    // 3
    {
      .LL = 16,
      .HL = { 12, 8, 4, 0 },
      .LH = { 12, 8, 4, 0 },
      .HH = { 8,  4, 0, 0 }
    },
    // 4
    {
      .LL = 20,
      .HL = { 16, 12, 8, 4 },
      .LH = { 16, 12, 8, 4 },
      .HH = { 12,  8, 4, 0 }
    },
  },
  //  VC2ENCODER_WFT_HAAR_SINGLE_SHIFT
  {
    // 0
    {
      .LL = 0,
      .HL = { 0, 0, 0, 0 },
      .LH = { 0, 0, 0, 0 },
      .HH = { 0, 0, 0, 0 }
    },
    // 1
    {
      .LL = 8,
      .HL = { 4, 4, 4, 4 },
      .LH = { 4, 4, 4, 4 },
      .HH = { 0, 0, 0, 0 }
    },
    // 2
    {
      .LL = 8,
      .HL = { 4, 4, 4, 4 },
      .LH = { 4, 4, 4, 4 },
      .HH = { 0, 0, 0, 0 }
    },
    // 3
    {
      .LL = 8,
      .HL = { 4, 4, 4, 4 },
      .LH = { 4, 4, 4, 4 },
      .HH = { 0, 0, 0, 0 }
    },
    // 4
    {
      .LL = 8,
      .HL = { 4, 4, 4, 4 },
      .LH = { 4, 4, 4, 4 },
      .HH = { 0, 0, 0, 0 }
    },
  },
  //  VC2ENCODER_WFT_FIDELITY
  {
    // 0
    {
      .LL = 0,
      .HL = { 0, 0, 0, 0 },
      .LH = { 0, 0, 0, 0 },
      .HH = { 0, 0, 0, 0 }
    },
    // 1
    {
      .LL = 0,
      .HL = { 4,  8, 13, 17 },
      .LH = { 4,  8, 13, 17 },
      .HH = { 8, 12, 17, 21 }
    },
    // 2
    {
      .LL = 0,
      .HL = { 4,  8, 13, 17 },
      .LH = { 4,  8, 13, 17 },
      .HH = { 8, 12, 17, 21 }
    },
    // 3
    {
      .LL = 0,
      .HL = { 4,  8, 13, 17 },
      .LH = { 4,  8, 13, 17 },
      .HH = { 8, 12, 17, 21 }
    },
    // 4
    {
      .LL = 0,
      .HL = { 4,  8, 13, 17 },
      .LH = { 4,  8, 13, 17 },
      .HH = { 8, 12, 17, 21 }
    },
  },
  //  VC2ENCODER_WFT_DAUBECHIES_9_7
  {
    // 0
    {
      .LL = 0,
      .HL = { 0, 0, 0, 0 },
      .LH = { 0, 0, 0, 0 },
      .HH = { 0, 0, 0, 0 }
    },
    // 1
    {
      .LL = 3,
      .HL = { 1, 4, 6, 9 },
      .LH = { 1, 4, 6, 9 },
      .HH = { 0, 2, 5, 7 }
    },
    // 2
    {
      .LL = 3,
      .HL = { 1, 4, 6, 9 },
      .LH = { 1, 4, 6, 9 },
      .HH = { 0, 2, 5, 7 }
    },
    // 3
    {
      .LL = 3,
      .HL = { 1, 4, 6, 9 },
      .LH = { 1, 4, 6, 9 },
      .HH = { 0, 2, 5, 7 }
    },
    // 4
    {
      .LL = 3,
      .HL = { 1, 4, 6, 9 },
      .LH = { 1, 4, 6, 9 },
      .HH = { 0, 2, 5, 7 }
    },
  },
};

class QuantisationMatrices {
public:
  QuantisationMatrices(int bw, int bh, int d, int t, int q_min, int q_max);

  ~QuantisationMatrices();

  const uint16_t *qf(int qi, int c) {
    if (c == 0)
      return &mQF[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen)];
    else if (c == 1)
      return &mQF[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen) + mMatrixYLen];
    else
      return &mQF[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen) + mMatrixYLen + mMatrixCLen];
  }

  const uint16_t *m(int qi, int c) {
    if (c == 0)
      return &mM[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen)];
    else if (c == 1)
      return &mM[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen) + mMatrixYLen];
    else
      return &mM[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen) + mMatrixYLen + mMatrixCLen];
  }

  const uint8_t *sh(int qi, int c) {
    if (c == 0)
      return &mSh[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen)];
    else if (c == 1)
      return &mSh[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen) + mMatrixYLen];
    else
      return &mSh[(qi-mQmin)*(mMatrixYLen + 2*mMatrixCLen) + mMatrixYLen + mMatrixCLen];
  }

protected:
  uint16_t *mQF;
  uint16_t *mM;
  uint8_t  *mSh;

  int mBW;
  int mBH;
  int mDepth;
  int mTransform;

  int mQmin;
  int mQmax;

  int mMatrixYLen;
  int mMatrixCLen;
};

#endif /* __QUANTISE_HPP__ */
