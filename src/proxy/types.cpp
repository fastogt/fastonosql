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

#include "proxy/types.h"

#include <common/macros.h>

namespace fastonosql {
namespace proxy {

const std::vector<const char*> g_display_strategy_types = {"FULL_KEY", "KEY_NAME"};

const std::vector<const char*> g_supported_views_text = {"Tree", "Table", "Text"};

}  // namespace proxy
}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::proxy::SupportedViews v) {
  return fastonosql::proxy::g_supported_views_text[v];
}

bool ConvertFromString(const std::string& from, fastonosql::proxy::SupportedViews* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < fastonosql::proxy::g_supported_views_text.size(); ++i) {
    if (from == fastonosql::proxy::g_supported_views_text[i]) {
      *out = static_cast<fastonosql::proxy::SupportedViews>(i);
      return true;
    }
  }

  NOTREACHED();
  return false;
}

}  // namespace common
