/*****************************************************************************
 * haar_transform.hpp : Haar Transform: AVX2 version
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

template<int shift, class T> void Haar_transform_H_inplace_10P2_avx2(const char *_idata,
                                                                      const int istride,
                                                                      void **_odata,
                                                                      const int ostride,
                                                                      const int iwidth,
                                                                      const int iheight,
                                                                      const int owidth,
                                                                      const int oheight);

template<> void Haar_transform_H_inplace_10P2_avx2<0,int16_t>(const char *_idata,
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
  const __m256i OFFSET = _mm256_set1_epi16(1 << 9);
  const __m256i ONE    = _mm256_set1_epi16(1);
  const __m256i SHUF   = _mm256_set_epi8(31, 30, 27, 26, 23, 22, 19, 18,
                                         29, 28, 25, 24, 21, 20, 17, 16,
                                         15, 14, 11, 10,  7,  6,  3,  2,
                                         13, 12,  9,  8,  5,  4,  1,  0);

  int y = 0;
  for (; y < iheight; y+=2*skip) {
    int x = 0;
    for (; x < iwidth; x += 32) {
      __m256i ZA0, ZA8, ZB0, ZB8;
      {
        __m256i D0  = _mm256_loadu_si256((__m256i *)&idata[y*istride + x +  0]);
        __m256i D16 = _mm256_loadu_si256((__m256i *)&idata[y*istride + x + 16]);

        D0  = _mm256_sub_epi16(D0, OFFSET);
        D16 = _mm256_sub_epi16(D16, OFFSET);

        D0  = _mm256_shuffle_epi8(D0,  SHUF);
        D16 = _mm256_shuffle_epi8(D16, SHUF);

        D0  = _mm256_permute4x64_epi64(D0, 0xD8);
        D16 = _mm256_permute4x64_epi64(D16, 0xD8);

        __m256i E0 = _mm256_permute2x128_si256(D0, D16, 0x20); // equivalent to an 128-bit unpack lo
        __m256i O1 = _mm256_permute2x128_si256(D0, D16, 0x31); // equivalent to an 128-bit unpack hi

        __m256i X1 = _mm256_sub_epi16(O1, E0);
        __m256i X0 = _mm256_add_epi16(E0, _mm256_srai_epi16(_mm256_add_epi16(X1, ONE), 1));

        __m256i Y0 = _mm256_unpacklo_epi16(X0, X1);
        __m256i Y8 = _mm256_unpackhi_epi16(X0, X1);

        ZA0 = _mm256_permute2x128_si256(Y0, Y8, 0x20);
        ZA8 = _mm256_permute2x128_si256(Y0, Y8, 0x31);
      }

      {
        __m256i D0  = _mm256_loadu_si256((__m256i *)&idata[(y + skip)*istride + x +  0]);
        __m256i D16 = _mm256_loadu_si256((__m256i *)&idata[(y + skip)*istride + x + 16]);

        D0  = _mm256_sub_epi16(D0, OFFSET);
        D16 = _mm256_sub_epi16(D16, OFFSET);

        D0  = _mm256_shuffle_epi8(D0,  SHUF);
        D16 = _mm256_shuffle_epi8(D16, SHUF);

        D0  = _mm256_permute4x64_epi64(D0, 0xD8);
        D16 = _mm256_permute4x64_epi64(D16, 0xD8);

        __m256i E0 = _mm256_permute2x128_si256(D0, D16, 0x20); // equivalent to an 128-bit unpack lo
        __m256i O1 = _mm256_permute2x128_si256(D0, D16, 0x31); // equivalent to an 128-bit unpack hi

        __m256i X1 = _mm256_sub_epi16(O1, E0);
        __m256i X0 = _mm256_add_epi16(E0, _mm256_srai_epi16(_mm256_add_epi16(X1, ONE), 1));

        __m256i Y0 = _mm256_unpacklo_epi16(X0, X1);
        __m256i Y8 = _mm256_unpackhi_epi16(X0, X1);

        ZB0 = _mm256_permute2x128_si256(Y0, Y8, 0x20);
        ZB8 = _mm256_permute2x128_si256(Y0, Y8, 0x31);
      }

      {
        __m256i X1 = _mm256_sub_epi16(ZB0, ZA0);
        __m256i X0 = _mm256_add_epi16(ZA0, _mm256_srai_epi16(_mm256_add_epi16(X1, ONE), 1));

        _mm256_store_si256((__m256i *)&odata[(y + 0)*ostride + x + 0], X0);
        _mm256_store_si256((__m256i *)&odata[(y + 1)*ostride + x + 0], X1);
      }

      {
        __m256i X1 = _mm256_sub_epi16(ZB8, ZA8);
        __m256i X0 = _mm256_add_epi16(ZA8, _mm256_srai_epi16(_mm256_add_epi16(X1, ONE), 1));

        _mm256_store_si256((__m256i *)&odata[(y + 0)*ostride + x + 16], X0);
        _mm256_store_si256((__m256i *)&odata[(y + 1)*ostride + x + 16], X1);
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
    memcpy(&odata[(y + 0)*ostride], &odata[(2*iheight - y - 2*skip)*ostride], owidth);
    memcpy(&odata[(y + 1)*ostride], &odata[(2*iheight - y - 1*skip)*ostride], owidth);
  }
}
