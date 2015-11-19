/*****************************************************************************
 * legall_transform.hpp : LeGall transform: Plain C++ version
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

template<class T> void LeGall_5_3_transform_H_inplace_V210(const char *_idata,
                                                           const int istride,
                                                           void **_odata,
                                                           const int ostride,
                                                           const int iwidth,
                                                           const int iheight,
                                                           const int owidth,
                                                           const int oheight) {
  T *odata_y = ((T **)_odata)[0];
  T *odata_u = ((T **)_odata)[1];
  T *odata_v = ((T **)_odata)[2];
  const int skip = 1;
  const int shift = 1;

  const uint8_t *idata = (const uint8_t *)_idata;
  const int32_t offset = 1 << 9;
  int32_t U0, U1, U2, U3, U4, U5,
    V0, V1, V2, V3, V4, V5,
    Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7, Y8, Y9, Y10, Y11;
  int32_t YXm1, UXm1, VXm1;
  int32_t Ym2, Um2, Vm2,
    Ym1, Um1, Vm1;

#define LOAD_SAMPLES(Y,X)                                     \
  {                                                           \
    uint32_t D;                                               \
    D = *((uint32_t *)&idata[(Y)*istride + (X) +  0]);        \
    U0  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;   \
    Y0  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;   \
    V0  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;   \
    D = *((uint32_t *)&idata[(Y)*istride + (X) +  4]);        \
    Y1  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;   \
    U1  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;   \
    Y2  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;   \
    D = *((uint32_t *)&idata[(Y)*istride + (X) +  8]);        \
    V1  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;   \
    Y3  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;   \
    U2  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;   \
    D = *((uint32_t *)&idata[(Y)*istride + (X) + 12]);        \
    Y4  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;   \
    V2  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;   \
    Y5  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;   \
    D = *((uint32_t *)&idata[(Y)*istride + (X) + 16]);        \
    U3  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;   \
    Y6  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;   \
    V3  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;   \
    D = *((uint32_t *)&idata[(Y)*istride + (X) + 20]);        \
    Y7  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;   \
    U4  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;   \
    Y8  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;   \
    D = *((uint32_t *)&idata[(Y)*istride + (X) + 24]);        \
    V4  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;   \
    Y9  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;   \
    U5  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;   \
    D = *((uint32_t *)&idata[(Y)*istride + (X) + 28]);        \
    Y10 = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;   \
    V5  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;   \
    Y11 = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;   \
  }

#define TRANSFORM_WRITE(ODATA,D0,D1,D2,XM1,STRIDE,OFFS) \
  {                                                     \
    if ((OFFS) < iwidth) {                              \
      int32_t Xp1 = (D1) - (((D0) + (D2) + 1) >> 1);    \
      int32_t X   = (D0) + (((XM1) + Xp1 + 2) >> 2);    \
                                                        \
      (ODATA)[y*(STRIDE) + (OFFS) + 0*skip] = X;        \
      (ODATA)[y*(STRIDE) + (OFFS) + 1*skip] = Xp1;      \
      (XM1) = Xp1;                                      \
    }                                                   \
  }


  int y = 0;
  for (; y < iheight; y+=skip) {
    int x = 0;

    {
      LOAD_SAMPLES(y, x/12*32);

      {
        int32_t Xp1 = Y1 - ((Y0 + Y2 + 1) >> 1);
        YXm1 = Xp1;
        int32_t X   = Y0 + ((YXm1 + Xp1 + 2) >> 2);

        odata_y[y*ostride + x + 0*skip] = X;
        odata_y[y*ostride + x + 1*skip] = Xp1;
      }
      TRANSFORM_WRITE(odata_y,  Y2,  Y3, Y4,  YXm1, ostride,    x   +  2);
      TRANSFORM_WRITE(odata_y,  Y4,  Y5, Y6,  YXm1, ostride,    x   +  4);
      TRANSFORM_WRITE(odata_y,  Y6,  Y7, Y8,  YXm1, ostride,    x   +  6);
      TRANSFORM_WRITE(odata_y,  Y8,  Y9, Y10, YXm1, ostride,    x   +  8);
      Ym2 = Y10; Ym1 = Y11;

      {
        int32_t Xp1 = U1 - ((U0 + U2 + 1) >> 1);
        UXm1 = Xp1;
        int32_t X   = U0 + ((UXm1 + Xp1 + 2) >> 2);

        odata_u[y*ostride/2 + x/2 + 0*skip] = X;
        odata_u[y*ostride/2 + x/2 + 1*skip] = Xp1;
      }
      TRANSFORM_WRITE(odata_u,  U2,  U3, U4, UXm1, ostride/2,  x/2 +  2);
      Um2 = U4; Um1 = U5;


      {
        int32_t Xp1 = V1 - ((V0 + V2 + 1) >> 1);
        VXm1 = Xp1;
        int32_t X   = V0 + ((VXm1 + Xp1 + 2) >> 2);

        odata_v[y*ostride/2 + x/2 + 0*skip] = X;
        odata_v[y*ostride/2 + x/2 + 1*skip] = Xp1;
      }
      TRANSFORM_WRITE(odata_v,  V2,  V3, V4, VXm1, ostride/2,  x/2 +  2);
      Vm2 = V4; Vm1 = V5;
    }
    x += 12*skip;

    for (; x < (iwidth + 47)/48*48 - 12; x += 12*skip) {
      LOAD_SAMPLES(y, x/12*32);

      TRANSFORM_WRITE(odata_y,  Ym2, Ym1, Y0,  YXm1, ostride,  x   -  2);
      TRANSFORM_WRITE(odata_y,  Y0,  Y1,  Y2,  YXm1, ostride,  x   +  0);
      TRANSFORM_WRITE(odata_y,  Y2,  Y3,  Y4,  YXm1, ostride,  x   +  2);
      TRANSFORM_WRITE(odata_y,  Y4,  Y5,  Y6,  YXm1, ostride,  x   +  4);
      TRANSFORM_WRITE(odata_y,  Y6,  Y7,  Y8,  YXm1, ostride,  x   +  6);
      TRANSFORM_WRITE(odata_y,  Y8,  Y9,  Y10, YXm1, ostride,  x   +  8);
      Ym2 = Y10; Ym1 = Y11;


      TRANSFORM_WRITE(odata_u,  Um2, Um1, U0, UXm1, ostride/2, x/2 -  2);
      TRANSFORM_WRITE(odata_u,  U0,  U1,  U2, UXm1, ostride/2, x/2 +  0);
      TRANSFORM_WRITE(odata_u,  U2,  U3,  U4, UXm1, ostride/2, x/2 +  2);
      Um2 = U4; Um1 = U5;


      TRANSFORM_WRITE(odata_v,  Vm2, Vm1, V0, VXm1, ostride/2, x/2 -  2);
      TRANSFORM_WRITE(odata_v,  V0,  V1,  V2, VXm1, ostride/2, x/2 +  0);
      TRANSFORM_WRITE(odata_v,  V2,  V3,  V4, VXm1, ostride/2, x/2 +  2);
      Vm2 = V4; Vm1 = V5;
    }

    {
      LOAD_SAMPLES(y, x/12*32);

      int32_t Y12 = Y10,
        U6 = U4,
        V6 = V4;

      if (x > iwidth) { Y0 = Ym2; U0 = Um2; V0 = Vm2; }
      else if (x + 4 > iwidth) { Y4 = Y0; U2 = U0; V2 = V0; }
      else if (x + 8 > iwidth) { Y8 = Y4; U4 = U2; V4 = V2; }

      TRANSFORM_WRITE(odata_y,  Ym2, Ym1, Y0,  YXm1, ostride,  x   -  2);
      TRANSFORM_WRITE(odata_y,  Y0,  Y1,  Y2,  YXm1, ostride,  x   +  0);
      TRANSFORM_WRITE(odata_y,  Y2,  Y3,  Y4,  YXm1, ostride,  x   +  2);
      TRANSFORM_WRITE(odata_y,  Y4,  Y5,  Y6,  YXm1, ostride,  x   +  4);
      TRANSFORM_WRITE(odata_y,  Y6,  Y7,  Y8,  YXm1, ostride,  x   +  6);
      TRANSFORM_WRITE(odata_y,  Y8,  Y9,  Y10, YXm1, ostride,  x   +  8);
      TRANSFORM_WRITE(odata_y,  Y10, Y11, Y12, YXm1, ostride,  x   + 10);


      TRANSFORM_WRITE(odata_u,  Um2, Um1, U0, UXm1, ostride/2, x/2 -  2);
      TRANSFORM_WRITE(odata_u,  U0,  U1,  U2, UXm1, ostride/2, x/2 +  0);
      TRANSFORM_WRITE(odata_u,  U2,  U3,  U4, UXm1, ostride/2, x/2 +  2);
      TRANSFORM_WRITE(odata_u,  U4,  U5,  U6, UXm1, ostride/2, x/2 +  4);


      TRANSFORM_WRITE(odata_v,  Vm2, Vm1, V0, VXm1, ostride/2, x/2 -  2);
      TRANSFORM_WRITE(odata_v,  V0,  V1,  V2, VXm1, ostride/2, x/2 +  0);
      TRANSFORM_WRITE(odata_v,  V2,  V3,  V4, VXm1, ostride/2, x/2 +  2);
      TRANSFORM_WRITE(odata_v,  V4,  V5,  V6, VXm1, ostride/2, x/2 +  4);
    }
  }
  for (; y < oheight; y+=skip) {
    memcpy(&odata_y[y*ostride], &odata_y[(2*iheight - y - skip)*ostride], owidth*2);
    memcpy(&odata_u[y*ostride/2], &odata_u[(2*iheight - y - skip)*ostride/2], owidth);
    memcpy(&odata_v[y*ostride/2], &odata_v[(2*iheight - y - skip)*ostride/2], owidth);
  }

#undef LOAD_SAMPLES
#undef TRANSFORM_WRITE
}

template<class T> void LeGall_5_3_transform_H_inplace_10P2(const char *_idata,
                                                            const int istride,
                                                            void **_odata,
                                                            const int ostride,
                                                            const int iwidth,
                                                            const int iheight,
                                                            const int owidth,
                                                            const int oheight) {
  T *odata = *((T **)_odata);
  const int skip = 1;

  (void)iwidth;
  (void)iheight;

  const uint16_t *idata = (const uint16_t *)_idata;
  const int16_t offset = 1 << 9;

  int y;
  for (y = 0; y < iheight; y+=skip) {
    int32_t D, Dp1, Dp2;
    int x = 0;
    int Xm1, X, Xp1;
    D   = (((int32_t)idata[y*istride + x]) - offset) << 1;
    Dp1 = (((int32_t)idata[y*istride + x + 1*skip]) - offset) << 1;
    Dp2 = (((int32_t)idata[y*istride + x + 2*skip]) - offset) << 1;

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      Xm1 = Xp1;
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;

      D   = Dp2;
      Dp1 = (((int32_t)idata[y*istride + x + 3*skip]) - offset) << 1;
      Dp2 = (((int32_t)idata[y*istride + x + 4*skip]) - offset) << 1;
    }
    x += 2*skip;

    for (; x < iwidth - 4*skip; x += 2*skip) {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;

      D   = Dp2;
      Xm1 = Xp1;
      Dp1 = (((int32_t)idata[y*istride + x + 3*skip]) - offset) << 1;
      Dp2 = (((int32_t)idata[y*istride + x + 4*skip]) - offset) << 1;
    }

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;

      D   = Dp2;
      Xm1 = Xp1;
      Dp1 = (((int32_t)idata[y*istride + x + 3*skip]) - offset) << 1;
    }
    x += 2*skip;

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;
    }
    x += 2*skip;

    for (; x < owidth; x += 2*skip) {
      odata[y*ostride + x + 0*skip] = odata[y*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[y*ostride + x + 1*skip] = odata[y*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }
  for (y = iheight; y < oheight; y+=skip) {
    memcpy(&odata[y*ostride], &odata[(2*iheight - y - skip)*ostride], owidth*2);
  }
  (void)oheight;
}

template<int skip, class T> void LeGall_5_3_transform_H_inplace(void *_idata,
                                                                const int istride,
                                                                const int width,
                                                                const int height,
                                                                const int) {
  T *idata = (T *)_idata;
  for (int y = 0; y < height; y+=skip) {
    int32_t D, Dp1, Dp2;
    int x = 0;
    int Xm1, X, Xp1;
    D   = idata[y*istride + x + 0*skip] << 1;
    Dp1 = idata[y*istride + x + 1*skip] << 1;
    Dp2 = idata[y*istride + x + 2*skip] << 1;

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      Xm1 = Xp1;
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      D   = Dp2;
      Dp1 = idata[y*istride + x + 3*skip] << 1;
      Dp2 = idata[y*istride + x + 4*skip] << 1;
    }
    x += 2*skip;

    for (; x < width - 4*skip; x += 2*skip) {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      D   = Dp2;
      Xm1 = Xp1;
      Dp1 = idata[y*istride + x + 3*skip] << 1;
      Dp2 = idata[y*istride + x + 4*skip] << 1;
    }

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      D   = Dp2;
      Xm1 = Xp1;
      Dp1 = idata[y*istride + x + 3*skip] << 1;
    }
    x += 2*skip;

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;
    }
  }
}

template<class T> void LeGall_5_3_transform_H_inplace_dynamic(void *_idata,
                                                              const int istride,
                                                              const int width,
                                                              const int height,
                                                              const int skip) {
  T *idata = (T *)_idata;
  for (int y = 0; y < height; y+=skip) {
    int32_t D, Dp1, Dp2;
    int x = 0;
    int Xm1, X, Xp1;
    D   = idata[y*istride + x + 0*skip] << 1;
    Dp1 = idata[y*istride + x + 1*skip] << 1;
    Dp2 = idata[y*istride + x + 2*skip] << 1;

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      Xm1 = Xp1;
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      D   = Dp2;
      Dp1 = idata[y*istride + x + 3*skip] << 1;
      Dp2 = idata[y*istride + x + 4*skip] << 1;
    }
    x += 2*skip;

    for (; x < width - 4*skip; x += 2*skip) {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      D   = Dp2;
      Xm1 = Xp1;
      Dp1 = idata[y*istride + x + 3*skip] << 1;
      Dp2 = idata[y*istride + x + 4*skip] << 1;
    }

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;

      D   = Dp2;
      Xm1 = Xp1;
      Dp1 = idata[y*istride + x + 3*skip] << 1;
    }
    x += 2*skip;

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;
    }
  }
}

template<int skip, class T>void LeGall_5_3_transform_V_inplace(void *_idata,
                                                               const int istride,
                                                               const int width,
                                                               const int height,
                                                               const int) {
  T *idata = (T *)_idata;
  if (height < 4*skip) {
    for (int x = 0; x < width; x+=skip) {

      int32_t D, Dp1, Dp2;
      int y = 0;
      int32_t Xm1, X, Xp1;
      D   = idata[y*istride + x];
      Dp1 = idata[(y + 1*skip)*istride + x];
      Dp2 = D;

      {
        Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
        Xm1 = Xp1;
        X   = D   + ((Xm1 + Xp1 + 2) >> 2);

        idata[(y + 0*skip)*istride + x] = X;
        idata[(y + 1*skip)*istride + x] = Xp1;
      }
    }
  } else if (height < 6*skip) {
    for (int x = 0; x < width; x+=skip) {

      int32_t D, Dp1, Dp2, Dp3;
      int y = 0;
      int32_t X, Xp1, Xp2, Xp3;
      D   = idata[(y + 0*skip)*istride + x];
      Dp1 = idata[(y + 1*skip)*istride + x];
      Dp2 = idata[(y + 2*skip)*istride + x];
      Dp3 = idata[(y + 3*skip)*istride + x];

      {
        Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
        X   = D   + ((Xp1 + Xp1 + 2) >> 2);
        Xp3 = Dp3 - ((Dp2 + Dp2 + 1) >> 1);
        Xp2 = Dp2 + ((Xp1 + Xp3 + 2) >> 2);

        idata[(y + 0*skip)*istride + x] = X;
        idata[(y + 1*skip)*istride + x] = Xp1;
        idata[(y + 2*skip)*istride + x] = Xp2;
        idata[(y + 3*skip)*istride + x] = Xp3;
      }
    }
  } else {
    for (int x = 0; x < width; x+=skip) {

      int32_t D, Dp1, Dp2;
      int y = 0;
      int32_t Xm1, X, Xp1;
      D   = idata[y*istride + x];
      Dp1 = idata[(y + 1*skip)*istride + x];
      Dp2 = idata[(y + 2*skip)*istride + x];

      {
        Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
        Xm1 = Xp1;
        X   = D   + ((Xm1 + Xp1 + 2) >> 2);

        idata[(y + 0*skip)*istride + x] = X;
        idata[(y + 1*skip)*istride + x] = Xp1;

        Xm1 = Xp1;
        D   = Dp2;
        Dp1 = idata[(y + 3*skip)*istride + x];
        Dp2 = idata[(y + 4*skip)*istride + x];
      }
      y += 2*skip;

      for (; y < height - 4*skip; y += 2*skip) {
        Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
        X   = D   + ((Xm1 + Xp1 + 2) >> 2);

        idata[(y + 0*skip)*istride + x] = X;
        idata[(y + 1*skip)*istride + x] = Xp1;

        Xm1 = Xp1;
        D   = Dp2;
        Dp1 = idata[(y + 3*skip)*istride + x];
        Dp2 = idata[(y + 4*skip)*istride + x];
      }

      {
        Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
        X   = D   + ((Xm1 + Xp1 + 2) >> 2);

        idata[(y + 0*skip)*istride + x] = X;
        idata[(y + 1*skip)*istride + x] = Xp1;

        Xm1 = Xp1;
        D   = Dp2;
        Dp1 = idata[(y + 3*skip)*istride + x];
      }
      y += 2*skip;

      {
        Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
        X   = D   + ((Xm1 + Xp1 + 2) >> 2);

        idata[(y + 0*skip)*istride + x] = X;
        idata[(y + 1*skip)*istride + x] = Xp1;
      }
    }
  }
}

template<class T> void LeGall_5_3_transform_V_inplace_dynamic(void *_idata,
                                                              const int istride,
                                                              const int width,
                                                              const int height,
                                                              const int skip) {
  T *idata = (T *)_idata;
  for (int x = 0; x < width; x+=skip) {

    int32_t D, Dp1, Dp2;
    int y = 0;
    int Xm1, X, Xp1;
    D   = idata[y*istride + x];
    Dp1 = idata[(y + 1*skip)*istride + x];
    Dp2 = idata[(y + 2*skip)*istride + x];

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      Xm1 = Xp1;
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;

      Xm1 = Xp1;
      D   = Dp2;
      Dp1 = idata[(y + 3*skip)*istride + x];
      Dp2 = idata[(y + 4*skip)*istride + x];
    }
    y += 2*skip;

    for (; y < height - 4*skip; y += 2*skip) {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;

      Xm1 = Xp1;
      D   = Dp2;
      Dp1 = idata[(y + 3*skip)*istride + x];
      Dp2 = idata[(y + 4*skip)*istride + x];
    }

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;

      Xm1 = Xp1;
      D   = Dp2;
      Dp1 = idata[(y + 3*skip)*istride + x];
    }
    y += 2*skip;

    {
      Xp1 = Dp1 - ((D   + Dp2 + 1) >> 1);
      X   = D   + ((Xm1 + Xp1 + 2) >> 2);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;
    }
  }
}
