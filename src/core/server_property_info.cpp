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

#include "core/server_property_info.h"

#include <stddef.h>                     // for size_t

#include <vector>                       // for vector

#include "common/macros.h"              // for DCHECK, DNOTREACHED
#include "common/value.h"               // for ArrayValue

#include "global/global.h"              // for FastoObjectArray

namespace fastonosql {
namespace core {

ServerPropertiesInfo makeServerProperty(const FastoObjectArray* array) {
  if (!array) {
    DNOTREACHED();
    return ServerPropertiesInfo();
  }

  common::ArrayValue* ar = array->array();
  if (!ar) {
    DNOTREACHED();
    return ServerPropertiesInfo();
  }

  ServerPropertiesInfo inf;
  for (size_t i = 0; i < ar->size(); i += 2) {
    std::string c1;
    std::string c2;
    bool res = ar->getString(i, &c1);
    DCHECK(res);
    res = ar->getString(i + 1, &c2);
    DCHECK(res);
    inf.properties.push_back(std::make_pair(c1, c2));
  }
  return inf;
}

}  // namespace core
}  // namespace fastonosql
