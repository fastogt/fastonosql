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

#include "core/connection_types.h"

#include <string>

namespace {
  const std::string connnectionMode[] = { "Latency mode", "Slave mode",
                                          "Get RDB mode", "Pipe mode",
                                          "Find big keys mode", "Stat mode",
                                          "Scan mode", "Interactive mode" };
  const std::string serverTypes[] = { "Master", "Slave" };
}  // namespace

namespace common {

template<>
fastonosql::core::connectionTypes convertFromString(const std::string& text) {
  for (size_t i = 0; i < SIZEOFMASS(fastonosql::core::connnectionType); ++i) {
    if (text == fastonosql::core::connnectionType[i]) {
      return static_cast<fastonosql::core::connectionTypes>(i + fastonosql::core::connectionTypes::REDIS);
    }
  }

  NOTREACHED();
  return fastonosql::core::REDIS;
}

std::string convertToString(fastonosql::core::connectionTypes t) {
  return fastonosql::core::connnectionType[t];
}

template<>
fastonosql::core::serverTypes convertFromString(const std::string& text) {
  for (size_t i = 0; i < SIZEOFMASS(serverTypes); ++i) {
    if (text == serverTypes[i]) {
      return static_cast<fastonosql::core::serverTypes>(i + fastonosql::core::serverTypes::MASTER);
    }
  }

  NOTREACHED();
  return fastonosql::core::MASTER;
}

std::string convertToString(fastonosql::core::serverTypes st) {
  return serverTypes[st];
}

std::string convertToString(fastonosql::core::ConnectionMode t) {
  return connnectionMode[t];
}

}  // namespace common
