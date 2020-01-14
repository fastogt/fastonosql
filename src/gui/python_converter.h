/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

bool string_from_pickle(const convert_in_t& value, convert_out_t* out);
bool string_to_pickle(const convert_in_t& data, convert_out_t* out);

}  // namespace gui
}  // namespace fastonosql
