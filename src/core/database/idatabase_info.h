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

#include <stddef.h>  // for size_t

#include <memory>  // for shared_ptr
#include <string>  // for string
#include <vector>  // for vector

#include <common/types.h>  // for ClonableBase

#include "core/connection_types.h"  // for connectionTypes
#include "core/db_key.h"            // for NDbKValue

namespace common {
class Value;
}

namespace fastonosql {
namespace core {

class IDataBaseInfo : public common::ClonableBase<IDataBaseInfo> {
 public:
  typedef std::vector<NDbKValue> keys_container_t;
  connectionTypes type() const;
  std::string name() const;
  size_t dbKeysCount() const;
  void setDBKeysCount(size_t size);
  size_t loadedKeysCount() const;

  bool isDefault() const;
  void setIsDefault(bool isDef);

  virtual ~IDataBaseInfo();

  keys_container_t keys() const;
  void setKeys(const keys_container_t& keys);
  void clearKeys();

 protected:
  IDataBaseInfo(const std::string& name,
                bool isDefault,
                connectionTypes type,
                size_t dbkcount,
                const keys_container_t& keys);

 private:
  const std::string name_;
  bool is_default_;
  size_t db_kcount_;
  keys_container_t keys_;

  const connectionTypes type_;
};

typedef common::shared_ptr<IDataBaseInfo> IDataBaseInfoSPtr;

}  // namespace core
}  // namespace fastonosql
