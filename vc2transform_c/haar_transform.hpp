/*****************************************************************************
 * haar_transform.hpp : Haar transform: Plain C++ version
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

template<int shift, class T> void Haar_transform_H_inplace_V210(const char *_idata,
                                                                const int istride,
                                                                void **_odata,
                                                                const int ostride,
                                                                const int iwidth,
                                                                const int iheight,
                                                                const int owidth,
                                                                const int oheight) {
  (void)iwidth;
  (void)iheight;
  T *odata_y = ((T **)_odata)[0];
  T *odata_u = ((T **)_odata)[1];
  T *odata_v = ((T **)_odata)[2];
  const int skip = 1;

  const uint8_t *idata = (const uint8_t *)_idata;
  const int32_t offset = 1 << 9;

  int y = 0;
  for (; y < iheight; y+=skip) {
    int x = 0;
    for (; x < (iwidth + 47)/48*48; x += 12*skip) {
      uint32_t D;
      D = *((uint32_t *)&idata[y*istride + x/12*32 +  0]);
      int32_t U0  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;
      int32_t Y0  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;
      int32_t V0  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;
      D = *((uint32_t *)&idata[y*istride + x/12*32 +  4]);
      int32_t Y1  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;
      int32_t U1  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;
      int32_t Y2  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;
      D = *((uint32_t *)&idata[y*istride + x/12*32 +  8]);
      int32_t V1  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;
      int32_t Y3  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;
      int32_t U2  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;
      D = *((uint32_t *)&idata[y*istride + x/12*32 + 12]);
      int32_t Y4  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;
      int32_t V2  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;
      int32_t Y5  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;
      D = *((uint32_t *)&idata[y*istride + x/12*32 + 16]);
      int32_t U3  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;
      int32_t Y6  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;
      int32_t V3  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;
      D = *((uint32_t *)&idata[y*istride + x/12*32 + 20]);
      int32_t Y7  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;
      int32_t U4  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;
      int32_t Y8  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;
      D = *((uint32_t *)&idata[y*istride + x/12*32 + 24]);
      int32_t V4  = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;
      int32_t Y9  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;
      int32_t U5  = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;
      D = *((uint32_t *)&idata[y*istride + x/12*32 + 28]);
      int32_t Y10 = (((int32_t)((D >>  0)&0x3ff)) - offset) << shift;
      int32_t V5  = (((int32_t)((D >> 10)&0x3ff)) - offset) << shift;
      int32_t Y11 = (((int32_t)((D >> 20)&0x3ff)) - offset) << shift;

#define TRANSFORM_WRITE(ODATA,D0,D1,STRIDE,OFFS) \
      { \
        int32_t Xp1 = (D1) - (D0);             \
        int32_t X   = (D0) + ((Xp1 + 1) >> 1);  \
\
        if ((OFFS) < iwidth) { \
          (ODATA)[y*(STRIDE) + (OFFS) + 0*skip] = X;       \
          (ODATA)[y*(STRIDE) + (OFFS) + 1*skip] = Xp1;   \
        } \
      }

      TRANSFORM_WRITE(odata_y,  Y0,  Y1, ostride,    x   +  0);
      TRANSFORM_WRITE(odata_y,  Y2,  Y3, ostride,    x   +  2);
      TRANSFORM_WRITE(odata_y,  Y4,  Y5, ostride,    x   +  4);
      TRANSFORM_WRITE(odata_y,  Y6,  Y7, ostride,    x   +  6);
      TRANSFORM_WRITE(odata_y,  Y8,  Y9, ostride,    x   +  8);
      TRANSFORM_WRITE(odata_y, Y10, Y11, ostride,    x   + 10);

      TRANSFORM_WRITE(odata_u,  U0,  U1, ostride/2,  x/2 +  0);
      TRANSFORM_WRITE(odata_u,  U2,  U3, ostride/2,  x/2 +  2);
      TRANSFORM_WRITE(odata_u,  U4,  U5, ostride/2,  x/2 +  4);

      TRANSFORM_WRITE(odata_v,  V0,  V1, ostride/2,  x/2 +  0);
      TRANSFORM_WRITE(odata_v,  V2,  V3, ostride/2,  x/2 +  2);
      TRANSFORM_WRITE(odata_v,  V4,  V5, ostride/2,  x/2 +  4);

#undef TRANSFORM_WRITE
    }
    for (x = iwidth; x < owidth; x += 4*skip) {
      odata_y[y*ostride + x + 0*skip] = odata_y[y*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata_y[y*ostride + x + 1*skip] = odata_y[y*ostride + (2*iwidth - x - 2*skip) + 1*skip];
      odata_y[y*ostride + x + 2*skip] = odata_y[y*ostride + (2*iwidth - x - 4*skip) + 0*skip];
      odata_y[y*ostride + x + 3*skip] = odata_y[y*ostride + (2*iwidth - x - 4*skip) + 1*skip];

      odata_u[y*ostride/2 + x/2 + 0*skip] = odata_u[y*ostride/2 + (2*iwidth - x - 2*skip)/2 + 0*skip];
      odata_u[y*ostride/2 + x/2 + 1*skip] = odata_u[y*ostride/2 + (2*iwidth - x - 2*skip)/2 + 1*skip];

      odata_v[y*ostride/2 + x/2 + 0*skip] = odata_v[y*ostride/2 + (2*iwidth - x - 2*skip)/2 + 0*skip];
      odata_v[y*ostride/2 + x/2 + 1*skip] = odata_v[y*ostride/2 + (2*iwidth - x - 2*skip)/2 + 1*skip];
    }
  }
  for (; y < oheight; y+=skip) {
    memcpy(&odata_y[y*ostride], &odata_y[(2*iheight - y - skip)*ostride], owidth*2);
    memcpy(&odata_u[y*ostride/2], &odata_u[(2*iheight - y - skip)*ostride/2], owidth);
    memcpy(&odata_v[y*ostride/2], &odata_v[(2*iheight - y - skip)*ostride/2], owidth);
  }
}

template<int shift, class T> void Haar_transform_H_inplace_10P2(const char *_idata,
                                                                 const int istride,
                                                                 void **_odata,
                                                                 const int ostride,
                                                                 const int iwidth,
                                                                 const int iheight,
                                                                 const int owidth,
                                                                 const int oheight) {
  (void)iwidth;
  (void)iheight;
  T *odata = *((T **)_odata);
  const int skip = 1;

  const uint16_t *idata = (const uint16_t *)_idata;
  const int32_t offset = 1 << 9;

  int y = 0;
  for (; y < iheight; y+=skip) {
    int x = 0;
    for (; x < iwidth; x += 2*skip) {
      int32_t D   = (((int32_t)idata[y*istride + x]) - offset) << shift;
      int32_t Dp1 = (((int32_t)idata[y*istride + x + 1*skip]) - offset) << shift;

      int32_t Xp1 = Dp1 - D;
      int32_t X   = D   + ((Xp1 + 1) >> 1);

      odata[y*ostride + x + 0*skip] = X;
      odata[y*ostride + x + 1*skip] = Xp1;
    }
    for (; x < owidth; x += 2*skip) {
      odata[y*ostride + x + 0*skip] = odata[y*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[y*ostride + x + 1*skip] = odata[y*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }
  for (; y < oheight; y+=skip) {
    memcpy(&odata[y*ostride], &odata[(2*iheight - y - skip)*ostride], owidth*2);
  }
}

template<int skip, int shift, class T> void Haar_transform_H_inplace(void *_idata,
                                                                     const int istride,
                                                                     const int width,
                                                                     const int height,
                                                                     const int) {
  T *idata = (T *)_idata;
  for (int y = 0; y < height; y+=skip) {
    for (int x = 0; x < width; x += 2*skip) {
      int32_t D   = idata[y*istride + x + 0*skip] << shift;
      int32_t Dp1 = idata[y*istride + x + 1*skip] << shift;

      int32_t Xp1 = Dp1 - D;
      int32_t X   = D   + ((Xp1 + 1) >> 1);

      idata[y*istride + x + 0*skip] = X;
      idata[y*istride + x + 1*skip] = Xp1;
    }
  }
}

template<int skip, class T>void Haar_transform_V_inplace(void *_idata,
                                                         const int istride,
                                                         const int width,
                                                         const int height,
                                                         const int) {
  T *idata = (T *)_idata;
  for (int x = 0; x < width; x+=skip) {
    for (int y = 0; y < height; y += 2*skip) {
      int32_t D   = idata[(y + 0*skip)*istride + x];
      int32_t Dp1 = idata[(y + 1*skip)*istride + x];

      int32_t Xp1 = Dp1 - D;
      int32_t X   = D   + ((Xp1 + 1) >> 1);

      idata[(y + 0*skip)*istride + x] = X;
      idata[(y + 1*skip)*istride + x] = Xp1;
    }
  }
}
