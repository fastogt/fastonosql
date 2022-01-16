/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/text_converter.h"

#include <string>

#include <json-c/json_tokener.h>

#include <common/text_decoders/base64_edcoder.h>
#include <common/text_decoders/compress_bzip2_edcoder.h>
#include <common/text_decoders/compress_lz4_edcoder.h>
#include <common/text_decoders/compress_snappy_edcoder.h>
#include <common/text_decoders/compress_zlib_edcoder.h>
#include <common/text_decoders/xhex_edcoder.h>

#include <common/convert2string.h>

#include <fastonosql/core/types.h>

namespace {
struct json_object* json_tokener_parse_hacked(const char* str, int len) {
  struct json_tokener* tok = json_tokener_new();
  if (!tok) {
    return nullptr;
  }

  struct json_object* obj = json_tokener_parse_ex(tok, str, len);
  if (tok->err != json_tokener_success) {
    if (obj) {
      json_object_put(obj);
    }
    obj = nullptr;
  }

  json_tokener_free(tok);
  return obj;
}
}  // namespace

namespace fastonosql {
namespace gui {

bool string_from_json(const convert_in_t& value, convert_out_t* out) {
  if (!out || value.empty()) {
    return false;
  }

  json_object* obj = json_tokener_parse_hacked(value.data(), static_cast<int>(value.size()));
  if (!obj) {
    return false;
  }

  size_t len = 0;
  const char* jstring = json_object_to_json_string_length(obj, JSON_C_TO_STRING_PLAIN, &len);
  *out = GEN_CMD_STRING_SIZE(jstring, len);
  json_object_put(obj);
  return true;
}

bool string_to_json(const convert_in_t& data, convert_out_t* out) {
  if (!out || data.empty()) {
    return false;
  }

  json_object* obj = json_tokener_parse_hacked(data.data(), static_cast<int>(data.size()));
  if (!obj) {
    return false;
  }

  size_t len = 0;
  const char* jstring = json_object_to_json_string_length(obj, JSON_C_TO_STRING_PRETTY, &len);
  *out = GEN_CMD_STRING_SIZE(jstring, len);
  json_object_put(obj);
  return true;
}

bool string_from_hex(const convert_in_t& value, convert_out_t* out) {
  common::XHexEDcoder enc(core::ReadableString::is_lower_hex);

  convert_out_t sout;
  common::Error err = enc.Decode(value, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_to_hex(const convert_in_t& data, convert_out_t* out) {
  common::XHexEDcoder enc(core::ReadableString::is_lower_hex);

  convert_out_t sout;
  common::Error err = enc.Encode(data, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_to_unicode(const convert_in_t& data, convert_out_t* out) {
  if (!out) {
    return false;
  }

  core::command_buffer_writer_t wr;
  const common::string16 s16 = common::ConvertToString16(data);
  convert_out_t unicoded;
  common::utils::unicode::encode(s16, true, &unicoded);
  for (size_t i = 0; i < unicoded.size(); i += 4) {
    wr << "\\u";
    wr << unicoded[i];
    wr << unicoded[i + 1];
    wr << unicoded[i + 2];
    wr << unicoded[i + 3];
  }

  core::command_buffer_t result = wr.str();
  if (result.size() % 6 != 0) {
    return false;
  }

  *out = result;
  return true;
}

bool string_from_unicode(const convert_in_t& value, convert_out_t* out) {
  const size_t len = value.size();
  if (!out || len % 6 != 0) {
    return false;
  }

  std::string unicode_digits;
  for (size_t i = 0; i < len; i += 6) {
    auto c1 = value[i];
    auto c2 = value[i + 1];
    if (c1 == '\\' && c2 == 'u') {
      unicode_digits += value[i + 2];
      unicode_digits += value[i + 3];
      unicode_digits += value[i + 4];
      unicode_digits += value[i + 5];
    } else {
      return false;
    }
  }

  common::string16 s16;
  bool is_ok = common::utils::unicode::decode(unicode_digits, &s16);
  DCHECK(is_ok) << "Can't un unicoded: " << unicode_digits;
  *out = common::ConvertToCharBytes(s16);
  return true;
}

bool string_from_snappy(const convert_in_t& value, convert_out_t* out) {
  common::CompressSnappyEDcoder enc;

  convert_out_t sout;
  common::Error err = enc.Decode(value, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_to_snappy(const convert_in_t& data, convert_out_t* out) {
  common::CompressSnappyEDcoder enc;

  convert_out_t sout;
  common::Error err = enc.Encode(data, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_from_zlib(const convert_in_t& value, convert_out_t* out) {
  common::CompressZlibEDcoder enc(false, common::CompressZlibEDcoder::ZLIB_DEFLATE);

  convert_out_t sout;
  common::Error err = enc.Decode(value, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_to_zlib(const convert_in_t& data, convert_out_t* out) {
  common::CompressZlibEDcoder enc(false, common::CompressZlibEDcoder::ZLIB_DEFLATE);
  common::StringPiece piece_data(data.data(), data.size());

  convert_out_t sout;
  common::Error err = enc.Encode(piece_data, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_from_gzip(const convert_in_t& value, convert_out_t* out) {
  common::CompressZlibEDcoder enc(false, common::CompressZlibEDcoder::GZIP_DEFLATE);
  common::StringPiece piece_data(value.data(), value.size());

  convert_out_t sout;
  common::Error err = enc.Decode(value, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_to_gzip(const convert_in_t& data, convert_out_t* out) {
  common::CompressZlibEDcoder enc(false, common::CompressZlibEDcoder::GZIP_DEFLATE);

  convert_out_t sout;
  common::Error err = enc.Encode(data, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_from_lz4(const convert_in_t& value, convert_out_t* out) {
  common::CompressLZ4EDcoder enc;

  convert_out_t sout;
  common::Error err = enc.Decode(value, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_to_lz4(const convert_in_t& data, convert_out_t* out) {
  common::CompressLZ4EDcoder enc;

  convert_out_t sout;
  common::Error err = enc.Encode(data, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_from_bzip2(const convert_in_t& value, convert_out_t* out) {
  common::CompressBZip2EDcoder enc(false);
  common::StringPiece piece_data(value.data(), value.size());

  convert_out_t sout;
  common::Error err = enc.Decode(piece_data, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_to_bzip2(const convert_in_t& data, convert_out_t* out) {
  common::CompressBZip2EDcoder enc(false);
  common::StringPiece piece_data(data.data(), data.size());

  convert_out_t sout;
  common::Error err = enc.Encode(piece_data, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_from_base64(const convert_in_t& value, convert_out_t* out) {
  common::Base64EDcoder enc;

  convert_out_t sout;
  common::Error err = enc.Decode(value, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

bool string_to_base64(const convert_in_t& data, convert_out_t* out) {
  common::Base64EDcoder enc;

  convert_out_t sout;
  common::Error err = enc.Encode(data, &sout);
  if (err) {
    return false;
  }

  *out = sout;
  return true;
}

}  // namespace gui
}  // namespace fastonosql
