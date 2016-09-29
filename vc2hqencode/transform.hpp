/*****************************************************************************
 * transform.hpp : Transform
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

#ifndef __TRANSFORM_HPP__
#define __TRANSFORM_HPP__

#include <stdint.h>
#include <stdlib.h>

#include "internal.h"

typedef void (*InplaceTransform)(void *idata,
                                 const int istride,
                                 const int width,
                                 const int heigth,
                                 const int skip);

typedef void (*InplaceTransformInitial)(const char *idata,
                                        const int istride,
                                        void **odata,
                                        const int ostride,
                                        const int iwidth,
                                        const int iheight,
                                        const int owidth,
                                        const int oheight);

typedef InplaceTransformInitial (*GetHTransformInitial)(int wavelet_index, int active_bits, int coef_size, int c, VC2EncoderInputFormat fmt);
typedef InplaceTransform (*GetVTransform)(int wavelet_index, int level, int coef_size, VC2EncoderInputFormat fmt);
typedef InplaceTransform (*GetHTransform)(int wavelet_index, int level, int coef_size);

/*
   The needed overlap in job data to keep from getting artifacts for each transform and level
 */
const int TRANSFORM_OVERLAP[][4] = {
  {  4, 12, 28, 60 }, // VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7
  {  2,  6, 14, 30 }, // VC2ENCODER_WFT_LEGALL_5_3
  {  6, 18, 42, 90 }, // VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7
  {  0,  0,  0,  0 }, // VC2ENCODER_WFT_HAAR_NO_SHIFT
  {  0,  0,  0,  0 }, // VC2ENCODER_WFT_HAAR_SINGLE_SHIFT
  {  0,  0,  0,  0 }, // VC2ENCODER_WFT_FIDELITY
  {  0,  0,  0,  0 }, // VC2ENCODER_WFT_DAUBECHIES_9_7
};

#endif /* __TRANSFORM_HPP__ */
