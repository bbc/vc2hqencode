/*****************************************************************************
 * stream.hpp : VC2 Stream Syntax
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

#ifndef __STREAM_HPP__
#define __STREAM_HPP__

#include "internal.h"
#include <cstdlib>

class SequenceHeader {
public:
  SequenceHeader(VC2EncoderParams &params);
  ~SequenceHeader() { if(data) free(data); }

  void setPrevParseOffset(uint32_t l) {
    data[ 9] = (l >> 24)&0xFF;
    data[10] = (l >> 16)&0xFF;
    data[11] = (l >>  8)&0xFF;
    data[12] = (l >>  0)&0xFF;
  }

  uint8_t *data;
  int length;
};


class PictureHeader {
public:
  PictureHeader(VC2EncoderParams &params, int slices_x, int slices_y, int slice_size_scalar);
  ~PictureHeader() { if(data) free(data); }

  void setNextParseOffset(uint32_t l) {
    data[5] = (l >> 24)&0xFF;
    data[6] = (l >> 16)&0xFF;
    data[7] = (l >>  8)&0xFF;
    data[8] = (l >>  0)&0xFF;
  }

  void setPrevParseOffset(uint32_t l) {
    data[ 9] = (l >> 24)&0xFF;
    data[10] = (l >> 16)&0xFF;
    data[11] = (l >>  8)&0xFF;
    data[12] = (l >>  0)&0xFF;
  }

  void setPictureNumber(uint32_t l) {
    data[13] = (l >> 24)&0xFF;
    data[14] = (l >> 16)&0xFF;
    data[15] = (l >>  8)&0xFF;
    data[16] = (l >>  0)&0xFF;
  }

  uint8_t *data;
  int length;
};

#endif /* __STREAM_HPP__ */
