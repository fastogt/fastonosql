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
#include <stdint.h>  // for uint64_t

#include <string>  // for string
#include <vector>  // for vector

#include <common/error.h>   // for Error
#include <common/macros.h>  // for PROJECT_VERSION_GENERATE, etc

#include "core/connection_types.h"         // for connectionTypes::REDIS
#include "core/db/redis/config.h"          // for Config
#include "core/db_key.h"                   // for NDbKValue, NKey, etc
#include "core/global.h"                   // for FastoObject (ptr only), etc
#include "core/internal/cdb_connection.h"  // for CDBConnection
#include "core/internal/db_connection.h"   // for DBConnection<>::config_t
#include "core/server/iserver_info.h"
#include "core/ssh_info.h"  // for SSHInfo

namespace fastonosql {
namespace core {
class CDBConnectionClient;
}
}  // namespace fastonosql
namespace fastonosql {
namespace core {
class IDataBaseInfo;
}
}  // namespace fastonosql
struct redisContext;  // lines 49-49
struct redisReply;    // lines 50-50

#define INFO_REQUEST "INFO"
#define GET_SERVER_TYPE "CLUSTER NODES"
#define GET_SENTINEL_MASTERS "SENTINEL MASTERS"
#define GET_SENTINEL_SLAVES_PATTERN_1ARGS_S "SENTINEL SLAVES %s"

#define LATENCY_REQUEST "LATENCY"
#define RDM_REQUEST "RDM"
#define SYNC_REQUEST "SYNC"
#define FIND_BIG_KEYS_REQUEST "FIND_BIG_KEYS"
#define STAT_MODE_REQUEST "STAT"
#define SCAN_MODE_REQUEST "SCAN"

namespace fastonosql {
namespace core {
namespace redis {

typedef redisContext NativeConnection;
struct RConfig : public Config {
  explicit RConfig(const Config& config, const SSHInfo& sinfo);
  RConfig();

  SSHInfo ssh_info;
};

common::Error CreateConnection(const RConfig& config, NativeConnection** context);
common::Error TestConnection(const RConfig& rconfig);

common::Error DiscoveryClusterConnection(const RConfig& rconfig, std::vector<ServerDiscoveryClusterInfoSPtr>* infos);
common::Error DiscoverySentinelConnection(const RConfig& rconfig, std::vector<ServerDiscoverySentinelInfoSPtr>* infos);

class DBConnection : public core::internal::CDBConnection<NativeConnection, RConfig, REDIS> {
 public:
  typedef core::internal::CDBConnection<NativeConnection, RConfig, REDIS> base_class;
  explicit DBConnection(CDBConnectionClient* client);

  bool IsAuthenticated() const;

  common::Error Connect(const config_t& config);

  std::string CurrentDBName() const;

  common::Error SlaveMode(FastoObject* out) WARN_UNUSED_RESULT;

  common::Error ExecuteAsPipeline(const std::vector<FastoObjectCommandIPtr>& cmds,
                                  void (*log_command_cb)(FastoObjectCommandIPtr)) WARN_UNUSED_RESULT;

  common::Error CommonExec(int argc, const char** argv, FastoObject* out) WARN_UNUSED_RESULT;
  common::Error Auth(const std::string& password) WARN_UNUSED_RESULT;
  common::Error Monitor(int argc, const char** argv,
                        FastoObject* out) WARN_UNUSED_RESULT;  // interrupt
  common::Error Subscribe(int argc, const char** argv,
                          FastoObject* out) WARN_UNUSED_RESULT;  // interrupt

  common::Error SetEx(const NDbKValue& key, ttl_t ttl);
  common::Error SetNX(const NDbKValue& key, long long* result);

  common::Error Lpush(const NKey& key, NValue arr, long long* list_len);
  common::Error Lrange(const NKey& key, int start, int stop, NDbKValue* loaded_key);

  common::Error Sadd(const NKey& key, NValue set, long long* added);
  common::Error Smembers(const NKey& key, NDbKValue* loaded_key);

  common::Error Zadd(const NKey& key, NValue scores, long long* added);
  common::Error Zrange(const NKey& key, int start, int stop, bool withscores, NDbKValue* loaded_key);

  common::Error Hmset(const NKey& key, NValue hash);
  common::Error Hgetall(const NKey& key, NDbKValue* loaded_key);

  common::Error Decr(const NKey& key, long long* decr);
  common::Error DecrBy(const NKey& key, int inc, long long* decr);

  common::Error Incr(const NKey& key, long long* incr);
  common::Error IncrBy(const NKey& key, int inc, long long* incr);
  common::Error IncrByFloat(const NKey& key, double inc, std::string* str_incr);

 private:
  virtual common::Error ScanImpl(uint64_t cursor_in,
                                 const std::string& pattern,
                                 uint64_t count_keys,
                                 std::vector<std::string>* keys_out,
                                 uint64_t* cursor_out) override;
  virtual common::Error KeysImpl(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) override;
  virtual common::Error DBkcountImpl(size_t* size) override;
  virtual common::Error FlushDBImpl() override;
  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) override;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) override;
  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) override;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) override;
  virtual common::Error RenameImpl(const NKey& key, const std::string& new_key) override;
  virtual common::Error SetTTLImpl(const NKey& key, ttl_t ttl) override;
  virtual common::Error GetTTLImpl(const NKey& key, ttl_t* ttl) override;
  virtual common::Error QuitImpl() override;

  common::Error SendSync(unsigned long long* payload) WARN_UNUSED_RESULT;

  common::Error CliFormatReplyRaw(FastoObjectArray* ar, redisReply* r) WARN_UNUSED_RESULT;
  common::Error CliFormatReplyRaw(FastoObject* out, redisReply* r) WARN_UNUSED_RESULT;
  common::Error CliReadReply(FastoObject* out) WARN_UNUSED_RESULT;

  bool isAuth_;
  int cur_db_;
};

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
