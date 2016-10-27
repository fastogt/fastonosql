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

#include "core/ssdb/db_connection.h"

#include <memory>  // for __shared_ptr

#include <SSDB.h>  // for Status, Client

#include <common/convert2string.h>  // for ConvertFromString
#include <common/net/types.h>       // for HostAndPort
#include <common/sprintf.h>         // for MemSPrintf
#include <common/value.h>           // for Value, etc

#include "core/ssdb/config.h"               // for Config
#include "core/ssdb/connection_settings.h"  // for ConnectionSettings
#include "core/ssdb/database.h"
#include "core/ssdb/command_translator.h"

#include "global/global.h"  // for FastoObject, etc

namespace fastonosql {
namespace core {
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<ssdb::NativeConnection, ssdb::Config>::Connect(
    const ssdb::Config& config,
    ssdb::NativeConnection** hout) {
  ssdb::NativeConnection* context = nullptr;
  common::Error er = ssdb::CreateConnection(config, &context);
  if (er && er->isError()) {
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
bool ConnectionAllocatorTraits<ssdb::NativeConnection, ssdb::Config>::IsConnected(
    ssdb::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}
}
namespace ssdb {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  ::ssdb::Client* lcontext = ::ssdb::Client::connect(config.host.host, config.host.port);
  if (!lcontext) {
    return common::make_error_value("Fail connect to server!", common::ErrorValue::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error CreateConnection(ConnectionSettings* settings, NativeConnection** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  Config config = settings->Info();
  return CreateConnection(config, context);
}

common::Error TestConnection(ConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  ::ssdb::Client* ssdb = nullptr;
  common::Error er = CreateConnection(settings, &ssdb);
  if (er && er->isError()) {
    return er;
  }

  delete ssdb;

  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(ssdbCommands, client, new CommandTranslator) {}

const char* DBConnection::VersionApi() {
  return "1.9.3";
}

common::Error DBConnection::Info(const char* args, ServerInfo::Stats* statsout) {
  if (!statsout) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument for command: INFO",
                                    common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::vector<std::string> ret;
  auto st = connection_.handle_->info(args ? args : std::string(), &ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("info function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  ServerInfo::Stats lstatsout;
  for (size_t i = 0; i < ret.size(); i += 2) {
    if (ret[i] == SSDB_VERSION_LABEL) {
      lstatsout.version = ret[i + 1];
    } else if (ret[i] == SSDB_LINKS_LABEL) {
      lstatsout.links = common::ConvertFromString<uint32_t>(ret[i + 1]);
    } else if (ret[i] == SSDB_TOTAL_CALLS_LABEL) {
      lstatsout.total_calls = common::ConvertFromString<uint32_t>(ret[i + 1]);
    } else if (ret[i] == SSDB_DBSIZE_LABEL) {
      lstatsout.dbsize = common::ConvertFromString<uint32_t>(ret[i + 1]);
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
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
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

common::Error DBConnection::DBkcount(size_t* size) {
  if (!size) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::vector<std::string> ret;
  auto st = connection_.handle_->keys("", "", UINT64_MAX, &ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Couldn't determine DBKCOUNT error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *size = ret.size();
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

  auto st = connection_.handle_->setx(key, value, ttl);
  if (st.error()) {
    std::string buff = common::MemSPrintf("setx function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::SetInner(const std::string& key, const std::string& value) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->set(key, value);
  if (st.error()) {
    std::string buff = common::MemSPrintf("set function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::GetInner(const std::string& key, std::string* ret_val) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->get(key, ret_val);
  if (st.error()) {
    std::string buff = common::MemSPrintf("get function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }
  return common::Error();
}

common::Error DBConnection::DelInner(const std::string& key) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->del(key);
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

common::Error DBConnection::Keys(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  auto st = connection_.handle_->keys(key_start, key_end, limit, ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Keys function error: %s", st.code());
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

common::Error DBConnection::MultiGet(const std::vector<std::string>& keys,
                                     std::vector<std::string>* ret) {
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

common::Error DBConnection::Hget(const std::string& name,
                                 const std::string& key,
                                 std::string* val) {
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

common::Error DBConnection::Hset(const std::string& name,
                                 const std::string& key,
                                 const std::string& val) {
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

common::Error DBConnection::Hincr(const std::string& name,
                                  const std::string& key,
                                  int64_t incrby,
                                  int64_t* ret) {
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

common::Error DBConnection::MultiHset(const std::string& name,
                                      const std::map<std::string, std::string>& keys) {
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

common::Error DBConnection::Zincr(const std::string& name,
                                  const std::string& key,
                                  int64_t incrby,
                                  int64_t* ret) {
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

common::Error DBConnection::MultiZset(const std::string& name,
                                      const std::map<std::string, int64_t>& kss) {
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

common::Error DBConnection::MultiZdel(const std::string& name,
                                      const std::vector<std::string>& keys) {
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

common::Error DBConnection::Qslice(const std::string& name,
                                   int64_t begin,
                                   int64_t end,
                                   std::vector<std::string>* ret) {
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

common::Error DBConnection::Help(int argc, const char** argv) {
  UNUSED(argc);
  UNUSED(argv);

  return NotSupported("HELP");
}

common::Error DBConnection::Flushdb() {
  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::vector<std::string> ret;
  auto st = connection_.handle_->keys(std::string(), std::string(), 0, &ret);
  if (st.error()) {
    std::string buff = common::MemSPrintf("Flushdb function error: %s", st.code());
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  for (size_t i = 0; i < ret.size(); ++i) {
    std::string key = ret[i];
    common::Error err = DelInner(key);
    if (err && err->isError()) {
      return err;
    }
  }

  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  size_t kcount = 0;
  common::Error err = DBkcount(&kcount);
  DCHECK(!err);
  *info = new DataBaseInfo(name, true, kcount);
  return common::Error();
}

common::Error DBConnection::DeleteImpl(const NKeys& keys, NKeys* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    std::string key_str = key.Key();
    common::Error err = DelInner(key_str);
    if (err && err->isError()) {
      continue;
    }

    deleted_keys->push_back(key);
  }

  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, const std::string& new_key) {
  std::string key_str = key.Key();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->isError()) {
    return err;
  }

  err = DelInner(key_str);
  if (err && err->isError()) {
    return err;
  }

  err = SetInner(new_key, value_str);
  if (err && err->isError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  std::string key_str = key.KeyString();
  std::string value_str = key.ValueString();
  common::Error err = SetInner(key_str, value_str);
  if (err && err->isError()) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  std::string key_str = key.Key();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err && err->isError()) {
    return err;
  }

  NValue val(common::Value::createStringValue(value_str));
  *loaded_key = NDbKValue(key, val);
  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  UNUSED(key);
  UNUSED(ttl);
  return common::make_error_value("Sorry, but now " PROJECT_NAME_TITLE
                                  " for SSDB not supported TTL commands.",
                                  common::ErrorValue::E_ERROR);
}

common::Error info(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  ServerInfo::Stats statsout;
  common::Error er = ssdb->Info(argc == 1 ? argv[0] : 0, &statsout);
  if (!er) {
    ServerInfo sinf(statsout);
    common::StringValue* val = common::Value::createStringValue(sinf.ToString());
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error dbsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t dbsize = 0;
  common::Error er = ssdb->DBsize(&dbsize);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error auth(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Auth(argv[0]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error get(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  NDbKValue key_loaded;
  common::Error err = ssdb->Get(key, &key_loaded);
  if (err && err->isError()) {
    return err;
  }

  NValue val = key_loaded.Value();
  common::Value* copy = val->deepCopy();
  FastoObject* child = new FastoObject(out, copy, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error select(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Select(argv[0], nullptr);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  NValue string_val(common::Value::createStringValue(argv[1]));
  NDbKValue kv(key, string_val);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  NDbKValue key_added;
  common::Error err = ssdb->Set(kv, &key_added);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error setx(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Setx(argv[0], argv[1], common::ConvertFromString<int>(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error del(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  NKeys keysdel;
  for (int i = 0; i < argc; ++i) {
    keysdel.push_back(NKey(argv[i]));
  }

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  NKeys keys_deleted;
  common::Error err = ssdb->Delete(keysdel, &keys_deleted);
  if (err && err->isError()) {
    return err;
  }

  common::FundamentalValue* val = common::Value::createUIntegerValue(keys_deleted.size());
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error rename(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  NKey key(argv[0]);
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error err = ssdb->Rename(key, argv[1]);
  if (err && err->isError()) {
    return err;
  }

  common::StringValue* val = common::Value::createStringValue("OK");
  FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
  out->AddChildren(child);
  return common::Error();
}

common::Error set_ttl(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(out);
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  NKey key(argv[0]);
  ttl_t ttl = common::ConvertFromString<ttl_t>(argv[1]);
  common::Error er = ssdb->SetTTL(key, ttl);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error incr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t ret = 0;
  common::Error er = ssdb->Incr(argv[0], common::ConvertFromString<int64_t>(argv[1]), &ret);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error keys(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      ssdb->Keys(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error scan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      ssdb->Scan(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error rscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er =
      ssdb->Rscan(argv[0], argv[1], common::ConvertFromString<uint64_t>(argv[2]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_get(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error er = ssdb->MultiGet(keysget, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_set(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, std::string> keysset;
  for (int i = 0; i < argc; i += 2) {
    keysset[argv[i]] = argv[i + 1];
  }

  common::Error er = ssdb->MultiSet(keysset);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_del(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 0; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error er = ssdb->MultiDel(keysget);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error er = ssdb->Hget(argv[0], argv[1], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hgetall(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->Hgetall(argv[0], &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Hset(argv[0], argv[1], argv[2]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Hdel(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hincr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er =
      ssdb->Hincr(argv[0], argv[1], common::ConvertFromString<int64_t>(argv[2]), &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Hsize(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Hclear(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hkeys(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->Hkeys(argv[0], argv[1], argv[2],
                                 common::ConvertFromString<uint64_t>(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->Hscan(argv[0], argv[1], argv[2],
                                 common::ConvertFromString<uint64_t>(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error hrscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysout;
  common::Error er = ssdb->Hrscan(argv[0], argv[1], argv[2],
                                  common::ConvertFromString<uint64_t>(argv[3]), &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_hget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> keysout;
  common::Error er = ssdb->MultiHget(argv[0], keysget, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_hset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, std::string> keys;
  for (int i = 1; i < argc; i += 2) {
    keys[argv[i]] = argv[i + 1];
  }

  common::Error er = ssdb->MultiHset(argv[0], keys);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t ret;
  common::Error er = ssdb->Zget(argv[0], argv[1], &ret);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Zset(argv[0], argv[1], common::ConvertFromString<int64_t>(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Zdel(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }
  return er;
}

common::Error zincr(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t ret = 0;
  common::Error er =
      ssdb->Zincr(argv[0], argv[1], common::ConvertFromString<int64_t>(argv[2]), &ret);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zsize(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Zsize(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Zclear(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrank(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Zrank(argv[0], argv[1], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrrank(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Zrrank(argv[0], argv[1], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrange(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  common::Error er = ssdb->Zrange(argv[0], common::ConvertFromString<uint64_t>(argv[1]),
                                  common::ConvertFromString<uint64_t>(argv[2]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrrange(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  common::Error er = ssdb->Zrrange(argv[0], common::ConvertFromString<uint64_t>(argv[1]),
                                   common::ConvertFromString<uint64_t>(argv[2]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zkeys(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st = common::ConvertFromString<int64_t>(argv[2]);
  int64_t end = common::ConvertFromString<int64_t>(argv[3]);
  common::Error er =
      ssdb->Zkeys(argv[0], argv[1], &st, &end, common::ConvertFromString<uint64_t>(argv[5]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st = common::ConvertFromString<int64_t>(argv[2]);
  int64_t end = common::ConvertFromString<int64_t>(argv[3]);
  common::Error er =
      ssdb->Zscan(argv[0], argv[1], &st, &end, common::ConvertFromString<uint64_t>(argv[4]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error zrscan(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> res;
  int64_t st = common::ConvertFromString<int64_t>(argv[2]);
  int64_t end = common::ConvertFromString<int64_t>(argv[3]);
  common::Error er =
      ssdb->Zrscan(argv[0], argv[1], &st, &end, common::ConvertFromString<uint64_t>(argv[4]), &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_zget(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  std::vector<std::string> res;
  common::Error er = ssdb->MultiZget(argv[0], keysget, &res);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < res.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(res[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_zset(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::map<std::string, int64_t> keysget;
  for (int i = 1; i < argc; i += 2) {
    keysget[argv[i]] = common::ConvertFromString<int64_t>(argv[i + 1]);
  }

  common::Error er = ssdb->MultiZset(argv[0], keysget);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error multi_zdel(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::vector<std::string> keysget;
  for (int i = 1; i < argc; ++i) {
    keysget.push_back(argv[i]);
  }

  common::Error er = ssdb->MultiZdel(argv[0], keysget);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error qpush(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  common::Error er = ssdb->Qpush(argv[0], argv[1]);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("OK");
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error qpop(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  std::string ret;
  common::Error er = ssdb->Qpop(argv[0], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error qslice(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t begin = common::ConvertFromString<int64_t>(argv[1]);
  int64_t end = common::ConvertFromString<int64_t>(argv[2]);

  std::vector<std::string> keysout;
  common::Error er = ssdb->Qslice(argv[0], begin, end, &keysout);
  if (!er) {
    common::ArrayValue* ar = common::Value::createArrayValue();
    for (size_t i = 0; i < keysout.size(); ++i) {
      common::StringValue* val = common::Value::createStringValue(keysout[i]);
      ar->append(val);
    }
    FastoObjectArray* child = new FastoObjectArray(out, ar, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error qclear(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  int64_t res = 0;
  common::Error er = ssdb->Qclear(argv[0], &res);
  if (!er) {
    common::FundamentalValue* val = common::Value::createIntegerValue(res);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error dbkcount(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  size_t dbkcount = 0;
  common::Error er = ssdb->DBkcount(&dbkcount);
  if (!er) {
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbkcount);
    FastoObject* child = new FastoObject(out, val, ssdb->Delimiter());
    out->AddChildren(child);
  }

  return er;
}

common::Error help(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(out);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  return ssdb->Help(argc - 1, argv + 1);
}

common::Error flushdb(internal::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(out);

  DBConnection* ssdb = static_cast<DBConnection*>(handler);
  return ssdb->Flushdb();
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
