/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#pragma once

#include <fastonosql/core/basic_types.h>

namespace fastonosql {
namespace gui {

typedef core::readable_string_t convert_in_t;
typedef core::readable_string_t convert_out_t;

bool string_from_json(const convert_in_t& value, convert_out_t* out);
bool string_to_json(const convert_in_t& data, convert_out_t* out);

bool string_from_hex(const convert_in_t& value, convert_out_t* out);
bool string_to_hex(const convert_in_t& data, convert_out_t* out);

bool string_from_unicode(const convert_in_t& value, convert_out_t* out);
bool string_to_unicode(const convert_in_t& data, convert_out_t* out);

// snappy
bool string_from_snappy(const convert_in_t& value, convert_out_t* out);
bool string_to_snappy(const convert_in_t& data, convert_out_t* out);

// zlib
bool string_from_zlib(const convert_in_t& value, convert_out_t* out);
bool string_to_zlib(const convert_in_t& data, convert_out_t* out);

// gzip
bool string_from_gzip(const convert_in_t& value, convert_out_t* out);
bool string_to_gzip(const convert_in_t& data, convert_out_t* out);

// lz4
bool string_from_lz4(const convert_in_t& value, convert_out_t* out);
bool string_to_lz4(const convert_in_t& data, convert_out_t* out);

// bzip2
bool string_from_bzip2(const convert_in_t& value, convert_out_t* out);
bool string_to_bzip2(const convert_in_t& data, convert_out_t* out);

// msgpack
bool string_from_msgpack(const convert_in_t& value, convert_out_t* out);
bool string_to_msgpack(const convert_in_t& data, convert_out_t* out);

// base64
bool string_from_base64(const convert_in_t& value, convert_out_t* out);
bool string_to_base64(const convert_in_t& data, convert_out_t* out);

}  // namespace gui
}  // namespace fastonosql
