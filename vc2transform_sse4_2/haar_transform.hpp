/*****************************************************************************
 * haar_transform.hpp : Haar transform: SSE4.2 version
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

template<int shift, class T> void Haar_transform_H_inplace_V210_sse4_2(const char *_idata,
                                                                       const int istride,
                                                                       void **_odata,
                                                                       const int ostride,
                                                                       const int iwidth,
                                                                       const int iheight,
                                                                       const int owidth,
                                                                       const int oheight);

template<> void Haar_transform_H_inplace_V210_sse4_2<0, int16_t>(const char *_idata,
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
  const __m128i OFFSET = _mm_set1_epi16(1 << 9);
  const __m128i ONE    = _mm_set1_epi16(1);

  const __m128i V210_LUMA_MASK = _mm_set_epi32(0x3FF003FF, 0x000FFC00, 0x3FF003FF, 0x000FFC00);
  const __m128i V210_LUMA_SHUF = _mm_set_epi8(0xFF, 0xFF, 15, 14, 10, 9, 5, 4,
                                              0xFF, 0xFF, 13, 12,  7, 6, 2, 1);
  const __m128i V210_LUMA_SHIFT = _mm_set_epi16(0, 1 << 11, 1 << 13, 0x7FFF,
                                                0, 0x7FFF, 1 << 11, 1 << 13);
  const __m128i V210_CHROMA_SHUF = _mm_set_epi8(0xFF, 0xFF, 9, 8, 14, 13, 3, 2,
                                                0xFF, 0xFF, 6, 5, 11, 10, 1, 0);
  const __m128i V210_CHROMA_SHIFT = _mm_set_epi16(0, 0x7FFF, 1 << 13, 1 << 11,
                                                  0, 1 << 13, 1 << 11, 0x7FFF);

  int y = 0;
  for (; y < iheight; y+=skip) {
    int x = 0;
    int X = 0;
    for (; x < (iwidth + 47)/48*48; x += 12, X += 32) {
      __m128i D0 = _mm_loadu_si128((__m128i *)&idata[y*istride + X +  0]);
      __m128i D6 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 16]);
      {
        __m128i Y0 = _mm_and_si128(V210_LUMA_MASK, D0);
        __m128i Y6 = _mm_and_si128(V210_LUMA_MASK, D6);

        Y0 = _mm_shuffle_epi8(Y0, V210_LUMA_SHUF);
        Y6 = _mm_shuffle_epi8(Y6, V210_LUMA_SHUF);

        Y0 = _mm_mulhrs_epi16(Y0, V210_LUMA_SHIFT);
        Y6 = _mm_mulhrs_epi16(Y6, V210_LUMA_SHIFT);

        __m128i YE = _mm_unpacklo_epi64(Y0, Y6);
        __m128i YO = _mm_unpackhi_epi64(Y0, Y6);

        YE = _mm_sub_epi16(YE, OFFSET);
        YO = _mm_sub_epi16(YO, OFFSET);

        __m128i X1 = _mm_sub_epi16(YO, YE);
        __m128i X0 = _mm_add_epi16(YE, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

        __m128i Z0 = _mm_unpacklo_epi16(X0, X1);
        __m128i Z6 = _mm_unpackhi_epi16(X0, X1);

        _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 0], Z0);
        _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 6], Z6);
      }

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

        CE = _mm_sub_epi16(CE, OFFSET);
        CO = _mm_sub_epi16(CO, OFFSET);

        __m128i X1 = _mm_sub_epi16(CO, CE);
        __m128i X0 = _mm_add_epi16(CE, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

        __m128i U0 = _mm_unpacklo_epi16(X0, X1);
        __m128i V0 = _mm_unpackhi_epi16(X0, X1);

        _mm_storeu_si128((__m128i *)&odata_u[y*ostride/2 + x/2], U0);
        _mm_storeu_si128((__m128i *)&odata_v[y*ostride/2 + x/2], V0);
      }
    }
    for (x = iwidth; x < owidth; x += 2*skip) {
      odata_y[(y + 0)*ostride + x + 0*skip] = odata_y[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata_y[(y + 0)*ostride + x + 1*skip] = odata_y[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }
  for (; y < oheight; y+=skip) {
    memcpy(&odata_y[(y + 0)*ostride], &odata_y[(2*iheight - y - skip)*ostride], owidth*2);
  }
}

template<> void Haar_transform_H_inplace_V210_sse4_2<1, int16_t>(const char *_idata,
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
  const __m128i OFFSET = _mm_set1_epi16(1 << 9);
  const __m128i ONE    = _mm_set1_epi16(1);

  const __m128i V210_LUMA_MASK = _mm_set_epi32(0x3FF003FF, 0x000FFC00, 0x3FF003FF, 0x000FFC00);
  const __m128i V210_LUMA_SHUF = _mm_set_epi8(0xFF, 0xFF, 15, 14, 10, 9, 5, 4,
                                              0xFF, 0xFF, 13, 12,  7, 6, 2, 1);
  const __m128i V210_LUMA_SHIFT = _mm_set_epi16(0, 1 << 11, 1 << 13, 0x7FFF,
                                                0, 0x7FFF, 1 << 11, 1 << 13);
  const __m128i V210_CHROMA_SHUF = _mm_set_epi8(0xFF, 0xFF, 9, 8, 14, 13, 3, 2,
                                                0xFF, 0xFF, 6, 5, 11, 10, 1, 0);
  const __m128i V210_CHROMA_SHIFT = _mm_set_epi16(0, 0x7FFF, 1 << 13, 1 << 11,
                                                  0, 1 << 13, 1 << 11, 0x7FFF);

  int y = 0;
  for (; y < iheight; y+=skip) {
    int x = 0;
    int X = 0;
    for (; x < (iwidth + 47)/48*48; x += 12, X += 32) {
      __m128i D0 = _mm_loadu_si128((__m128i *)&idata[y*istride + X +  0]);
      __m128i D6 = _mm_loadu_si128((__m128i *)&idata[y*istride + X + 16]);
      {
        __m128i Y0 = _mm_and_si128(V210_LUMA_MASK, D0);
        __m128i Y6 = _mm_and_si128(V210_LUMA_MASK, D6);

        Y0 = _mm_shuffle_epi8(Y0, V210_LUMA_SHUF);
        Y6 = _mm_shuffle_epi8(Y6, V210_LUMA_SHUF);

        Y0 = _mm_mulhrs_epi16(Y0, V210_LUMA_SHIFT);
        Y6 = _mm_mulhrs_epi16(Y6, V210_LUMA_SHIFT);

        __m128i YE = _mm_unpacklo_epi64(Y0, Y6);
        __m128i YO = _mm_unpackhi_epi64(Y0, Y6);

        YE = _mm_sub_epi16(YE, OFFSET);
        YO = _mm_sub_epi16(YO, OFFSET);

        YE = _mm_slli_epi16(YE, 1);
        YO = _mm_slli_epi16(YO, 1);

        __m128i X1 = _mm_sub_epi16(YO, YE);
        __m128i X0 = _mm_add_epi16(YE, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

        __m128i Z0 = _mm_unpacklo_epi16(X0, X1);
        __m128i Z6 = _mm_unpackhi_epi16(X0, X1);

        _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 0], Z0);
        _mm_storeu_si128((__m128i *)&odata_y[y*ostride + x + 6], Z6);
      }

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

        CE = _mm_sub_epi16(CE, OFFSET);
        CO = _mm_sub_epi16(CO, OFFSET);

        CE = _mm_slli_epi16(CE, 1);
        CO = _mm_slli_epi16(CO, 1);

        __m128i X1 = _mm_sub_epi16(CO, CE);
        __m128i X0 = _mm_add_epi16(CE, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

        __m128i U0 = _mm_unpacklo_epi16(X0, X1);
        __m128i V0 = _mm_unpackhi_epi16(X0, X1);

        _mm_storeu_si128((__m128i *)&odata_u[y*ostride/2 + x/2], U0);
        _mm_storeu_si128((__m128i *)&odata_v[y*ostride/2 + x/2], V0);
      }
    }
    for (x = iwidth; x < owidth; x += 2*skip) {
      odata_y[(y + 0)*ostride + x + 0*skip] = odata_y[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata_y[(y + 0)*ostride + x + 1*skip] = odata_y[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }
  for (; y < oheight; y+=skip) {
    memcpy(&odata_y[(y + 0)*ostride], &odata_y[(2*iheight - y - skip)*ostride], owidth*2);
  }
}

template<int shift, class T> void Haar_transform_H_inplace_10P2_sse4_2(const char *_idata,
                                                                        const int istride,
                                                                        void **_odata,
                                                                        const int ostride,
                                                                        const int iwidth,
                                                                        const int iheight,
                                                                        const int owidth,
                                                                        const int oheight);

template<> void Haar_transform_H_inplace_10P2_sse4_2<0,int16_t>(const char *_idata,
                                                                 const int istride,
                                                                 void **_odata,
                                                                 const int ostride,
                                                                 const int iwidth,
                                                                 const int iheight,
                                                                 const int owidth,
                                                                 const int oheight){
  int16_t *odata = *((int16_t **)_odata);
  const int skip = 1;

  const uint16_t *idata = (const uint16_t *)_idata;
  const __m128i OFFSET = _mm_set1_epi16(1 << 9);
  const __m128i ONE    = _mm_set1_epi16(1);

  int y = 0;
  for (; y < iheight; y+=2*skip) {
    int x = 0;
    for (; x < iwidth; x += 16) {
      __m128i ZA0, ZA8, ZB0, ZB8;
      {
        __m128i D0 = _mm_loadu_si128((__m128i *)&idata[y*istride + x + 0]); // [  0  1  2  3  4  5  6  7 ]
        __m128i D8 = _mm_loadu_si128((__m128i *)&idata[y*istride + x + 8]); // [  8  9 10 11 12 13 14 15 ]

        D0 = _mm_sub_epi16(D0, OFFSET);
        D8 = _mm_sub_epi16(D8, OFFSET);

        __m128i A0 = _mm_unpacklo_epi16(D0, D8); // [  0  8  1  9  2 10  3 11 ]
        __m128i A4 = _mm_unpackhi_epi16(D0, D8); // [  4 12  5 13  6 14  7 15 ]
        __m128i B0 = _mm_unpacklo_epi16(A0, A4); // [  0  4  8 12  1  5  9 13 ]
        __m128i B2 = _mm_unpackhi_epi16(A0, A4); // [  2  6 10 14  3  7 11 15 ]

        __m128i E0 = _mm_unpacklo_epi16(B0, B2); // [  0  2  4  6  8 10 12 14 ]
        __m128i O1 = _mm_unpackhi_epi16(B0, B2); // [  1  3  5  7  9 11 13 15 ]

        __m128i X1 = _mm_sub_epi16(O1, E0);                                        // {  1  3  5  7  9 11 13 15 }
        __m128i X0 = _mm_add_epi16(E0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1)); // {  0  2  4  6  8 10 12 14 }

        ZA0 = _mm_unpacklo_epi16(X0, X1); // {  0  1  2  3  4  5  6  7 }
        ZA8 = _mm_unpackhi_epi16(X0, X1); // {  8  9 10 11 12 13 14 15 }
      }

      {
        __m128i D0 = _mm_loadu_si128((__m128i *)&idata[(y + 1)*istride + x + 0]); // [  0  1  2  3  4  5  6  7 ]
        __m128i D8 = _mm_loadu_si128((__m128i *)&idata[(y + 1)*istride + x + 8]); // [  8  9 10 11 12 13 14 15 ]

        D0 = _mm_sub_epi16(D0, OFFSET);
        D8 = _mm_sub_epi16(D8, OFFSET);

        __m128i A0 = _mm_unpacklo_epi16(D0, D8); // [  0  8  1  9  2 10  3 11 ]
        __m128i A4 = _mm_unpackhi_epi16(D0, D8); // [  4 12  5 13  6 14  7 15 ]
        __m128i B0 = _mm_unpacklo_epi16(A0, A4); // [  0  4  8 12  1  5  9 13 ]
        __m128i B2 = _mm_unpackhi_epi16(A0, A4); // [  2  6 10 14  3  7 11 15 ]

        __m128i E0 = _mm_unpacklo_epi16(B0, B2); // [  0  2  4  6  8 10 12 14 ]
        __m128i O1 = _mm_unpackhi_epi16(B0, B2); // [  1  3  5  7  9 11 13 15 ]

        __m128i X1 = _mm_sub_epi16(O1, E0);                                        // {  1  3  5  7  9 11 13 15 }
        __m128i X0 = _mm_add_epi16(E0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1)); // {  0  2  4  6  8 10 12 14 }

        ZB0 = _mm_unpacklo_epi16(X0, X1); // {  0  1  2  3  4  5  6  7 }
        ZB8 = _mm_unpackhi_epi16(X0, X1); // {  8  9 10 11 12 13 14 15 }
      }

      {
        __m128i X1 = _mm_sub_epi16(ZB0, ZA0);
        __m128i X0 = _mm_add_epi16(ZA0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

        _mm_store_si128((__m128i *)&odata[(y + 0)*ostride + x + 0], X0);
        _mm_store_si128((__m128i *)&odata[(y + 1)*ostride + x + 0], X1);
      }

      {
        __m128i X1 = _mm_sub_epi16(ZB8, ZA8);
        __m128i X0 = _mm_add_epi16(ZA8, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

        _mm_store_si128((__m128i *)&odata[(y + 0)*ostride + x + 8], X0);
        _mm_store_si128((__m128i *)&odata[(y + 1)*ostride + x + 8], X1);
      }
    }
    for (; x < owidth; x += 2*skip) {
      odata[(y + 0)*ostride + x + 0*skip] = odata[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[(y + 0)*ostride + x + 1*skip] = odata[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
      odata[(y + 1)*ostride + x + 0*skip] = odata[(y + 1)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[(y + 1)*ostride + x + 1*skip] = odata[(y + 1)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }
  for (; y < oheight; y+=2*skip) {
    memcpy(&odata[(y + 0)*ostride], &odata[(2*iheight - y - 2*skip)*ostride], owidth*2);
    memcpy(&odata[(y + 1)*ostride], &odata[(2*iheight - y - 1*skip)*ostride], owidth*2);
  }
}

template<> void Haar_transform_H_inplace_10P2_sse4_2<1,int16_t>(const char *_idata,
                                                                 const int istride,
                                                                 void **_odata,
                                                                 const int ostride,
                                                                 const int iwidth,
                                                                 const int iheight,
                                                                 const int owidth,
                                                                 const int oheight){
  int16_t *odata = *((int16_t **)_odata);
  const int skip = 1;

  const uint16_t *idata = (const uint16_t *)_idata;
  const __m128i OFFSET = _mm_set1_epi16(1 << 9);
  const __m128i ONE    = _mm_set1_epi16(1);

  int y = 0;
  for (; y < iheight; y+=2*skip) {
    int x = 0;
    for (; x < iwidth; x += 16) {
      __m128i ZA0, ZA8, ZB0, ZB8;

      {
        __m128i D0 = _mm_loadu_si128((__m128i *)&idata[y*istride + x + 0]); // [  0  1  2  3  4  5  6  7 ]
        __m128i D8 = _mm_loadu_si128((__m128i *)&idata[y*istride + x + 8]); // [  8  9 10 11 12 13 14 15 ]

        D0 = _mm_sub_epi16(D0, OFFSET);
        D8 = _mm_sub_epi16(D8, OFFSET);

        D0 = _mm_slli_epi16(D0, 1);
        D8 = _mm_slli_epi16(D8, 1);

        __m128i A0 = _mm_unpacklo_epi16(D0, D8); // [  0  8  1  9  2 10  3 11 ]
        __m128i A4 = _mm_unpackhi_epi16(D0, D8); // [  4 12  5 13  6 14  7 15 ]
        __m128i B0 = _mm_unpacklo_epi16(A0, A4); // [  0  4  8 12  1  5  9 13 ]
        __m128i B2 = _mm_unpackhi_epi16(A0, A4); // [  2  6 10 14  3  7 11 15 ]

        __m128i E0 = _mm_unpacklo_epi16(B0, B2); // [  0  2  4  6  8 10 12 14 ]
        __m128i O1 = _mm_unpackhi_epi16(B0, B2); // [  1  3  5  7  9 11 13 15 ]

        __m128i X1 = _mm_sub_epi16(O1, E0);                                        // {  1  3  5  7  9 11 13 15 }
        __m128i X0 = _mm_add_epi16(E0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1)); // {  0  2  4  6  8 10 12 14 }

        ZA0 = _mm_unpacklo_epi16(X0, X1); // {  0  1  2  3  4  5  6  7 }
        ZA8 = _mm_unpackhi_epi16(X0, X1); // {  8  9 10 11 12 13 14 15 }
      }

      {
        __m128i D0 = _mm_loadu_si128((__m128i *)&idata[(y + 1)*istride + x + 0]); // [  0  1  2  3  4  5  6  7 ]
        __m128i D8 = _mm_loadu_si128((__m128i *)&idata[(y + 1)*istride + x + 8]); // [  8  9 10 11 12 13 14 15 ]

        D0 = _mm_sub_epi16(D0, OFFSET);
        D8 = _mm_sub_epi16(D8, OFFSET);

        D0 = _mm_slli_epi16(D0, 1);
        D8 = _mm_slli_epi16(D8, 1);

        __m128i A0 = _mm_unpacklo_epi16(D0, D8); // [  0  8  1  9  2 10  3 11 ]
        __m128i A4 = _mm_unpackhi_epi16(D0, D8); // [  4 12  5 13  6 14  7 15 ]
        __m128i B0 = _mm_unpacklo_epi16(A0, A4); // [  0  4  8 12  1  5  9 13 ]
        __m128i B2 = _mm_unpackhi_epi16(A0, A4); // [  2  6 10 14  3  7 11 15 ]

        __m128i E0 = _mm_unpacklo_epi16(B0, B2); // [  0  2  4  6  8 10 12 14 ]
        __m128i O1 = _mm_unpackhi_epi16(B0, B2); // [  1  3  5  7  9 11 13 15 ]

        __m128i X1 = _mm_sub_epi16(O1, E0);                                        // {  1  3  5  7  9 11 13 15 }
        __m128i X0 = _mm_add_epi16(E0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1)); // {  0  2  4  6  8 10 12 14 }

        ZB0 = _mm_unpacklo_epi16(X0, X1); // {  0  1  2  3  4  5  6  7 }
        ZB8 = _mm_unpackhi_epi16(X0, X1); // {  8  9 10 11 12 13 14 15 }
      }

      {
        __m128i X1 = _mm_sub_epi16(ZB0, ZA0);
        __m128i X0 = _mm_add_epi16(ZA0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

        _mm_store_si128((__m128i *)&odata[(y + 0)*ostride + x + 0], X0);
        _mm_store_si128((__m128i *)&odata[(y + 1)*ostride + x + 0], X1);
      }

      {
        __m128i X1 = _mm_sub_epi16(ZB8, ZA8);
        __m128i X0 = _mm_add_epi16(ZA8, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

        _mm_store_si128((__m128i *)&odata[(y + 0)*ostride + x + 8], X0);
        _mm_store_si128((__m128i *)&odata[(y + 1)*ostride + x + 8], X1);
      }
    }
    for (; x < owidth; x += 2*skip) {
      odata[(y + 0)*ostride + x + 0*skip] = odata[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[(y + 0)*ostride + x + 1*skip] = odata[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
      odata[(y + 1)*ostride + x + 0*skip] = odata[(y + 1)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[(y + 1)*ostride + x + 1*skip] = odata[(y + 1)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }
  for (; y < oheight; y+=2*skip) {
    memcpy(&odata[(y + 0)*ostride], &odata[(2*iheight - y - 2*skip)*ostride], owidth*2);
    memcpy(&odata[(y + 1)*ostride], &odata[(2*iheight - y - 1*skip)*ostride], owidth*2);
  }
}

template<int shift> inline void Haar_transform_H_inplace_10P2_sse4_2_int32_t(const char *_idata,
                                                                             const int istride,
                                                                             void **_odata,
                                                                             const int ostride,
                                                                             const int iwidth,
                                                                             const int iheight,
                                                                             const int owidth,
                                                                             const int oheight){
  int32_t *odata = *((int32_t **)_odata);
  const int skip = 1;

  const uint16_t *idata = (const uint16_t *)_idata;
  const __m128i OFFSET = _mm_set1_epi16(1 << 9);
  const __m128i ONE    = _mm_set1_epi32(1);
  const __m128i ZERO   = _mm_set1_epi32(0);

  int y = 0;
  for (; y < iheight; y+=2*skip) {
    int x = 0;
    for (; x < iwidth; x += 8) {
      __m128i ZA0, ZA4, ZB0, ZB4;
      {
        __m128i D0 = _mm_loadu_si128((__m128i *)&idata[y*istride + x + 0]); // [  0  1  2  3  4  5  6  7 ]

        D0 = _mm_sub_epi16(D0, OFFSET);

        __m128i A0 = _mm_unpacklo_epi16(ZERO, D0); // [ X 0 X 1 X 2 X 3 ]
        __m128i A4 = _mm_unpackhi_epi16(ZERO, D0); // [ X 4 X 5 X 6 X 7 ]

        A0 = _mm_srai_epi32(A0, 16 - shift); // [ 0 1 2 3 ]
        A4 = _mm_srai_epi32(A4, 16 - shift); // [ 4 5 6 7 ]

        A0 = _mm_shuffle_epi32(A0, 0xD8); // [ 0 2 1 3 ]
        A4 = _mm_shuffle_epi32(A4, 0xD8); // [ 4 6 5 7 ]

        __m128i E0 = _mm_unpacklo_epi64(A0, A4); // [ 0 2 4 6 ]
        __m128i O1 = _mm_unpackhi_epi64(A0, A4); // [ 1 3 5 7 ]

        __m128i X1 = _mm_sub_epi32(O1, E0);                                        // {  1  3  5  7 }
        __m128i X0 = _mm_add_epi32(E0, _mm_srai_epi32(_mm_add_epi32(X1, ONE), 1)); // {  0  2  4  6 }

        ZA0 = _mm_unpacklo_epi32(X0, X1); // {  0  1  2  3 }
        ZA4 = _mm_unpackhi_epi32(X0, X1); // {  4  5  6  7 }
      }

      {
        __m128i D0 = _mm_loadu_si128((__m128i *)&idata[(y + 1)*istride + x + 0]); // [  0  1  2  3  4  5  6  7 ]

        D0 = _mm_sub_epi16(D0, OFFSET);

        __m128i A0 = _mm_unpacklo_epi16(ZERO, D0); // [ X 0 X 1 X 2 X 3 ]
        __m128i A4 = _mm_unpackhi_epi16(ZERO, D0); // [ X 4 X 5 X 6 X 7 ]

        A0 = _mm_srai_epi32(A0, 16 - shift); // [ 0 1 2 3 ]
        A4 = _mm_srai_epi32(A4, 16 - shift); // [ 4 5 6 7 ]

        A0 = _mm_shuffle_epi32(A0, 0xD8); // [ 0 2 1 3 ]
        A4 = _mm_shuffle_epi32(A4, 0xD8); // [ 4 6 5 7 ]

        __m128i E0 = _mm_unpacklo_epi64(A0, A4); // [ 0 2 4 6 ]
        __m128i O1 = _mm_unpackhi_epi64(A0, A4); // [ 1 3 5 7 ]

        __m128i X1 = _mm_sub_epi32(O1, E0);                                        // {  1  3  5  7 }
        __m128i X0 = _mm_add_epi32(E0, _mm_srai_epi32(_mm_add_epi32(X1, ONE), 1)); // {  0  2  4  6 }

        ZB0 = _mm_unpacklo_epi32(X0, X1); // {  0  1  2  3 }
        ZB4 = _mm_unpackhi_epi32(X0, X1); // {  4  5  6  7 }
      }

      {
        __m128i X1 = _mm_sub_epi32(ZB0, ZA0);
        __m128i X0 = _mm_add_epi32(ZA0, _mm_srai_epi32(_mm_add_epi32(X1, ONE), 1));

        _mm_store_si128((__m128i *)&odata[(y + 0)*ostride + x + 0], X0);
        _mm_store_si128((__m128i *)&odata[(y + 1)*ostride + x + 0], X1);
      }

      {
        __m128i X1 = _mm_sub_epi32(ZB4, ZA4);
        __m128i X0 = _mm_add_epi32(ZA4, _mm_srai_epi32(_mm_add_epi32(X1, ONE), 1));

        _mm_store_si128((__m128i *)&odata[(y + 0)*ostride + x + 4], X0);
        _mm_store_si128((__m128i *)&odata[(y + 1)*ostride + x + 4], X1);
      }
    }
    for (; x < owidth; x += 2*skip) {
      odata[(y + 0)*ostride + x + 0*skip] = odata[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[(y + 0)*ostride + x + 1*skip] = odata[(y + 0)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
      odata[(y + 1)*ostride + x + 0*skip] = odata[(y + 1)*ostride + (2*iwidth - x - 2*skip) + 0*skip];
      odata[(y + 1)*ostride + x + 1*skip] = odata[(y + 1)*ostride + (2*iwidth - x - 2*skip) + 1*skip];
    }
  }
  for (; y < oheight; y+=2*skip) {
    memcpy(&odata[(y + 0)*ostride], &odata[(2*iheight - y - 2*skip)*ostride], owidth*4);
    memcpy(&odata[(y + 1)*ostride], &odata[(2*iheight - y - 1*skip)*ostride], owidth*4);
  }
}

template<> void Haar_transform_H_inplace_10P2_sse4_2<1,int32_t>(const char *_idata,
                                                                const int istride,
                                                                void **_odata,
                                                                const int ostride,
                                                                const int iwidth,
                                                                const int iheight,
                                                                const int owidth,
                                                                const int oheight)
{
  Haar_transform_H_inplace_10P2_sse4_2_int32_t<1>(_idata,
                                                  istride,
                                                  _odata,
                                                  ostride,
                                                  iwidth,
                                                  iheight,
                                                  owidth,
                                                  oheight);
}

template<> void Haar_transform_H_inplace_10P2_sse4_2<0,int32_t>(const char *_idata,
                                                                const int istride,
                                                                void **_odata,
                                                                const int ostride,
                                                                const int iwidth,
                                                                const int iheight,
                                                                const int owidth,
                                                                const int oheight)
{
  Haar_transform_H_inplace_10P2_sse4_2_int32_t<0>(_idata,
                                                  istride,
                                                  _odata,
                                                  ostride,
                                                  iwidth,
                                                  iheight,
                                                  owidth,
                                                  oheight);
}

template<int skip, int shift> void Haar_transform_H_inplace_sse4_2(void *_idata,
                                                                 const int istride,
                                                                 const int width,
                                                                 const int height,
                                                                 const int);

template<> void Haar_transform_H_inplace_sse4_2<2, 0>(void *_idata,
                                                    const int istride,
                                                    const int width,
                                                    const int height,
                                                    const int) {
  int16_t *idata = (int16_t *)_idata;
  const int skip  = 2;

  const __m128i ONE    = _mm_set1_epi16(1);

  for (int y = 0; y < height; y+=skip) {
    for (int x = 0; x < width; x += 32) {
      __m128i D0  = _mm_load_si128((__m128i *)&idata[y*istride + x +  0]); // [  0  A  1  B  2  C  3  D ]
      __m128i D4  = _mm_load_si128((__m128i *)&idata[y*istride + x +  8]); // [  4  E  5  F  6  G  7  H ]
      __m128i D8  = _mm_load_si128((__m128i *)&idata[y*istride + x + 16]); // [  8  I  9  J 10  K 11  L ]
      __m128i D12 = _mm_load_si128((__m128i *)&idata[y*istride + x + 24]); // [ 12  M 13  N 14  O 15  P ]

      __m128i A0  = _mm_unpacklo_epi16(D0, D4);  // [  0  4  A  E  1  5  B  F ]
      __m128i A2  = _mm_unpackhi_epi16(D0, D4);  // [  2  6  C  G  3  7  D  H ]
      __m128i A8  = _mm_unpacklo_epi16(D8, D12); // [  8 12  I  M  9 13  J  N ]
      __m128i A10 = _mm_unpackhi_epi16(D8, D12); // [ 10 14  K  O 11 15  L  P ]

      __m128i B0  = _mm_unpacklo_epi16(A0, A2);  // [  0  2  4  6  A  C  E  G ]
      __m128i B1  = _mm_unpackhi_epi16(A0, A2);  // [  1  3  5  7  B  D  F  H ]
      __m128i B8  = _mm_unpacklo_epi16(A8, A10); // [  8 10 12 14  I  K  M  O ]
      __m128i B9  = _mm_unpackhi_epi16(A8, A10); // [  9 11 13 15  J  L  N  P ]

      __m128i E0 = _mm_unpacklo_epi64(B0, B8); // [  0  2  4  6  8 10 12 14 ]
      __m128i O1 = _mm_unpacklo_epi64(B1, B9); // [  1  3  5  7  9 11 13 15 ]

      __m128i YA = _mm_unpackhi_epi16(B0, B1); // [  A  B  C  D  E  F  G  H ]
      __m128i YI = _mm_unpackhi_epi16(B8, B9); // [  I  J  K  L  M  N  O  P ]

      __m128i X1 = _mm_sub_epi16(O1, E0);                                        // {  1  3  5  7  9 11 13 15 }
      __m128i X0 = _mm_add_epi16(E0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1)); // {  0  2  4  6  8 10 12 14 }

      __m128i W0 = _mm_unpacklo_epi16(X0, X1); // {  0  1  2  3  4  5  6  7 }
      __m128i W8 = _mm_unpackhi_epi16(X0, X1); // {  8  9 10 11 12 13 14 15 }

      __m128i Z0  = _mm_unpacklo_epi16(W0, YA); // {  0  A  1  B  2  C  3  D }
      __m128i Z4  = _mm_unpackhi_epi16(W0, YA); // {  4  E  5  F  6  G  7  H }
      __m128i Z8  = _mm_unpacklo_epi16(W8, YI); // {  8  I  9  J 10  K 11  L }
      __m128i Z12 = _mm_unpackhi_epi16(W8, YI); // { 12  M 13  N 14  O 15  P }

      _mm_store_si128((__m128i *)&idata[y*istride + x +  0], Z0);
      _mm_store_si128((__m128i *)&idata[y*istride + x +  8], Z4);
      _mm_store_si128((__m128i *)&idata[y*istride + x + 16], Z8);
      _mm_store_si128((__m128i *)&idata[y*istride + x + 24], Z12);
    }
  }
}

template<> void Haar_transform_H_inplace_sse4_2<2, 1>(void *_idata,
                                                    const int istride,
                                                    const int width,
                                                    const int height,
                                                    const int) {
  int16_t *idata = (int16_t *)_idata;
  const int skip  = 2;

  const __m128i ONE    = _mm_set1_epi16(1);

  for (int y = 0; y < height; y+=skip) {
    for (int x = 0; x < width; x += 32) {
      __m128i D0  = _mm_load_si128((__m128i *)&idata[y*istride + x +  0]); // [  0  A  1  B  2  C  3  D ]
      __m128i D4  = _mm_load_si128((__m128i *)&idata[y*istride + x +  8]); // [  4  E  5  F  6  G  7  H ]
      __m128i D8  = _mm_load_si128((__m128i *)&idata[y*istride + x + 16]); // [  8  I  9  J 10  K 11  L ]
      __m128i D12 = _mm_load_si128((__m128i *)&idata[y*istride + x + 24]); // [ 12  M 13  N 14  O 15  P ]

      __m128i A0  = _mm_unpacklo_epi16(D0, D4);  // [  0  4  A  E  1  5  B  F ]
      __m128i A2  = _mm_unpackhi_epi16(D0, D4);  // [  2  6  C  G  3  7  D  H ]
      __m128i A8  = _mm_unpacklo_epi16(D8, D12); // [  8 12  I  M  9 13  J  N ]
      __m128i A10 = _mm_unpackhi_epi16(D8, D12); // [ 10 14  K  O 11 15  L  P ]

      __m128i B0  = _mm_unpacklo_epi16(A0, A2);  // [  0  2  4  6  A  C  E  G ]
      __m128i B1  = _mm_unpackhi_epi16(A0, A2);  // [  1  3  5  7  B  D  F  H ]
      __m128i B8  = _mm_unpacklo_epi16(A8, A10); // [  8 10 12 14  I  K  M  O ]
      __m128i B9  = _mm_unpackhi_epi16(A8, A10); // [  9 11 13 15  J  L  N  P ]

      __m128i E0 = _mm_unpacklo_epi64(B0, B8); // [  0  2  4  6  8 10 12 14 ]
      __m128i O1 = _mm_unpacklo_epi64(B1, B9); // [  1  3  5  7  9 11 13 15 ]

      E0 = _mm_slli_epi16(E0, 1);
      O1 = _mm_slli_epi16(O1, 1);

      __m128i YA = _mm_unpackhi_epi16(B0, B1); // [  A  B  C  D  E  F  G  H ]
      __m128i YI = _mm_unpackhi_epi16(B8, B9); // [  I  J  K  L  M  N  O  P ]

      __m128i X1 = _mm_sub_epi16(O1, E0);                                        // {  1  3  5  7  9 11 13 15 }
      __m128i X0 = _mm_add_epi16(E0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1)); // {  0  2  4  6  8 10 12 14 }

      __m128i W0 = _mm_unpacklo_epi16(X0, X1); // {  0  1  2  3  4  5  6  7 }
      __m128i W8 = _mm_unpackhi_epi16(X0, X1); // {  8  9 10 11 12 13 14 15 }

      __m128i Z0  = _mm_unpacklo_epi16(W0, YA); // {  0  A  1  B  2  C  3  D }
      __m128i Z4  = _mm_unpackhi_epi16(W0, YA); // {  4  E  5  F  6  G  7  H }
      __m128i Z8  = _mm_unpacklo_epi16(W8, YI); // {  8  I  9  J 10  K 11  L }
      __m128i Z12 = _mm_unpackhi_epi16(W8, YI); // { 12  M 13  N 14  O 15  P }

      _mm_store_si128((__m128i *)&idata[y*istride + x +  0], Z0);
      _mm_store_si128((__m128i *)&idata[y*istride + x +  8], Z4);
      _mm_store_si128((__m128i *)&idata[y*istride + x + 16], Z8);
      _mm_store_si128((__m128i *)&idata[y*istride + x + 24], Z12);
    }
  }
}


template<int skip, class T>void Haar_transform_V_inplace_sse4_2(void *_idata,
                                                              const int istride,
                                                              const int width,
                                                              const int height,
                                                              const int);

template<>void Haar_transform_V_inplace_sse4_2<1, int16_t>(void *_idata,
                                                         const int istride,
                                                         const int width,
                                                         const int height,
                                                         const int) {
  int16_t *idata = (int16_t *)_idata;
  const int skip = 1;
  const __m128i ONE = _mm_set1_epi16(1);

  for (int y = 0; y < height; y += 2*skip) {
    for (int x = 0; x < width; x+= 8) {
      __m128i D0 = _mm_load_si128((__m128i *)&idata[(y + 0*skip)*istride + x]);
      __m128i D1 = _mm_load_si128((__m128i *)&idata[(y + 1*skip)*istride + x]);

      __m128i X1 = _mm_sub_epi16(D1, D0);
      __m128i X0 = _mm_add_epi16(D0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

      _mm_store_si128((__m128i *)&idata[(y + 0*skip)*istride + x], X0);
      _mm_store_si128((__m128i *)&idata[(y + 1*skip)*istride + x], X1);
    }
  }
}

template<>void Haar_transform_V_inplace_sse4_2<1, int32_t>(void *_idata,
                                                         const int istride,
                                                         const int width,
                                                         const int height,
                                                         const int) {
  int32_t *idata = (int32_t *)_idata;
  const int skip = 1;
  const __m128i ONE = _mm_set1_epi32(1);

  for (int y = 0; y < height; y += 2*skip) {
    for (int x = 0; x < width; x+= 4) {
      __m128i D0 = _mm_load_si128((__m128i *)&idata[(y + 0*skip)*istride + x]);
      __m128i D1 = _mm_load_si128((__m128i *)&idata[(y + 1*skip)*istride + x]);

      __m128i X1 = _mm_sub_epi32(D1, D0);
      __m128i X0 = _mm_add_epi32(D0, _mm_srai_epi32(_mm_add_epi32(X1, ONE), 1));

      _mm_store_si128((__m128i *)&idata[(y + 0*skip)*istride + x], X0);
      _mm_store_si128((__m128i *)&idata[(y + 1*skip)*istride + x], X1);
    }
  }
}

template<>void Haar_transform_V_inplace_sse4_2<2, int16_t>(void *_idata,
                                                         const int istride,
                                                         const int width,
                                                         const int height,
                                                         const int) {
  int16_t *idata = (int16_t *)_idata;
  const __m128i ONE = _mm_set1_epi16(1);
  const uint8_t BLENDMASK = 0x55;
  const int skip = 2;

  for (int y = 0; y < height; y += 2*skip) {
    for (int x = 0; x < width; x+= 8) {
      __m128i D0 = _mm_load_si128((__m128i *)&idata[(y + 0*skip)*istride + x]);
      __m128i D1 = _mm_load_si128((__m128i *)&idata[(y + 1*skip)*istride + x]);

      __m128i X1 = _mm_sub_epi16(D1, D0);
      __m128i X0 = _mm_add_epi16(D0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

      __m128i Z0 = _mm_blend_epi16(D0, X0, BLENDMASK);
      __m128i Z1 = _mm_blend_epi16(D1, X1, BLENDMASK);

      _mm_store_si128((__m128i *)&idata[(y + 0*skip)*istride + x], Z0);
      _mm_store_si128((__m128i *)&idata[(y + 1*skip)*istride + x], Z1);
    }
  }
}

template<>void Haar_transform_V_inplace_sse4_2<4, int16_t>(void *_idata,
                                                         const int istride,
                                                         const int width,
                                                         const int height,
                                                         const int) {
  int16_t *idata = (int16_t *)_idata;
  const __m128i ONE = _mm_set1_epi16(1);
  const uint8_t BLENDMASK = 0x11;
  const int skip = 4;

  for (int y = 0; y < height; y += 2*skip) {
    for (int x = 0; x < width; x+= 8) {
      __m128i D0 = _mm_load_si128((__m128i *)&idata[(y + 0*skip)*istride + x]);
      __m128i D1 = _mm_load_si128((__m128i *)&idata[(y + 1*skip)*istride + x]);

      __m128i X1 = _mm_sub_epi16(D1, D0);
      __m128i X0 = _mm_add_epi16(D0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

      __m128i Z0 = _mm_blend_epi16(D0, X0, BLENDMASK);
      __m128i Z1 = _mm_blend_epi16(D1, X1, BLENDMASK);

      _mm_store_si128((__m128i *)&idata[(y + 0*skip)*istride + x], Z0);
      _mm_store_si128((__m128i *)&idata[(y + 1*skip)*istride + x], Z1);
    }
  }
}

template<>void Haar_transform_V_inplace_sse4_2<8, int16_t>(void *_idata,
                                                         const int istride,
                                                         const int width,
                                                         const int height,
                                                         const int) {
  int16_t *idata = (int16_t *)_idata;
  const __m128i ONE = _mm_set1_epi16(1);
  const uint8_t BLENDMASK = 0x01;
  const int skip = 8;

  for (int y = 0; y < height; y += 2*skip) {
    for (int x = 0; x < width; x+= 8) {
      __m128i D0 = _mm_load_si128((__m128i *)&idata[(y + 0*skip)*istride + x]);
      __m128i D1 = _mm_load_si128((__m128i *)&idata[(y + 1*skip)*istride + x]);

      __m128i X1 = _mm_sub_epi16(D1, D0);
      __m128i X0 = _mm_add_epi16(D0, _mm_srai_epi16(_mm_add_epi16(X1, ONE), 1));

      __m128i Z0 = _mm_blend_epi16(D0, X0, BLENDMASK);
      __m128i Z1 = _mm_blend_epi16(D1, X1, BLENDMASK);

      _mm_store_si128((__m128i *)&idata[(y + 0*skip)*istride + x], Z0);
      _mm_store_si128((__m128i *)&idata[(y + 1*skip)*istride + x], Z1);
    }
  }
}
