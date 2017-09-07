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

#include "core/db/redis/db_connection.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef OS_POSIX
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>  // for setsockopt, SOL_SOCKET, etc
#endif

extern "C" {
#include "sds.h"
}

#include <hiredis/hiredis.h>
#include <libssh2.h>  // for libssh2_exit, etc

#include <common/utils.h>  // for c_strornull, usleep, etc

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
template <>
const char* ConnectionTraits<REDIS>::BasedOn() {
  return "hiredis";
}

template <>
const char* ConnectionTraits<REDIS>::VersionApi() {
  return HIREDIS_VERSION;
}
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
ConstantCommandsArray CDBConnection<redis::NativeConnection, redis::RConfig, REDIS>::Commands() {
  return redis::g_commands;
}
}  // namespace internal

namespace redis {
namespace {

common::Error valueFromReplay(redisReply* r, common::Value** out) {
  if (!out) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
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
      return common::make_error_value(str, common::ERROR_TYPE);
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
      return common::make_error_value(common::MemSPrintf("Unknown reply type: %d", r->type), common::ERROR_TYPE);
    }
  }

  return common::Error();
}

common::Error PrintRedisContextError(redisContext* context) {
  if (!context) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::ERROR_TYPE);
  }

  return common::make_error_value(common::MemSPrintf("Error: %s", context->errstr), common::ERROR_TYPE);
}

common::Error ExecRedisCommand(redisContext* c,
                               int argc,
                               const char** argv,
                               const size_t* argvlen,
                               redisReply** out_reply) {
  if (!c) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::ERROR_TYPE);
  }

  if (argc <= 0 || !argv || !out_reply) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  int res = redisAppendCommandArgv(c, argc, argv, argvlen);
  if (res == REDIS_ERR) {
    DNOTREACHED();
    return PrintRedisContextError(c);
  }

  void* reply = NULL;
  res = redisGetReply(c, &reply);
  if (res == REDIS_ERR) {
    /* Filter cases where we should reconnect */
    if (c->err == REDIS_ERR_IO && errno == ECONNRESET) {
      return common::make_error_value("Needed reconnect.", common::ERROR_TYPE);
    }
    if (c->err == REDIS_ERR_EOF) {
      return common::make_error_value("Needed reconnect.", common::ERROR_TYPE);
    }

    return PrintRedisContextError(c);
  }

  redisReply* rreply = static_cast<redisReply*>(reply);
  if (rreply->type == REDIS_REPLY_ERROR) {
    std::string str(rreply->str, rreply->len);
    freeReplyObject(rreply);
    return common::make_error_value(str, common::ERROR_TYPE);
  }

  *out_reply = rreply;
  return common::Error();
}

common::Error ExecRedisCommand(redisContext* c, command_buffer_t command, redisReply** out_reply) {
  if (command.empty() || !out_reply) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  int argc = 0;
  sds* argv = sdssplitargslong(command.data(), &argc);
  if (!argv) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  size_t* argvlen = reinterpret_cast<size_t*>(malloc(argc * sizeof(size_t)));
  for (int i = 0; i < argc; ++i) {
    argvlen[i] = sdslen(argv[i]);
  }

  common::Error err = ExecRedisCommand(c, argc, const_cast<const char**>(argv), argvlen, out_reply);
  sdsfreesplitres(argv, argc);
  free(argvlen);
  return err;
}

common::Error ExecRedisCommand(redisContext* c, const commands_args_t& argv, redisReply** out_reply) {
  if (argv.empty() || !out_reply) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  const char** argvc = static_cast<const char**>(calloc(sizeof(const char*), argv.size()));
  int argcc = 0;
  size_t* argvlen = reinterpret_cast<size_t*>(malloc(argv.size() * sizeof(size_t)));
  for (size_t i = 0; i < argv.size(); ++i) {
    argvc[i] = argv[i].data();
    argvlen[i] = argv[i].size();
    argcc++;
  }

  common::Error err = ExecRedisCommand(c, argcc, argvc, argvlen, out_reply);
  free(argvlen);
  free(argvc);
  return err;
}

common::Error AuthContext(redisContext* context, const std::string& auth_str) {
  if (auth_str.empty()) {
    return common::Error();
  }

  redisReply* reply = NULL;
  common::Error err = ExecRedisCommand(context, {"AUTH", auth_str}, &reply);
  if (err && err->IsError()) {
    return err;
  }
  freeReplyObject(reply);
  return common::Error();
}

}  // namespace

RConfig::RConfig(const Config& config, const SSHInfo& sinfo) : Config(config), ssh_info(sinfo) {}

RConfig::RConfig() : Config(), ssh_info() {}

