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

extern "C" {
#include "sds.h"
}

#include <hiredis/hiredis.h>
#include <libssh2.h>  // for libssh2_exit, etc

#include "core/db/redis/cluster_infos.h"  // for makeDiscoveryClusterInfo
#include "core/db/redis/command_translator.h"
#include "core/db/redis/database_info.h"  // for DataBaseInfo
#include "core/db/redis/internal/commands_api.h"
#include "core/db/redis/sentinel_info.h"  // for DiscoverySentinelInfo, etc

#define HIREDIS_VERSION    \
  STRINGIZE(HIREDIS_MAJOR) \
  "." STRINGIZE(HIREDIS_MINOR) "." STRINGIZE(HIREDIS_PATCH)

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
const char* ConnectionTraits<REDIS>::GetBasedOn() {
  return "hiredis";
}

template <>
const char* ConnectionTraits<REDIS>::GetVersionApi() {
  return HIREDIS_VERSION;
}
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<redis::NativeConnection, redis::RConfig>::Connect(
    const redis::RConfig& config,
    redis::NativeConnection** hout) {
  redis::NativeConnection* context = nullptr;
  common::Error err = redis::CreateConnection(config, &context);
  if (err) {
    return err;
  }

  *hout = context;
  // redisEnableKeepAlive(context);
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
const ConstantCommandsArray& CDBConnection<redis::NativeConnection, redis::RConfig, REDIS>::GetCommands() {
  return redis::g_commands;
}
}  // namespace internal

namespace redis {
namespace {

common::Error valueFromReplay(redisReply* r, common::Value** out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_inval();
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
      return common::make_error(str);
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
        common::Error err = valueFromReplay(r->element[i], &val);
        if (err) {
          delete arv;
          return err;
        }
        arv->Append(val);
      }
      *out = arv;
      break;
    }
    default: { return common::make_error(common::MemSPrintf("Unknown reply type: %d", r->type)); }
  }

  return common::Error();
}

common::Error PrintRedisContextError(redisContext* context) {
  if (!context) {
    DNOTREACHED();
    return common::make_error("Not connected");
  }

  return common::make_error(common::MemSPrintf("Error: %s", context->errstr));
}

common::Error ExecRedisCommand(redisContext* c,
                               int argc,
                               const char** argv,
                               const size_t* argvlen,
                               redisReply** out_reply) {
  if (!c) {
    DNOTREACHED();
    return common::make_error("Not connected");
  }

  if (argc <= 0 || !argv || !out_reply) {
    DNOTREACHED();
    return common::make_error_inval();
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
      return common::make_error("Needed reconnect.");
    }
    if (c->err == REDIS_ERR_EOF) {
      return common::make_error("Needed reconnect.");
    }

    return PrintRedisContextError(c);
  }

  redisReply* rreply = static_cast<redisReply*>(reply);
  if (rreply->type == REDIS_REPLY_ERROR) {
    std::string str(rreply->str, rreply->len);
    freeReplyObject(rreply);
    return common::make_error(str);
  }

  *out_reply = rreply;
  return common::Error();
}

common::Error ExecRedisCommand(redisContext* c, command_buffer_t command, redisReply** out_reply) {
  if (command.empty() || !out_reply) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  int argc = 0;
  sds* argv = sdssplitargslong(command.data(), &argc);
  if (!argv) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  void* raw_argvlen_ptr = malloc(static_cast<size_t>(argc) * sizeof(size_t));
  size_t* argvlen = reinterpret_cast<size_t*>(raw_argvlen_ptr);
  for (int i = 0; i < argc; ++i) {
    argvlen[i] = sdslen(argv[i]);
  }

  common::Error err = ExecRedisCommand(c, argc, const_cast<const char**>(argv), argvlen, out_reply);
  sdsfreesplitres(argv, argc);
  free(raw_argvlen_ptr);
  return err;
}

