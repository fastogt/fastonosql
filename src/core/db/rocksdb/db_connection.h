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

#include "core/internal/cdb_connection.h"

#include "core/db/rocksdb/config.h"
#include "core/db/rocksdb/server_info.h"

namespace rocksdb {
class Status;
}  // namespace rocksdb

namespace fastonosql {
namespace core {
namespace rocksdb {

class rocksdb_handle;
typedef rocksdb_handle NativeConnection;

common::Error CreateConnection(const Config& config, NativeConnection** context);
common::Error TestConnection(const Config& config);

class DBConnection : public core::internal::CDBConnection<NativeConnection, Config, ROCKSDB> {
 public:
  typedef core::internal::CDBConnection<NativeConnection, Config, ROCKSDB> base_class;
  explicit DBConnection(CDBConnectionClient* client);

  virtual std::string GetCurrentDBName() const override;

  common::Error Info(const std::string& args, ServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error Mget(const std::vector<std::string>& keys, std::vector<std::string>* ret);
  common::Error Merge(const std::string& key, const std::string& value) WARN_UNUSED_RESULT;

 private:
  common::Error CheckResultCommand(const std::string& cmd, const ::rocksdb::Status& err) WARN_UNUSED_RESULT;

  common::Error SetInner(const key_t& key, const value_t& value) WARN_UNUSED_RESULT;
  common::Error GetInner(const key_t& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error DelInner(const key_t& key) WARN_UNUSED_RESULT;

  virtual common::Error ScanImpl(cursor_t cursor_in,
                                 const std::string& pattern,
                                 keys_limit_t count_keys,
                                 std::vector<std::string>* keys_out,
                                 cursor_t* cursor_out) override;
  virtual common::Error KeysImpl(const std::string& key_start,
                                 const std::string& key_end,
                                 keys_limit_t limit,
                                 std::vector<std::string>* ret) override;
  virtual common::Error DBkcountImpl(size_t* size) override;
  virtual common::Error FlushDBImpl() override;
  virtual common::Error CreateDBImpl(const std::string& name, IDataBaseInfo** info) override;
  virtual common::Error RemoveDBImpl(const std::string& name, IDataBaseInfo** info) override;
  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) override;
  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) override;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) override;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) override;
  virtual common::Error RenameImpl(const NKey& key, const key_t& new_key) override;
  virtual common::Error QuitImpl() override;
  virtual common::Error ConfigGetDatabasesImpl(std::vector<std::string>* dbs) override;
};

}  // namespace rocksdb
}  // namespace core
}  // namespace fastonosql