common::Error CreateConnection(const RConfig& config, NativeConnection** context) {
  if (!context) {
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redisContext* lcontext = NULL;
  bool is_local = !config.hostsocket.empty();

  if (is_local) {
    const char* hostsocket = config.hostsocket.empty() ? NULL : config.hostsocket.c_str();
    lcontext = redisConnectUnix(hostsocket);
  } else {
    SSHInfo sinfo = config.ssh_info;
    std::string host_str = config.host.GetHost();
    const char* host = host_str.empty() ? NULL : host_str.c_str();
    uint16_t port = config.host.GetPort();
    const char* username = sinfo.user_name.empty() ? NULL : sinfo.user_name.c_str();
    const char* password = sinfo.password.empty() ? NULL : sinfo.password.c_str();
    common::net::HostAndPort ssh_host = sinfo.host;
    std::string ssh_host_str = ssh_host.GetHost();
    const char* ssh_address = ssh_host_str.empty() ? NULL : ssh_host_str.c_str();
    int ssh_port = ssh_host.GetPort();
    int curM = sinfo.current_method;
    const char* public_key = sinfo.public_key.empty() ? NULL : sinfo.public_key.c_str();
    const char* private_key = sinfo.private_key.empty() ? NULL : sinfo.private_key.c_str();
    const char* passphrase = sinfo.passphrase.empty() ? NULL : sinfo.passphrase.c_str();
    lcontext =
        redisConnect(host, port, ssh_address, ssh_port, username, password, public_key, private_key, passphrase, curM);
  }

  if (!lcontext) {
    if (is_local) {
      return common::make_error_value(
          common::MemSPrintf("Could not connect to Redis at %s : no context", config.hostsocket), common::ERROR_TYPE);
    }
    std::string host_str = common::ConvertToString(config.host);
    return common::make_error_value(common::MemSPrintf("Could not connect to Redis at %s : no context", host_str),
                                    common::ERROR_TYPE);
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
    return common::make_error_value(buff, common::ERROR_TYPE);
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

  err = AuthContext(context, rconfig.auth);
  if (err && err->IsError()) {
    redisFree(context);
    return err;
  }

  redisFree(context);
  return common::Error();
}

common::Error DiscoveryClusterConnection(const RConfig& rconfig, std::vector<ServerDiscoveryClusterInfoSPtr>* infos) {
  if (!infos) {
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err && err->IsError()) {
    return err;
  }

  err = AuthContext(context, rconfig.auth);
  if (err && err->IsError()) {
    redisFree(context);
    return err;
  }

  /* Send the GET CLUSTER command. */
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(context, GET_SERVER_TYPE));
  if (!reply) {
    err = common::make_error_value("I/O error", common::ERROR_TYPE);
    redisFree(context);
    return err;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    err = makeDiscoveryClusterInfo(rconfig.host, std::string(reply->str, reply->len), infos);
  } else if (reply->type == REDIS_REPLY_ERROR) {
    err = common::make_error_value(std::string(reply->str, reply->len), common::ERROR_TYPE);
  } else {
    NOTREACHED();
  }

  freeReplyObject(reply);
  redisFree(context);
  return err;
}

common::Error DiscoverySentinelConnection(const RConfig& rconfig, std::vector<ServerDiscoverySentinelInfoSPtr>* infos) {
  if (!infos) {
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err && err->IsError()) {
    return err;
  }

  err = AuthContext(context, rconfig.auth);
  if (err && err->IsError()) {
    redisFree(context);
    return err;
  }

  /* Send the GET MASTERS command. */
  redisReply* masters_reply = reinterpret_cast<redisReply*>(redisCommand(context, GET_SENTINEL_MASTERS));
  if (!masters_reply) {
    redisFree(context);
    return common::make_error_value("I/O error", common::ERROR_TYPE);
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
      return common::make_error_value("I/O error", common::ERROR_TYPE);
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
      return common::make_error_value(std::string(reply->str, reply->len), common::ERROR_TYPE);
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
    return common::make_inval_error_value(common::ERROR_TYPE);
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
    return common::make_error_value("Error writing to master", common::ERROR_TYPE);
  }

  /* Read $<payload>\r\n, making sure to read just up to
   * "\n" */
  p = buf;
  while (1) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(connection_.handle_, p, 1, &nread);
    if (res == REDIS_ERR) {
      return common::make_error_value("Error reading bulk length while SYNCing", common::ERROR_TYPE);
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
    return common::make_error_value(buf2, common::ERROR_TYPE);
  }

  *payload = strtoull(buf + 1, NULL, 10);
  return common::Error();
}

common::Error DBConnection::SlaveMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::ERROR_TYPE);
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
      return common::make_error_value("Error reading RDB payload while SYNCing", common::ERROR_TYPE);
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

  return common::make_error_value("Interrupted.", common::INTERRUPTED_TYPE);
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  const command_buffer_t pattern_result = core::internal::GetKeysPattern(cursor_in, pattern, count_keys);
  redisReply* reply = NULL;
  common::Error err = ExecRedisCommand(connection_.handle_, pattern_result, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type != REDIS_REPLY_ARRAY) {
    freeReplyObject(reply);
    return common::make_error_value("I/O error", common::ERROR_TYPE);
  }

  common::Value* val = nullptr;
  err = valueFromReplay(reply, &val);
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
    return common::make_error_value("I/O error", common::ERROR_TYPE);
  }

  common::ArrayValue* arr_keys = nullptr;
  if (!arr->GetList(1, &arr_keys)) {
    delete val;
    freeReplyObject(reply);
    return common::make_error_value("I/O error", common::ERROR_TYPE);
  }

  for (size_t i = 0; i < arr_keys->GetSize(); ++i) {
    std::string key;
    if (arr_keys->GetString(i, &key)) {
      keys_out->push_back(key);
    }
  }

  uint64_t lcursor_out;
  if (!common::ConvertFromString(cursor_out_str, &lcursor_out)) {
    return common::make_inval_error_value(common::ERROR_TYPE);
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
    return common::make_error_value("Couldn't determine DBSIZE!", common::ERROR_TYPE);
  }

  /* Grab the number of keys and free our reply */
  *size = static_cast<size_t>(reply->integer);
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "FLUSHDB"));
  if (!reply) {
    return PrintRedisContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  int num;
  if (!common::ConvertFromString(name, &num)) {
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  command_buffer_t select_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->SelectDBCommand(common::ConvertToString(num), &select_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, select_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  connection_.config_.dbnum = num;
  cur_db_ = num;
  size_t sz = 0;
  err = DBkcount(&sz);
  DCHECK(!err);
  DataBaseInfo* linfo = new DataBaseInfo(common::ConvertToString(num), true, sz);
  *info = linfo;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::DeleteImpl(const NKeys& keys, NKeys* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    command_buffer_t del_cmd;
    redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
    common::Error err = tran->DeleteKeyCommand(key, &del_cmd);
    if (err && err->IsError()) {
      return err;
    }

    redisReply* reply = NULL;
    err = ExecRedisCommand(connection_.handle_, del_cmd, &reply);
    if (err && err->IsError()) {
      return err;
    }

    if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
      deleted_keys->push_back(key);
    }
    freeReplyObject(reply);
  }

  return common::Error();
}

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  command_buffer_t set_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->CreateKeyCommand(key, &set_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, set_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  *added_key = key;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::GetImpl(const NKey& key, NDbKValue* loaded_key) {
  command_buffer_t get_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->LoadKeyCommand(key, common::Value::TYPE_STRING, &get_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, get_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  common::Value* val = nullptr;
  if (reply->type == REDIS_REPLY_STRING) {
    val = common::Value::CreateStringValue(reply->str);
  } else if (reply->type == REDIS_REPLY_NIL) {
    val = common::Value::CreateNullValue();
  } else {
    NOTREACHED();
  }
  *loaded_key = NDbKValue(key, NValue(val));
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, string_key_t new_key) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t rename_cmd;
  common::Error err = tran->RenameKeyCommand(key, key_t(new_key), &rename_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, rename_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  key_t key_str = key.GetKey();
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t ttl_cmd;
  common::Error err = tran->ChangeKeyTTLCommand(key, ttl, &ttl_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, ttl_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->integer == 0) {
    const std::string err_str = key_str.ToString() + " does not exist or the timeout could not be set.";
    return common::make_error_value(err_str, common::ERROR_TYPE);
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t ttl_cmd;
  common::Error err = tran->LoadKeyTTLCommand(key, &ttl_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, ttl_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type != REDIS_REPLY_INTEGER) {
    freeReplyObject(reply);
    return common::make_error_value("TTL command internal error", common::ERROR_TYPE);
  }

  *ttl = reply->integer;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::QuitImpl() {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "QUIT"));
  if (!reply) {
    return PrintRedisContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  base_class::Disconnect();
  return common::Error();
}

common::Error DBConnection::CliFormatReplyRaw(FastoObjectArray* ar, redisReply* r) {
  if (!ar || !r) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  switch (r->type) {
    case REDIS_REPLY_NIL: {
      common::Value* val = common::Value::CreateNullValue();
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_ERROR: {
      std::string str(r->str, r->len);
      return common::make_error_value(str, common::ERROR_TYPE);
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
      FastoObjectArray* child = new FastoObjectArray(ar, arv, GetDelimiter());
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
      return common::make_error_value(common::MemSPrintf("Unknown reply type: %d", r->type), common::ERROR_TYPE);
    }
  }

  return common::Error();
}

common::Error DBConnection::CliFormatReplyRaw(FastoObject* out, redisReply* r) {
  if (!out || !r) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  FastoObject* obj = nullptr;
  switch (r->type) {
    case REDIS_REPLY_NIL: {
      common::Value* val = common::Value::CreateNullValue();
      obj = new FastoObject(out, val, GetDelimiter());
      out->AddChildren(obj);
      break;
    }
    case REDIS_REPLY_ERROR: {
      if (common::strcasestr(r->str, "NOAUTH")) {  //"NOAUTH Authentication
                                                   // required."
        isAuth_ = false;
      }
      std::string str(r->str, r->len);
      return common::make_error_value(str, common::ERROR_TYPE);
    }
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING: {
      std::string str(r->str, r->len);
      common::StringValue* val = common::Value::CreateStringValue(str);
      obj = new FastoObject(out, val, GetDelimiter());
      out->AddChildren(obj);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      common::FundamentalValue* val = common::Value::CreateLongLongIntegerValue(r->integer);
      obj = new FastoObject(out, val, GetDelimiter());
      out->AddChildren(obj);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::CreateArrayValue();
      FastoObjectArray* child = new FastoObjectArray(out, arv, GetDelimiter());
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
      return common::make_error_value(common::MemSPrintf("Unknown reply type: %d", r->type), common::ERROR_TYPE);
    }
  }

  return common::Error();
}

common::Error DBConnection::CliReadReply(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::ERROR_TYPE);
  }

  void* _reply = NULL;
  if (redisGetReply(connection_.handle_, &_reply) != REDIS_OK) {
    /* Filter cases where we should reconnect */
    if (connection_.handle_->err == REDIS_ERR_IO && errno == ECONNRESET) {
      return common::make_error_value("Needed reconnect.", common::ERROR_TYPE);
    }
    if (connection_.handle_->err == REDIS_ERR_EOF) {
      return common::make_error_value("Needed reconnect.", common::ERROR_TYPE);
    }

    return PrintRedisContextError(connection_.handle_); /* avoid compiler warning */
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
    return common::make_error_value("Invalid input command", common::ERROR_TYPE);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::ERROR_TYPE);
  }

  // start piplene mode
  std::vector<FastoObjectCommandIPtr> valid_cmds;
  for (size_t i = 0; i < cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    command_buffer_t command = cmd->InputCommand();
    if (command.empty()) {
      continue;
    }

    if (log_command_cb) {
      log_command_cb(cmd);
    }

    int argc = 0;
    sds* argv = sdssplitargslong(command.data(), &argc);
    if (argv) {
      if (isPipeLineCommand(argv[0])) {
        valid_cmds.push_back(cmd);
        size_t* argvlen = reinterpret_cast<size_t*>(malloc(argc * sizeof(size_t)));
        for (int i = 0; i < argc; ++i) {
          argvlen[i] = sdslen(argv[i]);
        }
        redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
        free(argvlen);
      }
      sdsfreesplitres(argv, argc);
    }
  }

  for (size_t i = 0; i < valid_cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    common::Error err = CliReadReply(cmd.get());
    if (err && err->IsError()) {
      return err;
    }
  }
  // end piplene

  return common::Error();
}

common::Error DBConnection::CommonExec(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redisReply* reply = NULL;
  common::Error err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err && err->IsError()) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  return err;
}

common::Error DBConnection::Auth(const std::string& password) {
  common::Error err = AuthContext(connection_.handle_, password);
  if (err && err->IsError()) {
    isAuth_ = false;
    return err;
  }

  connection_.config_.auth = password;
  isAuth_ = true;
  return common::Error();
}

common::Error DBConnection::Monitor(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redisReply* reply = NULL;
  common::Error err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err && err->IsError()) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  reply = NULL;
  if (err && err->IsError()) {
    return err;
  }

  while (!IsInterrupted()) {  // listen loop
    err = CliReadReply(out);
    if (err && err->IsError()) {
      return err;
    }
  }

  return common::make_error_value("Interrupted.", common::INTERRUPTED_TYPE);
}

