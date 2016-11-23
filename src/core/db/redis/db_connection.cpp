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
#include <sys/socket.h>  // for setsockopt, SOL_SOCKET, etc
#include <netinet/in.h>
#include <netinet/tcp.h>
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

#include "third-party/redis/src/help.h"
#include "third-party/libssh2/include/libssh2.h"  // for libssh2_exit, etc

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

#include <common/qt/logger.h>  // for LOG_MSG

#include "core/icommand_translator.h"  // for translator_t, etc

#include "core/internal/connection.h"     // for Connection<>::config_t, etc
#include "core/command/command.h"         // for CreateCommand
#include "core/command/command_logger.h"  // for LOG_COMMAND

#include "core/db/redis/cluster_infos.h"        // for makeDiscoveryClusterInfo
#include "core/db/redis/command.h"              // for Command
#include "core/db/redis/connection_settings.h"  // for ConnectionSettings
#include "core/db/redis/database.h"             // for DataBaseInfo
#include "core/db/redis/sentinel_info.h"        // for DiscoverySentinelInfo, etc
#include "core/db/redis/command_translator.h"
#include "core/db/redis/internal/commands_api.h"

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

typedef struct {
  int type;
  int argc;
  sds* argv;
  sds full;

  /* Only used for help on commands */
  struct commandHelp* org;
} helpEntry;

const size_t helpEntriesLen =
    sizeof(commandHelp) / sizeof(struct commandHelp) + sizeof(commandGroups) / sizeof(char*);

const struct RedisInit {
  helpEntry helpEntries[helpEntriesLen];

  RedisInit() {
    libssh2_init(0);
    int pos = 0;

    for (size_t i = 0; i < sizeof(commandGroups) / sizeof(char*); ++i) {
      helpEntry tmp;

      tmp.argc = 1;
      tmp.argv = reinterpret_cast<sds*>(malloc(sizeof(sds)));
      tmp.argv[0] = sdscatprintf(sdsempty(), "@%s", commandGroups[i]);
      tmp.full = tmp.argv[0];
      tmp.type = CLI_HELP_GROUP;
      tmp.org = NULL;
      helpEntries[pos++] = tmp;
    }

    for (size_t i = 0; i < sizeof(commandHelp) / sizeof(struct commandHelp); ++i) {
      helpEntry tmp;

      tmp.argv = sdssplitargs(commandHelp[i].name, &tmp.argc);
      tmp.full = sdsnew(commandHelp[i].name);
      tmp.type = CLI_HELP_COMMAND;
      tmp.org = &commandHelp[i];
      helpEntries[pos++] = tmp;
    }
  }
  ~RedisInit() {
    for (size_t i = 0; i < sizeof(commandGroups) / sizeof(char*); i++) {
      helpEntry* entry = &helpEntries[i];
      sdsfree(entry->full);
      free(entry->argv);
    }

    for (size_t i = 0; i < sizeof(commandHelp) / sizeof(struct commandHelp); i++) {
      helpEntry* entry = &helpEntries[i + sizeof(commandGroups) / sizeof(char*)];
      sdsfree(entry->full);
      sdsfreesplitres(entry->argv, entry->argc);
    }

    libssh2_exit();
  }
} rInit;

/* Return the specified INFO field from the INFO command
 * output "info".
 * A new buffer is allocated for the result, that needs to
 * be free'd.
 * If the field is not found NULL is returned. */
char* getInfoField(char* info, const char* field) {
  char* p = strstr(info, field);
  char *n1, *n2;
  char* result;

  if (!p) {
    return NULL;
  }
  p += strlen(field) + 1;
  n1 = strchr(p, '\r');
  n2 = strchr(p, ',');
  if (n2 && n2 < n1) {
    n1 = n2;
  }
  result = reinterpret_cast<char*>(malloc(sizeof(char) * (n1 - p) + 1));
  memcpy(result, p, (n1 - p));
  result[n1 - p] = '\0';
  return result;
}

/* Like the above function but automatically convert the
 * result into
 * a long. On error (missing field) LONG_MIN is returned. */
long getLongInfoField(char* info, const char* field) {
  char* value = getInfoField(info, field);
  if (!value) {
    return LONG_MIN;
  }

  long l = strtol(value, NULL, 10);
  free(value);
  return l;
}

/* Convert number of bytes into a human readable string of
 * the form:
 * 100B, 2G, 100M, 4K, and so forth. */
void bytesToHuman(char* s, size_t len, int64_t n) {
  double d;

  if (n < 0) {
    *s = '-';
    s++;
    n = -n;
  }
  if (n < 1024) {
    /* Bytes */
    common::SNPrintf(s, len, "%lluB", n);
    return;
  } else if (n < (1024 * 1024)) {
    d = static_cast<double>(n / (1024));
    common::SNPrintf(s, len, "%.2fK", d);
  } else if (n < (1024LL * 1024 * 1024)) {
    d = static_cast<double>(n / (1024 * 1024));
    common::SNPrintf(s, len, "%.2fM", d);
  } else if (n < (1024LL * 1024 * 1024 * 1024)) {
    d = static_cast<double>(n / (1024LL * 1024 * 1024));
    common::SNPrintf(s, len, "%.2fG", d);
  }
}

