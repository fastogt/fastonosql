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

#include <string>
#include <vector>

namespace fastonosql {
namespace proxy {

enum NsDisplayStrategy { FULL_KEY = 0, KEY_NAME = 1 };
extern const std::vector<const char*> g_display_strategy_types;

enum SupportedViews { kTree = 0, kTable, kText };
extern const std::vector<const char*> g_supported_views_text;

}  // namespace proxy
}  // namespace fastonosql

namespace common {
std::string ConvertToString(fastonosql::proxy::SupportedViews v);
bool ConvertFromString(const std::string& from, fastonosql::proxy::SupportedViews* out);
}  // namespace common