common::Error DBConnection::Subscribe(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redisReply* reply = NULL;
  common::Error err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err && err->IsError()) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  reply = NULL;
  if (err && err->IsError()) {
    return err;
  }

  while (!IsInterrupted()) {  // listen loop
    err = CliReadReply(out);
    if (err && err->IsError()) {
      return err;
    }
  }

  return common::make_error_value("Interrupted.", common::INTERRUPTED_TYPE);
}

common::Error DBConnection::SetEx(const NDbKValue& key, ttl_t ttl) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t setex_cmd;
  common::Error err = tran->SetEx(key, ttl, &setex_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, setex_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (client_) {
    client_->OnKeyAdded(key);
  }
  if (client_) {
    client_->OnKeyTTLChanged(key.GetKey(), ttl);
  }
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SetNX(const NDbKValue& key, long long* result) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t setnx_cmd;
  common::Error err = tran->SetNX(key, &setnx_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, setnx_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_ && reply->integer) {
      client_->OnKeyAdded(key);
    }

    *result = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Lpush(const NKey& key, NValue arr, long long* list_len) {
  if (!arr || arr->GetType() != common::Value::TYPE_ARRAY || !list_len) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  NDbKValue rarr(key, arr);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t lpush_cmd;
  common::Error err = tran->CreateKeyCommand(rarr, &lpush_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, lpush_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnKeyAdded(rarr);
    }
    *list_len = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Lrange(const NKey& key, int start, int stop, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t lrange_cmd;
  common::Error err = tran->Lrange(key, start, stop, &lrange_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, lrange_cmd, &reply);
  if (err && err->IsError()) {
    return err;
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
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Sadd(const NKey& key, NValue set, long long* added) {
  if (!set || set->GetType() != common::Value::TYPE_SET || !added) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  NDbKValue rset(key, set);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t sadd_cmd;
  common::Error err = tran->CreateKeyCommand(rset, &sadd_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, sadd_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnKeyAdded(rset);
    }
    *added = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Smembers(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t smembers_cmd;
  common::Error err = tran->Smembers(key, &smembers_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, smembers_cmd, &reply);
  if (err && err->IsError()) {
    return err;
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
      return common::make_error_value("Conversion error array to set", common::ERROR_TYPE);
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
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zadd(const NKey& key, NValue scores, long long* added) {
  if (!scores || scores->GetType() != common::Value::TYPE_ZSET || !added) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  NDbKValue rzset(key, scores);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t zadd_cmd;
  common::Error err = tran->CreateKeyCommand(rzset, &zadd_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, zadd_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnKeyAdded(rzset);
    }
    *added = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zrange(const NKey& key, int start, int stop, bool withscores, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t zrange;
  common::Error err = tran->Zrange(key, start, stop, withscores, &zrange);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, zrange, &reply);
  if (err && err->IsError()) {
    return err;
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
      return common::make_error_value("Conversion error array to zset", common::ERROR_TYPE);
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
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Hmset(const NKey& key, NValue hash) {
  if (!hash || hash->GetType() != common::Value::TYPE_HASH) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  NDbKValue rhash(key, hash);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t hmset_cmd;
  common::Error err = tran->CreateKeyCommand(rhash, &hmset_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, hmset_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_STATUS) {
    if (client_) {
      client_->OnKeyAdded(rhash);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Hgetall(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t hgetall_cmd;
  common::Error err = tran->Hgetall(key, &hgetall_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, hgetall_cmd, &reply);
  if (err && err->IsError()) {
    return err;
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
      return common::make_error_value("Conversion error array to hash", common::ERROR_TYPE);
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
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Decr(const NKey& key, long long* decr) {
  if (!decr) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  std::string decr_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->Decr(key, &decr_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, decr_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateIntegerValue(reply->integer));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *decr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::DecrBy(const NKey& key, int dec, long long* decr) {
  if (!decr) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  std::string decrby_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->DecrBy(key, dec, &decrby_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, decrby_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *decr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Incr(const NKey& key, long long* incr) {
  if (!incr) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  std::string incr_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->Incr(key, &incr_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incr_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateIntegerValue(reply->integer));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *incr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::IncrBy(const NKey& key, int inc, long long* incr) {
  if (!incr) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  std::string incrby_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->IncrBy(key, inc, &incrby_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incrby_cmd, &reply);
  if (err && err->IsError()) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnKeyAdded(NDbKValue(key, val));
    }
    *incr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::IncrByFloat(const NKey& key, double inc, std::string* str_incr) {
  if (!str_incr) {
    DNOTREACHED();
    return common::make_inval_error_value(common::ERROR_TYPE);
  }

  std::string incrfloat_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->IncrByFloat(key, inc, &incrfloat_cmd);
  if (err && err->IsError()) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incrfloat_cmd, &reply);
  if (err && err->IsError()) {
    return err;
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
  }

  NOTREACHED();
  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
