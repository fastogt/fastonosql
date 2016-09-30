/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include "global/types.h"

#include <stddef.h>  // for size_t

#include <common/convert2string.h>
#include <common/macros.h>  // for SIZEOFMASS

namespace fastonosql {}  // namespace fastonosql

namespace common {

std::string ConvertToString(fastonosql::supportedViews v) {
  return fastonosql::viewsText[v];
}

template <>
fastonosql::supportedViews ConvertFromString(const std::string& from) {
  for (size_t i = 0; i < SIZEOFMASS(fastonosql::viewsText); ++i) {
    if (from == fastonosql::viewsText[i]) {
      return static_cast<fastonosql::supportedViews>(i);
    }
  }

  return fastonosql::Tree;
}

}  // namespace common
