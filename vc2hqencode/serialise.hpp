/*****************************************************************************
 * serialise.hpp : Serialise
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

#ifndef __SERIALISE_HPP__
#define __SERIALISE_HPP__

#include "datastructures.hpp"
#include "internal.h"
#include "logger.hpp"
#include "debug.hpp"

#include <x86intrin.h>

template<class T> void serialise_slices(CodedSlice<T> *slices, int n_slices, char *odata, int olength, int n_samples, int slice_size_scalar, int slices_per_frag, uint32_t picnum, uint32_t *final_offset, int sx, int sy, int slices_per_line) {
  int ocounter = 0;
  int bits = 0;
  uint32_t accum = 0x00;
  uint8_t *optr = (uint8_t *)odata;

  uint16_t *codewords   = slices[0].codewords[0];
  uint8_t  *wordlengths = slices[0].wordlengths[0];

  int remaining_length = olength;

#ifdef DEBUG_OP_SLICESIZES
  writelog(LOG_INFO, "----------------------------------------------------------\n");
  writelog(LOG_INFO, "  Slice Sizes\n");
  writelog(LOG_INFO, "----------------------------------------------------------\n");
  writelog(LOG_INFO, "  Block  length +    pad = output  \n");
  writelog(LOG_INFO, "-----------------------------------\n");
#endif

  int N = 0;
  int last_frag_start = -1;
  int slices_til_next_frag_start = 0;
  for (; N < n_slices - 1; N++) {
    if (slices_per_frag) {
      if (slices_til_next_frag_start == 0) {
        if (last_frag_start >= 0) {
          int frag_length = ocounter - last_frag_start;
          optr[last_frag_start + 0] = 0x42;
          optr[last_frag_start + 1] = 0x42;
          optr[last_frag_start + 2] = 0x43;
          optr[last_frag_start + 3] = 0x44;

          optr[last_frag_start + 4] = 0xEC;

          optr[last_frag_start + 5] = (frag_length >> 24)&0xFF;
          optr[last_frag_start + 6] = (frag_length >> 16)&0xFF;
          optr[last_frag_start + 7] = (frag_length >>  8)&0xFF;
          optr[last_frag_start + 8] = (frag_length >>  0)&0xFF;

          optr[ocounter +  9] = (frag_length >> 24)&0xFF;
          optr[ocounter + 10] = (frag_length >> 16)&0xFF;
          optr[ocounter + 11] = (frag_length >>  8)&0xFF;
          optr[ocounter + 12] = (frag_length >>  0)&0xFF;

          optr[last_frag_start + 13] = (picnum >> 24)&0xFF;
          optr[last_frag_start + 14] = (picnum >> 16)&0xFF;
          optr[last_frag_start + 15] = (picnum >>  8)&0xFF;
          optr[last_frag_start + 16] = (picnum >>  0)&0xFF;

          frag_length -= 25;

          optr[last_frag_start + 17] = (frag_length >> 8)&0xFF;
          optr[last_frag_start + 18] = (frag_length >> 0)&0xFF;

          optr[last_frag_start + 19] = (slices_per_frag >> 8)&0xFF;
          optr[last_frag_start + 20] = (slices_per_frag >> 0)&0xFF;

          optr[last_frag_start + 21] = (sx >> 8)&0xFF;
          optr[last_frag_start + 22] = (sx >> 0)&0xFF;

          optr[last_frag_start + 23] = (sy >> 8)&0xFF;
          optr[last_frag_start + 24] = (sy >> 0)&0xFF;

          sx += slices_per_frag;
          sy += (sx/slices_per_line);
          sx %= slices_per_line;
        }

        last_frag_start = ocounter;
        slices_til_next_frag_start = slices_per_frag - 1;
        ocounter += 25;
      } else
        slices_til_next_frag_start--;
    }
    int opadding = slices[N].padding;
    if (opadding < 0) {
      writelog(LOG_ERROR, "%s:%d: opadding = %d, remaining_length = %d, slice %u: sizes (%d,%d,%d)\n", __FILE__, __LINE__, opadding, remaining_length, N, slices[N].length[0], slices[N].length[1], slices[N].length[2]);
      throw VC2ENCODER_CODEROVERRUN;
    }
    optr[ocounter++] = (uint8_t)slices[N].qindex;
    for (int c = 0; c < 3; c++) {
      int p = 0;
      {
        int l = slices[N].length[c];
        int L = (l + slice_size_scalar - 1)/slice_size_scalar;
        if (L > 255) {
          writelog(LOG_ERROR, "slice %u: %d > %d, sizes (%d,%d,%d)\n", N, L, 255, slices[N].length[0], slices[N].length[1], slices[N].length[2]);
          throw VC2ENCODER_CODEROVERRUN;
        }
        p = (255 - L)*slice_size_scalar;
        if (p > opadding)
          p = opadding;
        opadding -= p;
        p += (L*slice_size_scalar - l);
        optr[ocounter++] = (uint8_t)((l + p)/slice_size_scalar);
#ifdef DEBUG_OP_SLICESIZES
        if (l > 0)
          writelog(LOG_INFO, "  %5u  %6d + %6d = %6d\n", N, l, p, (l + p)/slice_size_scalar*slice_size_scalar );
        else
          writelog(LOG_INFO, "! %5u  %6d + %6d = %6d\n", N, l, p, (l + p)/slice_size_scalar*slice_size_scalar );
#endif
      }

      accum = 0;
      for (int n = 0; n < slices[N].samples[c]; n++) {
        int l = wordlengths[n];
        accum |= (((uint32_t)codewords[n]) << (32 - l - bits));
        bits += l;

        *((uint32_t *)&optr[ocounter]) = __builtin_bswap32(accum);
        accum <<= (bits/8)*8;
        ocounter += (bits/8);
        bits%=8;
      }

      if (bits)
        optr[ocounter++] = (accum >> 24) | (0xFF >> bits);
      bits = 0;

      for (int n = 0; n < p; n++) {
        optr[ocounter++] = 0xFF;
      }

      codewords += n_samples;
      wordlengths += n_samples;

#ifdef DEBUG
      if (ocounter > olength) {
        writelog(LOG_ERROR, "%s:%d:  coder overrun", __FILE__, __LINE__);
        throw VC2ENCODER_CODEROVERRUN;
      }
#endif /* DEBUG */
    }
    remaining_length -= remaining_length/(n_slices - N);
  }
  {
    if (slices_per_frag) {
      if (slices_til_next_frag_start == 0) {
        if (last_frag_start >= 0) {
          int frag_length = ocounter - last_frag_start;
          optr[last_frag_start + 0] = 0x42;
          optr[last_frag_start + 1] = 0x42;
          optr[last_frag_start + 2] = 0x43;
          optr[last_frag_start + 3] = 0x44;

          optr[last_frag_start + 4] = 0xEC;

          optr[last_frag_start + 5] = (frag_length >> 24)&0xFF;
          optr[last_frag_start + 6] = (frag_length >> 16)&0xFF;
          optr[last_frag_start + 7] = (frag_length >>  8)&0xFF;
          optr[last_frag_start + 8] = (frag_length >>  0)&0xFF;

          optr[ocounter +  9] = (frag_length >> 24)&0xFF;
          optr[ocounter + 10] = (frag_length >> 16)&0xFF;
          optr[ocounter + 11] = (frag_length >>  8)&0xFF;
          optr[ocounter + 12] = (frag_length >>  0)&0xFF;

          optr[last_frag_start + 13] = (picnum >> 24)&0xFF;
          optr[last_frag_start + 14] = (picnum >> 16)&0xFF;
          optr[last_frag_start + 15] = (picnum >>  8)&0xFF;
          optr[last_frag_start + 16] = (picnum >>  0)&0xFF;

          frag_length -= 25;

          optr[last_frag_start + 17] = (frag_length >> 8)&0xFF;
          optr[last_frag_start + 18] = (frag_length >> 0)&0xFF;

          optr[last_frag_start + 19] = (slices_per_frag >> 8)&0xFF;
          optr[last_frag_start + 20] = (slices_per_frag >> 0)&0xFF;

          optr[last_frag_start + 21] = (sx >> 8)&0xFF;
          optr[last_frag_start + 22] = (sx >> 0)&0xFF;

          optr[last_frag_start + 23] = (sy >> 8)&0xFF;
          optr[last_frag_start + 24] = (sy >> 0)&0xFF;

          sx += slices_per_frag;
          sy += (sx/slices_per_line);
          sx %= slices_per_line;
        }

        last_frag_start = ocounter;
        slices_til_next_frag_start = slices_per_frag - 1;
        ocounter += 25;
      } else
        slices_til_next_frag_start--;
    }
    int opadding = slices[N].padding;
    optr[ocounter++] = (uint8_t)slices[N].qindex;
    for (int c = 0; c < 3; c++) {
      int p = 0;
      {
        int l = slices[N].length[c];
        int L = (l + slice_size_scalar - 1)/slice_size_scalar*slice_size_scalar;
        p = 255*slice_size_scalar - L;
        if (p > opadding)
          p = opadding;
        opadding -= p;
        p += L - l;
        optr[ocounter++] = (uint8_t)((l + p)/slice_size_scalar);
      }

      accum = 0;
      for (int n = 0; n < slices[N].samples[c]; n++) {
        int l = wordlengths[n];
        accum |= (((uint32_t)codewords[n]) << (32 - l - bits));
        bits += l;

        if (bits >= 8) {
          optr[ocounter++] = accum >> 24;
          accum <<= 8;
          bits -= 8;

          if (bits >= 8) {
            optr[ocounter++] = accum >> 24;
            accum <<= 8;
            bits -= 8;

            if (bits >= 8) {
              optr[ocounter++] = accum >> 24;
              accum <<= 8;
              bits -= 8;

              if (bits >= 8) {
                optr[ocounter++] = accum >> 24;
                accum <<= 8;
                bits -= 8;
              }
            }
          }
        }
      }

      if (bits)
        optr[ocounter++] = (accum >> 24) | (0xFF >> bits);
      bits = 0;

      for (int n = 0; n < p; n++) {
        optr[ocounter++] = 0xFF;
      }

      codewords += n_samples;
      wordlengths += n_samples;

#ifdef DEBUG
      if (ocounter > olength) {
        writelog(LOG_ERROR, "%s:%d:  coder overrun", __FILE__, __LINE__);
        throw VC2ENCODER_CODEROVERRUN;
      }
#endif /* DEBUG */
    }
  }
  for (; ocounter < olength;)
    optr[ocounter++] = 0xFF;

  if (slices_per_frag) {
    if (last_frag_start >= 0) {
      int slices = slices_per_frag - slices_til_next_frag_start;
      int frag_length = ocounter - last_frag_start;
      optr[last_frag_start + 0] = 0x42;
      optr[last_frag_start + 1] = 0x42;
      optr[last_frag_start + 2] = 0x43;
      optr[last_frag_start + 3] = 0x44;

      optr[last_frag_start + 4] = 0xEC;

      optr[last_frag_start + 5] = (frag_length >> 24)&0xFF;
      optr[last_frag_start + 6] = (frag_length >> 16)&0xFF;
      optr[last_frag_start + 7] = (frag_length >>  8)&0xFF;
      optr[last_frag_start + 8] = (frag_length >>  0)&0xFF;

      // Prev parse offset not filled out here

      *final_offset = frag_length;

      optr[last_frag_start + 13] = (picnum >> 24)&0xFF;
      optr[last_frag_start + 14] = (picnum >> 16)&0xFF;
      optr[last_frag_start + 15] = (picnum >>  8)&0xFF;
      optr[last_frag_start + 16] = (picnum >>  0)&0xFF;

      frag_length -= 25;

      optr[last_frag_start + 17] = (frag_length >> 8)&0xFF;
      optr[last_frag_start + 18] = (frag_length >> 0)&0xFF;

      optr[last_frag_start + 19] = (slices >> 8)&0xFF;
      optr[last_frag_start + 20] = (slices >> 0)&0xFF;

      optr[last_frag_start + 21] = (sx >> 8)&0xFF;
      optr[last_frag_start + 22] = (sx >> 0)&0xFF;

      optr[last_frag_start + 23] = (sy >> 8)&0xFF;
      optr[last_frag_start + 24] = (sy >> 0)&0xFF;
    }
  }

#ifdef DEBUG_OP_SLICESIZES
  writelog(LOG_INFO, "----------------------------------------------------------\n");
#endif
}

#endif /* __SERIALISE_HPP__ */
