/*****************************************************************************
 * VideoFormat.hpp : Video Format information
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

#ifndef __VIDEOFORMAT_HPP__
#define __VIDEOFORMAT_HPP__

#include "vc2hqencode.h"

namespace vc2 {
  struct FrameRate {
    uint32_t numer;
    uint32_t denom;
  };

  const FrameRate preset_framerates[] = {
    {
      .numer = 0,
      .denom = 0
    },
    {
      .numer = 24000,
      .denom = 1001
    },
    {
      .numer = 24,
      .denom = 1
    },
    {
      .numer = 25,
      .denom = 1
    },
    {
      .numer = 30000,
      .denom = 1001
    },
    {
      .numer = 30,
      .denom = 1
    },
    {
      .numer = 50,
      .denom = 1
    },
    {
      .numer = 60000,
      .denom = 1001
    },
    {
      .numer = 60,
      .denom = 1
    },
    {
      .numer = 15000,
      .denom = 1001
    },
    {
      .numer = 25,
      .denom = 2
    },
    {
      .numer = 48,
      .denom = 1
    }
  };

  const uint32_t preset_pixel_aspect_ratios[][2] = {
    { 0, 0 },
    { 1, 1 },
    { 10, 11 },
    { 12, 11 },
    { 40, 33 },
    { 16, 11 },
    { 4, 3 }
  };

  struct SignalRange {
    uint32_t luma_offset;
    uint32_t luma_excursion;
    uint32_t luma_bytes_per_sample;
    uint32_t luma_active_bits;
    uint32_t color_diff_offset;
    uint32_t color_diff_excursion;
    uint32_t color_diff_bytes_per_sample;
    uint32_t color_diff_active_bits;
  };

  const SignalRange preset_signal_ranges[] = {
    // VC2ENCODER_PSR_CUSTOM
    {
      .luma_offset                 = 0,
      .luma_excursion              = 0,
      .luma_bytes_per_sample       = 0,
      .luma_active_bits            = 0,
      .color_diff_offset           = 0,
      .color_diff_excursion        = 0,
      .color_diff_bytes_per_sample = 0,
      .color_diff_active_bits      = 0
    },
    // VC2ENCODER_PSR_8BITFULL
    {
      .luma_offset                 = 0,
      .luma_excursion              = 255,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 255,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8
    },
    // VC2ENCODER_PSR_8BITVID
    {
      .luma_offset                 = 16,
      .luma_excursion              = 219,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 224,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8
    },
    // VC2ENCODER_PSR_10BITVID
    {
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10
    },
    // VC2ENCODER_PSR_12BITVID
    {
      .luma_offset                 = 256,
      .luma_excursion              = 3504,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 12,
      .color_diff_offset           = 2048,
      .color_diff_excursion        = 3584,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 12
    }
  };

  struct ColorSpec {
    uint32_t color_primaries;
    uint32_t color_matrix;
    uint32_t transfer_function;
  };

  const ColorSpec preset_color_specs[] = {
    //   VC2ENCODER_CSP_CUSTOM
    {
      .color_primaries   = 0,
      .color_matrix      = 0,
      .transfer_function = 0
    },
    //  VC2ENCODER_CSP_SDTV525
    {
      .color_primaries   = VC2ENCODER_CPR_SDTV525,
      .color_matrix      = VC2ENCODER_CMA_SDTV,
      .transfer_function = VC2ENCODER_TRF_TVGAMMA
    },
    //  VC2ENCODER_CSP_SDTV625
    {
      .color_primaries   = VC2ENCODER_CPR_SDTV625,
      .color_matrix      = VC2ENCODER_CMA_SDTV,
      .transfer_function = VC2ENCODER_TRF_TVGAMMA
    },
    //  VC2ENCODER_CSP_HDTV
    {
      .color_primaries   = VC2ENCODER_CPR_HDTV,
      .color_matrix      = VC2ENCODER_CMA_HDTV,
      .transfer_function = VC2ENCODER_TRF_TVGAMMA
    },
    //  VC2ENCODER_CSP_DCINE
    {
      .color_primaries   = VC2ENCODER_CPR_DCINE,
      .color_matrix      = VC2ENCODER_CMA_REVERSIBLE,
      .transfer_function = VC2ENCODER_TRF_DCINE
    }
  };

  struct VideoFormat {
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t color_diff_format_index;
    uint32_t source_sampling;
    bool     top_field_first;
    uint32_t frame_rate_numer;
    uint32_t frame_rate_denom;
    uint32_t pixel_aspect_ratio_numer;
    uint32_t pixel_aspect_ratio_denom;
    uint32_t clean_width;
    uint32_t clean_height;
    uint32_t left_offset;
    uint32_t top_offset;
    uint32_t luma_offset;
    uint32_t luma_excursion;
    uint32_t luma_bytes_per_sample;
    uint32_t luma_active_bits;
    uint32_t color_diff_offset;
    uint32_t color_diff_excursion;
    uint32_t color_diff_bytes_per_sample;
    uint32_t color_diff_active_bits;
    uint32_t color_primaries;
    uint32_t color_matrix;
    uint32_t transfer_function;
  };

  const VideoFormat preset_formats[] = {
    // VC2ENCODER_BVF_CUSTOM
    {
      .frame_width                 = 640,
      .frame_height                = 480,
      .color_diff_format_index     = VC2ENCODER_CDS_420,
      .source_sampling             = 0,
      .top_field_first             = false,
      .frame_rate_numer            = 24000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 640,
      .clean_height                = 480,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 0,
      .luma_excursion              = 255,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 255,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_QSIF525
    {
      .frame_width                 = 176,
      .frame_height                = 120,
      .color_diff_format_index     = VC2ENCODER_CDS_420,
      .source_sampling             = 0,
      .top_field_first             = false,
      .frame_rate_numer            = 15000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 10,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 176,
      .clean_height                = 120,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 0,
      .luma_excursion              = 255,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 255,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8,
      .color_primaries             = VC2ENCODER_CPR_SDTV525,
      .color_matrix                = VC2ENCODER_CMA_SDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_QCIF
    {
      .frame_width                 = 176,
      .frame_height                = 144,
      .color_diff_format_index     = VC2ENCODER_CDS_420,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 25,
      .frame_rate_denom            = 2,
      .pixel_aspect_ratio_numer    = 12,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 176,
      .clean_height                = 144,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 0,
      .luma_excursion              = 255,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 255,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8,
      .color_primaries             = VC2ENCODER_CPR_SDTV625,
      .color_matrix                = VC2ENCODER_CMA_SDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_SIF525
    {
      .frame_width                 = 352,
      .frame_height                = 240,
      .color_diff_format_index     = VC2ENCODER_CDS_420,
      .source_sampling             = 0,
      .top_field_first             = false,
      .frame_rate_numer            = 15000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 10,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 352,
      .clean_height                = 240,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 0,
      .luma_excursion              = 255,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 255,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8,
      .color_primaries             = VC2ENCODER_CPR_SDTV525,
      .color_matrix                = VC2ENCODER_CMA_SDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_CIF
    {
      .frame_width                 = 352,
      .frame_height                = 288,
      .color_diff_format_index     = VC2ENCODER_CDS_420,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 25,
      .frame_rate_denom            = 2,
      .pixel_aspect_ratio_numer    = 12,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 352,
      .clean_height                = 288,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 0,
      .luma_excursion              = 255,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 255,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8,
      .color_primaries             = VC2ENCODER_CPR_SDTV625,
      .color_matrix                = VC2ENCODER_CMA_SDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_4SIF525
    {
      .frame_width                 = 704,
      .frame_height                = 480,
      .color_diff_format_index     = VC2ENCODER_CDS_420,
      .source_sampling             = 0,
      .top_field_first             = false,
      .frame_rate_numer            = 15000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 10,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 704,
      .clean_height                = 480,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 0,
      .luma_excursion              = 255,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 255,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8,
      .color_primaries             = VC2ENCODER_CPR_SDTV525,
      .color_matrix                = VC2ENCODER_CMA_SDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_4CIF
    {
      .frame_width                 = 704,
      .frame_height                = 576,
      .color_diff_format_index     = VC2ENCODER_CDS_420,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 25,
      .frame_rate_denom            = 2,
      .pixel_aspect_ratio_numer    = 12,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 704,
      .clean_height                = 576,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 0,
      .luma_excursion              = 255,
      .luma_bytes_per_sample       = 1,
      .luma_active_bits            = 8,
      .color_diff_offset           = 128,
      .color_diff_excursion        = 255,
      .color_diff_bytes_per_sample = 1,
      .color_diff_active_bits      = 8,
      .color_primaries             = VC2ENCODER_CPR_SDTV625,
      .color_matrix                = VC2ENCODER_CMA_SDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_SD480I_60
    {
      .frame_width                 = 720,
      .frame_height                = 480,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 1,
      .top_field_first             = false,
      .frame_rate_numer            = 30000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 10,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 704,
      .clean_height                = 480,
      .left_offset                 = 8,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_SDTV525,
      .color_matrix                = VC2ENCODER_CMA_SDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_SD576I_50
    {
      .frame_width                 = 720,
      .frame_height                = 576,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 1,
      .top_field_first             = true,
      .frame_rate_numer            = 25,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 12,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 704,
      .clean_height                = 576,
      .left_offset                 = 8,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_SDTV625,
      .color_matrix                = VC2ENCODER_CMA_SDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_HD720P_60
    {
      .frame_width                 = 1280,
      .frame_height                = 720,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 60000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 1280,
      .clean_height                = 720,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_HD720P_50
    {
      .frame_width                 = 1280,
      .frame_height                = 720,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 50,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 1280,
      .clean_height                = 720,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_HD1080I_60
    {
      .frame_width                 = 1920,
      .frame_height                = 1080,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 1,
      .top_field_first             = true,
      .frame_rate_numer            = 30000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 1920,
      .clean_height                = 1080,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_HD1080I_50
    {
      .frame_width                 = 1920,
      .frame_height                = 1080,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 1,
      .top_field_first             = true,
      .frame_rate_numer            = 25,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 1920,
      .clean_height                = 1080,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_HD1080P_60
    {
      .frame_width                 = 1920,
      .frame_height                = 1080,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 60000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 1920,
      .clean_height                = 1080,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_HD1080P_50
    {
      .frame_width                 = 1920,
      .frame_height                = 1080,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 50,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 1920,
      .clean_height                = 1080,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_DC2K
    {
      .frame_width                 = 2048,
      .frame_height                = 1080,
      .color_diff_format_index     = VC2ENCODER_CDS_444,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 24,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 2048,
      .clean_height                = 1080,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 256,
      .luma_excursion              = 3504,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 12,
      .color_diff_offset           = 2048,
      .color_diff_excursion        = 3584,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 12,
      .color_primaries             = VC2ENCODER_CPR_DCINE,
      .color_matrix                = VC2ENCODER_CMA_REVERSIBLE,
      .transfer_function           = VC2ENCODER_TRF_DCINE
    },
    // VC2ENCODER_BVF_DC4K
    {
      .frame_width                 = 4096,
      .frame_height                = 2160,
      .color_diff_format_index     = VC2ENCODER_CDS_444,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 24,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 4096,
      .clean_height                = 2160,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 256,
      .luma_excursion              = 3504,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 12,
      .color_diff_offset           = 2048,
      .color_diff_excursion        = 3584,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 12,
      .color_primaries             = VC2ENCODER_CPR_DCINE,
      .color_matrix                = VC2ENCODER_CMA_REVERSIBLE,
      .transfer_function           = VC2ENCODER_TRF_DCINE
    },
    // VC2ENCODER_BVF_UHDTV4K_60
    {
      .frame_width                 = 3840,
      .frame_height                = 2160,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 60000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 3840,
      .clean_height                = 2160,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_UHDTV4K_50
    {
      .frame_width                 = 3840,
      .frame_height                = 2160,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 50,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 3840,
      .clean_height                = 2160,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_UHDTV8K_60
    {
      .frame_width                 = 7680,
      .frame_height                = 4320,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 60000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 7680,
      .clean_height                = 4320,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_UHDTV8K_50
    {
      .frame_width                 = 7680,
      .frame_height                = 4320,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 50,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 7680,
      .clean_height                = 4320,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_HD1080P_24
    {
      .frame_width                 = 1920,
      .frame_height                = 1080,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 0,
      .top_field_first             = true,
      .frame_rate_numer            = 24,
      .frame_rate_denom            = 1,
      .pixel_aspect_ratio_numer    = 1,
      .pixel_aspect_ratio_denom    = 1,
      .clean_width                 = 1920,
      .clean_height                = 1080,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    },
    // VC2ENCODER_BVF_SDPRO486
    {
      .frame_width                 = 720,
      .frame_height                = 486,
      .color_diff_format_index     = VC2ENCODER_CDS_422,
      .source_sampling             = 1,
      .top_field_first             = false,
      .frame_rate_numer            = 30000,
      .frame_rate_denom            = 1001,
      .pixel_aspect_ratio_numer    = 10,
      .pixel_aspect_ratio_denom    = 11,
      .clean_width                 = 720,
      .clean_height                = 486,
      .left_offset                 = 0,
      .top_offset                  = 0,
      .luma_offset                 = 64,
      .luma_excursion              = 876,
      .luma_bytes_per_sample       = 2,
      .luma_active_bits            = 10,
      .color_diff_offset           = 512,
      .color_diff_excursion        = 896,
      .color_diff_bytes_per_sample = 2,
      .color_diff_active_bits      = 10,
      .color_primaries             = VC2ENCODER_CPR_HDTV,
      .color_matrix                = VC2ENCODER_CMA_HDTV,
      .transfer_function           = VC2ENCODER_TRF_TVGAMMA
    }
  };

  const uint32_t transform_block_overlap[] = {
    //  VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7
    0,
    //  VC2ENCODER_WFT_LEGALL_5_3
    2,
    //  VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7
    0,
    //  VC2ENCODER_WFT_HAAR_NO_SHIFT
    0,
    //  VC2ENCODER_WFT_HAAR_SINGLE_SHIFT
    0,
    //  VC2ENCODER_WFT_FIDELITY
    0,
    //  VC2ENCODER_WFT_DAUBECHIES_9_7
    0
  };
}



#endif /* __VIDEOFORMAT_HPP__ */
