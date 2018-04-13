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

#include "core/db_key.h"  // for NDbKValue, NKey, NKeys, ttl_t
#include "core/module_info.h"

namespace fastonosql {
namespace core {

class IDataBaseInfo;

class CDBConnectionClient {
 public:
  virtual void OnCreatedDB(IDataBaseInfo* info) = 0;
  virtual void OnRemovedDB(IDataBaseInfo* info) = 0;

  virtual void OnFlushedCurrentDB() = 0;
  virtual void OnChangedCurrentDB(IDataBaseInfo* info) = 0;

  virtual void OnRemovedKeys(const NKeys& keys) = 0;
  virtual void OnAddedKey(const NDbKValue& key) = 0;
  virtual void OnLoadedKey(const NDbKValue& key) = 0;
  virtual void OnRenamedKey(const NKey& key, const key_t& new_key) = 0;
  virtual void OnChangedKeyTTL(const NKey& key, ttl_t ttl) = 0;
  virtual void OnLoadedKeyTTL(const NKey& key, ttl_t ttl) = 0;

  virtual void OnUnLoadedModule(const ModuleInfo& module) = 0;
  virtual void OnLoadedModule(const ModuleInfo& module) = 0;

  virtual void OnQuited() = 0;

  virtual ~CDBConnectionClient();
};

}  // namespace core
}  // namespace fastonosql
