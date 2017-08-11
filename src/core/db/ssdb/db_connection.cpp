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

#include "core/db/ssdb/db_connection.h"

#include <memory>  // for __shared_ptr

#include <SSDB.h>  // for Status, Client

#include <common/convert2string.h>  // for ConvertFromString
#include <common/net/types.h>       // for HostAndPort
#include <common/sprintf.h>         // for MemSPrintf
#include <common/value.h>           // for Value, etc

#include "core/db/ssdb/command_translator.h"
#include "core/db/ssdb/config.h"  // for Config
#include "core/db/ssdb/database_info.h"
#include "core/db/ssdb/internal/commands_api.h"

namespace fastonosql {
namespace core {
namespace {
std::string ConvertToSSDBSlice(const key_t& key) {
  return common::ConvertToString(key.ToString());
}
}  // namespace
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<ssdb::NativeConnection, ssdb::Config>::Connect(const ssdb::Config& config,
                                                                                       ssdb::NativeConnection** hout) {
  ssdb::NativeConnection* context = nullptr;
  common::Error er = ssdb::CreateConnection(config, &context);
  if (er && er->IsError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<ssdb::NativeConnection, ssdb::Config>::Disconnect(
    ssdb::NativeConnection** handle) {
  destroy(handle);
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<ssdb::NativeConnection, ssdb::Config>::IsConnected(ssdb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const char* CDBConnection<ssdb::NativeConnection, ssdb::Config, SSDB>::BasedOn() {
  return "ssdb";
}

template <>
const char* CDBConnection<ssdb::NativeConnection, ssdb::Config, SSDB>::VersionApi() {
  return "1.9.4";
}

template <>
ConstantCommandsArray CDBConnection<ssdb::NativeConnection, ssdb::Config, SSDB>::Commands() {
  return ssdb::g_commands;
}
}  // namespace internal
namespace ssdb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  ::ssdb::Client* lcontext = ::ssdb::Client::connect(config.host.host, config.host.port);
  if (!lcontext) {
    return common::make_error_value("Fail connect to server!", common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  ::ssdb::Client* ssdb = nullptr;
  common::Error er = CreateConnection(config, &ssdb);
  if (er && er->IsError()) {
    return er;
  }

  delete ssdb;
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::Commands())) {}

common::Error DBConnection::Info(const std::string& args, ServerInfo::Stats* statsout) {
  if (!statsout) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::vector<std::string> ret;
  auto st = connection_.handle_->info(args, &ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("info function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  ServerInfo::Stats lstatsout;
  for (size_t i = 0; i < ret.size(); i += 2) {
    if (ret[i] == SSDB_VERSION_LABEL) {
      lstatsout.version = ret[i + 1];
    } else if (ret[i] == SSDB_LINKS_LABEL) {
      uint32_t links;
      if (common::ConvertFromString(ret[i + 1], &links)) {
        lstatsout.links = links;
      }
    } else if (ret[i] == SSDB_TOTAL_CALLS_LABEL) {
      uint32_t total_calls;
      if (common::ConvertFromString(ret[i + 1], &total_calls)) {
        lstatsout.total_calls = total_calls;
      }
    } else if (ret[i] == SSDB_DBSIZE_LABEL) {
      uint32_t dbsize;
      if (common::ConvertFromString(ret[i + 1], &dbsize)) {
        lstatsout.dbsize = dbsize;
      }
    } else if (ret[i] == SSDB_BINLOGS_LABEL) {
      lstatsout.binlogs = ret[i + 1];
    }
  }

  *statsout = lstatsout;
  return common::Error();
}

common::Error DBConnection::DBsize(int64_t* size) {
  if (!size) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  int64_t ret;
  auto st = connection_.handle_->dbsize(&ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Couldn't determine DBSIZE error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = ret;
  return common::Error();
}

common::Error DBConnection::Auth(const std::string& password) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->auth(password);
  if (st.error()) {
    std::string buff = common::MemSPrintf("password function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Setx(const std::string& key, const std::string& value, ttl_t ttl) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->setx(key, value, static_cast<int>(ttl));
  if (st.error()) {
    std::string buff = common::MemSPrintf("setx function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::SetInner(key_t key, const std::string& value) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  const std::string key_slice = ConvertToSSDBSlice(key);
  auto st = connection_.handle_->set(key_slice, value);
  if (st.error()) {
    std::string buff = common::MemSPrintf("SET function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  const std::string key_slice = ConvertToSSDBSlice(key);
  auto st = connection_.handle_->get(key_slice, ret_val);
  if (st.error()) {
    std::string buff = common::MemSPrintf("GET function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::DelInner(key_t key) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  const std::string key_slice = ConvertToSSDBSlice(key);
  auto st = connection_.handle_->del(key_slice);
  if (st.error()) {
    std::string buff = common::MemSPrintf("del function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Incr(const std::string& key, int64_t incrby, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->incr(key, incrby, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Incr function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Scan(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->scan(key_start, key_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("scan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Rscan(const std::string& key_start,
                                  const std::string& key_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->rscan(key_start, key_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("rscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::MultiGet(const std::vector<std::string>& keys, std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->multi_get(keys, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("multi_get function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::MultiSet(const std::map<std::string, std::string>& kvs) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->multi_set(kvs);
  if (st.error()) {
    std::string buff = common::MemSPrintf("multi_set function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::MultiDel(const std::vector<std::string>& keys) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->multi_del(keys);
  if (st.error()) {
    std::string buff = common::MemSPrintf("multi_del function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hget(const std::string& name, const std::string& key, std::string* val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hget(name, key, val);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hget function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hgetall(const std::string& name, std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hgetall(name, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hgetall function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hset(const std::string& name, const std::string& key, const std::string& val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hset(name, key, val);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error DBConnection::Hdel(const std::string& name, const std::string& key) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hdel(name, key);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hdel function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hincr(name, key, incrby, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hincr function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hsize(const std::string& name, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hsize(name, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hclear(const std::string& name, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hclear(name, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hclear function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hkeys(const std::string& name,
                                  const std::string& key_start,
                                  const std::string& key_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hkeys(name, key_start, key_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hkeys function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hscan(const std::string& name,
                                  const std::string& key_start,
                                  const std::string& key_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hscan(name, key_start, key_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Hrscan(const std::string& name,
                                   const std::string& key_start,
                                   const std::string& key_end,
                                   uint64_t limit,
                                   std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->hrscan(name, key_start, key_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hrscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::MultiHget(const std::string& name,
                                      const std::vector<std::string>& keys,
                                      std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->multi_hget(name, keys, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("hrscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::MultiHset(const std::string& name, const std::map<std::string, std::string>& keys) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->multi_hset(name, keys);
  if (st.error()) {
    std::string buff = common::MemSPrintf("multi_hset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zget(const std::string& name, const std::string& key, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zget(name, key, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zget function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zset(const std::string& name, const std::string& key, int64_t score) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zset(name, key, score);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zdel(const std::string& name, const std::string& key) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zdel(name, key);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Zdel function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zincr(name, key, incrby, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Zincr function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zsize(const std::string& name, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zsize(name, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zsize function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zclear(const std::string& name, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zclear(name, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zclear function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zrank(const std::string& name, const std::string& key, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zrank(name, key, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zrank function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zrrank(const std::string& name, const std::string& key, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zrrank(name, key, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zrrank function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zrange(const std::string& name,
                                   uint64_t offset,
                                   uint64_t limit,
                                   std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zrange(name, offset, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zrange function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zrrange(const std::string& name,
                                    uint64_t offset,
                                    uint64_t limit,
                                    std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zrrange(name, offset, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zrrange function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zkeys(const std::string& name,
                                  const std::string& key_start,
                                  int64_t* score_start,
                                  int64_t* score_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zkeys(name, key_start, score_start, score_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zkeys function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zscan(const std::string& name,
                                  const std::string& key_start,
                                  int64_t* score_start,
                                  int64_t* score_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zscan(name, key_start, score_start, score_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Zrscan(const std::string& name,
                                   const std::string& key_start,
                                   int64_t* score_start,
                                   int64_t* score_end,
                                   uint64_t limit,
                                   std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->zrscan(name, key_start, score_start, score_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("zrscan function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::MultiZget(const std::string& name,
                                      const std::vector<std::string>& keys,
                                      std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->multi_zget(name, keys, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("multi_zget function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::MultiZset(const std::string& name, const std::map<std::string, int64_t>& kss) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->multi_zset(name, kss);
  if (st.error()) {
    std::string buff = common::MemSPrintf("multi_zset function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::MultiZdel(const std::string& name, const std::vector<std::string>& keys) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->multi_zdel(name, keys);
  if (st.error()) {
    std::string buff = common::MemSPrintf("multi_zdel function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Qpush(const std::string& name, const std::string& item) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->qpush(name, item);
  if (st.error()) {
    std::string buff = common::MemSPrintf("qpush function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Qpop(const std::string& name, std::string* item) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->qpop(name, item);
  if (st.error()) {
    std::string buff = common::MemSPrintf("qpop function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Qslice(const std::string& name, int64_t begin, int64_t end, std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->qslice(name, begin, end, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("qslice function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Qclear(const std::string& name, int64_t* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->qclear(name, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("qclear function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::Expire(key_t key, ttl_t ttl) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  const std::string key_slice = ConvertToSSDBSlice(key);
  auto st = connection_.handle_->expire(key_slice, static_cast<int>(ttl));
  if (st.error()) {
    std::string buff = common::MemSPrintf("EXPIRE function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::TTL(key_t key, ttl_t* ttl) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  int lttl = 0;
  const std::string key_slice = ConvertToSSDBSlice(key);
  auto st = connection_.handle_->ttl(key_slice, &lttl);
  if (st.error()) {
    std::string buff = common::MemSPrintf("TTL function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  *ttl = lttl;
  return common::Error();
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  std::vector<std::string> ret;
  auto st = connection_.handle_->keys(std::string(), std::string(), 0, &ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Flushdb function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  uint64_t offset_pos = cursor_in;
  uint64_t lcursor_out = 0;
  std::vector<std::string> lkeys_out;
  for (size_t i = 0; i < ret.size(); ++i) {
    std::string key = ret[i];
    if (lkeys_out.size() < count_keys) {
      if (common::MatchPattern(key, pattern)) {
        if (offset_pos == 0) {
          lkeys_out.push_back(key);
        } else {
          offset_pos--;
        }
      }
    } else {
      lcursor_out = cursor_in + count_keys;
      break;
    }
  }

  *keys_out = lkeys_out;
  *cursor_out = lcursor_out;
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  auto st = connection_.handle_->keys(key_start, key_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Keys function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  std::vector<std::string> ret;
  auto st = connection_.handle_->keys(std::string(), std::string(), UINT64_MAX, &ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Couldn't determine DBKCOUNT error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = ret.size();
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  std::vector<std::string> ret;
  auto st = connection_.handle_->keys(std::string(), std::string(), 0, &ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Flushdb function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  for (size_t i = 0; i < ret.size(); ++i) {
    key_t key(ret[i]);
    common::Error err = DelInner(key);
    if (err && err->IsError()) {
      return err;
    }
  }

  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  if (name != CurrentDBName()) {
    return ICommandTranslator::InvalidInputArguments(SELECTDB_COMMAND);
  }

  size_t kcount = 0;
  common::Error err = DBkcount(&kcount);
  DCHECK(!err);
  *info = new DataBaseInfo(name, true, kcount);
  return common::Error();
}

common::Error DBConnection::DeleteImpl(const NKeys& keys, NKeys* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    key_t key_str = key.GetKey();
    common::Error err = DelInner(key_str);
    if (err && err->IsError()) {
      continue;
    }

    deleted_keys->push_back(key);
  }

  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, string_key_t new_key) {
  key_t key_str = key.GetKey();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->IsError()) {
    return err;
  }

  err = DelInner(key_str);
  if (err && err->IsError()) {
    return err;
  }

  err = SetInner(key_t(new_key), value_str);
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  std::string value_str = key.ValueString();
  common::Error err = SetInner(key_str, value_str);
  if (err && err->IsError()) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  key_t key_str = key.GetKey();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->IsError()) {
    return err;
  }

  NValue val(common::Value::CreateStringValue(value_str));
  *loaded_key = NDbKValue(key, val);
  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  key_t key_str = key.GetKey();
  common::Error err = Expire(key_str, ttl);
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  key_t key_str = key.GetKey();
  common::Error err = TTL(key_str, ttl);
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