common::Error ExecRedisCommand(redisContext* c, const commands_args_t& argv, redisReply** out_reply) {
  if (argv.empty() || !out_reply) {
    DNOTREACHED();
    return common::make_error_inval();
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
  if (err) {
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
    return common::make_error_inval();
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
    bool is_ssl = config.is_ssl;
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
    lcontext = redisConnect(host, port, ssh_address, ssh_port, username, password, public_key, private_key, passphrase,
                            is_ssl, curM);
  }

  if (!lcontext) {
    if (is_local) {
      return common::make_error(common::MemSPrintf("Could not connect to Redis at %s : no context", config.hostsocket));
    }
    std::string host_str = common::ConvertToString(config.host);
    return common::make_error(common::MemSPrintf("Could not connect to Redis at %s : no context", host_str));
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
    return common::make_error(buff);
  }

  *context = lcontext;
  return common::Error();
}

common::Error TestConnection(const RConfig& rconfig) {
  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err) {
    return err;
  }

  err = AuthContext(context, rconfig.auth);
  if (err) {
    redisFree(context);
    return err;
  }

  redisFree(context);
  return common::Error();
}

common::Error DiscoveryClusterConnection(const RConfig& rconfig, std::vector<ServerDiscoveryClusterInfoSPtr>* infos) {
  if (!infos) {
    return common::make_error_inval();
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err) {
    return err;
  }

  err = AuthContext(context, rconfig.auth);
  if (err) {
    redisFree(context);
    return err;
  }

  /* Send the GET CLUSTER command. */
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(context, GET_SERVER_TYPE));
  if (!reply) {
    err = common::make_error("I/O error");
    redisFree(context);
    return err;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    err = makeDiscoveryClusterInfo(rconfig.host, std::string(reply->str, reply->len), infos);
  } else if (reply->type == REDIS_REPLY_ERROR) {
    err = common::make_error(std::string(reply->str, reply->len));
  } else {
    DNOTREACHED();
  }

  freeReplyObject(reply);
  redisFree(context);
  return err;
}

common::Error DiscoverySentinelConnection(const RConfig& rconfig, std::vector<ServerDiscoverySentinelInfoSPtr>* infos) {
  if (!infos) {
    return common::make_error_inval();
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(rconfig, &context);
  if (err) {
    return err;
  }

  err = AuthContext(context, rconfig.auth);
  if (err) {
    redisFree(context);
    return err;
  }

  /* Send the GET MASTERS command. */
  redisReply* masters_reply = reinterpret_cast<redisReply*>(redisCommand(context, GET_SENTINEL_MASTERS));
  if (!masters_reply) {
    redisFree(context);
    return common::make_error("I/O error");
  }

  for (size_t i = 0; i < masters_reply->elements; ++i) {
    redisReply* master_info = masters_reply->element[i];
    ServerCommonInfo sinf;
    common::Error lerr = MakeServerCommonInfo(master_info, &sinf);
    if (lerr) {
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
      return common::make_error("I/O error");
    }

    if (reply->type == REDIS_REPLY_ARRAY) {
      for (size_t j = 0; j < reply->elements; ++j) {
        redisReply* server_info = reply->element[j];
        ServerCommonInfo slsinf;
        lerr = MakeServerCommonInfo(server_info, &slsinf);
        if (lerr) {
          continue;
        }
        ServerDiscoverySentinelInfoSPtr lsent(new DiscoverySentinelInfo(slsinf));
        infos->push_back(lsent);
      }
    } else if (reply->type == REDIS_REPLY_ERROR) {
      freeReplyObject(reply);
      freeReplyObject(masters_reply);
      redisFree(context);
      return common::make_error(std::string(reply->str, reply->len));
    } else {
      DNOTREACHED();
    }
    freeReplyObject(reply);
  }

  freeReplyObject(masters_reply);
  redisFree(context);
  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::GetCommands())), is_auth_(false), cur_db_(-1) {}

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

  /* Do AUTH and select the right DB. */
  err = Auth(config->auth);
  if (err) {
    return err;
  }

  int db_num = config->db_num;
  err = Select(common::ConvertToString(db_num), NULL);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::Disconnect() {
  cur_db_ = -1;
  is_auth_ = false;
  return base_class::Disconnect();
}

std::string DBConnection::GetCurrentDBName() const {
  if (IsAuthenticated()) {
    return common::ConvertToString(cur_db_);
  }

  DNOTREACHED();
  return base_class::GetCurrentDBName();
}

