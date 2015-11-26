/*****************************************************************************
 * legall_transform.hpp : LeGall transform: SSE4.2 version
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

void LeGall_5_3_transform_H_inplace_V210_sse4_2(const char *_idata,
                                                const int istride,
                                                void **_odata,
                                                const int ostride,
                                                const int iwidth,
                                                const int iheight,
                                                const int owidth,
                                                const int oheight) {
  int16_t *odata_y = ((int16_t **)_odata)[0];
  int16_t *odata_u = ((int16_t **)_odata)[1];
  int16_t *odata_v = ((int16_t **)_odata)[2];
  const int skip = 1;

  const uint8_t *idata = (const uint8_t *)_idata;
  const __m128i ONE    = _mm_set1_epi16(1);
  const __m128i TWO    = _mm_set1_epi16(2);

  const __m128i V210_LUMA_MASK = _mm_set_epi32(0x3FF003FF, 0x000FFC00, 0x3FF003FF, 0x000FFC00);
  const __m128i V210_LUMA_SHUF = _mm_set_epi8(0xFF, 0xFF, 15, 14, 10, 9, 5, 4,
                                              0, -1, 13, 12,  7, 6, 2, 1);
  const __m128i V210_LUMA_SHIFT = _mm_set_epi16(1 << 13, 1 << 11, 1 << 13, 0x7FFF,
                                                1 << 11, 0x7FFF, 1 << 11, 1 << 13);
  const __m128i V210_CHROMA_SHUF = _mm_set_epi8(0xFF, 0xFF, 9, 8, 14, 13, 3, 2,
                                                0xFF, 0xFF, 6, 5, 11, 10, 1, 0);
  const __m128i V210_CHROMA_SHIFT = _mm_set_epi16(0, 0x7FFF, 1 << 13, 1 << 11,
                                                  0, 1 << 13, 1 << 11, 0x7FFF);

  int y = 0;
  for (; y < iheight; y+=skip) {
    {
      int x = 0;
      int X = 0;
      __m128i YE_n;
      __m128i YO_n;
      __m128i Xm1;
      {
        __m128i D0  = _mm_loadu_si128((__m128i *)&idata[y*istride + X +  0]);
        __m128i D6  = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 16]);
        __m128i D12 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 32]);
        __m128i D18 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 48]);
        {
          __m128i Y0 = _mm_and_si128(V210_LUMA_MASK, D0);
          __m128i Y6 = _mm_and_si128(V210_LUMA_MASK, D6);

          Y0 = _mm_shuffle_epi8(Y0, V210_LUMA_SHUF);
          Y6 = _mm_shuffle_epi8(Y6, V210_LUMA_SHUF);

          Y0 = _mm_mulhrs_epi16(Y0, V210_LUMA_SHIFT);
          Y6 = _mm_mulhrs_epi16(Y6, V210_LUMA_SHIFT);

          __m128i YE = _mm_unpacklo_epi64(Y0, Y6);
          __m128i YO = _mm_unpackhi_epi64(Y0, Y6);

          YE = _mm_sub_epi16(YE, _mm_slli_epi16(ONE, 9));
          YO = _mm_sub_epi16(YO, _mm_slli_epi16(ONE, 9));

          YE = _mm_slli_epi16(YE, 1);
          YO = _mm_slli_epi16(YO, 1);

          __m128i Y12 = _mm_and_si128(V210_LUMA_MASK, D12);
          __m128i Y18 = _mm_and_si128(V210_LUMA_MASK, D18);

          Y12 = _mm_shuffle_epi8(Y12, V210_LUMA_SHUF);
          Y18 = _mm_shuffle_epi8(Y18, V210_LUMA_SHUF);

          Y12 = _mm_mulhrs_epi16(Y12, V210_LUMA_SHIFT);
          Y18 = _mm_mulhrs_epi16(Y18, V210_LUMA_SHIFT);

          YE_n = _mm_unpacklo_epi64(Y12, Y18);
          YO_n = _mm_unpackhi_epi64(Y12, Y18);

          YE_n = _mm_sub_epi16(YE_n, _mm_slli_epi16(ONE, 9));
          YO_n = _mm_sub_epi16(YO_n, _mm_slli_epi16(ONE, 9));

          YE_n = _mm_slli_epi16(YE_n, 1);
          YO_n = _mm_slli_epi16(YO_n, 1);

          __m128i YE2 = _mm_alignr_epi8(YE_n, YE, 2);
          YE2 = _mm_shufflelo_epi16(YE2, 0xB4);
          YE2 = _mm_shufflehi_epi16(YE2, 0xB4);

          __m128i X1 = _mm_sub_epi16(YO, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(YE, YE2), ONE), 1));
          Xm1 = _mm_slli_si128(X1, 4);
          Xm1 = _mm_shufflelo_epi16(Xm1, 0x3A);
          Xm1 = _mm_shufflehi_epi16(Xm1, 0x38);
          __m128i X0 = _mm_add_epi16(YE, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(X1, Xm1),  TWO), 2));

          __m128i Z0 = _mm_unpacklo_epi16(X0, X1);
          __m128i Z6 = _mm_unpackhi_epi16(X0, X1);

          _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 0], Z0);
          _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 6], Z6);

          Xm1 = X1;
        }
      }
      x += 12;
      X += 32;

      for (; x < (iwidth + 47)/48*48 - 12; x += 12, X += 32) {
        __m128i D12 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 32]);
        __m128i D18 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 48]);
        {
          __m128i YE = YE_n;
          __m128i YO = YO_n;

          __m128i Y12 = _mm_and_si128(V210_LUMA_MASK, D12);
          __m128i Y18 = _mm_and_si128(V210_LUMA_MASK, D18);

          Y12 = _mm_shuffle_epi8(Y12, V210_LUMA_SHUF);
          Y18 = _mm_shuffle_epi8(Y18, V210_LUMA_SHUF);

          Y12 = _mm_mulhrs_epi16(Y12, V210_LUMA_SHIFT);
          Y18 = _mm_mulhrs_epi16(Y18, V210_LUMA_SHIFT);

          YE_n = _mm_unpacklo_epi64(Y12, Y18);
          YO_n = _mm_unpackhi_epi64(Y12, Y18);

          YE_n = _mm_sub_epi16(YE_n, _mm_slli_epi16(ONE, 9));
          YO_n = _mm_sub_epi16(YO_n, _mm_slli_epi16(ONE, 9));

          YE_n = _mm_slli_epi16(YE_n, 1);
          YO_n = _mm_slli_epi16(YO_n, 1);


          __m128i YE2 = _mm_alignr_epi8(YE_n, YE, 2);
          YE2 = _mm_shufflelo_epi16(YE2, 0xB4);
          YE2 = _mm_shufflehi_epi16(YE2, 0xB4);

          __m128i X1 = _mm_sub_epi16(YO, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(YE, YE2), ONE), 1));
          Xm1 = _mm_alignr_epi8(X1, Xm1, 12);
          Xm1 = _mm_shufflelo_epi16(Xm1, 0x38);
          Xm1 = _mm_shufflehi_epi16(Xm1, 0x38);
          __m128i X0 = _mm_add_epi16(YE, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(X1, Xm1),  TWO), 2));

          __m128i Z0 = _mm_unpacklo_epi16(X0, X1);
          __m128i Z6 = _mm_unpackhi_epi16(X0, X1);

          _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 0], Z0);
          _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 6], Z6);

          Xm1 = X1;
        }
      }

      {
        {
          __m128i YE = YE_n;
          __m128i YO = YO_n;

          __m128i YE2 = _mm_srli_si128(YE, 2);
          YE2 = _mm_shufflelo_epi16(YE2, 0xB4);
          YE2 = _mm_shufflehi_epi16(YE2, 0xD4);

          __m128i X1 = _mm_sub_epi16(YO, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(YE, YE2), ONE), 1));
          Xm1 = _mm_alignr_epi8(X1, Xm1, 12);
          Xm1 = _mm_shufflelo_epi16(Xm1, 0x38);
          Xm1 = _mm_shufflehi_epi16(Xm1, 0x38);
          __m128i X0 = _mm_add_epi16(YE, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(X1, Xm1),  TWO), 2));

          __m128i Z0 = _mm_unpacklo_epi16(X0, X1);
          __m128i Z6 = _mm_unpackhi_epi16(X0, X1);

          _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 0], Z0);
          _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 6], Z6);
        }
      }
      for (x = iwidth; x < owidth; x += 2*skip) {
        odata_y[(y + 0)*ostride + x + 0*skip] = odata_y[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
        odata_y[(y + 0)*ostride + x + 1*skip] = odata_y[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
      }
    }

    {
      int x = 0;
      int X = 0;
      __m128i CE_n;
      __m128i CO_n;
      __m128i Xm1;
      {
        __m128i D0  = _mm_loadu_si128((__m128i *)&idata[y*istride + X +  0]);
        __m128i D6  = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 16]);
        __m128i D12 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 32]);
        __m128i D18 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 48]);
        {
          __m128i C0 = _mm_andnot_si128(V210_LUMA_MASK, D0);
          __m128i C6 = _mm_andnot_si128(V210_LUMA_MASK, D6);

          C0 = _mm_shuffle_epi8(C0, V210_CHROMA_SHUF);
          C6 = _mm_shuffle_epi8(C6, V210_CHROMA_SHUF);

          C0 = _mm_mulhrs_epi16(C0, V210_CHROMA_SHIFT);
          C6 = _mm_mulhrs_epi16(C6, V210_CHROMA_SHIFT);

          __m128i CE = _mm_blend_epi16(C0, C6, 0x44);
          __m128i CO = _mm_blend_epi16(C0, C6, 0x33);
          CO = _mm_shufflelo_epi16(CO, 0xD2);
          CO = _mm_shufflehi_epi16(CO, 0xD2);

          CE = _mm_sub_epi16(CE, _mm_slli_epi16(ONE, 9));
          CO = _mm_sub_epi16(CO, _mm_slli_epi16(ONE, 9));

          CE = _mm_slli_epi16(CE, 1);
          CO = _mm_slli_epi16(CO, 1);

          __m128i C12 = _mm_andnot_si128(V210_LUMA_MASK, D12);
          __m128i C18 = _mm_andnot_si128(V210_LUMA_MASK, D18);

          C12 = _mm_shuffle_epi8(C12, V210_CHROMA_SHUF);
          C18 = _mm_shuffle_epi8(C18, V210_CHROMA_SHUF);

          C12 = _mm_mulhrs_epi16(C12, V210_CHROMA_SHIFT);
          C18 = _mm_mulhrs_epi16(C18, V210_CHROMA_SHIFT);

          CE_n = _mm_blend_epi16(C12, C18, 0x44);
          CO_n = _mm_blend_epi16(C12, C18, 0x33);
          CO_n = _mm_shufflelo_epi16(CO_n, 0xD2);
          CO_n = _mm_shufflehi_epi16(CO_n, 0xD2);

          CE_n = _mm_sub_epi16(CE_n, _mm_slli_epi16(ONE, 9));
          CO_n = _mm_sub_epi16(CO_n, _mm_slli_epi16(ONE, 9));

          CE_n = _mm_slli_epi16(CE_n, 1);
          CO_n = _mm_slli_epi16(CO_n, 1);

          __m128i CE2 = _mm_blend_epi16(_mm_srli_si128(CE, 2), _mm_slli_si128(CE_n, 4), 0x44);

          __m128i X1 = _mm_sub_epi16(CO, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(CE, CE2), ONE), 1));
          Xm1 = _mm_slli_si128(X1, 4);
          Xm1 = _mm_shufflelo_epi16(Xm1, 0x3A);
          Xm1 = _mm_shufflehi_epi16(Xm1, 0x38);
          __m128i X0 = _mm_add_epi16(CE, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(X1, Xm1),  TWO), 2));

          __m128i ZU = _mm_unpacklo_epi16(X0, X1);
          __m128i ZV = _mm_unpackhi_epi16(X0, X1);

          _mm_storeu_si128((__m128i *)&odata_u[y*ostride/2 + x/2], ZU);
          _mm_storeu_si128((__m128i *)&odata_v[y*ostride/2 + x/2], ZV);

          Xm1 = X1;
        }
      }
      x += 12;
      X += 32;

      for (; x < (iwidth + 47)/48*48 - 12; x += 12, X += 32) {
        __m128i D12 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 32]);
        __m128i D18 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 48]);
        {
          __m128i CE = CE_n;
          __m128i CO = CO_n;

          __m128i C12 = _mm_andnot_si128(V210_LUMA_MASK, D12);
          __m128i C18 = _mm_andnot_si128(V210_LUMA_MASK, D18);

          C12 = _mm_shuffle_epi8(C12, V210_CHROMA_SHUF);
          C18 = _mm_shuffle_epi8(C18, V210_CHROMA_SHUF);

          C12 = _mm_mulhrs_epi16(C12, V210_CHROMA_SHIFT);
          C18 = _mm_mulhrs_epi16(C18, V210_CHROMA_SHIFT);

          CE_n = _mm_blend_epi16(C12, C18, 0x44);
          CO_n = _mm_blend_epi16(C12, C18, 0x33);
          CO_n = _mm_shufflelo_epi16(CO_n, 0xD2);
          CO_n = _mm_shufflehi_epi16(CO_n, 0xD2);

          CE_n = _mm_sub_epi16(CE_n, _mm_slli_epi16(ONE, 9));
          CO_n = _mm_sub_epi16(CO_n, _mm_slli_epi16(ONE, 9));

          CE_n = _mm_slli_epi16(CE_n, 1);
          CO_n = _mm_slli_epi16(CO_n, 1);

          __m128i CE2 = _mm_blend_epi16(_mm_srli_si128(CE, 2), _mm_slli_si128(CE_n, 4), 0x44);

          __m128i X1 = _mm_sub_epi16(CO, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(CE, CE2), ONE), 1));
          Xm1 = _mm_blend_epi16(_mm_slli_si128(X1, 2), _mm_srli_si128(Xm1, 4), 0x11);
          __m128i X0 = _mm_add_epi16(CE, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(X1, Xm1),  ONE), 1));

          __m128i ZU = _mm_unpacklo_epi16(X0, X1);
          __m128i ZV = _mm_unpackhi_epi16(X0, X1);

          _mm_storeu_si128((__m128i *)&odata_u[y*ostride/2 + x/2], ZU);
          _mm_storeu_si128((__m128i *)&odata_v[y*ostride/2 + x/2], ZV);

          Xm1 = X1;
        }
      }

      {
        {
          __m128i CE = CE_n;
          __m128i CO = CO_n;

          __m128i CE2 = _mm_srli_si128(CE, 2);
          CE2 = _mm_shufflelo_epi16(CE2, 0x14);
          CE2 = _mm_shufflehi_epi16(CE2, 0x14);

          __m128i X1 = _mm_sub_epi16(CO, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(CE, CE2), ONE), 1));
          Xm1 = _mm_blend_epi16(_mm_slli_si128(X1, 2), _mm_srli_si128(Xm1, 4), 0x11);
          __m128i X0 = _mm_add_epi16(CE, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(X1, Xm1),  ONE), 1));

          __m128i ZU = _mm_unpacklo_epi16(X0, X1);
          __m128i ZV = _mm_unpackhi_epi16(X0, X1);

          _mm_storeu_si128((__m128i *)&odata_u[y*ostride/2 + x/2], ZU);
          _mm_storeu_si128((__m128i *)&odata_v[y*ostride/2 + x/2], ZV);
        }
      }
      for (x = iwidth; x < owidth; x += 2*skip) {
        odata_u[(y + 0)*ostride/2 + x/2] = odata_u[(y + 0)*ostride/2 + (2*iwidth/2 - x/2 - 2*skip)];
        odata_v[(y + 0)*ostride/2 + x/2] = odata_v[(y + 0)*ostride/2 + (2*iwidth/2 - x/2 - 2*skip)];
      }
    }
  }
  for (; y < oheight; y+=skip) {
    memcpy(&odata_y[(y + 0)*ostride],   &odata_y[(2*iheight - y - skip)*ostride],   2*owidth);
    memcpy(&odata_u[(y + 0)*ostride/2], &odata_u[(2*iheight - y - skip)*ostride/2],   owidth);
    memcpy(&odata_v[(y + 0)*ostride/2], &odata_v[(2*iheight - y - skip)*ostride/2],   owidth);
  }
}


void LeGall_5_3_transform_H_inplace_10P2_sse4_2(const char *idata,
                                                 const int istride,
                                                 void **_odata,
                                                 const int ostride,
                                                 const int iwidth,
                                                 const int iheight,
                                                 const int owidth,
                                                 const int oheight) {
  int16_t *odata = *((int16_t **)_odata);
  const int skip = 1;

  const __m128i offset = _mm_set1_epi16(1 << 9);
  const __m128i ONE = _mm_set1_epi16(1);
  const __m128i TWO = _mm_set1_epi16(2);

  int y = 0;
  for (; y < iheight; y+=skip) {
    int x = 0;

    __m128i D0, D8, D16, D24,
      Yz, Ym, Y0, Y1,
      Z0, Z4,
      A0, A4, B0, B2,
      A16, A20, B16, B22,
      E0, E2, E16,
      O1, O17;

    D0 = _mm_sub_epi16(_mm_loadu_si128((__m128i *)&idata[(y*istride + x + 0)*2]), offset); // [  0  1  2  3  4  5  6  7 ]
    D8 = _mm_sub_epi16(_mm_loadu_si128((__m128i *)&idata[(y*istride + x + 8)*2]), offset); // [  8  9 10 11 12 13 14 15 ]
    _mm_prefetch(&idata[(y*istride + x + 16)*2], _MM_HINT_T0);

    A0 = _mm_unpacklo_epi16(D0, D8); // [  0  8  1  9  2 10  3 11 ]
    A4 = _mm_unpackhi_epi16(D0, D8); // [  4 12  5 13  6 14  7 15 ]
    B0 = _mm_unpacklo_epi16(A0, A4); // [  0  4  8 12  1  5  9 13 ]
    B2 = _mm_unpackhi_epi16(A0, A4); // [  2  6 10 14  3  7 11 15 ]
    E0 = _mm_unpacklo_epi16(B0, B2); // [  0  2  4  6  8 10 12 14 ]
    O1 = _mm_unpackhi_epi16(B0, B2); // [  1  3  5  7  9 11 13 15 ]

    {
      D16 = _mm_sub_epi16(_mm_loadu_si128((__m128i *)&idata[(y*istride + x + 16)*2]), offset); // [ 16 17 18 19 20 21 22 23 ]
      D24 = _mm_sub_epi16(_mm_loadu_si128((__m128i *)&idata[(y*istride + x + 24)*2]), offset); // [ 24 25 26 27 28 29 30 31 ]
      _mm_prefetch(&idata[(y*istride + x + 32)*2], _MM_HINT_T0);

      A16 = _mm_unpacklo_epi16(D16, D24); // [ 16 24 17 25 18 26 19 27 ]
      A20 = _mm_unpackhi_epi16(D16, D24); // [ 20 28 21 29 22 30 23 31 ]
      B16 = _mm_unpacklo_epi16(A16, A20); // [ 16 20 24 28 17 21 25 29 ]
      B22 = _mm_unpackhi_epi16(A16, A20); // [ 18 22 26 30 19 23 27 31 ]

      E16 = _mm_unpacklo_epi16(B16, B22); // [ 16 18 20 22 24 26 28 30 ]
      O17 = _mm_unpackhi_epi16(B16, B22); // [ 17 19 21 23 25 27 29 31 ]

      E2 = _mm_alignr_epi8(E16, E0, 2); // [  2  4  6  8 10 12 14 16 ]

      Y1 = _mm_sub_epi16(_mm_slli_epi16(O1, 1), _mm_srai_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(E0, E2), 1), ONE), 1)); // {  1  3  5  7  9 11 13 15 }
      Ym = _mm_insert_epi16(_mm_slli_si128(Y1, 2), _mm_extract_epi16(Y1, 0), 0); // {  1  1  3  5  7  9 11 13 }
      Y0 = _mm_add_epi16(_mm_slli_epi16(E0, 1), _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Ym, Y1), TWO), 2)); // {  0  2  4  6  8 10 12 14 }

      Z0 = _mm_unpacklo_epi16(Y0, Y1); // {  0  1  2  3  4  5  6  7 }
      _mm_store_si128((__m128i *)&odata[y*ostride + x +  0], Z0);
      Z4 = _mm_unpackhi_epi16(Y0, Y1); // {  8  9 10 11 12 13 14 15 }
      _mm_store_si128((__m128i *)&odata[y*ostride + x +  8], Z4);

      E0 = E16;
      O1 = O17;
      Yz = Y1;
    }

    x += 16;

    for (; x < iwidth - 16; x += 16) {
      D16 = _mm_sub_epi16(_mm_loadu_si128((__m128i *)&idata[(y*istride + x + 16)*2]), offset); // [ 16 17 18 19 20 21 22 23 ]
      D24 = _mm_sub_epi16(_mm_loadu_si128((__m128i *)&idata[(y*istride + x + 24)*2]), offset); // [ 24 25 26 27 28 29 30 31 ]
      _mm_prefetch(&idata[(y*istride + x + 32)*2], _MM_HINT_T0);

      A16 = _mm_unpacklo_epi16(D16, D24); // [ 16 24 17 25 18 26 19 27 ]
      A20 = _mm_unpackhi_epi16(D16, D24); // [ 20 28 21 29 22 30 23 31 ]
      B16 = _mm_unpacklo_epi16(A16, A20); // [ 16 20 24 28 17 21 25 29 ]
      B22 = _mm_unpackhi_epi16(A16, A20); // [ 18 22 26 30 19 23 27 31 ]

      E16 = _mm_unpacklo_epi16(B16, B22); // [ 16 18 20 22 24 26 28 30 ]
      O17 = _mm_unpackhi_epi16(B16, B22); // [ 17 19 21 23 25 27 29 31 ]

      E2 = _mm_alignr_epi8(E16, E0, 2); // [  2  4  6  8 10 12 14 16 ]

      Y1 = _mm_sub_epi16(_mm_slli_epi16(O1, 1), _mm_srai_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(E0, E2), 1), ONE), 1)); // {  1  3  5  7  9 11 13 15 }
      Ym = _mm_alignr_epi8(Y1, Yz, 14); // { -1  1  3  5  7  9 11 13 }
      Y0 = _mm_add_epi16(_mm_slli_epi16(E0, 1), _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Ym, Y1), TWO), 2)); // {  0  2  4  6  8 10 12 14 }

      Z0 = _mm_unpacklo_epi16(Y0, Y1); // {  0  1  2  3  4  5  6  7 }
      _mm_store_si128((__m128i *)&odata[y*ostride + x +  0], Z0);
      Z4 = _mm_unpackhi_epi16(Y0, Y1); // {  8  9 10 11 12 13 14 15 }
      _mm_store_si128((__m128i *)&odata[y*ostride + x +  8], Z4);

      E0 = E16;
      O1 = O17;
      Yz = Y1;
    }

    {
      E2 = _mm_insert_epi16(_mm_srli_si128(E0, 2), _mm_extract_epi16(E0, 7), 7); // [  2  4  6  8 10 12 14 14 ]

      Y1 = _mm_sub_epi16(_mm_slli_epi16(O1, 1), _mm_srai_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(E0, E2), 1), ONE), 1)); // {  1  3  5  7  9 11 13 15 }
      Ym = _mm_alignr_epi8(Y1, Yz, 14); // { -1  1  3  5  7  9 11 13 }
      Y0 = _mm_add_epi16(_mm_slli_epi16(E0, 1), _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Ym, Y1), TWO), 2)); // {  0  2  4  6  8 10 12 14 }

      Z0 = _mm_unpacklo_epi16(Y0, Y1); // {  0  1  2  3  4  5  6  7 }
      _mm_store_si128((__m128i *)&odata[y*ostride + x +  0], Z0);
      Z4 = _mm_unpackhi_epi16(Y0, Y1); // {  8  9 10 11 12 13 14 15 }
      _mm_store_si128((__m128i *)&odata[y*ostride + x +  8], Z4);
    }
    x += 16;

    for (; x < owidth; x += 2*skip) {
      odata[y*ostride + x + 0*skip] = odata[y*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[y*ostride + x + 1*skip] = odata[y*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }
  for (; y < oheight; y+=skip) {
    memcpy(&odata[y*ostride], &odata[(2*iheight - y - skip)*ostride], 2*owidth);
  }
}

template<int skip> void LeGall_5_3_transform_H_inplace_sse4_2(void *_idata,
                                                            const int istride,
                                                            const int width,
                                                            const int height,
                                                            const int);

template<> void LeGall_5_3_transform_H_inplace_sse4_2<2>(void *_idata,
                                                       const int istride,
                                                       const int width,
                                                       const int height,
                                                       const int) {
  int16_t *idata = (int16_t *)_idata;
  const __m128i ONE = _mm_set1_epi16(1);
  const __m128i TWO = _mm_set1_epi16(2);
  //  const __m128i MOVMASK = _mm_set1_epi32(0x00008080);

  const int skip = 2;

  for (int y = 0; y < height; y+=skip) {
    int x = 0;

    __m128i
      D0, D4, D8, D12,
      A0, A2, A8, A10,
      E0, E2, O1, E8, O9,
      Xz, Xm, X1, X0,
      Z0, Z4;

    {
      D0  = _mm_load_si128((__m128i *)&idata[y*istride + x +  0]); // [  0  X  1  X  2  X  3  X ]
      D4  = _mm_load_si128((__m128i *)&idata[y*istride + x +  8]); // [  4  X  5  X  6  X  7  X ]

      A0  = _mm_unpacklo_epi32( D0,  D4); // [  0  X  4  X  1  X  5  X ]
      A2  = _mm_unpackhi_epi32( D0,  D4); // [  2  X  6  X  3  X  7  X ]
      E0  = _mm_unpacklo_epi32( A0,  A2); // [  0  X  2  X  4  X  6  X ]
      O1  = _mm_unpackhi_epi32( A0,  A2); // [  1  X  3  X  5  X  7  X ]
    }

    {
      D8  = _mm_load_si128((__m128i *)&idata[y*istride + x + 16]); // [  8  X  9  X 10  X 11  X ]
      D12 = _mm_load_si128((__m128i *)&idata[y*istride + x + 24]); // [ 12  X 13  X 14  X 15  X ]

      A8  = _mm_unpacklo_epi32( D8, D12); // [  8  X 12  X  9  X 13  X ]
      A10 = _mm_unpackhi_epi32( D8, D12); // [ 10  X 14  X 11  X 15  X ]
      E8  = _mm_unpacklo_epi32( A8, A10); // [  8  X 10  X 12  X 14  X ]
      O9  = _mm_unpackhi_epi32( A8, A10); // [  9  X 11  X 13  X 15  X ]
    }

    {
      E2  = _mm_alignr_epi8(E8, E0, 4);  // [  2  X  4  X  6  X  8  X ]

      X1  = _mm_sub_epi16(_mm_slli_epi16(O1, 1), _mm_srai_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(E0, E2), 1), ONE), 1)); // {  1  X  3  X  5  X  7  X }
      Xm  = _mm_insert_epi32(_mm_slli_si128(X1, 4), _mm_extract_epi32(X1, 0), 0); // {  1  X  1  X  3  X  5  X }
      X0  = _mm_add_epi16(_mm_slli_epi16(E0, 1), _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Xm, X1), TWO), 2)); // {  0  X  2  X  4  X  6  X }

      Z0  = _mm_unpacklo_epi32(X0, X1);  // {  0  X  1  X  2  X  3  X }
      //      _mm_maskmoveu_si128(Z0, MOVMASK, (char *)&idata[y*istride + x +  0]);
      Z0  = _mm_blend_epi16(Z0, D0, 0xAA);
      _mm_store_si128((__m128i*)&idata[y*istride + x +  0], Z0);
      Z4  = _mm_unpackhi_epi32(X0, X1);  // {  4  X  5  X  6  X  7  X }
      //      _mm_maskmoveu_si128(Z4, MOVMASK, (char *)&idata[y*istride + x +  8]);
      Z4  = _mm_blend_epi16(Z4, D4, 0xAA);
      _mm_store_si128((__m128i*)&idata[y*istride + x +  8], Z4);

      D0 = D8;
      D4 = D12;
      E0 = E8;
      O1 = O9;

      Xz = X1;
    }

    x += 16;

    for (; x < width - 16; x+=16) {
      {
        D8  = _mm_load_si128((__m128i *)&idata[y*istride + x + 16]); // [  8  X  9  X 10  X 11  X ]
        D12 = _mm_load_si128((__m128i *)&idata[y*istride + x + 24]); // [ 12  X 13  X 14  X 15  X ]

        A8  = _mm_unpacklo_epi32( D8, D12); // [  8  X 12  X  9  X 13  X ]
        A10 = _mm_unpackhi_epi32( D8, D12); // [ 10  X 14  X 11  X 15  X ]
        E8  = _mm_unpacklo_epi32( A8, A10); // [  8  X 10  X 12  X 14  X ]
        O9  = _mm_unpackhi_epi32( A8, A10); // [  9  X 11  X 13  X 15  X ]
      }

      {
        E2  = _mm_alignr_epi8(E8, E0, 4);  // [  2  X  4  X  6  X  8  X ]

        X1  = _mm_sub_epi16(_mm_slli_epi16(O1, 1), _mm_srai_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(E0, E2), 1), ONE), 1)); // {  1  X  3  X  5  X  7  X }
        Xm  = _mm_alignr_epi8(X1, Xz, 12); // { -1  X  1  X  3  X  5  X }
        X0  = _mm_add_epi16(_mm_slli_epi16(E0, 1), _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Xm, X1), TWO), 2)); // {  0  X  2  X  4  X  6  X }

        Z0  = _mm_unpacklo_epi32(X0, X1);  // {  0  X  1  X  2  X  3  X }
        //        _mm_maskmoveu_si128(Z0, MOVMASK, (char *)&idata[y*istride + x +  0]);
        Z0  = _mm_blend_epi16(Z0, D0, 0xAA);
        _mm_store_si128((__m128i*)&idata[y*istride + x +  0], Z0);
        Z4  = _mm_unpackhi_epi32(X0, X1);  // {  4  X  5  X  6  X  7  X }
        //        _mm_maskmoveu_si128(Z4, MOVMASK, (char *)&idata[y*istride + x +  8]);
        Z4  = _mm_blend_epi16(Z4, D4, 0xAA);
        _mm_store_si128((__m128i*)&idata[y*istride + x +  8], Z4);

        D0 = D8;
        D4 = D12;
        E0 = E8;
        O1 = O9;

        Xz = X1;
      }
    }

    {
      E2  = _mm_insert_epi32(_mm_srli_si128(E0, 4), _mm_extract_epi32(E0, 3), 3); // [  2  X  4  X  6  X  6  X ]

      X1  = _mm_sub_epi16(_mm_slli_epi16(O1, 1), _mm_srai_epi16(_mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(E0, E2), 1), ONE), 1)); // {  1  X  3  X  5  X  7  X }
      Xm  = _mm_alignr_epi8(X1, Xz, 12); // { -1  X  1  X  3  X  5  X }
      X0  = _mm_add_epi16(_mm_slli_epi16(E0, 1), _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Xm, X1), TWO), 2)); // {  0  X  2  X  4  X  6  X }

      Z0  = _mm_unpacklo_epi32(X0, X1);  // {  0  X  1  X  2  X  3  X }
      Z0  = _mm_blend_epi16(Z0, D0, 0xAA);
      _mm_store_si128((__m128i*)&idata[y*istride + x +  0], Z0);
      Z4  = _mm_unpackhi_epi32(X0, X1);  // {  4  X  5  X  6  X  7  X }
      Z4  = _mm_blend_epi16(Z4, D4, 0xAA);
      _mm_store_si128((__m128i*)&idata[y*istride + x +  8], Z4);
    }
  }
}

template<int skip> void LeGall_5_3_transform_V_inplace_sse4_2(void *_idata,
                                                              const int istride,
                                                              const int width,
                                                              const int height,
                                                              const int) {
  int16_t *idata = (int16_t *)_idata;
  const __m128i ONE = _mm_set1_epi16(1);
  const __m128i TWO = _mm_set1_epi16(2);
  const int BLENDMASK = (skip == 1)?0x00:((skip == 2)?0xAA:((skip == 4)?0xEE:0xFE));
  const int xskip = (skip > 8)?skip:8;

#define BLEND_FOR_WRITE(A,B) ((skip == 1)?(A):_mm_blend_epi16(A,B,BLENDMASK))

  __m128i D, Dp1, Dp2;
  int y = 0;
  int x = 0;
  __m128i Xm1, X, Xp1;

  for (int XX = 0; XX < width; XX += 1024) {
    y = 0;
    for (x = XX; x < width && x < XX + 1024; x+=xskip) {
      D   = _mm_load_si128((__m128i *)&idata[(y + 0*skip)*istride + x]);
      Dp1 = _mm_load_si128((__m128i *)&idata[(y + 1*skip)*istride + x]);
      Dp2 = _mm_load_si128((__m128i *)&idata[(y + 2*skip)*istride + x]);

      Xp1 = _mm_sub_epi16(Dp1, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(D,   Dp2), ONE), 1));
      Xm1 = Xp1;
      X   = _mm_add_epi16(D,   _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Xm1, Xp1) ,TWO), 2));
      _mm_store_si128((__m128i*)&idata[(y + 0*skip)*istride + x], BLEND_FOR_WRITE(X,   D));
      _mm_store_si128((__m128i*)&idata[(y + 1*skip)*istride + x], BLEND_FOR_WRITE(Xp1, Dp1));
    }
    y += 2*skip;

    for (; y < height - 2*skip; y += 2*skip) {
      for (x = XX; x < width && x < XX + 1024; x+=xskip) {
        D   = _mm_load_si128((__m128i *)&idata[(y + 0*skip)*istride + x]);
        Dp1 = _mm_load_si128((__m128i *)&idata[(y + 1*skip)*istride + x]);
        Dp2 = _mm_load_si128((__m128i *)&idata[(y + 2*skip)*istride + x]);

        Xp1 = _mm_sub_epi16(Dp1, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(D,   Dp2), ONE), 1));
        Xm1 = _mm_load_si128((__m128i *)&idata[(y - 1*skip)*istride + x]);
        X   = _mm_add_epi16(D,   _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Xm1, Xp1) ,TWO), 2));
        _mm_store_si128((__m128i*)&idata[(y + 0*skip)*istride + x], BLEND_FOR_WRITE(X,   D));
        _mm_store_si128((__m128i*)&idata[(y + 1*skip)*istride + x], BLEND_FOR_WRITE(Xp1, Dp1));
      }
    }

    for (x = XX; x < width && x < XX + 1024; x+=xskip) {
      D   = _mm_load_si128((__m128i *)&idata[(y + 0*skip)*istride + x]);
      Dp1 = _mm_load_si128((__m128i *)&idata[(y + 1*skip)*istride + x]);
      Dp2 = D;

      Xp1 = _mm_sub_epi16(Dp1, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(D,   Dp2), ONE), 1));
      Xm1 = _mm_load_si128((__m128i *)&idata[(y - 1*skip)*istride + x]);
      X   = _mm_add_epi16(D,   _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(Xm1, Xp1) ,TWO), 2));
      _mm_store_si128((__m128i*)&idata[(y + 0*skip)*istride + x], BLEND_FOR_WRITE(X,   D));
      _mm_store_si128((__m128i*)&idata[(y + 1*skip)*istride + x], BLEND_FOR_WRITE(Xp1, Dp1));
    }
  }

#undef BLEND_FOR_WRITE
}
