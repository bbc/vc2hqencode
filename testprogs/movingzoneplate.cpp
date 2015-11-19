/*****************************************************************************
 * movingzoneplate.cpp : Generate Moving Zoneplate
 *****************************************************************************
 * Copyright (C) 2014-2015 BBC
 *
 * Authors: Jonathan Rosser <jonathan.rosser@bbc.co.uk>
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
#include <string.h>

static const uint8_t sine_table[256] = {
  128, 131, 134, 137, 140, 143, 146, 149,
  152, 156, 159, 162, 165, 168, 171, 174,
  176, 179, 182, 185, 188, 191, 193, 196,
  199, 201, 204, 206, 209, 211, 213, 216,
  218, 220, 222, 224, 226, 228, 230, 232,
  234, 236, 237, 239, 240, 242, 243, 245,
  246, 247, 248, 249, 250, 251, 252, 252,
  253, 254, 254, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 254, 254,
  253, 252, 252, 251, 250, 249, 248, 247,
  246, 245, 243, 242, 240, 239, 237, 236,
  234, 232, 230, 228, 226, 224, 222, 220,
  218, 216, 213, 211, 209, 206, 204, 201,
  199, 196, 193, 191, 188, 185, 182, 179,
  176, 174, 171, 168, 165, 162, 159, 156,
  152, 149, 146, 143, 140, 137, 134, 131,
  128, 124, 121, 118, 115, 112, 109, 106,
  103, 99, 96, 93, 90, 87, 84, 81,
  79, 76, 73, 70, 67, 64, 62, 59,
  56, 54, 51, 49, 46, 44, 42, 39,
  37, 35, 33, 31, 29, 27, 25, 23,
  21, 19, 18, 16, 15, 13, 12, 10,
  9, 8, 7, 6, 5, 4, 3, 3,
  2, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1,
  2, 3, 3, 4, 5, 6, 7, 8,
  9, 10, 12, 13, 15, 16, 18, 19,
  21, 23, 25, 27, 29, 31, 33, 35,
  37, 39, 42, 44, 46, 49, 51, 54,
  56, 59, 62, 64, 67, 70, 73, 76,
  79, 81, 84, 87, 90, 93, 96, 99,
  103, 106, 109, 112, 115, 118, 121, 124
};

void generateMovingZonePlate(uint8_t *odata, int w, int h, int stride)
{
  static int t=0;

  //parameters
  const int P_xoffset=0;
  const int P_yoffset=0;
  const int P_kx2 = 128;
  const int P_ky2 = (P_kx2 * h) / w;
  const int P_kt = 1;

  int i;
  int j;
  int xreset = -(w / 2) - P_xoffset;   /* starting values for x^2 and y^2, centering the ellipse */
  int yreset = -(h / 2) - P_yoffset;

  int x, y;
  int scale_kx2 = 0xffff / w;

  /* Zoneplate equation:
   *
   * phase = k0 + kx*x + ky*y + kt*t
   *       + kxt*x*t + kyt*y*t + kxy*x*y
   *       + kx2*x*x + ky2*y*y + Kt2*t*t
   */

  t++;

  /* version that only includes second order terms for circles/ellipses */
  const int x_mult = P_kx2 * scale_kx2;
  const int time_varying = P_kt * t;
  int jr;
  for (j=0, y=yreset, jr=h-1; j < h/2; j++, y++, jr--) {
    const int ky2 = (P_ky2 * y * y) / h;
    const int A = ky2 + time_varying;
    int ir;
    unsigned short *line = (unsigned short *)&odata[2*j*stride];
    for (i=0, x = xreset, ir=w-1; i < w/2; i++, x++, ir--) {
      /*second order */
      /*normalise x/y terms to rate of change of phase at the picture edge */
      /*phase = phase + ((v->kx2 * x * x)/w) + ((v->ky2 * y * y)/h) + ((v->kt2 * t * t)>>1); */
      int phase = ((x_mult * x * x) >> 16) + A;
      unsigned short sine = sine_table[phase & 0xff];
      line[i]  = sine << 2;
      line[ir] = sine << 2;
    }
    memcpy(&odata[2*jr*stride], line, w*2);
  }

  for(j=0; j<h; j++) {
    unsigned short *line = (unsigned short *)&odata[w*h*2 + j*stride];
    for (int x = 0; x < w/2; x++) {
      line[x] = 0x200;
    }
  }

  for(j=0; j<h; j++) {
    unsigned short *line = (unsigned short *)&odata[w*h*2 + w*h + j*stride];
    for (int x = 0; x < w/2; x++) {
      line[x] = 0x200;
    }
  }
}