/* Sends SYNC and reads the number of bytes in the payload.
 * Used both by
 * slaveMode() and getRDB(). */
common::Error DBConnection::SendSync(unsigned long long* payload) {
  if (!payload) {
    DNOTREACHED();
    return common::make_error_inval();
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
    return common::make_error("Error writing to master");
  }

  /* Read $<payload>\r\n, making sure to read just up to
   * "\n" */
  p = buf;
  while (1) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(connection_.handle_, p, 1, &nread);
    if (res == REDIS_ERR) {
      return common::make_error("Error reading bulk length while SYNCing");
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
    return common::make_error(buf2);
  }

  *payload = strtoull(buf + 1, NULL, 10);
  return common::Error();
}

common::Error DBConnection::SlaveMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  unsigned long long payload = 0;
  err = SendSync(&payload);
  if (err) {
    return err;
  }

  char buf[1024];
  /* Discard the payload. */
  while (payload) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(connection_.handle_, buf, (payload > sizeof(buf)) ? sizeof(buf) : payload, &nread);
    if (res == REDIS_ERR) {
      return common::make_error("Error reading RDB payload while SYNCing");
    }
    payload -= nread;
  }

  /* Now we can use hiredis to read the incoming protocol.
   */
  while (!IsInterrupted()) {
    err = CliReadReply(out);
    if (err) {
      return err;
    }
  }

  return common::make_error(common::COMMON_EINTR);
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  const command_buffer_t pattern_result = core::internal::GetKeysPattern(cursor_in, pattern, count_keys);
  redisReply* reply = NULL;
  common::Error err = ExecRedisCommand(connection_.handle_, pattern_result, &reply);
  if (err) {
    return err;
  }

  if (reply->type != REDIS_REPLY_ARRAY) {
    freeReplyObject(reply);
    return common::make_error("I/O error");
  }

  common::Value* val = nullptr;
  err = valueFromReplay(reply, &val);
  if (err) {
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
    return common::make_error("I/O error");
  }

  common::ArrayValue* arr_keys = nullptr;
  if (!arr->GetList(1, &arr_keys)) {
    delete val;
    freeReplyObject(reply);
    return common::make_error("I/O error");
  }

  for (size_t i = 0; i < arr_keys->GetSize(); ++i) {
    std::string key;
    if (arr_keys->GetString(i, &key)) {
      keys_out->push_back(key);
    }
  }

  uint64_t lcursor_out;
  if (!common::ConvertFromString(cursor_out_str, &lcursor_out)) {
    return common::make_error_inval();
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
    return common::make_error("Couldn't determine " DB_DBKCOUNT_COMMAND "!");
  }

  /* Grab the number of keys and free our reply */
  *size = static_cast<size_t>(reply->integer);
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, DB_FLUSHDB_COMMAND));
  if (!reply) {
    return PrintRedisContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  int num;
  if (!common::ConvertFromString(name, &num)) {
    return common::make_error_inval();
  }

  command_buffer_t select_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  common::Error err = tran->SelectDBCommand(common::ConvertToString(num), &select_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, select_cmd, &reply);
  if (err) {
    return err;
  }

  connection_.config_->db_num = num;
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
    if (err) {
      return err;
    }

    redisReply* reply = NULL;
    err = ExecRedisCommand(connection_.handle_, del_cmd, &reply);
    if (err) {
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
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, set_cmd, &reply);
  if (err) {
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
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, get_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_NIL) {
    key_t key_str = key.GetKey();
    return GenerateError(DB_GET_KEY_COMMAND, "key not found.");
  }

  CHECK(reply->type == REDIS_REPLY_STRING) << "Unexpected replay type: " << reply->type;
  common::Value* val = common::Value::CreateStringValue(reply->str);
  *loaded_key = NDbKValue(key, NValue(val));
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, string_key_t new_key) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t rename_cmd;
  common::Error err = tran->RenameKeyCommand(key, key_t(new_key), &rename_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, rename_cmd, &reply);
  if (err) {
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
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, ttl_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->integer == 0) {
    return GenerateError(DB_SET_TTL_COMMAND, "key does not exist or the timeout could not be set.");
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t ttl_cmd;
  common::Error err = tran->LoadKeyTTLCommand(key, &ttl_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, ttl_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type != REDIS_REPLY_INTEGER) {
    freeReplyObject(reply);
    return common::make_error("TTL command internal error");
  }

  *ttl = reply->integer;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::QuitImpl() {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, DB_QUIT_COMMAND));
  if (!reply) {
    return PrintRedisContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  common::Error err = Disconnect();
  UNUSED(err);
  return common::Error();
}