bool isPipeLineCommand(const char* command) {
  if (!command) {
    DNOTREACHED();
    return false;
  }

  bool skip = strcasecmp(command, "quit") == 0 || strcasecmp(command, "exit") == 0 ||
              strcasecmp(command, "connect") == 0 || strcasecmp(command, "help") == 0 ||
              strcasecmp(command, "?") == 0 || strcasecmp(command, "shutdown") == 0 ||
              strcasecmp(command, "monitor") == 0 || strcasecmp(command, "subscribe") == 0 ||
              strcasecmp(command, "psubscribe") == 0 || strcasecmp(command, "sync") == 0 ||
              strcasecmp(command, "psync") == 0;

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
  if (er && er->isError()) {
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
bool ConnectionAllocatorTraits<redis::NativeConnection, redis::RConfig>::IsConnected(
    redis::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  return true;
}

template <>
const char* CDBConnection<redis::NativeConnection, redis::RConfig, REDIS>::VersionApi() {
  return HIREDIS_VERSION;
}

template <>
std::vector<CommandHolder>
CDBConnection<redis::NativeConnection, redis::RConfig, REDIS>::Commands() {
  return redis::redisCommands;
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
      *out = common::Value::createNullValue();
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
      *out = common::Value::createStringValue(str);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      *out = common::Value::createIntegerValue(r->integer);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::createArrayValue();
      for (size_t i = 0; i < r->elements; ++i) {
        common::Value* val = NULL;
        common::Error er = valueFromReplay(r->element[i], &val);
        if (er && er->isError()) {
          delete arv;
          return er;
        }
        arv->append(val);
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

common::Error toIntType(char* key, char* type, int* res) {
  if (!strcmp(type, "string")) {
    *res = RTYPE_STRING;
    return common::Error();
  } else if (!strcmp(type, "list")) {
    *res = RTYPE_LIST;
    return common::Error();
  } else if (!strcmp(type, "set")) {
    *res = RTYPE_SET;
    return common::Error();
  } else if (!strcmp(type, "hash")) {
    *res = RTYPE_HASH;
    return common::Error();
  } else if (!strcmp(type, "zset")) {
    *res = RTYPE_ZSET;
    return common::Error();
  } else if (!strcmp(type, "none")) {
    *res = RTYPE_NONE;
    return common::Error();
  } else {
    return common::make_error_value(common::MemSPrintf("Unknown type '%s' for key '%s'", type, key),
                                    common::Value::E_ERROR);
  }
}

common::Error cliPrintContextError(redisContext* context) {
  if (!context) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string buff = common::MemSPrintf("Error: %s", context->errstr);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

common::Error cliOutputCommandHelp(FastoObject* out,
                                   struct commandHelp* help,
                                   int group,
                                   const std::string& delimiter) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string buff = common::MemSPrintf("name: %s %s\n  summary: %s\n  since: %s", help->name,
                                        help->params, help->summary, help->since);
  common::StringValue* val = common::Value::createStringValue(buff);
  FastoObject* child = new FastoObject(out, val, delimiter);
  out->AddChildren(child);
  if (group) {
    std::string buff2 = common::MemSPrintf("group: %s", commandGroups[help->group]);
    val = common::Value::createStringValue(buff2);
    FastoObject* gchild = new FastoObject(out, val, delimiter);
    out->AddChildren(gchild);
  }

  return common::Error();
}

common::Error cliOutputGenericHelp(FastoObject* out, const std::string& delimiter) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::StringValue* val = common::Value::createStringValue(
      PROJECT_NAME_TITLE " based on hiredis " HIREDIS_VERSION
                         "\r\n"
                         "Type: \"help @<group>\" to get a list of "
                         "commands in <group>\r\n"
                         "      \"help <command>\" for help on "
                         "<command>\r\n"
                         "      \"help <tab>\" to get a list of possible "
                         "help topics\r\n"
                         "      \"quit\" to exit");
  FastoObject* child = new FastoObject(out, val, delimiter);
  out->AddChildren(child);

  return common::Error();
}

common::Error cliOutputHelp(int argc,
                            const char** argv,
                            FastoObject* out,
                            const std::string& delimiter) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  int group = -1;
  const helpEntry* entry;
  struct commandHelp* help;

  if (argc == 0) {
    return cliOutputGenericHelp(out, delimiter);
  } else if (argc > 0 && argv[0][0] == '@') {
    size_t len = sizeof(commandGroups) / sizeof(char*);
    for (size_t i = 0; i < len; i++) {
      if (strcasecmp(argv[0] + 1, commandGroups[i]) == 0) {
        group = i;
        break;
      }
    }
  }

  DCHECK(argc > 0);
  for (size_t i = 0; i < helpEntriesLen; i++) {
    entry = &rInit.helpEntries[i];
    if (entry->type != CLI_HELP_COMMAND) {
      continue;
    }

    help = entry->org;
    if (group == -1) {
      /* Compare all arguments */
      if (argc == entry->argc) {
        int j;
        for (j = 0; j < argc; j++) {
          if (strcasecmp(argv[j], entry->argv[j]) != 0) {
            break;
          }
        }
        if (j == argc) {
          common::Error er = cliOutputCommandHelp(out, help, 1, delimiter);
          if (er && er->isError()) {
            return er;
          }
        }
      }
    } else {
      if (group == help->group) {
        common::Error er = cliOutputCommandHelp(out, help, 0, delimiter);
        if (er && er->isError()) {
          return er;
        }
      }
    }
  }

  return common::Error();
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
    lcontext = redisConnect(host, port, ssh_address, ssh_port, username, password, public_key,
                            private_key, passphrase, curM);
  }

  if (!lcontext) {
    std::string buff;
    if (!is_local) {
      std::string host_str = common::ConvertToString(config.host);
      buff = common::MemSPrintf("Could not connect to Redis at %s : no context", host_str);
    } else {
      buff = common::MemSPrintf("Could not connect to Redis at %s : no context", config.hostsocket);
    }
    return common::make_error_value(buff, common::Value::E_ERROR);
  }

  if (lcontext->err) {
    std::string buff;
    if (is_local) {
      buff = common::MemSPrintf("Could not connect to Redis at %s : %s", config.hostsocket,
                                lcontext->errstr);
    } else {
      std::string host_str = common::ConvertToString(config.host);
      buff =
          common::MemSPrintf("Could not connect to Redis at %s : %s", host_str, lcontext->errstr);
    }
    redisFree(lcontext);
    return common::make_error_value(buff, common::Value::E_ERROR);
  }

  *context = lcontext;
  return common::Error();
}

common::Error CreateConnection(ConnectionSettings* settings, NativeConnection** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  RConfig rconfig(settings->Info(), settings->SSHInfo());
  return CreateConnection(rconfig, context);
}

common::Error TestConnection(ConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(settings, &context);
  if (err && err->isError()) {
    return err;
  }

  Config config = settings->Info();
  const char* auth_str = common::utils::c_strornull(config.auth);
  err = authContext(auth_str, context);
  if (err && err->isError()) {
    redisFree(context);
    return err;
  }

  redisFree(context);
  return common::Error();
}

common::Error DiscoveryClusterConnection(ConnectionSettings* settings,
                                         std::vector<ServerDiscoveryClusterInfoSPtr>* infos) {
  if (!settings || !infos) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(settings, &context);
  if (err && err->isError()) {
    return err;
  }

  Config config = settings->Info();
  const char* auth_str = common::utils::c_strornull(config.auth);
  err = authContext(auth_str, context);
  if (err && err->isError()) {
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
    err = makeDiscoveryClusterInfo(config.host, std::string(reply->str, reply->len), infos);
  } else if (reply->type == REDIS_REPLY_ERROR) {
    err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
  } else {
    NOTREACHED();
  }

  freeReplyObject(reply);
  redisFree(context);
  return err;
}

common::Error DiscoverySentinelConnection(ConnectionSettings* settings,
                                          std::vector<ServerDiscoverySentinelInfoSPtr>* infos) {
  if (!settings || !infos) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* context = NULL;
  common::Error err = CreateConnection(settings, &context);
  if (err && err->isError()) {
    return err;
  }

  Config config = settings->Info();
  const char* auth_str = common::utils::c_strornull(config.auth);
  err = authContext(auth_str, context);
  if (err && err->isError()) {
    redisFree(context);
    return err;
  }

  /* Send the GET MASTERS command. */
  redisReply* masters_reply =
      reinterpret_cast<redisReply*>(redisCommand(context, GET_SENTINEL_MASTERS));
  if (!masters_reply) {
    redisFree(context);
    return common::make_error_value("I/O error", common::Value::E_ERROR);
  }

  for (size_t i = 0; i < masters_reply->elements; ++i) {
    redisReply* master_info = masters_reply->element[i];
    ServerCommonInfo sinf;
    common::Error lerr = MakeServerCommonInfo(master_info, &sinf);
    if (lerr && lerr->isError()) {
      continue;
    }

    const char* master_name = sinf.name.c_str();
    ServerDiscoverySentinelInfoSPtr sent(new DiscoverySentinelInfo(sinf));
    infos->push_back(sent);
    /* Send the GET SLAVES command. */
    redisReply* reply = reinterpret_cast<redisReply*>(
        redisCommand(context, GET_SENTINEL_SLAVES_PATTERN_1ARGS_S, master_name));
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
        if (lerr && lerr->isError()) {
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
    : base_class(client, new CommandTranslator), isAuth_(false), cur_db_(-1) {}

bool DBConnection::IsAuthenticated() const {
  if (!IsConnected()) {
    return false;
  }

  return isAuth_;
}

common::Error DBConnection::Connect(const config_t& config) {
  common::Error err = base_class::Connect(config);
  if (err && err->isError()) {
    return err;
  }

  /* Do AUTH and select the right DB. */
  err = Auth(connection_.config_.auth);
  if (err && err->isError()) {
    return err;
  }

  err = Select(common::ConvertToString(connection_.config_.dbnum), NULL);
  if (err && err->isError()) {
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

common::Error DBConnection::LatencyMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  FastoObjectCommandIPtr cmd = CreateCommand<Command>(out, "PING", common::Value::C_INNER);
  if (!cmd) {
    return common::make_error_value("Invalid CreateCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  common::time64_t start;
  uint64_t min = 0, max = 0, tot = 0, count = 0;
  uint64_t history_interval = connection_.config_.interval ? connection_.config_.interval / 1000
                                                           : LATENCY_HISTORY_DEFAULT_INTERVAL;
  common::time64_t history_start = common::time::current_mstime();

  FastoObjectIPtr child;
  std::string command = cmd->InputCommand();

  double avg;
  while (!IsInterrupted()) {
    start = common::time::current_mstime();
    redisReply* reply =
        reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, command.c_str()));
    if (!reply) {
      return common::make_error_value("I/O error", common::Value::E_ERROR);
    }

    common::time64_t cur_time = common::time::current_mstime();

    uint64_t latency = cur_time - start;
    freeReplyObject(reply);
    count++;
    if (count == 1) {
      min = max = tot = latency;
      avg = static_cast<double>(latency);
    } else {
      if (latency < min) {
        min = latency;
      }
      if (latency > max) {
        max = latency;
      }
      tot += latency;
      avg = static_cast<double>(tot) / static_cast<double>(count);
    }

    std::string avg_str = common::ConvertToString(avg, 2);
    std::string buff = common::MemSPrintf("min: %llu, max: %llu, avg: %s (%llu samples)", min, max,
                                          avg_str, count);
    common::Value* val = common::Value::createStringValue(buff);

    if (!child) {
      child = make_fasto_object<FastoObject>(cmd.get(), val, Delimiter());
      cmd->AddChildren(child);
      continue;
    }

    if (connection_.config_.latency_history && (cur_time - history_start > history_interval)) {
      child = make_fasto_object<FastoObject>(cmd.get(), val, Delimiter());
      cmd->AddChildren(child);
      history_start = cur_time;
      min = max = tot = count = 0;
    } else {
      child->SetValue(common::ValueSPtr(val));
    }

    common::utils::msleep(LATENCY_SAMPLE_RATE);
  }

  return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
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
      return common::make_error_value("Error reading bulk length while SYNCing",
                                      common::ErrorValue::E_ERROR);
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
  if (err && err->isError()) {
    return err;
  }

  FastoObjectCommandIPtr cmd = CreateCommand<Command>(out, SYNC_REQUEST, common::Value::C_INNER);
  if (!cmd) {
    return common::make_error_value("Invalid CreateCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  char buf[1024];
  /* Discard the payload. */
  while (payload) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(connection_.handle_, buf,
                                (payload > sizeof(buf)) ? sizeof(buf) : payload, &nread);
    if (res == REDIS_ERR) {
      return common::make_error_value("Error reading RDB payload while SYNCing",
                                      common::ErrorValue::E_ERROR);
    }
    payload -= nread;
  }

  /* Now we can use hiredis to read the incoming protocol.
   */
  while (!IsInterrupted()) {
    err = CliReadReply(cmd.get());
    if (err && err->isError()) {
      return err;
    }
  }

  return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
}

/* This function implements --rdb, so it uses the
 * replication protocol in order
 * to fetch the RDB file from a remote server. */
common::Error DBConnection::GetRDB(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  unsigned long long payload = 0;
  common::Error er = SendSync(&payload);
  if (er && er->isError()) {
    return er;
  }

  common::ArrayValue* val = NULL;
  FastoObjectCommandIPtr cmd = CreateCommand<Command>(out, RDM_REQUEST, common::Value::C_INNER);
  if (!cmd) {
    return common::make_error_value("Invalid CreateCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  int fd = INVALID_DESCRIPTOR;
  /* Write to file. */
  if (connection_.config_.rdb_filename == "-") {
    val = new common::ArrayValue;
    FastoObjectIPtr child = make_fasto_object<FastoObject>(cmd.get(), val, Delimiter());
    cmd->AddChildren(child);
  } else {
    fd = open(connection_.config_.rdb_filename.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd == INVALID_DESCRIPTOR) {
      std::string bufeEr = common::MemSPrintf("Error opening '%s': %s",
                                              connection_.config_.rdb_filename, strerror(errno));
      return common::make_error_value(bufeEr, common::ErrorValue::E_ERROR);
    }
  }

  char buf[4096];

  while (payload) {
    ssize_t nread = 0, nwritten = 0;

    int res = redisReadToBuffer(connection_.handle_, buf,
                                (payload > sizeof(buf)) ? sizeof(buf) : payload, &nread);
    if (res == REDIS_ERR) {
      return common::make_error_value("Error reading RDB payload while SYNCing",
                                      common::ErrorValue::E_ERROR);
    }

    if (!nread) {
      continue;
    }

    if (fd != INVALID_DESCRIPTOR) {
      nwritten = write(fd, buf, nread);
    } else {
      val->appendString(buf);
    }

    if (nwritten != nread) {
      std::string bufeEr = common::MemSPrintf("Error writing data to file: %s", strerror(errno));
      if (fd != INVALID_DESCRIPTOR) {
        close(fd);
      }

      return common::make_error_value(bufeEr, common::ErrorValue::E_ERROR);
    }

    payload -= nread;
  }

  if (fd != INVALID_DESCRIPTOR) {
    close(fd);
  }

  LOG_MSG("Transfer finished with success.", common::logging::L_INFO, true);
  return common::Error();
}

common::Error DBConnection::SendScan(unsigned long long* it, redisReply** out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "SCAN %llu", *it));

  /* Handle any error conditions */
  if (!reply) {
    return common::make_error_value("I/O error", common::Value::E_ERROR);
  } else if (reply->type == REDIS_REPLY_ERROR) {
    std::string buff = common::MemSPrintf("SCAN error: %s", reply->str);
    return common::make_error_value(buff, common::Value::E_ERROR);
  } else if (reply->type != REDIS_REPLY_ARRAY) {
    return common::make_error_value("Non ARRAY response from SCAN!", common::Value::E_ERROR);
  } else if (reply->elements != 2) {
    return common::make_error_value("Invalid element count from SCAN!", common::Value::E_ERROR);
  }

  /* Validate our types are correct */
  DCHECK(reply->element[0]->type == REDIS_REPLY_STRING);
  DCHECK(reply->element[1]->type == REDIS_REPLY_ARRAY);

  /* Update iterator */
  *it = common::ConvertFromString<unsigned long long>(reply->element[0]->str);
  *out = reply;
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

  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, DBSIZE));

  if (!reply || reply->type != REDIS_REPLY_INTEGER) {
    return common::make_error_value("Couldn't determine DBSIZE!", common::Value::E_ERROR);
  }

  /* Grab the number of keys and free our reply */
  *size = reply->integer;
  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::GetKeyTypes(redisReply* keys, int* types) {
  if (!types) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  /* Pipeline TYPE commands */
  for (size_t i = 0; i < keys->elements; i++) {
    redisAppendCommand(connection_.handle_, "TYPE %s", keys->element[i]->str);
  }

  redisReply* reply = NULL;
  /* Retrieve types */
  for (size_t i = 0; i < keys->elements; i++) {
    if (redisGetReply(connection_.handle_, reinterpret_cast<void**>(&reply)) != REDIS_OK) {
      std::string buff =
          common::MemSPrintf("Error getting type for key '%s' (%d: %s)", keys->element[i]->str,
                             connection_.handle_->err, connection_.handle_->errstr);
      return common::make_error_value(buff, common::Value::E_ERROR);
    } else if (reply->type != REDIS_REPLY_STATUS) {
      std::string buff = common::MemSPrintf("Invalid reply type (%d) for TYPE on key '%s'!",
                                            reply->type, keys->element[i]->str);
      return common::make_error_value(buff, common::Value::E_ERROR);
    }

    int res = 0;
    common::Error err = toIntType(keys->element[i]->str, reply->str, &res);
    freeReplyObject(reply);
    if (err && err->isError()) {
      return err;
    }

    types[i] = res;
  }

  return common::Error();
}

common::Error DBConnection::GetKeySizes(redisReply* keys, int* types, unsigned long long* sizes) {
  const char* sizecmds[] = {"STRLEN", "LLEN", "SCARD", "HLEN", "ZCARD"};

  /* Pipeline size commands */
  for (size_t i = 0; i < keys->elements; i++) {
    /* Skip keys that were deleted */
    if (types[i] == RTYPE_NONE) {
      continue;
    }

    redisAppendCommand(connection_.handle_, "%s %s", sizecmds[types[i]], keys->element[i]->str);
  }

  /* Retreive sizes */
  for (size_t i = 0; i < keys->elements; i++) {
    /* Skip keys that dissapeared between SCAN and TYPE */
    if (types[i] == RTYPE_NONE) {
      sizes[i] = 0;
      continue;
    }

    redisReply* reply;
    /* Retreive size */
    if (redisGetReply(connection_.handle_, reinterpret_cast<void**>(&reply)) != REDIS_OK) {
      std::string buff =
          common::MemSPrintf("Error getting size for key '%s' (%d: %s)", keys->element[i]->str,
                             connection_.handle_->err, connection_.handle_->errstr);
      return common::make_error_value(buff, common::Value::E_ERROR);
    } else if (reply->type != REDIS_REPLY_INTEGER) {
      /* Theoretically the key could have been removed and
      * added as a different type between TYPE and SIZE */
      std::string buff = common::MemSPrintf("%s on '%s' failed (may have changed type)",
                                            sizecmds[types[i]], keys->element[i]->str);
      LOG_MSG(buff, common::logging::L_WARNING, true);
      sizes[i] = 0;
    } else {
      sizes[i] = reply->integer;
    }

    freeReplyObject(reply);
  }

  return common::Error();
}

common::Error DBConnection::FindBigKeys(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  FastoObjectCommandIPtr cmd =
      CreateCommand<Command>(out, FIND_BIG_KEYS_REQUEST, common::Value::C_INNER);
  if (!cmd) {
    return common::make_error_value("Invalid CreateCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  unsigned long long biggest[5] = {0}, counts[5] = {0}, totalsize[5] = {0};
  unsigned long long sampled = 0, totlen = 0, *sizes = NULL, it = 0;
  size_t total_keys = 0;
  sds maxkeys[5] = {0};
  const char* typeName[] = {"string", "list", "set", "hash", "zset"};
  const char* typeunit[] = {"bytes", "items", "members", "fields", "members"};
  redisReply *reply, *keys;
  unsigned int arrsize = 0;
  int type, *types = NULL;

  /* Total keys pre scanning */
  common::Error er = DBkcount(&total_keys);
  if (er && er->isError()) {
    return er;
  }

  /* Status message */
  LOG_MSG(
      "# Scanning the entire keyspace to find biggest keys "
      "as well as",
      common::logging::L_INFO, true);
  LOG_MSG(
      "# average sizes per key type.  You can use -i 0.1 "
      "to sleep 0.1 sec",
      common::logging::L_INFO, true);
  LOG_MSG("# per 100 SCAN commands (not usually needed).", common::logging::L_INFO, true);

  /* New up sds strings to keep track of overall biggest per
   * type */
  for (int i = 0; i < RTYPE_NONE; i++) {
    maxkeys[i] = sdsempty();
    if (!maxkeys[i]) {
      return common::make_error_value(
          "Failed to allocate memory for largest key "
          "names!",
          common::Value::E_ERROR);
    }
  }

  /* SCAN loop */
  do {
    /* Calculate approximate percentage completion */
    double pct = 100 * static_cast<double>(sampled / total_keys);

    /* Grab some keys and point to the keys array */
    er = SendScan(&it, &reply);
    if (er && er->isError()) {
      return er;
    }

    keys = reply->element[1];

    /* Reallocate our type and size array if we need to */
    if (keys->elements > arrsize) {
      int* ltypes = reinterpret_cast<int*>(realloc(types, sizeof(int) * keys->elements));
      if (!ltypes) {
        free(types);
        return common::make_error_value("Failed to allocate storage for keys!",
                                        common::Value::E_ERROR);
      }
      types = ltypes;
      unsigned long long* lsizes = reinterpret_cast<unsigned long long*>(
          realloc(sizes, sizeof(unsigned long long) * keys->elements));
      if (!lsizes) {
        free(sizes);
        return common::make_error_value("Failed to allocate storage for keys!",
                                        common::Value::E_ERROR);
      }
      sizes = lsizes;
      arrsize = keys->elements;
    }

    /* Retreive types and then sizes */
    er = GetKeyTypes(keys, types);
    if (er && er->isError()) {
      return er;
    }

    er = GetKeySizes(keys, types, sizes);
    if (er && er->isError()) {
      return er;
    }

    /* Now update our stats */
    for (size_t i = 0; i < keys->elements; i++) {
      if ((type = types[i]) == RTYPE_NONE) {
        continue;
      }

      totalsize[type] += sizes[i];
      counts[type]++;
      totlen += keys->element[i]->len;
      sampled++;

      if (biggest[type] < sizes[i]) {
        std::string buff = common::MemSPrintf(
            "[%05.2f%%] Biggest %-6s found so far '%s' "
            "with %llu %s",
            pct, typeName[type], keys->element[i]->str, sizes[i], typeunit[type]);
        LOG_MSG(buff, common::logging::L_INFO, true);

        /* Keep track of biggest key name for this type */
        maxkeys[type] = sdscpy(maxkeys[type], keys->element[i]->str);
        if (!maxkeys[type]) {
          return common::make_error_value("Failed to allocate memory for key!",
                                          common::Value::E_ERROR);
        }

        /* Keep track of the biggest size for this type */
        biggest[type] = sizes[i];
      }
    }

    /* Sleep if we've been directed to do so */
    if (sampled && (sampled % 100) == 0 && connection_.config_.interval) {
      common::utils::usleep(connection_.config_.interval);
    }

    freeReplyObject(reply);
  } while (it != 0);

  common::utils::freeifnotnull(types);
  common::utils::freeifnotnull(sizes);

  /* We're done */
  std::string buff = common::MemSPrintf("Sampled %llu keys in the keyspace!", sampled);
  LOG_MSG(buff, common::logging::L_INFO, true);

  buff = common::MemSPrintf("Total key length in bytes is %llu (avg len %.2f)", totlen,
                            totlen ? static_cast<double>(totlen / sampled) : 0);
  LOG_MSG(buff, common::logging::L_INFO, true);

  /* Output the biggest keys we found, for types we did find
   */
  for (int i = 0; i < RTYPE_NONE; i++) {
    if (sdslen(maxkeys[i]) > 0) {
      memset(&buff, 0, sizeof(buff));
      buff = common::MemSPrintf("Biggest %6s found '%s' has %llu %s", typeName[i], maxkeys[i],
                                biggest[i], typeunit[i]);
      common::StringValue* val = common::Value::createStringValue(buff);
      FastoObjectIPtr obj = make_fasto_object<FastoObject>(cmd.get(), val, Delimiter());
      cmd->AddChildren(obj);
    }
  }

  for (int i = 0; i < RTYPE_NONE; i++) {
    memset(&buff, 0, sizeof(buff));
    buff = common::MemSPrintf(
        "%llu %ss with %llu %s (%05.2f%% of keys, avg size "
        "%.2f)",
        counts[i], typeName[i], totalsize[i], typeunit[i],
        sampled ? 100 * static_cast<double>(counts[i] / sampled) : 0,
        counts[i] ? static_cast<double>(totalsize[i] / counts[i]) : 0);
    common::StringValue* val = common::Value::createStringValue(buff);
    FastoObjectIPtr obj = make_fasto_object<FastoObject>(cmd.get(), val, Delimiter());
    cmd->AddChildren(obj);
  }

  /* Free sds strings containing max keys */
  for (int i = 0; i < RTYPE_NONE; i++) {
    sdsfree(maxkeys[i]);
  }

  /* Success! */
  return common::Error();
}

common::Error DBConnection::StatMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  FastoObjectCommandIPtr cmd = CreateCommand<Command>(out, INFO_REQUEST, common::Value::C_INNER);
  if (!cmd) {
    return common::make_error_value("Invalid CreateCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  if (connection_.config_.interval == 0) {
    connection_.config_.interval = 1000000;
  }

  std::string command = cmd->InputCommand();
  long requests = 0;

  while (!IsInterrupted()) {
    redisReply* reply = NULL;
    while (!reply) {
      reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, command.c_str()));
      if (connection_.handle_->err &&
          !(connection_.handle_->err & (REDIS_ERR_IO | REDIS_ERR_EOF))) {
        std::string buff = common::MemSPrintf("ERROR: %s", connection_.handle_->errstr);
        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
      }
    }

    if (reply->type == REDIS_REPLY_ERROR) {
      std::string buff = common::MemSPrintf("ERROR: %s", reply->str);
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    char buf[64] = {0};
    /* Keys */
    int64_t aux = 0;
    for (int j = 0; j < 20; j++) {
      common::SNPrintf(buf, sizeof(buf), "db%d:keys", j);
      long k = getLongInfoField(reply->str, buf);
      if (k == LONG_MIN) {
        continue;
      }
      aux += k;
    }

    std::string result;

    common::SNPrintf(buf, sizeof(buf), "keys %ld", aux);
    result += buf;

    /* Used memory */
    aux = getLongInfoField(reply->str, "used_memory");
    bytesToHuman(buf, sizeof(buf), aux);
    result += " used_memory: ";
    result += buf;

    /* Clients */
    aux = getLongInfoField(reply->str, "connected_clients");
    common::SNPrintf(buf, sizeof(buf), " connected_clients: %ld", aux);
    result += buf;

    /* Blocked (BLPOPPING) Clients */
    aux = getLongInfoField(reply->str, "blocked_clients");
    common::SNPrintf(buf, sizeof(buf), " blocked_clients: %ld", aux);
    result += buf;

    /* Requets */
    aux = getLongInfoField(reply->str, "total_commands_processed");
    common::SNPrintf(buf, sizeof(buf), " total_commands_processed: %ld (+%ld)", aux,
                     requests == 0 ? 0 : aux - requests);
    result += buf;
    requests = aux;

    /* Connections */
    aux = getLongInfoField(reply->str, "total_connections_received");
    common::SNPrintf(buf, sizeof(buf), " total_connections_received: %ld", aux);
    result += buf;

    /* Children */
    aux = getLongInfoField(reply->str, "bgsave_in_progress");
    aux |= getLongInfoField(reply->str, "aof_rewrite_in_progress") << 1;
    switch (aux) {
      case 0:
        break;
      case 1:
        result += " SAVE";
        break;
      case 2:
        result += " AOF";
        break;
      case 3:
        result += " SAVE+AOF";
        break;
    }

    common::StringValue* val = common::Value::createStringValue(result);
    FastoObjectIPtr obj = make_fasto_object<FastoObject>(cmd.get(), val, Delimiter());
    cmd->AddChildren(obj);

    freeReplyObject(reply);

    common::utils::usleep(connection_.config_.interval);
  }

  return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
}

common::Error DBConnection::ScanMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  FastoObjectCommandIPtr cmd =
      CreateCommand<Command>(out, SCAN_MODE_REQUEST, common::Value::C_INNER);
  if (!cmd) {
    return common::make_error_value("Invalid CreateCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  redisReply* reply = NULL;
  unsigned long long cur = 0;
  const char* pattern = common::utils::c_strornull(connection_.config_.pattern);

  do {
    if (pattern) {
      reply = reinterpret_cast<redisReply*>(
          redisCommand(connection_.handle_, "SCAN %llu MATCH %s", cur, pattern));
    } else {
      reply = reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "SCAN %llu", cur));
    }
    if (reply == NULL) {
      return common::make_error_value("I/O error", common::ErrorValue::E_ERROR);
    } else if (reply->type == REDIS_REPLY_ERROR) {
      std::string buff = common::MemSPrintf("ERROR: %s", reply->str);
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    } else {
      cur = strtoull(reply->element[0]->str, NULL, 10);
      for (size_t j = 0; j < reply->element[1]->elements; j++) {
        common::StringValue* val =
            common::Value::createStringValue(reply->element[1]->element[j]->str);
        FastoObjectIPtr obj = make_fasto_object<FastoObject>(cmd.get(), val, Delimiter());
        cmd->AddChildren(obj);
      }
    }
    freeReplyObject(reply);
  } while (cur != 0);

  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "FLUSHDB"));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  freeReplyObject(reply);
  return common::Error();
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  int num = common::ConvertFromString<int>(name);
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "SELECT %d", num));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
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
    if (err && err->isError()) {
      return err;
    }
    redisReply* reply =
        reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, del_cmd.c_str()));
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
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(connection_.handle_, "SET %s %s", key_str.c_str(), value_str.c_str()));
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
  std::string key_str = key.Key();
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, "GET %s", key_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  common::Value* val = nullptr;
  if (reply->type == REDIS_REPLY_STRING) {
    val = common::Value::createStringValue(reply->str);
  } else if (reply->type == REDIS_REPLY_NIL) {
    val = common::Value::createNullValue();
  } else if (reply->type == REDIS_REPLY_ERROR) {
    common::Error err =
        common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
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
  if (err && err->isError()) {
    return err;
  }
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, rename_cmd.c_str()));
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
  std::string key_str = key.Key();
  translator_t tran = Translator();
  std::string ttl_cmd;
  common::Error err = tran->ChangeKeyTTLCommand(key, ttl, &ttl_cmd);
  if (err && err->isError()) {
    return err;
  }
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, ttl_cmd.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ERROR) {
    std::string str(reply->str, reply->len);
    freeReplyObject(reply);
    return common::make_error_value(str, common::ErrorValue::E_ERROR);
  }

  if (reply->integer == 0) {
    return common::make_error_value(
        common::MemSPrintf("%s does not exist or the timeout could not be set.", key_str.c_str()),
        common::ErrorValue::E_ERROR);
  }

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
      common::Value* val = common::Value::createNullValue();
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_ERROR: {
      std::string str(r->str, r->len);
      common::ErrorValue* val = common::Value::createErrorValue(str, common::ErrorValue::E_NONE,
                                                                common::logging::L_WARNING);
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING: {
      common::StringValue* val = common::Value::createStringValue(std::string(r->str, r->len));
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      common::FundamentalValue* val = common::Value::createIntegerValue(r->integer);
      ar->Append(val);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::createArrayValue();
      FastoObjectArray* child = new FastoObjectArray(ar, arv, Delimiter());
      ar->AddChildren(child);

      for (size_t i = 0; i < r->elements; ++i) {
        common::Error er = CliFormatReplyRaw(child, r->element[i]);
        if (er && er->isError()) {
          return er;
        }
      }
      break;
    }
    default: {
      common::ErrorValue* val =
          common::Value::createErrorValue(common::MemSPrintf("Unknown reply type: %d", r->type),
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
      common::Value* val = common::Value::createNullValue();
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
      common::StringValue* val = common::Value::createStringValue(str);
      obj = new FastoObject(out, val, Delimiter());
      out->AddChildren(obj);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      common::FundamentalValue* val = common::Value::createIntegerValue(r->integer);
      obj = new FastoObject(out, val, Delimiter());
      out->AddChildren(obj);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::createArrayValue();
      FastoObjectArray* child = new FastoObjectArray(out, arv, Delimiter());
      out->AddChildren(child);

      for (size_t i = 0; i < r->elements; ++i) {
        common::Error er = CliFormatReplyRaw(child, r->element[i]);
        if (er && er->isError()) {
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
  if (connection_.config_.cluster_mode && reply->type == REDIS_REPLY_ERROR &&
      (!strncmp(reply->str, "MOVED", 5) || !strcmp(reply->str, "ASK"))) {
    char *p = reply->str, *s;
    int slot;

    s = strchr(p, ' ');     /* MOVED[S]3999 127.0.0.1:6381 */
    p = strchr(s + 1, ' '); /* MOVED[S]3999[P]127.0.0.1:6381 */
    *p = '\0';
    slot = common::ConvertFromString<int>(s + 1);
    s = strchr(p + 1, ':'); /* MOVED 3999[P]127.0.0.1[S]6381 */
    *s = '\0';
    connection_.config_.host =
        common::net::HostAndPort(p + 1, common::ConvertFromString<uint16_t>(s + 1));
    std::string host_str = common::ConvertToString(connection_.config_.host);
    std::string redir =
        common::MemSPrintf("-> Redirected to slot [%d] located at %s", slot, host_str);
    common::StringValue* val = common::Value::createStringValue(redir);
    FastoObject* child = new FastoObject(out, val, Delimiter());
    out->AddChildren(child);
    connection_.config_.cluster_reissue_command = 1;

    freeReplyObject(reply);
    return common::Error();
  }

  common::Error er = CliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  return er;
}

common::Error DBConnection::ExecuteAsPipeline(const std::vector<FastoObjectCommandIPtr>& cmds) {
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
    common::CommandValue* cmdc = cmd->Cmd();

    std::string command = cmdc->inputCommand();
    const char* ccommand = common::utils::c_strornull(command);
    if (!ccommand) {
      continue;
    }

    LOG_COMMAND(cmd);
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
    if (er && er->isError()) {
      return er;
    }
  }
  // end piplene

  return common::Error();
}

common::Error DBConnection::CommonExec(int argc, const char** argv, FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  size_t* argvlen = reinterpret_cast<size_t*>(malloc(argc * sizeof(size_t)));
  for (int j = 0; j < argc; j++) {
    char* carg = const_cast<char*>(argv[j]);
    size_t len = sdslen(carg);
    argvlen[j] = len;
  }

  redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
  free(argvlen);
  common::Error err = CliReadReply(out);
  if (err && err->isError()) {
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
  if (err && err->isError()) {
    isAuth_ = false;
    return err;
  }

  connection_.config_.auth = password;
  isAuth_ = true;
  return common::Error();
}

common::Error DBConnection::Help(int argc, const char** argv, FastoObject* out) {
  return cliOutputHelp(argc, argv, out, Delimiter());
}

common::Error DBConnection::Monitor(int argc, const char** argv, FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  size_t* argvlen = reinterpret_cast<size_t*>(malloc(argc * sizeof(size_t)));
  for (int j = 0; j < argc; j++) {
    char* carg = const_cast<char*>(argv[j]);
    size_t len = sdslen(carg);
    argvlen[j] = len;
  }

  redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
  free(argvlen);
  common::Error err = CliReadReply(out);
  if (err && err->isError()) {
    return err;
  }

  while (true) {
    common::Error er = CliReadReply(out);
    if (er && er->isError()) {
      return er;
    }

    if (IsInterrupted()) {
      return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
    }
  }

  return common::Error();
}

common::Error DBConnection::Subscribe(int argc, const char** argv, FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  size_t* argvlen = reinterpret_cast<size_t*>(malloc(argc * sizeof(size_t)));
  for (int j = 0; j < argc; j++) {
    char* carg = const_cast<char*>(argv[j]);
    size_t len = sdslen(carg);
    argvlen[j] = len;
  }

  redisAppendCommandArgv(connection_.handle_, argc, const_cast<const char**>(argv), argvlen);
  free(argvlen);
  common::Error err = CliReadReply(out);
  if (err && err->isError()) {
    return err;
  }

  while (true) {
    common::Error er = CliReadReply(out);
    if (er && er->isError()) {
      return er;
    }

    if (IsInterrupted()) {
      return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
    }
  }

  return common::Error();
}

common::Error DBConnection::Lrange(const NKey& key, int start, int stop, NDbKValue* loaded_key) {
  if (!IsConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.Key();
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(connection_.handle_, "LRANGE %s %d %d", key_str.c_str(), start, stop));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err && err->isError()) {
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
    common::Error err =
        common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
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

  std::string key_str = key.Key();
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(connection_.handle_, "SMEMBERS %s", key_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err && err->isError()) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    common::ArrayValue* arr = nullptr;
    if (!val->getAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error_value("Conversion error array to set", common::Value::E_ERROR);
    }

    common::SetValue* set = common::Value::createSetValue();
    for (size_t i = 0; i < arr->size(); ++i) {
      common::Value* lval = nullptr;
      if (arr->get(i, &lval)) {
        set->insert(lval->deepCopy());
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
    common::Error err =
        common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

common::Error DBConnection::Zrange(const NKey& key,
                                   int start,
                                   int stop,
                                   bool withscores,
                                   NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string key_str = key.Key();
  std::string line;
  if (withscores) {
    line = common::MemSPrintf("ZRANGE %s %d %d WITHSCORES", key_str.c_str(), start, stop);
  } else {
    line = common::MemSPrintf("ZRANGE %s %d %d", key_str.c_str(), start, stop);
  }
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(connection_.handle_, line.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err && err->isError()) {
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
    if (!val->getAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error_value("Conversion error array to zset", common::Value::E_ERROR);
    }

    common::ZSetValue* zset = common::Value::createZSetValue();
    for (size_t i = 0; i < arr->size(); i += 2) {
      common::Value* lmember = nullptr;
      common::Value* lscore = nullptr;
      if (arr->get(i, &lmember) && arr->get(i + 1, &lscore)) {
        zset->insert(lscore->deepCopy(), lmember->deepCopy());
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
    common::Error err =
        common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
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

  std::string key_str = key.Key();
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(connection_.handle_, "HGETALL %s", key_str.c_str()));
  if (!reply) {
    return cliPrintContextError(connection_.handle_);
  }

  if (reply->type == REDIS_REPLY_ARRAY) {
    common::Value* val = nullptr;
    common::Error err = valueFromReplay(reply, &val);
    if (err && err->isError()) {
      delete val;
      freeReplyObject(reply);
      return err;
    }

    common::ArrayValue* arr = nullptr;
    if (!val->getAsList(&arr)) {
      delete val;
      freeReplyObject(reply);
      return common::make_error_value("Conversion error array to hash", common::Value::E_ERROR);
    }

    common::HashValue* hash = common::Value::createHashValue();
    for (size_t i = 0; i < arr->size(); i += 2) {
      common::Value* lkey = nullptr;
      common::Value* lvalue = nullptr;
      if (arr->get(i, &lkey) && arr->get(i + 1, &lvalue)) {
        hash->insert(lkey->deepCopy(), lvalue->deepCopy());
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
    common::Error err =
        common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
    freeReplyObject(reply);
    return err;
  }

  NOTREACHED();
  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
