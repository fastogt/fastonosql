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

#include "core/database/idatabase_info.h"

#include <algorithm>  // for remove_if

namespace fastonosql {
namespace core {

IDataBaseInfo::IDataBaseInfo(const std::string& name, bool isDefault, size_t dbkcount, const keys_container_t& keys)
    : name_(name), is_default_(isDefault), db_kcount_(dbkcount), keys_(keys) {}

IDataBaseInfo::~IDataBaseInfo() {}

std::string IDataBaseInfo::GetName() const {
  return name_;
}

size_t IDataBaseInfo::GetDBKeysCount() const {
  return db_kcount_;
}

void IDataBaseInfo::SetDBKeysCount(size_t size) {
  db_kcount_ = size;
}

size_t IDataBaseInfo::LoadedKeysCount() const {
  return keys_.size();
}

bool IDataBaseInfo::IsDefault() const {
  return is_default_;
}

void IDataBaseInfo::SetIsDefault(bool isDef) {
  is_default_ = isDef;
}

void IDataBaseInfo::SetKeys(const keys_container_t& keys) {
  keys_ = keys;
}

void IDataBaseInfo::ClearKeys() {
  keys_.clear();
}

bool IDataBaseInfo::RenameKey(const NKey& okey, const key_t& new_name) {
  for (auto& kv : keys_) {
    NKey cur_key = kv.GetKey();
    if (cur_key.GetKey() == okey.GetKey()) {
      NKey okv = kv.GetKey();
      okv.SetKey(new_name);
      kv.SetKey(okv);
      return true;
    }
  }

  return false;
}

bool IDataBaseInfo::InsertKey(const NDbKValue& key) {
  const NKey in_key = key.GetKey();
  for (auto& kv : keys_) {
    NKey cur_key = kv.GetKey();
    if (cur_key.GetKey() == in_key.GetKey()) {
      kv.SetValue(key.GetValue());
      return false;
    }
  }

  keys_.push_back(key);
  db_kcount_++;
  return true;
}

bool IDataBaseInfo::UpdateKeyTTL(const NKey& key, ttl_t ttl) {
  for (auto& kv : keys_) {
    NKey cur_key = kv.GetKey();
    if (cur_key.GetKey() == key.GetKey()) {
      NKey okv = kv.GetKey();
      if (okv.GetTTL() == ttl) {
        return false;
      }

      okv.SetTTL(ttl);
      kv.SetKey(okv);
      return true;
    }
  }

  return false;
}

bool IDataBaseInfo::RemoveKey(const NKey& key) {
  auto it = std::remove_if(keys_.begin(), keys_.end(), [key](NDbKValue kv) {
    const NKey in_key = kv.GetKey();
    return in_key.GetKey() == key.GetKey();
  });
  if (it == keys_.end()) {
    return false;
  }

  keys_.erase(it);
  db_kcount_--;
  return true;
}

IDataBaseInfo::keys_container_t IDataBaseInfo::GetKeys() const {
  return keys_;
}

}  // namespace core
}  // namespace fastonosql
