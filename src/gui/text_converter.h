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

#pragma once

#include <fastonosql/core/types.h>

namespace fastonosql {
namespace gui {

bool string_from_json(const core::readable_string_t& data, std::string* out);
bool string_to_hex(const core::readable_string_t& data, std::string* out);
bool string_to_unicode(const core::readable_string_t& data, std::string* out);
bool string_from_snappy(const core::readable_string_t& data, std::string* out);
bool string_from_zlib(const core::readable_string_t& data, std::string* out);
bool string_from_lz4(const core::readable_string_t& data, std::string* out);
bool string_from_bzip2(const core::readable_string_t& data, std::string* out);
bool string_from_msgpack(const core::readable_string_t& data, std::string* out);

}  // namespace gui
}  // namespace fastonosql
