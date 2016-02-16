/*****************************************************************************
 * deslaurier_dubuc_9_7_transform.hpp : DD9,7 transform: Plain C++ version
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

#include "transform.hpp"
#include <string.h>
#include <x86intrin.h>

template<class T> void Deslauriers_Dubuc_9_7_transform_H_inplace_10P2(const char *_idata,
                                                                       const int istride,
                                                                       void **_odata,
                                                                       const int ostride,
                                                                       const int iwidth,
                                                                       const int iheight,
                                                                       const int owidth,
                                                                       const int oheight) {
  const int skip = 1;

  const uint16_t *idata = (const uint16_t *)_idata;
  T *odata = *((T **)_odata);
  const T offset = 1 << 9;

  (void)iwidth;
  (void)iheight;

  int y = 0;
  for (; y < iheight; y+=skip) {
    int x = 0;
    int32_t Dm2, D, Dp1, Dp2, Dp3, Dp4;
    int Xm1, X, Xp1;
    D   = (((int32_t)idata[y*istride + x + 0*skip]) - offset) << 1;
    Dp1 = (((int32_t)idata[y*istride + x + 1*skip]) - offset) << 1;
    Dp2 = (((int32_t)idata[y*istride + x + 2*skip]) - offset) << 1;
    Dp3 = (((int32_t)idata[y*istride + x + 3*skip]) - offset) << 1;
    Dp4 = (((int32_t)idata[y*istride + x + 4*skip]) - offset) << 1;

    {
      Dm2 = D;
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      Xm1 = Xp1;
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = (((int32_t)idata[y*istride + x + 5*skip]) - offset) << 1;
      Dp4 = (((int32_t)idata[y*istride + x + 6*skip]) - offset) << 1;

      Xm1 = Xp1;
    }
    x += 2*skip;

    for (; x < iwidth - 6*skip; x += 2*skip) {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = (((int32_t)idata[y*istride + x + 5*skip]) - offset) << 1;
      Dp4 = (((int32_t)idata[y*istride + x + 6*skip]) - offset) << 1;

      Xm1 = Xp1;
    }

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = (((int32_t)idata[y*istride + x + 5*skip]) - offset) << 1;
      Dp4 = Dp4;

      Xm1 = Xp1;
    }
    x += 2*skip;

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = D;
      Dp3 = Dp3;
      Dp4 = Dm2;

      Xm1 = Xp1;
    }
    x += 2*skip;

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;
    }
    x += 2*skip;

    for (; x < owidth; x += 2*skip) {
      odata[y*ostride + x + 0*skip] = odata[y*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[y*ostride + x + 1*skip] = odata[y*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }

  for (; y < oheight; y+=skip) {
    memcpy(&odata[y*ostride], &odata[(2*iheight - y - skip)*ostride], owidth);
  }
}

template<int skip, class T> void Deslauriers_Dubuc_9_7_transform_H_inplace(void *_idata,
                                                                           const int istride,
                                                                           const int width,
                                                                           const int height,
                                                                           const int) {
  T *idata = (T *)_idata;
  for (int y = 0; y < height; y+=skip) {
    int x = 0;
    int32_t Dm2, D, Dp1, Dp2, Dp3, Dp4;
    int Xm1, X, Xp1;
    D   = idata[y*istride + x + 0*skip] << 1;
    Dp1 = idata[y*istride + x + 1*skip] << 1;
    Dp2 = idata[y*istride + x + 2*skip] << 1;
    Dp3 = idata[y*istride + x + 3*skip] << 1;
    Dp4 = idata[y*istride + x + 4*skip] << 1;

    {
      Dm2 = D;
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      Xm1 = Xp1;
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = idata[y*istride + x + 5*skip] << 1;
      Dp4 = idata[y*istride + x + 6*skip] << 1;

      Xm1 = Xp1;
    }
    x += 2*skip;

    for (; x < width - 6*skip; x += 2*skip) {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = idata[y*istride + x + 5*skip] << 1;
      Dp4 = idata[y*istride + x + 6*skip] << 1;

      Xm1 = Xp1;
    }

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = idata[y*istride + x + 5*skip] << 1;
      Dp4 = Dp4;

      Xm1 = Xp1;
    }
    x += 2*skip;

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = D;
      Dp3 = Dp3;
      Dp4 = Dm2;

      Xm1 = Xp1;
    }
    x += 2*skip;

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;
    }
  }
}

template<int skip, class T>void Deslauriers_Dubuc_9_7_transform_V_inplace(void *_idata,
                                                                          const int istride,
                                                                          const int width,
                                                                          const int height,
                                                                          const int) {
  T *idata = (T *)_idata;
  for (int x = 0; x < width; x+=skip) {
    int y = 0;
    int32_t Dm2, D, Dp1, Dp2, Dp3, Dp4;
    int Xm1, X, Xp1;
    D   = idata[(y + 0*skip)*istride + x];
    Dp1 = idata[(y + 1*skip)*istride + x];
    Dp2 = idata[(y + 2*skip)*istride + x];
    Dp3 = idata[(y + 3*skip)*istride + x];
    Dp4 = idata[(y + 4*skip)*istride + x];

    {
      Dm2 = D;
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      Xm1 = Xp1;
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = idata[(y + 5*skip)*istride + x];
      Dp4 = idata[(y + 6*skip)*istride + x];

      Xm1 = Xp1;
    }
    y += 2*skip;

    for (; y < height - 6*skip; y += 2*skip) {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = idata[(y + 5*skip)*istride + x];
      Dp4 = idata[(y + 6*skip)*istride + x];

      Xm1 = Xp1;
    }

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = Dp4;
      Dp3 = idata[(y + 5*skip)*istride + x];
      Dp4 = Dp4;

      Xm1 = Xp1;
    }
    y += 2*skip;

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;

      Dm2 = D;
      D   = Dp2;
      Dp1 = Dp3;
      Dp2 = D;
      Dp3 = Dp3;
      Dp4 = Dm2;

      Xm1 = Xp1;
    }
    y += 2*skip;

    {
      Xp1 = Dp1 - ((-Dm2 + 9*D + 9*Dp2 - Dp4 + 8) >> 4);
      X   = D   + (( Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;
    }
  }
}
