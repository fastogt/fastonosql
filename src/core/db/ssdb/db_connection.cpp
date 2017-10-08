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

#include <SSDB.h>  // for Status, Client

#include "core/db/ssdb/command_translator.h"
#include "core/db/ssdb/database_info.h"
#include "core/db/ssdb/internal/commands_api.h"

namespace fastonosql {
namespace core {
template <>
const char* ConnectionTraits<SSDB>::GetBasedOn() {
  return "ssdb";
}

template <>
const char* ConnectionTraits<SSDB>::GetVersionApi() {
  return "1.9.4";
}
namespace {
std::string ConvertToSSDBSlice(const key_t& key) {
  return key.ToBytes();
}
}  // namespace
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<ssdb::NativeConnection, ssdb::Config>::Connect(const ssdb::Config& config,
                                                                                       ssdb::NativeConnection** hout) {
  ssdb::NativeConnection* context = nullptr;
  common::Error err = ssdb::CreateConnection(config, &context);
  if (err) {
    return err;
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
const ConstantCommandsArray& CDBConnection<ssdb::NativeConnection, ssdb::Config, SSDB>::GetCommands() {
  return ssdb::g_commands;
}
}  // namespace internal
namespace ssdb {
namespace {
common::Error AuthContext(::ssdb::Client* context, const std::string& password) {
  if (password.empty()) {  // handle in checkresult
    return common::Error();
  }

  auto st = context->auth(password);
  if (st.error()) {
    return common::make_error("AUTH function error: need authentification!");
  }
  return common::Error();
}
}  // namespace
common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  DCHECK(*context == nullptr);
  ::ssdb::Client* lcontext = ::ssdb::Client::connect(config.host.GetHost(), config.host.GetPort());
  if (!lcontext) {
    return common::make_error("Fail connect to server!");
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  ::ssdb::Client* ssdb = nullptr;
  common::Error err = CreateConnection(config, &ssdb);
  if (err) {
    return err;
  }

  err = AuthContext(ssdb, config.auth);
  if (err) {
    delete ssdb;
    return err;
  }

  delete ssdb;
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::GetCommands())), is_auth_(false) {}

bool DBConnection::IsAuthenticated() const {
  if (!base_class::IsAuthenticated()) {
    return false;
  }

  return is_auth_;
}

common::Error DBConnection::Connect(const config_t& config) {
  common::Error err = base_class::Connect(config);
  if (err) {
    return err;
  }

  err = Auth(config->auth);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::Disconnect() {
  is_auth_ = false;
  return base_class::Disconnect();
}

common::Error DBConnection::Info(const std::string& args, ServerInfo::Stats* statsout) {
  if (!statsout) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::vector<std::string> ret;
  err = CheckResultCommand(DB_INFO_COMMAND, connection_.handle_->info(args, &ret));
  if (err) {
    return err;
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
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  int64_t ret;
  err = CheckResultCommand("DBSIZE", connection_.handle_->dbsize(&ret));
  if (err) {
    return err;
  }

  *size = ret;
  return common::Error();
}

common::Error DBConnection::Auth(const std::string& password) {
  common::Error err = TestIsConnected();
  if (err) {
    return err;
  }

  err = AuthContext(connection_.handle_, password);
  if (err) {
    is_auth_ = false;
    return err;
  }

  is_auth_ = true;
  return common::Error();
}

common::Error DBConnection::Setx(const std::string& key, const std::string& value, ttl_t ttl) {
  if (key.empty() || value.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("SETX", connection_.handle_->setx(key, value, static_cast<int>(ttl)));
}

common::Error DBConnection::SetInner(key_t key, const std::string& value) {
  const std::string key_slice = ConvertToSSDBSlice(key);
  return CheckResultCommand(DB_SET_KEY_COMMAND, connection_.handle_->set(key_slice, value));
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  const std::string key_slice = ConvertToSSDBSlice(key);
  return CheckResultCommand(DB_GET_KEY_COMMAND, connection_.handle_->get(key_slice, ret_val));
}

common::Error DBConnection::DelInner(key_t key) {
  const std::string key_slice = ConvertToSSDBSlice(key);
  return CheckResultCommand(DB_DELETE_KEY_COMMAND, connection_.handle_->del(key_slice));
}

common::Error DBConnection::Incr(const std::string& key, int64_t incrby, int64_t* ret) {
  if (key.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("INCR", connection_.handle_->incr(key, incrby, ret));
}

common::Error DBConnection::ScanSsdb(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  if (!ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("SCAN", connection_.handle_->scan(key_start, key_end, limit, ret));
}

common::Error DBConnection::Rscan(const std::string& key_start,
                                  const std::string& key_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (!ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("RSCAN", connection_.handle_->rscan(key_start, key_end, limit, ret));
}

common::Error DBConnection::MultiGet(const std::vector<std::string>& keys, std::vector<std::string>* ret) {
  if (keys.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("MULTIGET", connection_.handle_->multi_get(keys, ret));
}

common::Error DBConnection::MultiSet(const std::map<std::string, std::string>& kvs) {
  if (kvs.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("MULTISET", connection_.handle_->multi_set(kvs));
}

common::Error DBConnection::MultiDel(const std::vector<std::string>& keys) {
  if (keys.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("MULTIDEL", connection_.handle_->multi_del(keys));
}

common::Error DBConnection::Hget(const std::string& name, const std::string& key, std::string* val) {
  if (name.empty() || key.empty() || !val) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HGET", connection_.handle_->hget(name, key, val));
}

common::Error DBConnection::Hgetall(const std::string& name, std::vector<std::string>* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HGETALL", connection_.handle_->hgetall(name, ret));
}

common::Error DBConnection::Hset(const std::string& name, const std::string& key, const std::string& val) {
  if (name.empty() || key.empty() || val.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HSET", connection_.handle_->hset(name, key, val));
}

common::Error DBConnection::Hdel(const std::string& name, const std::string& key) {
  if (name.empty() || key.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HDEL", connection_.handle_->hdel(name, key));
}

common::Error DBConnection::Hincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret) {
  if (name.empty() || key.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HINCR", connection_.handle_->hincr(name, key, incrby, ret));
}

common::Error DBConnection::Hsize(const std::string& name, int64_t* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HSIZE", connection_.handle_->hsize(name, ret));
}

common::Error DBConnection::Hclear(const std::string& name, int64_t* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HCLEAR", connection_.handle_->hclear(name, ret));
}

common::Error DBConnection::Hkeys(const std::string& name,
                                  const std::string& key_start,
                                  const std::string& key_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HKEYS", connection_.handle_->hkeys(name, key_start, key_end, limit, ret));
}

common::Error DBConnection::Hscan(const std::string& name,
                                  const std::string& key_start,
                                  const std::string& key_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HSCAN", connection_.handle_->hscan(name, key_start, key_end, limit, ret));
}

common::Error DBConnection::Hrscan(const std::string& name,
                                   const std::string& key_start,
                                   const std::string& key_end,
                                   uint64_t limit,
                                   std::vector<std::string>* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("HRSCAN", connection_.handle_->hrscan(name, key_start, key_end, limit, ret));
}

common::Error DBConnection::MultiHget(const std::string& name,
                                      const std::vector<std::string>& keys,
                                      std::vector<std::string>* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("MULTIHGET", connection_.handle_->multi_hget(name, keys, ret));
}

common::Error DBConnection::MultiHset(const std::string& name, const std::map<std::string, std::string>& keys) {
  if (name.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("MULTIHSET", connection_.handle_->multi_hset(name, keys));
}

common::Error DBConnection::Zget(const std::string& name, const std::string& key, int64_t* ret) {
  if (name.empty() || key.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZGET", connection_.handle_->zget(name, key, ret));
}

common::Error DBConnection::Zset(const std::string& name, const std::string& key, int64_t score) {
  if (name.empty() || key.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZSET", connection_.handle_->zset(name, key, score));
}

common::Error DBConnection::Zdel(const std::string& name, const std::string& key) {
  if (name.empty() || key.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZDEL", connection_.handle_->zdel(name, key));
}

common::Error DBConnection::Zincr(const std::string& name, const std::string& key, int64_t incrby, int64_t* ret) {
  if (name.empty() || key.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZINCR", connection_.handle_->zincr(name, key, incrby, ret));
}

common::Error DBConnection::Zsize(const std::string& name, int64_t* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZSIZE", connection_.handle_->zsize(name, ret));
}

common::Error DBConnection::Zclear(const std::string& name, int64_t* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZCLEAR", connection_.handle_->zclear(name, ret));
}

common::Error DBConnection::Zrank(const std::string& name, const std::string& key, int64_t* ret) {
  if (name.empty() || key.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZRANK", connection_.handle_->zrank(name, key, ret));
}

common::Error DBConnection::Zrrank(const std::string& name, const std::string& key, int64_t* ret) {
  if (name.empty() || key.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZRRANK", connection_.handle_->zrrank(name, key, ret));
}

common::Error DBConnection::Zrange(const std::string& name,
                                   uint64_t offset,
                                   uint64_t limit,
                                   std::vector<std::string>* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZRANGE", connection_.handle_->zrange(name, offset, limit, ret));
}

common::Error DBConnection::Zrrange(const std::string& name,
                                    uint64_t offset,
                                    uint64_t limit,
                                    std::vector<std::string>* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZRRANGE", connection_.handle_->zrrange(name, offset, limit, ret));
}

common::Error DBConnection::Zkeys(const std::string& name,
                                  const std::string& key_start,
                                  int64_t* score_start,
                                  int64_t* score_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (name.empty() || !score_start || !score_end || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZKEYS", connection_.handle_->zkeys(name, key_start, score_start, score_end, limit, ret));
}

common::Error DBConnection::Zscan(const std::string& name,
                                  const std::string& key_start,
                                  int64_t* score_start,
                                  int64_t* score_end,
                                  uint64_t limit,
                                  std::vector<std::string>* ret) {
  if (name.empty() || !score_start || !score_end || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZSCAN", connection_.handle_->zscan(name, key_start, score_start, score_end, limit, ret));
}

common::Error DBConnection::Zrscan(const std::string& name,
                                   const std::string& key_start,
                                   int64_t* score_start,
                                   int64_t* score_end,
                                   uint64_t limit,
                                   std::vector<std::string>* ret) {
  if (name.empty() || !score_start || !score_end || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("ZRSCAN", connection_.handle_->zrscan(name, key_start, score_start, score_end, limit, ret));
}

common::Error DBConnection::MultiZget(const std::string& name,
                                      const std::vector<std::string>& keys,
                                      std::vector<std::string>* ret) {
  if (name.empty() || keys.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("MULTIZGET", connection_.handle_->multi_zget(name, keys, ret));
}

common::Error DBConnection::MultiZset(const std::string& name, const std::map<std::string, int64_t>& kss) {
  if (name.empty() || kss.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("MULTIZSET", connection_.handle_->multi_zset(name, kss));
}

common::Error DBConnection::MultiZdel(const std::string& name, const std::vector<std::string>& keys) {
  if (name.empty() || keys.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("MULTIZDEL", connection_.handle_->multi_zdel(name, keys));
}

common::Error DBConnection::Qpush(const std::string& name, const std::string& item) {
  if (name.empty() || item.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("QPUSH", connection_.handle_->qpush(name, item));
}

common::Error DBConnection::Qpop(const std::string& name, std::string* item) {
  if (name.empty() || !item) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("QPOP", connection_.handle_->qpop(name, item));
}

common::Error DBConnection::Qslice(const std::string& name, int64_t begin, int64_t end, std::vector<std::string>* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("QSLICE", connection_.handle_->qslice(name, begin, end, ret));
}

common::Error DBConnection::Qclear(const std::string& name, int64_t* ret) {
  if (name.empty() || !ret) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("QCLEAR", connection_.handle_->qclear(name, ret));
}

common::Error DBConnection::Expire(key_t key, ttl_t ttl) {
  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  const std::string key_slice = ConvertToSSDBSlice(key);
  return CheckResultCommand(DB_SET_TTL_COMMAND, connection_.handle_->expire(key_slice, static_cast<int>(ttl)));
}

common::Error DBConnection::TTL(key_t key, ttl_t* ttl) {
  if (!ttl) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  int lttl = 0;
  const std::string key_slice = ConvertToSSDBSlice(key);
  err = CheckResultCommand(DB_GET_TTL_COMMAND, connection_.handle_->ttl(key_slice, &lttl));
  if (err) {
    return err;
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
  common::Error err =
      CheckResultCommand("SCAN", connection_.handle_->keys(std::string(), std::string(), count_keys, &ret));
  if (err) {
    return err;
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
  return CheckResultCommand("KEYS", connection_.handle_->keys(key_start, key_end, limit, ret));
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  std::vector<std::string> ret;
  common::Error err =
      CheckResultCommand(DB_DBKCOUNT_COMMAND, connection_.handle_->keys(std::string(), std::string(), UINT64_MAX, &ret));
  if (err) {
    return err;
  }

  *size = ret.size();
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  std::vector<std::string> ret;
  common::Error err = CheckResultCommand(DB_FLUSHDB_COMMAND, connection_.handle_->keys(std::string(), std::string(), 0, &ret));
  if (err) {
    return err;
  }

  for (size_t i = 0; i < ret.size(); ++i) {
    key_t key(ret[i]);
    common::Error err = DelInner(key);
    if (err) {
      return err;
    }
  }

  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  if (name != GetCurrentDBName()) {
    return ICommandTranslator::InvalidInputArguments(DB_SELECTDB_COMMAND);
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
    if (err) {
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
  if (err) {
    return err;
  }

  err = DelInner(key_str);
  if (err) {
    return err;
  }

  err = SetInner(key_t(new_key), value_str);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  std::string value_str = key.GetValueString();
  common::Error err = SetInner(key_str, value_str);
  if (err) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  key_t key_str = key.GetKey();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err) {
    return err;
  }

  NValue val(common::Value::CreateStringValue(value_str));
  *loaded_key = NDbKValue(key, val);
  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  key_t key_str = key.GetKey();
  common::Error err = Expire(key_str, ttl);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  key_t key_str = key.GetKey();
  common::Error err = TTL(key_str, ttl);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::CheckResultCommand(const std::string& cmd, const ::ssdb::Status& err) {
  if (err.error()) {
    if (err.code() == "noauth") {
      is_auth_ = false;
    }
    return GenerateError(cmd, err.code());
  }

  return common::Error();
}

}  // namespace ssdb
}  // namespace core
}  // namespace fastonosql
