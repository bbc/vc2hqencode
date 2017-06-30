/*****************************************************************************
 * vc2hqencode.cpp : C-compatible interface
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

#include "internal.h"
#include "VC2Encoder.hpp"

#define VC2ENCODER_BEGIN \
  try {\
    VC2Encoder *encoder = (VC2Encoder *)handle;

#define VC2ENCODER_END \
  } catch (VC2EncoderResult &r) { \
    return r;\
  } catch (...) { \
    return VC2ENCODER_UNKNOWN_ERROR;\
  }

void vc2encode_init() {
  detect_cpu_features();
}

VC2EncoderHandle vc2encode_create() {
  VC2Encoder *encoder = new VC2Encoder();
  return (VC2EncoderHandle)encoder;
}

VC2EncoderResult vc2encode_set_parameters(VC2EncoderHandle handle, VC2EncoderParams params) {
  VC2ENCODER_BEGIN

  encoder->setParams(params);
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_get_parameters(VC2EncoderHandle handle, VC2EncoderParams *params) {
  VC2ENCODER_BEGIN

  *params = encoder->getParams();
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_get_sequence_start_size(VC2EncoderHandle handle, uint32_t *size) {
  VC2ENCODER_BEGIN

  *size = ( encoder->getParseInfoSize()
            + encoder->getSequenceHeaderSize() );
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_start_sequence(VC2EncoderHandle handle, char **data, uint32_t *next_parse_offset) {
  VC2ENCODER_BEGIN

  *next_parse_offset = encoder->startSequence(data);
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_repeat_sequence_start(VC2EncoderHandle handle, char **data, uint32_t prev_parse_offset, uint32_t *next_parse_offset) {
  VC2ENCODER_BEGIN

    *next_parse_offset = encoder->repeatSequenceStart(data, prev_parse_offset);
  return VC2ENCODER_OK;

  VC2ENCODER_END}

VC2EncoderResult vc2encode_get_coded_picture_start_size(VC2EncoderHandle handle, uint32_t *size) {
  VC2ENCODER_BEGIN

  *size = ( encoder->getParseInfoSize()
            + encoder->getPictureHeaderSize()
            + encoder->getTransformParametersSize() );
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_start_picture(VC2EncoderHandle handle, char **data, uint32_t prev_parse_offset, uint32_t picture_number, int data_length, uint32_t *next_parse_offset) {
  VC2ENCODER_BEGIN

  *next_parse_offset = encoder->startPicture(data, prev_parse_offset, picture_number, data_length);
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_get_fragment_headers_for_picture_size(VC2EncoderHandle handle, uint32_t length, uint32_t *size) {
   VC2ENCODER_BEGIN

   *size = encoder->getExtraLengthForFragmentHeaders(length);
   return VC2ENCODER_OK;

   VC2ENCODER_END
}

VC2EncoderResult vc2encode_encode_data(VC2EncoderHandle handle, char **idata, int *istride, char **odata, int length) {
  VC2ENCODER_BEGIN

    if (encoder->encodeData(idata, istride, odata, length, NULL))
    return VC2ENCODER_OK;
  else
    return VC2ENCODER_ENCODE_FAILED;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_encode_fragmented_data(VC2EncoderHandle handle, char **idata, int *istride, char **odata, int length, uint32_t prev_parse_offset, uint32_t *next_parse_offset) {
  VC2ENCODER_BEGIN

  *next_parse_offset = prev_parse_offset;
  if (encoder->encodeData(idata, istride, odata, length, next_parse_offset))
    return VC2ENCODER_OK;
  else
    return VC2ENCODER_ENCODE_FAILED;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_get_auxiliary_data_start_size(VC2EncoderHandle handle, uint32_t *size) {
   VC2ENCODER_BEGIN

  *size = encoder->getParseInfoSize();
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_start_auxiliary_data(VC2EncoderHandle handle, char **data, uint32_t prev_parse_offset, int data_length, uint32_t *next_parse_offset) {
  VC2ENCODER_BEGIN

  *next_parse_offset = encoder->startAuxiliaryData(data, prev_parse_offset, data_length);
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_get_sequence_end_size(VC2EncoderHandle handle, uint32_t *size) {
  VC2ENCODER_BEGIN

  *size = encoder->getParseInfoSize();
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

VC2EncoderResult vc2encode_end_sequence(VC2EncoderHandle handle, char **data, uint32_t prev_parse_offset) {
  VC2ENCODER_BEGIN

  encoder->endSequence(data, prev_parse_offset);
  return VC2ENCODER_OK;

  VC2ENCODER_END
}

int vc2encode_get_callcounts(VC2EncoderHandle handle) {
  return ((VC2Encoder *)handle)->getCallCount();
}


void vc2encode_destroy(VC2EncoderHandle handle) {
  VC2Encoder *encoder = (VC2Encoder *)handle;
  delete encoder;
}
