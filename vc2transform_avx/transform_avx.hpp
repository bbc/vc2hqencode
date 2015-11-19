/*****************************************************************************
 * transform_avx.hpp : Transform Selection: AVX version
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

#ifndef __TRANSFORM_AVX_HPP__
#define __TRANSFORM_AVX_HPP__

#include "transform.hpp"

InplaceTransformInitial get_htransforminitial_avx(int wavelet_index, int active_bits, int coef_size, int c, VC2EncoderInputFormat fmt);
InplaceTransform get_vtransform_avx(int wavelet_index, int level, int coef_size, VC2EncoderInputFormat fmt);
InplaceTransform get_htransform_avx(int wavelet_index, int level, int coef_size);

#endif /* __TRANSFORM_AVX_HPP__ */
