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

#include "core/db/redis/db_connection.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef OS_POSIX
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>  // for setsockopt, SOL_SOCKET, etc
#endif

#include <limits.h>  // for LONG_MIN
#include <stdarg.h>  // for va_end, va_list, va_start
#include <stdint.h>  // for uint64_t, uint16_t, int64_t
#include <stdio.h>   // for vsnprintf
#include <stdlib.h>  // for free, malloc, realloc, etc
#include <string.h>  // for strcasecmp, NULL, strcmp, etc

#include <memory>  // for __shared_ptr
#include <string>
#include <vector>

extern "C" {
#include "sds.h"
}

#include <hiredis/hiredis.h>
#include <libssh2.h>  // for libssh2_exit, etc

#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/intrusive_ptr.h>   // for intrusive_ptr
#include <common/log_levels.h>      // for LEVEL_LOG::L_INFO, etc
#include <common/macros.h>          // for INVALID_DESCRIPTOR
#include <common/net/types.h>       // for HostAndPort, etc
#include <common/sprintf.h>         // for MemSPrintf, SNPrintf
#include <common/time.h>            // for current_mstime
#include <common/types.h>           // for time64_t
#include <common/utils.h>           // for c_strornull, usleep, etc
#include <common/value.h>           // for ErrorValue, etc

#include "core/icommand_translator.h"  // for translator_t, etc

#include "core/command_holder.h"  // for CommandHolder

#include "core/internal/cdb_connection_client.h"
#include "core/internal/connection.h"  // for Connection<>::config_t, etc

#include "core/db/redis/cluster_infos.h"  // for makeDiscoveryClusterInfo
#include "core/db/redis/command_translator.h"
#include "core/db/redis/database_info.h"  // for DataBaseInfo
#include "core/db/redis/internal/commands_api.h"
#include "core/db/redis/sentinel_info.h"  // for DiscoverySentinelInfo, etc

#define HIREDIS_VERSION    \
  STRINGIZE(HIREDIS_MAJOR) \
  "." STRINGIZE(HIREDIS_MINOR) "." STRINGIZE(HIREDIS_PATCH)
#define REDIS_CLI_KEEPALIVE_INTERVAL 15 /* seconds */
#define CLI_HELP_COMMAND 1
#define CLI_HELP_GROUP 2

#define DBSIZE "DBSIZE"

#define GET_PASSWORD "CONFIG get requirepass"

#define LATENCY_SAMPLE_RATE 10                 /* milliseconds. */
#define LATENCY_HISTORY_DEFAULT_INTERVAL 15000 /* milliseconds. */

#define RTYPE_STRING 0
#define RTYPE_LIST 1
#define RTYPE_SET 2
#define RTYPE_HASH 3
#define RTYPE_ZSET 4
#define RTYPE_NONE 5

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

namespace {

void anetSetError(char* err, const char* fmt, ...) {
  va_list ap;

  if (!err) {
    return;
  }
  va_start(ap, fmt);
  vsnprintf(err, ANET_ERR_LEN, fmt, ap);
  va_end(ap);
}

/* Set TCP keep alive option to detect dead peers. The
 * interval option
 * is only used for Linux as we are using Linux-specific
 * APIs to set
 * the probe send time, interval, and count. */
int anetKeepAlive(char* err, int fd, int interval) {
  int val = 1;
  const char* cval = reinterpret_cast<const char*>(&val);
  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, cval, sizeof(val)) == -1) {
    anetSetError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
    return ANET_ERR;
  }

#ifdef __linux__
  /* Default settings are more or less garbage, with the
   * keepalive time
   * set to 7200 by default on Linux. Modify settings to
   * make the feature
   * actually useful. */

  /* Send first probe after interval. */
  val = interval;
  if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
    anetSetError(err, "setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
    return ANET_ERR;
  }

  /* Send next probes after the specified interval. Note
   * that we set the
   * delay as interval / 3, as we send three probes before
   * detecting
   * an error (see the next setsockopt call). */
  val = interval / 3;
  if (val == 0) {
    val = 1;
  }
  if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
    anetSetError(err, "setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
    return ANET_ERR;
  }

  /* Consider the socket in error state after three we send
   * three ACK
   * probes without getting a reply. */
  val = 3;
  if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
    anetSetError(err, "setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
    return ANET_ERR;
  }
#else
  UNUSED(interval);
#endif

  return ANET_OK;
}

