/*****************************************************************************
 * fidelity_transform.hpp : VC-2 Fidelity transform: Plain C++ version
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

void Fidelity_transform_H_inplace_10bit(const char *_idata,
                                        const int istride,
                                        int16_t *odata,
                                        const int ostride,
                                        const int width,
                                        const int height) {
  const int skip = 1;

  const uint16_t *idata = (const uint16_t *)_idata;
  const int16_t offset = 1 << 9;

  for (int y = 0; y < height; y+=skip) {
    int x = 0;
    int32_t Do[8]; // x-7, x-5, x-3, x-1, x+1, x+3, x+5, x+7
    int32_t De[8]; // x-8, x-6, x-4, x-2, x+0, x+2, x+4, x+8
    int32_t Xm7;
    int32_t Xe[8]; // x-14, x-12, x-10, x-8, x-6, x-4, x-2, x

    for (int i = 0; i < 4; i++) {
      De[3 - i] = De[4 + i] = (((int32_t)idata[y*istride + x + (2*i + 0)*skip]) - offset) << 1;
      Do[3 - i] = Do[4 + i] = (((int32_t)idata[y*istride + x + (2*i + 1)*skip]) - offset) << 1;
    }

    {
      Xe[7 - 1] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = (((int32_t)idata[y*istride + x + 8*skip]) - offset) << 1;
      Do[7] = (((int32_t)idata[y*istride + x + 9*skip]) - offset) << 1;

      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    x += 2*skip;

    {
      Xe[7 - 3] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = (((int32_t)idata[y*istride + x + 8*skip]) - offset) << 1;
      Do[7] = (((int32_t)idata[y*istride + x + 9*skip]) - offset) << 1;

      Xe[7 - 4] = Xe[7 - 3];
      Xe[7 - 3] = Xe[7 - 2];
      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    x += 2*skip;

    {
      Xe[7 - 5] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = (((int32_t)idata[y*istride + x + 8*skip]) - offset) << 1;
      Do[7] = (((int32_t)idata[y*istride + x + 9*skip]) - offset) << 1;

      Xe[7 - 6] = Xe[7 - 5];
      Xe[7 - 5] = Xe[7 - 4];
      Xe[7 - 4] = Xe[7 - 3];
      Xe[7 - 3] = Xe[7 - 2];
      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    x += 2*skip;

    {
      Xe[7 - 7] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = (((int32_t)idata[y*istride + x + 8*skip]) - offset) << 1;
      Do[7] = (((int32_t)idata[y*istride + x + 9*skip]) - offset) << 1;

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    x += 2*skip;

    for (; x < width - 8*skip; x += 2*skip) {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = (((int32_t)idata[y*istride + x + 8*skip]) - offset) << 1;
      Do[7] = (((int32_t)idata[y*istride + x + 9*skip]) - offset) << 1;

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = De[6];
      Do[7] = De[6];

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    x += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;

      for (int i = 0; i < 6; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      for (int i = 6; i < 8; i++) {
        De[i] = De[11 - i];
        Do[i] = Do[11 - i];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    x += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;

      for (int i = 0; i < 5; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      for (int i = 5; i < 8; i++) {
        De[i] = De[9 - i];
        Do[i] = Do[9 - i];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    x += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;

      for (int i = 0; i < 4; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
      Xe[7] = Xe[6];
    }
    x += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;

      for (int i = 0; i < 3; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 6; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 6; i < 8; i++) {
        Xe[i] = Xe[11 - i];
      }
    }
    x += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;

      for (int i = 0; i < 2; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 5; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 5; i < 8; i++) {
        Xe[i] = Xe[9 - i];
      }
    }
    x += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;

      Do[0] = Do[1];

      for (int i = 0; i < 4; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 4; i < 8; i++) {
        Xe[i] = Xe[7 - i];
      }
    }
    x += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      odata[y*ostride + x - 8*skip] = Xe[3];
      odata[y*ostride + x - 7*skip] = Xm7;
    }
  }
}

template<int skip> void Fidelity_transform_H_inplace(int16_t *idata,
                                                     const int istride,
                                                     const int width,
                                                     const int height,
                                                     const int) {
  for (int y = 0; y < height; y+=skip) {
    int x = 0;
    int32_t Do[8]; // x-7, x-5, x-3, x-1, x+1, x+3, x+5, x+7
    int32_t De[8]; // x-8, x-6, x-4, x-2, x+0, x+2, x+4, x+8
    int32_t Xm7;
    int32_t Xe[8]; // x-14, x-12, x-10, x-8, x-6, x-4, x-2, x

    for (int i = 0; i < 4; i++) {
      De[3 - i] = De[4 + i] = idata[y*istride + x + (2*i + 0)*skip] << 1;
      Do[3 - i] = Do[4 + i] = idata[y*istride + x + (2*i + 1)*skip] << 1;
    }

    {
      Xe[7 - 1] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[y*istride + x + 8*skip] << 1;
      Do[7] = idata[y*istride + x + 9*skip] << 1;

      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    x += 2*skip;

    {
      Xe[7 - 3] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[y*istride + x + 8*skip] << 1;
      Do[7] = idata[y*istride + x + 9*skip] << 1;

      Xe[7 - 4] = Xe[7 - 3];
      Xe[7 - 3] = Xe[7 - 2];
      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    x += 2*skip;

    {
      Xe[7 - 5] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[y*istride + x + 8*skip] << 1;
      Do[7] = idata[y*istride + x + 9*skip] << 1;

      Xe[7 - 6] = Xe[7 - 5];
      Xe[7 - 5] = Xe[7 - 4];
      Xe[7 - 4] = Xe[7 - 3];
      Xe[7 - 3] = Xe[7 - 2];
      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    x += 2*skip;

    {
      Xe[7 - 7] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[y*istride + x + 8*skip] << 1;
      Do[7] = idata[y*istride + x + 9*skip] << 1;

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    x += 2*skip;

    for (; x < width - 8*skip; x += 2*skip) {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[y*istride + x + 8*skip] << 1;
      Do[7] = idata[y*istride + x + 9*skip] << 1;

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = De[6];
      Do[7] = De[6];

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    x += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;

      for (int i = 0; i < 6; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      for (int i = 6; i < 8; i++) {
        De[i] = De[11 - i];
        Do[i] = Do[11 - i];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    x += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;

      for (int i = 0; i < 5; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      for (int i = 5; i < 8; i++) {
        De[i] = De[9 - i];
        Do[i] = Do[9 - i];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    x += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;

      for (int i = 0; i < 4; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
      Xe[7] = Xe[6];
    }
    x += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;

      for (int i = 0; i < 3; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 6; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 6; i < 8; i++) {
        Xe[i] = Xe[11 - i];
      }
    }
    x += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;

      for (int i = 0; i < 2; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 5; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 5; i < 8; i++) {
        Xe[i] = Xe[9 - i];
      }
    }
    x += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;

      Do[0] = Do[1];

      for (int i = 0; i < 4; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 4; i < 8; i++) {
        Xe[i] = Xe[7 - i];
      }
    }
    x += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[y*istride + x - 8*skip] = Xe[3];
      idata[y*istride + x - 7*skip] = Xm7;
    }
  }
}

template<int skip>void Fidelity_transform_V_inplace(int16_t *idata,
                                                      const int istride,
                                                      const int width,
                                                      const int height,
                                                      const int) {
  for (int x = 0; x < width; x+=skip) {
    int y = 0;
    int32_t Do[8]; // y-7, y-5, y-3, y-1, y+1, y+3, y+5, y+7
    int32_t De[8]; // y-8, y-6, y-4, y-2, y+0, y+2, y+4, y+8
    int32_t Xm7;
    int32_t Xe[8]; // y-14, y-12, y-10, y-8, y-6, y-4, y-2, y

    for (int i = 0; i < 4; i++) {
      De[3 - i] = De[4 + i] = idata[(y + (2*i + 0)*skip)*istride + x];
      Do[3 - i] = Do[4 + i] = idata[(y + (2*i + 1)*skip)*istride + x];
    }

    {
      Xe[7 - 1] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[(y + 8*skip)*istride + x];
      Do[7] = idata[(y + 9*skip)*istride + x];

      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    y += 2*skip;

    {
      Xe[7 - 3] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[(y + 8*skip)*istride + x];
      Do[7] = idata[(y + 9*skip)*istride + x];

      Xe[7 - 4] = Xe[7 - 3];
      Xe[7 - 3] = Xe[7 - 2];
      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    y += 2*skip;

    {
      Xe[7 - 5] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);
      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[(y + 8*skip)*istride + x];
      Do[7] = idata[(y + 9*skip)*istride + x];

      Xe[7 - 6] = Xe[7 - 5];
      Xe[7 - 5] = Xe[7 - 4];
      Xe[7 - 4] = Xe[7 - 3];
      Xe[7 - 3] = Xe[7 - 2];
      Xe[7 - 2] = Xe[7 - 1];
      Xe[7 - 1] = Xe[7 - 0];
    }
    y += 2*skip;

    {
      Xe[7 - 7] = Xe[7 - 0] = De[4] + (( -8*(  Do[0] + Do[7])
                                         +21*( Do[1] + Do[6])
                                         -46*( Do[2] + Do[5])
                                         +161*(Do[3] + Do[4]) + 128) >> 8);

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[(y + 8*skip)*istride + x];
      Do[7] = idata[(y + 9*skip)*istride + x];

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    y += 2*skip;

    for (; y < height - 8*skip; y += 2*skip) {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = idata[(y + 8*skip)*istride + x];
      Do[7] = idata[(y + 9*skip)*istride + x];

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;

      for (int i = 0; i < 7; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      De[7] = De[6];
      Do[7] = De[6];

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    y += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;

      for (int i = 0; i < 6; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      for (int i = 6; i < 8; i++) {
        De[i] = De[11 - i];
        Do[i] = Do[11 - i];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    y += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;

      for (int i = 0; i < 5; i++) {
        De[i] = De[i+1];
        Do[i] = Do[i+1];
      }
      for (int i = 5; i < 8; i++) {
        De[i] = De[9 - i];
        Do[i] = Do[9 - i];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
    }
    y += 2*skip;

    {
      Xe[7] = De[4] + (( -8*(  Do[0] + Do[7])
                         +21*( Do[1] + Do[6])
                         -46*( Do[2] + Do[5])
                         +161*(Do[3] + Do[4]) + 128) >> 8);
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;

      for (int i = 0; i < 4; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 7; i++) {
        Xe[i] = Xe[i+1];
      }
      Xe[7] = Xe[6];
    }
    y += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;

      for (int i = 0; i < 3; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 6; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 6; i < 8; i++) {
        Xe[i] = Xe[11 - i];
      }
    }
    y += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;

      for (int i = 0; i < 2; i++) {
        Do[i] = Do[i+1];
      }

      for (int i = 0; i < 5; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 5; i < 8; i++) {
        Xe[i] = Xe[9 - i];
      }
    }
    y += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;

      Do[0] = Do[1];

      for (int i = 0; i < 4; i++) {
        Xe[i] = Xe[i+1];
      }
      for (int i = 4; i < 8; i++) {
        Xe[i] = Xe[7 - i];
      }
    }
    y += 2*skip;

    {
      Xm7 = Do[0] + (( -2*( Xe[0] + Xe[7])
                       +10*(Xe[1] + Xe[6])
                       -25*(Xe[2] + Xe[5])
                       +81*(Xe[3] + Xe[4]) + 128) >> 8);

      idata[(y - 8*skip)*istride + x] = Xe[3];
      idata[(y - 7*skip)*istride + x] = Xm7;
    }
  }
}
