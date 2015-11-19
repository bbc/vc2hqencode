/*****************************************************************************
 * encode.hpp : Variable Length Encoding
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

#ifndef __ENCODE_HPP__
#define __ENCODE_HPP__

#include "datastructures.hpp"
#include "quantise.hpp"

enum {
  QUANTISER_SELECTION_FULLSEARCH = 0,
  QUANTISER_SELECTION_HALFSEARCH = 1,
  QUANTISER_SELECTION_QUARTERSEARCH = 2,
  QUANTISER_SELECTION_EIGHTHSEARCH = 3,
};

typedef void (*SliceEncoderFunc16)(CodedSlice<int16_t> *, int, QuantisationMatrices *, int olength, int wavelet_index, int slice_size_scalar, int w, int h, int d);
typedef void (*SliceEncoderFunc32)(CodedSlice<int32_t> *, int, QuantisationMatrices *, int olength, int wavelet_index, int slice_size_scalar, int w, int h, int d);

SliceEncoderFunc16 get_slice_encoder16(int w, int h, int d, int QUAL, int passes);
SliceEncoderFunc32 get_slice_encoder32(int w, int h, int d, int QUAL, int passes);

uint8_t encode_uint(uint16_t x, uint32_t *output, uint8_t *lengthout);

#endif /* __ENCODE_HPP__ */