const struct RedisInit {
  RedisInit() { libssh2_init(0); }
  ~RedisInit() { libssh2_exit(); }
} rInit;

bool isPipeLineCommand(const char* command) {
  if (!command) {
    DNOTREACHED();
    return false;
  }

  bool skip =
      strcasecmp(command, "quit") == 0 || strcasecmp(command, "exit") == 0 || strcasecmp(command, "connect") == 0 ||
      strcasecmp(command, "help") == 0 || strcasecmp(command, "?") == 0 || strcasecmp(command, "shutdown") == 0 ||
      strcasecmp(command, "monitor") == 0 || strcasecmp(command, "subscribe") == 0 ||
      strcasecmp(command, "psubscribe") == 0 || strcasecmp(command, "sync") == 0 || strcasecmp(command, "psync") == 0;

  return !skip;
}

}  // namespace

namespace fastonosql {
namespace core {
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<redis::NativeConnection, redis::RConfig>::Connect(
    const redis::RConfig& config,
    redis::NativeConnection** hout) {
  redis::NativeConnection* context = nullptr;
  common::Error er = redis::CreateConnection(config, &context);
  if (er && er->IsError()) {
    return er;
  }

  *hout = context;
  anetKeepAlive(NULL, context->fd, REDIS_CLI_KEEPALIVE_INTERVAL);
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<redis::NativeConnection, redis::RConfig>::Disconnect(
    redis::NativeConnection** handle) {
  redis::NativeConnection* lhandle = *handle;
  if (lhandle) {
    redisFree(lhandle);
  }
  lhandle = nullptr;
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<redis::NativeConnection, redis::RConfig>::IsConnected(redis::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const char* CDBConnection<redis::NativeConnection, redis::RConfig, REDIS>::BasedOn() {
  return "hiredis";
}

template <>
const char* CDBConnection<redis::NativeConnection, redis::RConfig, REDIS>::VersionApi() {
  return HIREDIS_VERSION;
}

template <>
ConstantCommandsArray CDBConnection<redis::NativeConnection, redis::RConfig, REDIS>::Commands() {
  return redis::g_commands;
}
}  // namespace internal

namespace redis {
namespace {

common::Error valueFromReplay(redisReply* r, common::Value** out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  switch (r->type) {
    case REDIS_REPLY_NIL: {
      *out = common::Value::CreateNullValue();
      break;
    }
    case REDIS_REPLY_ERROR: {
      if (common::strcasestr(r->str, "NOAUTH")) {  //"NOAUTH Authentication
                                                   // required."
      }
      std::string str(r->str, r->len);
      return common::make_error_value(str, common::ErrorValue::E_ERROR);
    }
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING: {
      std::string str(r->str, r->len);
      *out = common::Value::CreateStringValue(str);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      *out = common::Value::CreateLongLongIntegerValue(r->integer);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::CreateArrayValue();
      for (size_t i = 0; i < r->elements; ++i) {
        common::Value* val = NULL;
        common::Error er = valueFromReplay(r->element[i], &val);
        if (er && er->IsError()) {
          delete arv;
          return er;
        }
        arv->Append(val);
      }
      *out = arv;
      break;
    }
    default: {
      return common::make_error_value(common::MemSPrintf("Unknown reply type: %d", r->type),
                                      common::ErrorValue::E_ERROR);
    }
  }

  return common::Error();
}

common::Error cliPrintContextError(redisContext* context) {
  if (!context) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string buff = common::MemSPrintf("Error: %s", context->errstr);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

common::Error authContext(const char* auth_str, redisContext* context) {
  if (!auth_str) {
    return common::Error();
  }

  redisReply* reply = static_cast<redisReply*>(redisCommand(context, "AUTH %s", auth_str));
  if (reply) {
    if (reply->type == REDIS_REPLY_ERROR) {
      std::string buff = common::MemSPrintf("Authentification error: %s", reply->str);
      freeReplyObject(reply);
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    freeReplyObject(reply);
    return common::Error();
  }

  return cliPrintContextError(context);
}

}  // namespace

RConfig::RConfig(const Config& config, const SSHInfo& sinfo) : Config(config), ssh_info(sinfo) {}

RConfig::RConfig() : Config(), ssh_info() {}

common::Error CreateConnection(const RConfig& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* lcontext = NULL;
  bool is_local = !config.hostsocket.empty();

  if (is_local) {
    const char* hostsocket = common::utils::c_strornull(config.hostsocket);
    lcontext = redisConnectUnix(hostsocket);
  } else {
    SSHInfo sinfo = config.ssh_info;
    const char* host = common::utils::c_strornull(config.host.host);
    uint16_t port = config.host.port;
    const char* username = common::utils::c_strornull(sinfo.user_name);
    const char* password = common::utils::c_strornull(sinfo.password);
    common::net::HostAndPort ssh_host = sinfo.host;
    const char* ssh_address = common::utils::c_strornull(ssh_host.host);
    int ssh_port = ssh_host.port;
    int curM = sinfo.current_method;
    const char* public_key = common::utils::c_strornull(sinfo.public_key);
    const char* private_key = common::utils::c_strornull(sinfo.private_key);
    const char* passphrase = common::utils::c_strornull(sinfo.passphrase);
    lcontext =
        redisConnect(host, port, ssh_address, ssh_port, username, password, public_key, private_key, passphrase, curM);
  }

  if (!lcontext) {
    if (is_local) {
      return common::make_error_value(
          common::MemSPrintf("Could not connect to Redis at %s : no context", config.hostsocket),
          common::Value::E_ERROR);
    }
    std::string host_str = common::ConvertToString(config.host);
    return common::make_error_value(common::MemSPrintf("Could not connect to Redis at %s : no context", host_str),
                                    common::Value::E_ERROR);
  }

  if (lcontext->err) {
    std::string buff;
    if (is_local) {
      buff = common::MemSPrintf("Could not connect to Redis at %s : %s", config.hostsocket, lcontext->errstr);
    } else {
      std::string host_str = common::ConvertToString(config.host);
      buff = common::MemSPrintf("Could not connect to Redis at %s : %s", host_str, lcontext->errstr);
    }
    redisFree(lcontext);
    return common::make_error_value(buff, common::Value::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const RConfig& rconfig) {
  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err && err->IsError()) {
    return err;
  }

  const char* auth_str = common::utils::c_strornull(rconfig.auth);
  err = authContext(auth_str, context);
  if (err && err->IsError()) {
    redisFree(context);
    return err;
  }

  redisFree(context);
  return common::Error();
}

common::Error DiscoveryClusterConnection(const RConfig& rconfig, std::vector<ServerDiscoveryClusterInfoSPtr>* infos) {
  if (!infos) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err && err->IsError()) {
    return err;
  }

  const char* auth_str = common::utils::c_strornull(rconfig.auth);
  err = authContext(auth_str, context);
  if (err && err->IsError()) {
    redisFree(context);
    return err;
  }

  /* Send the GET CLUSTER command. */
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(context, GET_SERVER_TYPE));
  if (!reply) {
    err = common::make_error_value("I/O error", common::Value::E_ERROR);
    redisFree(context);
    return err;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    err = makeDiscoveryClusterInfo(rconfig.host, std::string(reply->str, reply->len), infos);
  } else if (reply->type == REDIS_REPLY_ERROR) {
    err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
  } else {
    NOTREACHED();
  }

  freeReplyObject(reply);
  redisFree(context);
  return err;
}

common::Error DiscoverySentinelConnection(const RConfig& rconfig, std::vector<ServerDiscoverySentinelInfoSPtr>* infos) {
  if (!infos) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err && err->IsError()) {
    return err;
  }

  const char* auth_str = common::utils::c_strornull(rconfig.auth);
  err = authContext(auth_str, context);
  if (err && err->IsError()) {
    redisFree(context);
    return err;
  }

  /* Send the GET MASTERS command. */
  redisReply* masters_reply = reinterpret_cast<redisReply*>(redisCommand(context, GET_SENTINEL_MASTERS));
  if (!masters_reply) {
    redisFree(context);
    return common::make_error_value("I/O error", common::Value::E_ERROR);
  }

  for (size_t i = 0; i < masters_reply->elements; ++i) {
    redisReply* master_info = masters_reply->element[i];
    ServerCommonInfo sinf;
    common::Error lerr = MakeServerCommonInfo(master_info, &sinf);
    if (lerr && lerr->IsError()) {
      continue;
    }

    const char* master_name = sinf.name.c_str();
    ServerDiscoverySentinelInfoSPtr sent(new DiscoverySentinelInfo(sinf));
    infos->push_back(sent);
    /* Send the GET SLAVES command. */
    redisReply* reply =
        reinterpret_cast<redisReply*>(redisCommand(context, GET_SENTINEL_SLAVES_PATTERN_1ARGS_S, master_name));
    if (!reply) {
      freeReplyObject(masters_reply);
      redisFree(context);
      return common::make_error_value("I/O error", common::Value::E_ERROR);
    }

    if (reply->type == REDIS_REPLY_ARRAY) {
      for (size_t j = 0; j < reply->elements; ++j) {
        redisReply* server_info = reply->element[j];
        ServerCommonInfo slsinf;
        lerr = MakeServerCommonInfo(server_info, &slsinf);
        if (lerr && lerr->IsError()) {
          continue;
        }
        ServerDiscoverySentinelInfoSPtr lsent(new DiscoverySentinelInfo(slsinf));
        infos->push_back(lsent);
      }
    } else if (reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      freeReplyObject(masters_reply);
      redisFree(context);
      return common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    } else {
      NOTREACHED();
    }
    freeReplyObject(reply);
  }

  freeReplyObject(masters_reply);
  redisFree(context);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::Commands())), isAuth_(false), cur_db_(-1) {}

bool DBConnection::IsAuthenticated() const {
  if (!IsConnected()) {
    return false;
  }

  return isAuth_;
}

common::Error DBConnection::Connect(const config_t& config) {
  common::Error err = base_class::Connect(config);
  if (err && err->IsError()) {
    return err;
  }

  /* Do AUTH and select the right DB. */
  err = Auth(connection_.config_.auth);
  if (err && err->IsError()) {
    return err;
  }

  err = Select(common::ConvertToString(connection_.config_.dbnum), NULL);
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

std::string DBConnection::CurrentDBName() const {
  if (cur_db_ != -1) {
    return common::ConvertToString(cur_db_);
  }

  DNOTREACHED();
  return base_class::CurrentDBName();
}

/* Sends SYNC and reads the number of bytes in the payload.
 * Used both by
 * slaveMode() and getRDB(). */
common::Error DBConnection::SendSync(unsigned long long* payload) {
  if (!payload) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  /* To start we need to send the SYNC command and return
   * the payload.
   * The hiredis client lib does not understand this part of
   * the protocol
   * and we don't want to mess with its buffers, so
   * everything is performed
   * using direct low-level I/O. */
  char buf[4096], *p;

  /* Send the SYNC command. */
  ssize_t nwrite = 0;
  if (redisWriteFromBuffer(connection_.handle_, "SYNC\r\n", &nwrite) == REDIS_ERR) {
    return common::make_error_value("Error writing to master", common::ErrorValue::E_ERROR);
  }

  /* Read $<payload>\r\n, making sure to read just up to
   * "\n" */
  p = buf;
  while (1) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(connection_.handle_, p, 1, &nread);
    if (res == REDIS_ERR) {
      return common::make_error_value("Error reading bulk length while SYNCing", common::ErrorValue::E_ERROR);
    }

    if (!nread) {
      continue;
    }

    if (*p == '\n' && p != buf) {
      break;
    }
    if (*p != '\n') {
      p++;
    }
  }
  *p = '\0';
  if (buf[0] == '-') {
    std::string buf2 = common::MemSPrintf("SYNC with master failed: %s", buf);
    return common::make_error_value(buf2, common::ErrorValue::E_ERROR);
  }

  *payload = strtoull(buf + 1, NULL, 10);
  return common::Error();
}

common::Error DBConnection::SlaveMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  unsigned long long payload = 0;
  common::Error err = SendSync(&payload);
  if (err && err->IsError()) {
    return err;
  }

  char buf[1024];
  /* Discard the payload. */
  while (payload) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(connection_.handle_, buf, (payload > sizeof(buf)) ? sizeof(buf) : payload, &nread);
    if (res == REDIS_ERR) {
      return common::make_error_value("Error reading RDB payload while SYNCing", common::ErrorValue::E_ERROR);
    }
    payload -= nread;
  }

  /* Now we can use hiredis to read the incoming protocol.
   */
  while (!IsInterrupted()) {
    err = CliReadReply(out);
    if (err && err->IsError()) {
      return err;
    }
  }

  return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  const std::string pattern_result = core::internal::GetKeysPattern(cursor_in, pattern, count_keys);
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, pattern_result.c_str()));
  if (!reply || reply->type != REDIS_REPLY_ARRAY) {
    return common::make_error_value("I/O error", common::ErrorValue::E_ERROR);
  }

