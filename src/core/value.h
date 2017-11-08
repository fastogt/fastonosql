/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <common/value.h>  // for ArrayValue (ptr only), etc

namespace fastonosql {
namespace core {

namespace detail {
bool have_space(const std::string& data);
std::string hex_string(const common::buffer_t& value);
std::string hex_string(const std::string& value);
std::string string_from_hex(const common::buffer_t& value);
std::string string_from_hex(const std::string& value);
}  // namespace detail

std::string ConvertValue(common::Value* value, const std::string& delimiter, bool for_cmd);
std::string ConvertValue(common::ArrayValue* array, const std::string& delimiter, bool for_cmd);
std::string ConvertValue(common::SetValue* set, const std::string& delimiter, bool for_cmd);
std::string ConvertValue(common::ZSetValue* zset, const std::string& delimiter, bool for_cmd);
std::string ConvertValue(common::HashValue* hash, const std::string& delimiter, bool for_cmd);
std::string ConvertValue(common::FundamentalValue* value, const std::string& delimiter, bool for_cmd);
std::string ConvertValue(common::StringValue* value, const std::string& delimiter, bool for_cmd);
std::string ConvertValue(common::JsonValue* value, const std::string& delimiter, bool for_cmd);
std::string ConvertValue(common::ByteArrayValue* value, const std::string& delimiter, bool for_cmd);

std::string ConvertToHumanReadable(common::Value* value, const std::string& delimiter = std::string());

}  // namespace core
}  // namespace fastonosql
