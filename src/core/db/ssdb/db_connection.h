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

#include "core/db/ssdb/config.h"
#include "core/db/ssdb/server_info.h"

namespace ssdb {
class Client;
class Status;
}  // namespace ssdb

namespace fastonosql {
namespace core {
namespace ssdb {

typedef ::ssdb::Client NativeConnection;

common::Error CreateConnection(const Config& config, NativeConnection** context);
common::Error TestConnection(const Config& config);

class DBConnection : public core::internal::CDBConnection<NativeConnection, Config, SSDB> {
 public:
  typedef core::internal::CDBConnection<NativeConnection, Config, SSDB> base_class;
  explicit DBConnection(CDBConnectionClient* client);

  virtual bool IsAuthenticated() const override;

  virtual common::Error Connect(const config_t& config) override WARN_UNUSED_RESULT;
  virtual common::Error Disconnect() override WARN_UNUSED_RESULT;

  common::Error Info(const std::string& args, ServerInfo::Stats* statsout) WARN_UNUSED_RESULT;
  common::Error Auth(const std::string& password) WARN_UNUSED_RESULT;
  common::Error Setx(const std::string& key, const std::string& value, ttl_t ttl) WARN_UNUSED_RESULT;
  common::Error Incr(const std::string& key, int64_t incrby, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error ScanSsdb(const std::string& key_start,
                         const std::string& key_end,
                         uint64_t limit,
                         std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Rscan(const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiGet(const std::vector<std::string>& keys, std::vector<std::string>* ret);
  common::Error MultiSet(const std::map<std::string, std::string>& kvs) WARN_UNUSED_RESULT;
  common::Error MultiDel(const std::vector<std::string>& keys) WARN_UNUSED_RESULT;
  common::Error Hget(const std::string& name, const std::string& key, std::string* val) WARN_UNUSED_RESULT;
  common::Error Hgetall(const std::string& name, std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Hset(const std::string& name, const std::string& key, const std::string& val) WARN_UNUSED_RESULT;
  common::Error Hdel(const std::string& name, const std::string& key) WARN_UNUSED_RESULT;
  common::Error Hincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Hsize(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Hclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Hkeys(const std::string& name,
                      const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Hscan(const std::string& name,
                      const std::string& key_start,
                      const std::string& key_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Hrscan(const std::string& name,
                       const std::string& key_start,
                       const std::string& key_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiHget(const std::string& name,
                          const std::vector<std::string>& keys,
                          std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiHset(const std::string& name, const std::map<std::string, std::string>& keys) WARN_UNUSED_RESULT;
  common::Error Zget(const std::string& name, const std::string& key, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zset(const std::string& name, const std::string& key, int64_t score) WARN_UNUSED_RESULT;
  common::Error Zdel(const std::string& name, const std::string& key) WARN_UNUSED_RESULT;
  common::Error Zincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zsize(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zrank(const std::string& name, const std::string& key, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zrrank(const std::string& name, const std::string& key, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error Zrange(const std::string& name, uint64_t offset, uint64_t limit, std::vector<std::string>* ret)
      WARN_UNUSED_RESULT;
  common::Error Zrrange(const std::string& name, uint64_t offset, uint64_t limit, std::vector<std::string>* ret)
      WARN_UNUSED_RESULT;
  common::Error Zkeys(const std::string& name,
                      const std::string& key_start,
                      int64_t* score_start,
                      int64_t* score_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Zscan(const std::string& name,
                      const std::string& key_start,
                      int64_t* score_start,
                      int64_t* score_end,
                      uint64_t limit,
                      std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error Zrscan(const std::string& name,
                       const std::string& key_start,
                       int64_t* score_start,
                       int64_t* score_end,
                       uint64_t limit,
                       std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiZget(const std::string& name,
                          const std::vector<std::string>& keys,
                          std::vector<std::string>* ret) WARN_UNUSED_RESULT;
  common::Error MultiZset(const std::string& name, const std::map<std::string, int64_t>& kss);
  common::Error MultiZdel(const std::string& name, const std::vector<std::string>& keys) WARN_UNUSED_RESULT;
  common::Error Qpush(const std::string& name, const std::string& item) WARN_UNUSED_RESULT;
  common::Error Qpop(const std::string& name, std::string* item) WARN_UNUSED_RESULT;
  common::Error Qslice(const std::string& name, int64_t begin, int64_t end, std::vector<std::string>* ret)
      WARN_UNUSED_RESULT;
  common::Error Qclear(const std::string& name, int64_t* ret) WARN_UNUSED_RESULT;
  common::Error DBsize(int64_t* size) WARN_UNUSED_RESULT;

  common::Error Expire(key_t key, ttl_t ttl) WARN_UNUSED_RESULT;
  common::Error TTL(key_t key, ttl_t* ttl) WARN_UNUSED_RESULT;

 private:
  common::Error SetInner(const key_t& key, const value_t& value) WARN_UNUSED_RESULT;
  common::Error GetInner(const key_t& key, std::string* ret_val) WARN_UNUSED_RESULT;
  common::Error DelInner(const key_t& key) WARN_UNUSED_RESULT;

  virtual common::Error ConfigGetDatabasesImpl(std::vector<std::string>* dbs) override;
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
  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) override;
  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) override;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) override;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) override;
  virtual common::Error RenameImpl(const NKey& key, const key_t& new_key) override;
  virtual common::Error SetTTLImpl(const NKey& key, ttl_t ttl) override;
  virtual common::Error GetTTLImpl(const NKey& key, ttl_t* ttl) override;
  virtual common::Error QuitImpl() override;

 private:
  common::Error CheckResultCommand(const std::string& cmd, const ::ssdb::Status& err) WARN_UNUSED_RESULT;
  bool is_auth_;
};

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