  common::Value* val = nullptr;
  common::Error err = valueFromReplay(reply, &val);
  if (err && err->IsError()) {
    delete val;
    freeReplyObject(reply);
    return err;
  }

  common::ArrayValue* arr = static_cast<common::ArrayValue*>(val);
  CHECK_EQ(2, arr->GetSize());
  std::string cursor_out_str;
  if (!arr->GetString(0, &cursor_out_str)) {
    delete val;
    freeReplyObject(reply);
    return common::make_error_value("I/O error", common::ErrorValue::E_ERROR);
  }

  common::ArrayValue* arr_keys = nullptr;
  if (!arr->GetList(1, &arr_keys)) {
    delete val;
    freeReplyObject(reply);
    return common::make_error_value("I/O error", common::ErrorValue::E_ERROR);
  }

  for (size_t i = 0; i < arr_keys->GetSize(); ++i) {
    std::string key;
    if (arr_keys->GetString(i, &key)) {
      keys_out->push_back(key);
    }
  }

  uint64_t lcursor_out;
  if (!common::ConvertFromString(cursor_out_str, &lcursor_out)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  *cursor_out = lcursor_out;
  delete val;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  UNUSED(key_start);
  UNUSED(key_end);
  UNUSED(limit);
  UNUSED(ret);
  return ICommandTranslator::NotSupported("KEYS");
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, DBSIZE));

  if (!reply || reply->type != REDIS_REPLY_INTEGER) {
    return common::make_error_value("Couldn't determine DBSIZE!", common::Value::E_ERROR);
  }

  /* Grab the number of keys and free our reply */
  *size = static_cast<size_t>(reply->integer);
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "FLUSHDB"));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  int num;
  if (!common::ConvertFromString(name, &num)) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "SELECT %d", num));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  connection_.config_.dbnum = num;
  cur_db_ = num;
  size_t sz = 0;
  common::Error err = DBkcount(&sz);
  DCHECK(!err);
  DataBaseInfo* linfo = new DataBaseInfo(common::ConvertToString(num), true, sz);
  *info = linfo;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::DeleteImpl(const NKeys& keys, NKeys* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    std::string del_cmd;
    translator_t tran = Translator();
    common::Error err = tran->DeleteKeyCommand(key, &del_cmd);
    if (err && err->IsError()) {
      return err;
    }
    redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, del_cmd.c_str()));
    if (!reply) {
      return cliPrintContextError(connection_.handle_);
    }

    if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
      deleted_keys->push_back(key);
    }
    freeReplyObject(reply);
  }

  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  std::string key_str = key.KeyString();
  std::string value_str = key.ValueString();
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "SET %s %s", key_str.c_str(), value_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  *added_key = key;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  std::string key_str = key.GetKey();
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "GET %s", key_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  common::Value* val = nullptr;
  if (reply->type == REDIS_REPLY_STRING) {
    val = common::Value::CreateStringValue(reply->str);
  } else if (reply->type == REDIS_REPLY_NIL) {
    val = common::Value::CreateNullValue();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  } else {
    NOTREACHED();
  }
  *loaded_key = NDbKValue(key, NValue(val));
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, const std::string& new_key) {
  translator_t tran = Translator();
  std::string rename_cmd;
  common::Error err = tran->RenameKeyCommand(key, new_key, &rename_cmd);
  if (err && err->IsError()) {
    return err;
  }
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, rename_cmd.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  std::string key_str = key.GetKey();
  translator_t tran = Translator();
  std::string ttl_cmd;
  common::Error err = tran->ChangeKeyTTLCommand(key, ttl, &ttl_cmd);
  if (err && err->IsError()) {
    return err;
  }
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, ttl_cmd.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  if (reply->integer == 0) {
    return common::make_error_value(common::MemSPrintf("%s does not exist or the timeout could not be set.", key_str),
                                    common::ErrorValue::E_ERROR);
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  translator_t tran = Translator();
  std::string ttl_cmd;
  common::Error err = tran->LoadKeyTTLCommand(key, &ttl_cmd);
  if (err && err->IsError()) {
    return err;
  }
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, ttl_cmd.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  if (reply->type != REDIS_REPLY_INTEGER) {
    freeReplyObject(reply);
    return common::make_error_value("TTL command internal error", common::ErrorValue::E_ERROR);
  }

  *ttl = reply->integer;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::QuitImpl() {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "QUIT"));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  base_class::Disconnect();
  return common::Error();
}

