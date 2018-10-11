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
#include <common/text_decoders/msgpack_edcoder.h>          // for MsgPackEDcoder
#include <common/text_decoders/xhex_edcoder.h>             // for XHexEDcoder

#include <common/convert2string.h>

#include <fastonosql/core/types.h>

namespace fastonosql {
namespace gui {

bool string_from_json(const std::string& value, std::string* out) {
  *out = value;
  return true;
}

bool string_to_json(const core::readable_string_t& data, std::string* out) {
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

bool string_from_hex(const std::string& value, std::string* out) {
  common::XHexEDcoder enc(core::ReadableString::is_lower_hex);
  common::Error err = enc.Decode(value, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_to_hex(const core::readable_string_t& data, std::string* out) {
  common::XHexEDcoder enc(core::ReadableString::is_lower_hex);
  common::Error err = enc.Encode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_to_unicode(const core::readable_string_t& data, std::string* out) {
  std::ostringstream wr;
  common::string16 s16 = common::ConvertToString16(data);
  std::string unicoded = common::utils::unicode::encode(s16, true);
  for (size_t i = 0; i < unicoded.size(); i += 4) {
    wr << "\\u";
    wr << unicoded[i];
    wr << unicoded[i + 1];
    wr << unicoded[i + 2];
    wr << unicoded[i + 3];
  }

  std::string result = wr.str();
  if (result.size() % 6 != 0) {
    return false;
  }

  *out = result;
  return true;
}

bool string_from_unicode(const std::string& value, std::string* out) {
  size_t len = value.size();
  if (len % 6 != 0) {
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

  common::string16 s16 = common::utils::unicode::decode(unicode_digits);
  *out = common::ConvertToString(s16);
  return true;
}

bool string_from_snappy(const std::string& value, std::string* out) {
  common::CompressSnappyEDcoder enc;
  common::Error err = enc.Decode(value, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_to_snappy(const core::readable_string_t& data, std::string* out) {
  common::CompressSnappyEDcoder enc;
  common::Error err = enc.Encode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_from_zlib(const std::string& value, std::string* out) {
  common::CompressZlibEDcoder enc;
  common::Error err = enc.Decode(value, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_to_zlib(const core::readable_string_t& data, std::string* out) {
  common::CompressZlibEDcoder enc;
  common::Error err = enc.Encode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_from_lz4(const std::string& value, std::string* out) {
  common::CompressLZ4EDcoder enc;
  common::Error err = enc.Decode(value, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_to_lz4(const core::readable_string_t& data, std::string* out) {
  common::CompressLZ4EDcoder enc;
  common::Error err = enc.Encode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_from_bzip2(const std::string& value, std::string* out) {
  common::CompressBZip2EDcoder enc;
  common::Error err = enc.Decode(value, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_to_bzip2(const core::readable_string_t& data, std::string* out) {
  common::CompressBZip2EDcoder enc;
  common::Error err = enc.Encode(data, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_from_msgpack(const std::string& value, std::string* out) {
  common::MsgPackEDcoder enc;
  common::Error err = enc.Decode(value, out);
  if (err) {
    return false;
  }

  return true;
}

bool string_to_msgpack(const core::readable_string_t& data, std::string* out) {
  common::MsgPackEDcoder enc;
  common::Error err = enc.Encode(data, out);
  if (err) {
    return false;
  }

  return true;
}

}  // namespace gui
}  // namespace fastonosql
