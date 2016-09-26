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

#pragma once

#include "core/idatabase.h"

namespace fastonosql {
namespace core {

class CDBConnectionClient {
 public:
  virtual void onCurrentDataBaseChanged(IDataBaseInfo* info) = 0;
  virtual void onKeysRemoved(const keys_t& keys) = 0;
  virtual void onKeyAdded(const key_and_value_t& key) = 0;
  virtual void onKeyLoaded(const key_and_value_t& key) = 0;
  virtual void onKeyTTLChanged(const key_t& key, ttl_t ttl) = 0;
};
}  // namespace core
}  // namespace fastonosql