common::Error DBConnection::CliFormatReplyRaw(FastoObjectArray* ar, redisReply* r) {
  if (!ar || !r) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  switch (r->type) {
    case REDIS_REPLY_NIL: {
      common::Value* val = common::Value::CreateNullValue();
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_ERROR: {
      std::string str(r->str, r->len);
      common::ErrorValue* val =
          common::Value::CreateErrorValue(str, common::ErrorValue::E_NONE, common::logging::L_WARNING);
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING: {
      common::StringValue* val = common::Value::CreateStringValue(std::string(r->str, r->len));
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(r->integer);
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::CreateArrayValue();
      FastoObjectArray* child = new FastoObjectArray(ar, arv, Delimiter());
      ar->AddChildren(child);

      for (size_t i = 0; i < r->elements; ++i) {
        common::Error er = CliFormatReplyRaw(child, r->element[i]);
        if (er && er->IsError()) {
          return er;
        }
      }
      break;
    }
    default: {
      common::ErrorValue* val = common::Value::CreateErrorValue(common::MemSPrintf("Unknown reply type: %d", r->type),
                                                                common::ErrorValue::E_NONE, common::logging::L_WARNING);
      ar->Append(val);
    }
  }

  return common::Error();
}

common::Error DBConnection::CliFormatReplyRaw(FastoObject* out, redisReply* r) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  FastoObject* obj = nullptr;
  switch (r->type) {
    case REDIS_REPLY_NIL: {
      common::Value* val = common::Value::CreateNullValue();
      obj = new FastoObject(out, val, Delimiter());
      out->AddChildren(obj);
      break;
    }
    case REDIS_REPLY_ERROR: {
      if (common::strcasestr(r->str, "NOAUTH")) {  //"NOAUTH Authentication
                                                   // required."
        isAuth_ = false;
      }
      std::string str(r->str, r->len);
      return common::make_error_value(str, common::ErrorValue::E_ERROR);
    }
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING: {
      std::string str(r->str, r->len);
      common::StringValue* val = common::Value::CreateStringValue(str);
      obj = new FastoObject(out, val, Delimiter());
      out->AddChildren(obj);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(r->integer);
      obj = new FastoObject(out, val, Delimiter());
      out->AddChildren(obj);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::CreateArrayValue();
      FastoObjectArray* child = new FastoObjectArray(out, arv, Delimiter());
      out->AddChildren(child);

      for (size_t i = 0; i < r->elements; ++i) {
        common::Error er = CliFormatReplyRaw(child, r->element[i]);
        if (er && er->IsError()) {
          return er;
        }
      }
      break;
    }
    default: {
      return common::make_error_value(common::MemSPrintf("Unknown reply type: %d", r->type),
                                      common::ErrorValue::E_ERROR);
    }
  }

  return common::Error();
}

common::Error DBConnection::CliReadReply(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  void* _reply = NULL;
  if (redisGetReply(connection_.handle_, &_reply) != REDIS_OK) {
    /* Filter cases where we should reconnect */
    if (connection_.handle_->err == REDIS_ERR_IO && errno == ECONNRESET) {
      return common::make_error_value("Needed reconnect.", common::ErrorValue::E_ERROR);
    }
    if (connection_.handle_->err == REDIS_ERR_EOF) {
      return common::make_error_value("Needed reconnect.", common::ErrorValue::E_ERROR);
    }

    return cliPrintContextError(connection_.handle_); /* avoid compiler warning */
  }

  redisReply* reply = static_cast<redisReply*>(_reply);
  common::Error er = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  return er;
}

common::Error DBConnection::ExecuteAsPipeline(const std::vector<FastoObjectCommandIPtr>& cmds,
                                              void (*log_command_cb)(FastoObjectCommandIPtr command)) {
  if (cmds.empty()) {
    DNOTREACHED();
    return common::make_error_value("Invalid input command", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  // start piplene mode
  std::vector<FastoObjectCommandIPtr> valid_cmds;
  for (size_t i = 0; i < cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    std::string command = cmd->InputCommand();
    const char* ccommand = common::utils::c_strornull(command);
    if (!ccommand) {
      continue;
    }

    if (log_command_cb) {
      log_command_cb(cmd);
    }
    int argc = 0;
    sds* argv = sdssplitargslong(ccommand, &argc);

    if (argv) {
      if (isPipeLineCommand(argv[0])) {
        valid_cmds.push_back(cmd);
        redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), NULL);
      }
      sdsfreesplitres(argv, argc);
    }
  }

  for (size_t i = 0; i < valid_cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    common::Error er = CliReadReply(cmd.get());
    if (er && er->IsError()) {
      return er;
    }
  }
  // end piplene

  return common::Error();
}

common::Error DBConnection::CommonExec(int argc, const char** argv, FastoObject* out) {
  if (!out || argc < 1) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  size_t* argvlen = reinterpret_cast<size_t*>(malloc(static_cast<size_t>(argc) * sizeof(size_t)));
  for (int j = 0; j < argc; j++) {
    char* carg = const_cast<char*>(argv[j]);
    size_t len = sdslen(carg);
    argvlen[j] = len;
  }

  redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
  free(argvlen);
  common::Error err = CliReadReply(out);
  if (err && err->IsError()) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::Auth(const std::string& password) {
  if (!IsConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  const char* auth_str = common::utils::c_strornull(password);
  common::Error err = authContext(auth_str, connection_.handle_);
  if (err && err->IsError()) {
    isAuth_ = false;
    return err;
  }

  connection_.config_.auth = password;
  isAuth_ = true;
  return common::Error();
}

common::Error DBConnection::Monitor(int argc, const char** argv, FastoObject* out) {
  if (!out || argc < 1) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  size_t* argvlen = reinterpret_cast<size_t*>(malloc(static_cast<size_t>(argc) * sizeof(size_t)));
  for (int j = 0; j < argc; j++) {
    char* carg = const_cast<char*>(argv[j]);
    size_t len = sdslen(carg);
    argvlen[j] = len;
  }

  redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
  free(argvlen);
  common::Error err = CliReadReply(out);
  if (err && err->IsError()) {
    return err;
  }

  while (true) {
    common::Error er = CliReadReply(out);
    if (er && er->IsError()) {
      return er;
    }

    if (IsInterrupted()) {
      return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
    }
  }
}

common::Error DBConnection::Subscribe(int argc, const char** argv, FastoObject* out) {
  if (!out || argc < 1) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  size_t* argvlen = reinterpret_cast<size_t*>(malloc(static_cast<size_t>(argc) * sizeof(size_t)));
  for (int j = 0; j < argc; j++) {
    char* carg = const_cast<char*>(argv[j]);
    size_t len = sdslen(carg);
    argvlen[j] = len;
  }

  redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
  free(argvlen);
  common::Error err = CliReadReply(out);
  if (err && err->IsError()) {
    return err;
  }

  while (true) {
    common::Error er = CliReadReply(out);
    if (er && er->IsError()) {
      return er;
    }

    if (IsInterrupted()) {
      return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
    }
  }
}

common::Error DBConnection::SetEx(const NDbKValue& key, ttl_t ttl) {
  std::string key_str = key.KeyString();
  std::string value_str = key.ValueString();
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(connection_.handle_, "SETEX %s %d %s", key_str.c_str(), ttl, value_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  if (client_) {
    client_->OnKeyAdded(key);
  }
  if (client_) {
    client_->OnKeyTTLChanged(key.Key(), ttl);
  }
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SetNX(const NDbKValue& key, long long* result) {
  std::string key_str = key.KeyString();
  std::string value_str = key.ValueString();
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(connection_.handle_, "SETNX %s %s", key_str.c_str(), value_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_ && reply->integer) {
      client_->OnKeyAdded(key);
    }

    *result = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Lpush(const NKey& key, NValue arr, long long* list_len) {
  if (!arr || arr->GetType() != common::Value::TYPE_ARRAY || !list_len) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  NDbKValue rarr(key, arr);
  translator_t tran = Translator();
  std::string lpush_cmd;
  common::Error err = tran->CreateKeyCommand(rarr, &lpush_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, lpush_cmd.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnKeyAdded(rarr);
    }
    *list_len = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Lrange(const NKey& key, int start, int stop, NDbKValue* loaded_key) {
  if (!IsConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "LRANGE %s %d %d", key_str.c_str(), start, stop));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err && err->IsError()) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    *loaded_key = NDbKValue(key, NValue(val));
    if (client_) {
      client_->OnKeyLoaded(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Sadd(const NKey& key, NValue set, long long* added) {
  if (!set || set->GetType() != common::Value::TYPE_SET || !added) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  NDbKValue rset(key, set);
  translator_t tran = Translator();
  std::string sadd_cmd;
  common::Error err = tran->CreateKeyCommand(rset, &sadd_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, sadd_cmd.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnKeyAdded(rset);
    }
    *added = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Smembers(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "SMEMBERS %s", key_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err && err->IsError()) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error_value("Conversion error array to set", common::Value::E_ERROR);
    }

    common::SetValue* set = common::Value::CreateSetValue();
    for (size_t i = 0; i < arr->GetSize(); ++i) {
      common::Value* lval = nullptr;
      if (arr->Get(i, &lval)) {
        set->Insert(lval->DeepCopy());
      }
    }

    delete val;
    *loaded_key = NDbKValue(key, NValue(set));
    if (client_) {
      client_->OnKeyLoaded(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zadd(const NKey& key, NValue scores, long long* added) {
  if (!scores || scores->GetType() != common::Value::TYPE_ZSET || !added) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  NDbKValue rzset(key, scores);
  translator_t tran = Translator();
  std::string zadd_cmd;
  common::Error err = tran->CreateKeyCommand(rzset, &zadd_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, zadd_cmd.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnKeyAdded(rzset);
    }
    *added = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zrange(const NKey& key, int start, int stop, bool withscores, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  std::string line;
  if (withscores) {
    line = common::MemSPrintf("ZRANGE %s %d %d WITHSCORES", key_str.c_str(), start, stop);
  } else {
    line = common::MemSPrintf("ZRANGE %s %d %d", key_str.c_str(), start, stop);
  }
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, line.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err && err->IsError()) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    if (!withscores) {
      *loaded_key = NDbKValue(key, NValue(val));
      if (client_) {
        client_->OnKeyLoaded(*loaded_key);
      }
      freeReplyObject(reply);
      return common::Error();
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error_value("Conversion error array to zset", common::Value::E_ERROR);
    }

    common::ZSetValue* zset = common::Value::CreateZSetValue();
    for (size_t i = 0; i < arr->GetSize(); i += 2) {
      common::Value* lmember = nullptr;
      common::Value* lscore = nullptr;
      if (arr->Get(i, &lmember) && arr->Get(i + 1, &lscore)) {
        zset->Insert(lscore->DeepCopy(), lmember->DeepCopy());
      }
    }

    delete val;
    *loaded_key = NDbKValue(key, NValue(zset));
    if (client_) {
      client_->OnKeyLoaded(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Hmset(const NKey& key, NValue hash) {
  if (!hash || hash->GetType() != common::Value::TYPE_HASH) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  NDbKValue rhash(key, hash);
  translator_t tran = Translator();
  std::string hmset_cmd;
  common::Error err = tran->CreateKeyCommand(rhash, &hmset_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, hmset_cmd.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_STATUS) {
    if (client_) {
      client_->OnKeyAdded(rhash);
    }
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Hgetall(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "HGETALL %s", key_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err && err->IsError()) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error_value("Conversion error array to hash", common::Value::E_ERROR);
    }

    common::HashValue* hash = common::Value::CreateHashValue();
    for (size_t i = 0; i < arr->GetSize(); i += 2) {
      common::Value* lkey = nullptr;
      common::Value* lvalue = nullptr;
      if (arr->Get(i, &lkey) && arr->Get(i + 1, &lvalue)) {
        hash->Insert(lkey->DeepCopy(), lvalue->DeepCopy());
      }
    }

    delete val;
    *loaded_key = NDbKValue(key, NValue(hash));
    if (client_) {
      client_->OnKeyLoaded(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Decr(const NKey& key, long long* decr) {
  if (!decr) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "DECR %s", key_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateIntegerValue(reply->integer));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *decr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::DecrBy(const NKey& key, int dec, long long* decr) {
  if (!decr) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "DECRBY %s %d", key_str.c_str(), dec));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *decr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Incr(const NKey& key, long long* incr) {
  if (!incr) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "INCR %s", key_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateIntegerValue(reply->integer));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *incr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::IncrBy(const NKey& key, int inc, long long* incr) {
  if (!incr) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "INCRBY %s %d", key_str.c_str(), inc));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *incr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::IncrByFloat(const NKey& key, double inc, std::string* str_incr) {
  if (!str_incr) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.GetKey();
  std::string value_str = common::ConvertToString(inc);
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(connection_.handle_, "INCRBYFLOAT %s %s", key_str.c_str(), value_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_STRING) {
    std::string str(reply->str, reply->len);
    if (client_) {
      NValue val(common::Value::CreateStringValue(str));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *str_incr = str;
    freeReplyObject(reply);
    return common::Error();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  NOTREACHED();
  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
