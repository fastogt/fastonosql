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

#include "core/server_property_info.h"

namespace fastonosql {
namespace core {

ServerPropertiesInfo MakeServerProperty(const common::ArrayValue* array) {
  if (!array) {
    DNOTREACHED();
    return ServerPropertiesInfo();
  }

  std::vector<property_t> properties;
  for (size_t i = 0; i < array->GetSize(); i += 2) {
    std::string key, value;
    if (array->GetString(i, &key) && array->GetString(i + 1, &value)) {
      properties.push_back(std::make_pair(key, value));
    }
  }

  return {properties};
}

}  // namespace core
}  // namespace fastonosql