common::Error DBConnection::CliFormatReplyRaw(FastoObjectArray* ar, redisReply* r) {
  if (!ar || !r) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  switch (r->type) {
    case REDIS_REPLY_NIL: {
      common::Value* val = common::Value::CreateNullValue();
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_ERROR: {
      std::string str(r->str, r->len);
      return common::make_error(str);
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
        common::Error err = CliFormatReplyRaw(child, r->element[i]);
        if (err) {
          return err;
        }
      }
      break;
    }
    default: { return common::make_error(common::MemSPrintf("Unknown reply type: %d", r->type)); }
  }

  return common::Error();
}

common::Error DBConnection::CliFormatReplyRaw(FastoObject* out, redisReply* r) {
  if (!out || !r) {
    DNOTREACHED();
    return common::make_error_inval();
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
        is_auth_ = false;
      }
      std::string str(r->str, r->len);
      return common::make_error(str);
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
        common::Error err = CliFormatReplyRaw(child, r->element[i]);
        if (err) {
          return err;
        }
      }
      break;
    }
    default: { return common::make_error(common::MemSPrintf("Unknown reply type: %d", r->type)); }
  }

  return common::Error();
}

common::Error DBConnection::CliReadReply(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsConnected();
  if (err) {
    return err;
  }

  void* _reply = NULL;
  if (redisGetReply(connection_.handle_, &_reply) != REDIS_OK) {
    /* Filter cases where we should reconnect */
    if (connection_.handle_->err == REDIS_ERR_IO && errno == ECONNRESET) {
      return common::make_error("Needed reconnect.");
    }
    if (connection_.handle_->err == REDIS_ERR_EOF) {
      return common::make_error("Needed reconnect.");
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
    return common::make_error("Invalid input command");
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  // start piplene mode
  std::vector<FastoObjectCommandIPtr> valid_cmds;
  for (size_t i = 0; i < cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    command_buffer_t command = cmd->GetInputCommand();
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
        void* raw_argvlen_ptr = malloc(static_cast<size_t>(argc) * sizeof(size_t));
        size_t* argvlen = reinterpret_cast<size_t*>(raw_argvlen_ptr);
        for (int i = 0; i < argc; ++i) {
          argvlen[i] = sdslen(argv[i]);
        }
        redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
        free(raw_argvlen_ptr);
      }
      sdsfreesplitres(argv, argc);
    }
  }

  for (size_t i = 0; i < valid_cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    common::Error err = CliReadReply(cmd.get());
    if (err) {
      return err;
    }
  }
  // end piplene

  return common::Error();
}

common::Error DBConnection::CommonExec(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  return err;
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

common::Error DBConnection::Monitor(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  reply = NULL;
  if (err) {
    return err;
  }

  while (!IsInterrupted()) {  // listen loop
    err = CliReadReply(out);
    if (err) {
      return err;
    }
  }

  return common::make_error(common::COMMON_EINTR);
}

common::Error DBConnection::Subscribe(const commands_args_t& argv, FastoObject* out) {
  if (!out || argv.empty()) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, argv, &reply);
  if (err) {
    return err;
  }

  err = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  reply = NULL;
  if (err) {
    return err;
  }

  while (!IsInterrupted()) {  // listen loop
    err = CliReadReply(out);
    if (err) {
      return err;
    }
  }

  return common::make_error(common::COMMON_EINTR);
}

