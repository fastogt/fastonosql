/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/text_converter.h"

#include <json-c/json_tokener.h>

#include <common/text_decoders/compress_bzip2_edcoder.h>   // for CompressEDcoder
#include <common/text_decoders/compress_lz4_edcoder.h>     // for CompressEDcoder
#include <common/text_decoders/compress_snappy_edcoder.h>  // for CompressEDcoder
#include <common/text_decoders/compress_zlib_edcoder.h>    // for CompressEDcoder
#include <common/text_decoders/hex_edcoder.h>              // for HexEDcoder
#include <common/text_decoders/msgpack_edcoder.h>          // for MsgPackEDcoder

#include <fastonosql/core/types.h>

namespace fastonosql {
namespace gui {

bool string_from_json(const core::readable_string_t& data, std::string* out) {
  if (data.empty()) {
    return false;
  }

  json_object* obj = json_tokener_parse(data.c_str());
  if (!obj) {
    return false;
  }

  std::string jstring = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY);
  json_object_put(obj);
  *out = jstring;
  return true;
}

bool string_to_hex(const core::readable_string_t& data, std::string* out) {
  std::string result = core::detail::hex_string(data);
  if (result.empty()) {
    return false;
  }

  *out = result;
  return true;
}

bool string_to_unicode(const core::readable_string_t& data, std::string* out) {
  std::string result = core::detail::unicode_string(data);
  if (result.empty()) {
    return false;
  }

  *out = result;
  return true;
}

bool string_from_snappy(const core::readable_string_t& data, std::string* out) {
  common::CompressSnappyEDcoder enc;
  common::Error err = enc.Decode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_from_zlib(const core::readable_string_t& data, std::string* out) {
  common::CompressZlibEDcoder enc;
  common::Error err = enc.Decode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_from_lz4(const core::readable_string_t& data, std::string* out) {
  common::CompressLZ4EDcoder enc;
  common::Error err = enc.Decode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_from_bzip2(const core::readable_string_t& data, std::string* out) {
  common::CompressBZip2EDcoder enc;
  common::Error err = enc.Decode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_from_msgpack(const core::readable_string_t& data, std::string* out) {
  common::MsgPackEDcoder enc;
  common::Error err = enc.Decode(data, out);
  if (err) {
    return false;
  }

  return true;
}

}  // namespace gui
}  // namespace fastonosql
