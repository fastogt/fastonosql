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

#include "core/db_key.h"  // for NDbKValue, NKey, NKeys, ttl_t

namespace fastonosql {
namespace core {

class IDataBaseInfo;

class CDBConnectionClient {
 public:
  virtual void OnFlushedCurrentDB() = 0;
  virtual void OnCurrentDataBaseChanged(IDataBaseInfo* info) = 0;
  virtual void OnKeysRemoved(const NKeys& keys) = 0;
  virtual void OnKeyAdded(const NDbKValue& key) = 0;
  virtual void OnKeyLoaded(const NDbKValue& key) = 0;
  virtual void OnKeyRenamed(const NKey& key, const string_key_t& new_key) = 0;
  virtual void OnKeyTTLChanged(const NKey& key, ttl_t ttl) = 0;
  virtual void OnKeyTTLLoaded(const NKey& key, ttl_t ttl) = 0;
  virtual void OnQuited() = 0;
  virtual ~CDBConnectionClient();
};

}  // namespace core
}  // namespace fastonosql