common::Error DBConnection::SetEx(const NDbKValue& key, ttl_t ttl) {
  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t setex_cmd;
  err = tran->SetEx(key, ttl, &setex_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, setex_cmd, &reply);
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnAddedKey(key);
  }
  if (client_) {
    client_->OnChangedKeyTTL(key.GetKey(), ttl);
  }
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SetNX(const NDbKValue& key, long long* result) {
  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t setnx_cmd;
  err = tran->SetNX(key, &setnx_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, setnx_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_ && reply->integer) {
      client_->OnAddedKey(key);
    }

    *result = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Lpush(const NKey& key, NValue arr, long long* list_len) {
  if (!arr || arr->GetType() != common::Value::TYPE_ARRAY || !list_len) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  NDbKValue rarr(key, arr);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t lpush_cmd;
  err = tran->CreateKeyCommand(rarr, &lpush_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, lpush_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnAddedKey(rarr);
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
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t lrange_cmd;
  err = tran->Lrange(key, start, stop, &lrange_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, lrange_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    *loaded_key = NDbKValue(key, NValue(val));
    if (client_) {
      client_->OnLoadedKey(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Sadd(const NKey& key, NValue set, long long* added) {
  if (!set || set->GetType() != common::Value::TYPE_SET || !added) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  NDbKValue rset(key, set);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t sadd_cmd;
  err = tran->CreateKeyCommand(rset, &sadd_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, sadd_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnAddedKey(rset);
    }
    *added = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Smembers(const NKey& key, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t smembers_cmd;
  err = tran->Smembers(key, &smembers_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, smembers_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error("Conversion error array to set");
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
      client_->OnLoadedKey(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zadd(const NKey& key, NValue scores, long long* added) {
  if (!scores || scores->GetType() != common::Value::TYPE_ZSET || !added) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  NDbKValue rzset(key, scores);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t zadd_cmd;
  err = tran->CreateKeyCommand(rzset, &zadd_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, zadd_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      client_->OnAddedKey(rzset);
    }
    *added = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zrange(const NKey& key, int start, int stop, bool withscores, NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t zrange;
  err = tran->Zrange(key, start, stop, withscores, &zrange);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, zrange, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    if (!withscores) {
      *loaded_key = NDbKValue(key, NValue(val));
      if (client_) {
        client_->OnLoadedKey(*loaded_key);
      }
      freeReplyObject(reply);
      return common::Error();
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error("Conversion error array to zset");
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
      client_->OnLoadedKey(*loaded_key);
    }
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Hmset(const NKey& key, NValue hash) {
  if (!hash || hash->GetType() != common::Value::TYPE_HASH) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  NDbKValue rhash(key, hash);
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t hmset_cmd;
  err = tran->CreateKeyCommand(rhash, &hmset_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, hmset_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_STATUS) {
    if (client_) {
      client_->OnAddedKey(rhash);
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
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  command_buffer_t hgetall_cmd;
  err = tran->Hgetall(key, &hgetall_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, hgetall_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    common::ArrayValue* arr = nullptr;
    if (!val->GetAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error("Conversion error array to hash");
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
      client_->OnLoadedKey(*loaded_key);
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
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string decr_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->Decr(key, &decr_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, decr_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *decr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::DecrBy(const NKey& key, int dec, long long* decr) {
  if (!decr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string decrby_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->DecrBy(key, dec, &decrby_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, decrby_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *decr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::Incr(const NKey& key, long long* incr) {
  if (!incr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string incr_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->Incr(key, &incr_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incr_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *incr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::IncrBy(const NKey& key, int inc, long long* incr) {
  if (!incr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string incrby_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->IncrBy(key, inc, &incrby_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incrby_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_INTEGER) {
    if (client_) {
      NValue val(common::Value::CreateLongLongIntegerValue(reply->integer));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *incr = reply->integer;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

common::Error DBConnection::IncrByFloat(const NKey& key, double inc, std::string* str_incr) {
  if (!str_incr) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  std::string incrfloat_cmd;
  redis_translator_t tran = GetSpecificTranslator<CommandTranslator>();
  err = tran->IncrByFloat(key, inc, &incrfloat_cmd);
  if (err) {
    return err;
  }

  redisReply* reply = NULL;
  err = ExecRedisCommand(connection_.handle_, incrfloat_cmd, &reply);
  if (err) {
    return err;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    std::string str(reply->str, reply->len);
    if (client_) {
      NValue val(common::Value::CreateStringValue(str));
      client_->OnAddedKey(NDbKValue(key, val));
    }
    *str_incr = str;
    freeReplyObject(reply);
    return common::Error();
  }

  DNOTREACHED();
  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
