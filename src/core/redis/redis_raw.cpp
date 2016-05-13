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

#include "core/redis/redis_raw.h"

#ifdef __MINGW32__
char* strcasestr(const char* s, const char* find) {
  char c, sc;
  if ((c = *find++) != 0) {
    c = tolower((unsigned char)c);
    size_t len = strlen(find);
    do {
      do {
        if ((sc = *s++) == 0)
          return NULL;
      } while ((char)tolower((unsigned char)sc) != c);
    } while (strncasecmp(s, find, len) != 0);
    s--;
  }
  return ((char *)s);
}
#endif

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef OS_POSIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

extern "C" {
  #include "sds.h"
}

#include <hiredis/hiredis.h>

#include <vector>
#include <algorithm>
#include <string>

#include "third-party/redis/src/help.h"

#include "common/time.h"
#include "common/utils.h"
#include "common/string_util.h"
#include "common/sprintf.h"
#include "fasto/qt/logger.h"

#include "core/command_logger.h"
#include "core/command_key.h"

#include "core/redis/redis_sentinel_info.h"
#include "core/redis/redis_cluster_infos.h"
#include "core/redis/database.h"
#include "core/redis/command.h"

#define HIREDIS_VERSION STRINGIZE(HIREDIS_MAJOR) "." STRINGIZE(HIREDIS_MINOR) "." STRINGIZE(HIREDIS_PATCH)
#define REDIS_CLI_KEEPALIVE_INTERVAL 15 /* seconds */
#define CLI_HELP_COMMAND 1
#define CLI_HELP_GROUP 2

#define DBSIZE "DBSIZE"

#define GET_PASSWORD "CONFIG get requirepass"

#define LATENCY_SAMPLE_RATE 10 /* milliseconds. */
#define LATENCY_HISTORY_DEFAULT_INTERVAL 15000 /* milliseconds. */

#define RTYPE_STRING 0
#define RTYPE_LIST   1
#define RTYPE_SET    2
#define RTYPE_HASH   3
#define RTYPE_ZSET   4
#define RTYPE_NONE   5

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

/* Set TCP keep alive option to detect dead peers. The interval option
 * is only used for Linux as we are using Linux-specific APIs to set
 * the probe send time, interval, and count. */
int anetKeepAlive(char *err, int fd, int interval) {
  int val = 1;

  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&val, sizeof(val)) == -1) {
    anetSetError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
    return ANET_ERR;
  }

#ifdef __linux__
  /* Default settings are more or less garbage, with the keepalive time
   * set to 7200 by default on Linux. Modify settings to make the feature
   * actually useful. */

  /* Send first probe after interval. */
  val = interval;
  if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
    anetSetError(err, "setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
    return ANET_ERR;
  }

  /* Send next probes after the specified interval. Note that we set the
   * delay as interval / 3, as we send three probes before detecting
   * an error (see the next setsockopt call). */
  val = interval/3;
  if (val == 0) val = 1;
  if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
    anetSetError(err, "setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
    return ANET_ERR;
  }

  /* Consider the socket in error state after three we send three ACK
   * probes without getting a reply. */
  val = 3;
  if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
    anetSetError(err, "setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
    return ANET_ERR;
  }
#else
  ((void) interval); /* Avoid unused var warning for non Linux systems. */
#endif

  return ANET_OK;
}

typedef struct{
  int type;
  int argc;
  sds *argv;
  sds full;

    /* Only used for help on commands */
  struct commandHelp *org;
} helpEntry;

const int helpEntriesLen = sizeof(commandHelp)/sizeof(struct commandHelp) + sizeof(commandGroups)/sizeof(char*);

const struct RedisInit {
  helpEntry helpEntries[helpEntriesLen];

  RedisInit() {
    int pos = 0;

    for (size_t i = 0; i < sizeof(commandGroups)/sizeof(char*); ++i) {
      helpEntry tmp;

      tmp.argc = 1;
      tmp.argv = (sds*)malloc(sizeof(sds));
      tmp.argv[0] = sdscatprintf(sdsempty(), "@%s", commandGroups[i]);
      tmp.full = tmp.argv[0];
      tmp.type = CLI_HELP_GROUP;
      tmp.org = NULL;
      helpEntries[pos++] = tmp;
    }

    for (size_t i = 0; i < sizeof(commandHelp)/sizeof(struct commandHelp); ++i) {
      helpEntry tmp;

      tmp.argv = sdssplitargs(commandHelp[i].name, &tmp.argc);
      tmp.full = sdsnew(commandHelp[i].name);
      tmp.type = CLI_HELP_COMMAND;
      tmp.org = &commandHelp[i];
      helpEntries[pos++] = tmp;
    }
  }
} rInit;

/* Return the specified INFO field from the INFO command output "info".
 * A new buffer is allocated for the result, that needs to be free'd.
 * If the field is not found NULL is returned. */
char* getInfoField(char* info, const char* field) {
  char* p = strstr(info,field);
  char* n1, *n2;
  char* result;

  if (!p) return NULL;
  p += strlen(field)+1;
  n1 = strchr(p,'\r');
  n2 = strchr(p,',');
  if (n2 && n2 < n1) n1 = n2;
  result = (char*)malloc(sizeof(char)*(n1-p)+1);
  memcpy(result,p,(n1-p));
  result[n1-p] = '\0';
  return result;
}

/* Like the above function but automatically convert the result into
 * a long. On error (missing field) LONG_MIN is returned. */
long getLongInfoField(char *info, const char *field) {
  char* value = getInfoField(info,field);
  long l;

  if (!value) return LONG_MIN;
  l = strtol(value, NULL, 10);
  free(value);
  return l;
}

/* Convert number of bytes into a human readable string of the form:
 * 100B, 2G, 100M, 4K, and so forth. */
void bytesToHuman(char* s, size_t len, long long n) {
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
  } else if (n < (1024*1024)) {
    d = (double)n/(1024);
    common::SNPrintf(s, len, "%.2fK", d);
  } else if (n < (1024LL*1024*1024)) {
    d = (double)n/(1024*1024);
    common::SNPrintf(s, len, "%.2fM", d);
  } else if (n < (1024LL*1024*1024*1024)) {
    d = (double)n/(1024LL*1024*1024);
    common::SNPrintf(s, len, "%.2fG", d);
  }
}

bool isPipeLineCommand(const char* command) {
  if (!command) {
      DNOTREACHED();
      return false;
  }

  bool skip = strcasecmp(command, "quit") == 0
              || strcasecmp(command, "exit") == 0
              || strcasecmp(command, "connect") == 0
              || strcasecmp(command, "help") == 0
              || strcasecmp(command, "?") == 0
              || strcasecmp(command, "shutdown") == 0
              || strcasecmp(command, "monitor") == 0
              || strcasecmp(command, "subscribe") == 0
              || strcasecmp(command, "psubscribe") == 0
              || strcasecmp(command, "sync") == 0
              || strcasecmp(command, "psync") == 0;

  return !skip;
}

}

namespace fastonosql {
namespace core {
namespace redis {
namespace {

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

}

namespace {

common::Error createConnection(const Config& config,
                               const SSHInfo& sinfo, struct redisContext** context) {
    if (!context) {
      return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
    }

    redisContext* lcontext = nullptr;

    DCHECK(*context == nullptr);
    bool is_local = !config.hostsocket.empty();

    if (is_local) {
      const char* hostsocket = common::utils::c_strornull(config.hostsocket);
      lcontext = redisConnectUnix(hostsocket);
    } else {
      const char* host = common::utils::c_strornull(config.host.host);
      uint16_t port = config.host.port;
      const char* username = common::utils::c_strornull(sinfo.user_name);
      const char* password = common::utils::c_strornull(sinfo.password);
      common::net::hostAndPort ssh_host = sinfo.host;
      const char* ssh_address = common::utils::c_strornull(ssh_host.host);
      int ssh_port = ssh_host.port;
      int curM = sinfo.current_method;
      const char* publicKey = common::utils::c_strornull(sinfo.public_key);
      const char* privateKey = common::utils::c_strornull(sinfo.private_key);
      const char* passphrase = common::utils::c_strornull(sinfo.passphrase);

      lcontext = redisConnect(host, port, ssh_address, ssh_port, username, password,
                             publicKey, privateKey, passphrase, curM);
    }

    if (!lcontext) {
      std::string buff;
      if (!is_local) {
        std::string host_str = common::convertToString(config.host);
        buff = common::MemSPrintf("Could not connect to Redis at %s : no context", host_str);
      } else {
        buff = common::MemSPrintf( "Could not connect to Redis at %s : no context", config.hostsocket);
      }
      return common::make_error_value(buff, common::Value::E_ERROR);
    }

    if (lcontext->err) {
      std::string buff;
      if (is_local) {
        buff = common::MemSPrintf("Could not connect to Redis at %s : %s", config.hostsocket,
                                  lcontext->errstr);
      } else {
        std::string host_str = common::convertToString(config.host);
        buff = common::MemSPrintf("Could not connect to Redis at %s : %s", host_str,
                                  lcontext->errstr);
      }
      return common::make_error_value(buff, common::Value::E_ERROR);
    }

    *context = lcontext;
    return common::Error();
}

common::Error createConnection(RedisConnectionSettings* settings, redisContext** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  Config config = settings->info();
  SSHInfo sinfo = settings->sshInfo();
  return createConnection(config, sinfo, context);
}

common::Error cliPrintContextError(redisContext* context) {
  if (!context) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  std::string buff = common::MemSPrintf("Error: %s", context->errstr);
  return common::make_error_value(buff, common::ErrorValue::E_ERROR);
}

common::Error cliOutputCommandHelp(FastoObject* out, struct commandHelp* help, int group,
                                   const std::string& delimiter, const std::string& ns_separator) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  std::string buff = common::MemSPrintf("name: %s %s\n  summary: %s\n  since: %s",
                                        help->name, help->params, help->summary, help->since);
  common::StringValue* val =common::Value::createStringValue(buff);
  FastoObject* child = new FastoObject(out, val, delimiter, ns_separator);
  out->addChildren(child);
  if (group) {
    std::string buff2 = common::MemSPrintf("group: %s", commandGroups[help->group]);
    val = common::Value::createStringValue(buff2);
    FastoObject* gchild = new FastoObject(out, val, delimiter, ns_separator);
    out->addChildren(gchild);
  }

  return common::Error();
}

common::Error authContext(const Config& config, redisContext* context) {
  const char* auth_str = common::utils::c_strornull(config.auth);
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

}

common::Error testConnection(RedisConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* context = nullptr;
  common::Error err = createConnection(settings, &context);
  if (err && err->isError()) {
    return err;
  }

  Config config = settings->info();
  err = authContext(config, context);
  if (err && err->isError()) {
    redisFree(context);
    return err;
  }

  redisFree(context);
  return common::Error();
}

common::Error discoveryClusterConnection(RedisConnectionSettings* settings,
                                  std::vector<ServerDiscoveryClusterInfoSPtr>* infos) {
  if (!settings || !infos) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* context = nullptr;
  common::Error err = createConnection(settings, &context);
  if (err && err->isError()) {
    return err;
  }

  Config config = settings->info();
  err = authContext(config, context);
  if (err && err->isError()) {
    redisFree(context);
    return err;
  }

  /* Send the GET CLUSTER command. */
  redisReply* reply = (redisReply*)redisCommand(context, GET_SERVER_TYPE);
  if (!reply) {
    err = common::make_error_value("I/O error", common::Value::E_ERROR);
    redisFree(context);
    return err;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    err = makeDiscoveryClusterInfo(settings->host(), std::string(reply->str, reply->len), infos);
  } else if (reply->type == REDIS_REPLY_ERROR) {
    err = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
  } else {
    NOTREACHED();
  }

  freeReplyObject(reply);
  redisFree(context);
  return err;
}

common::Error discoverySentinelConnection(RedisConnectionSettings* settings,
                                          std::vector<ServerDiscoverySentinelInfoSPtr>* infos) {
  if (!settings || !infos) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisContext* context = NULL;
  common::Error err = createConnection(settings, &context);
  if (err && err->isError()) {
    return err;
  }

  Config config = settings->info();
  err = authContext(config, context);
  if (err && err->isError()) {
    redisFree(context);
    return err;
  }

  /* Send the GET MASTERS command. */
  redisReply* masters_reply = (redisReply*)redisCommand(context, GET_SENTINEL_MASTERS);
  if (!masters_reply) {
    redisFree(context);
    return common::make_error_value("I/O error", common::Value::E_ERROR);
  }

  for (size_t i = 0; i < masters_reply->elements; ++i) {
    redisReply* master_info = masters_reply->element[i];
    ServerCommonInfo sinf;
    common::Error lerr = makeServerCommonInfo(master_info, &sinf);
    if (lerr && lerr->isError()) {
      continue;
    }

    const char* master_name = sinf.name.c_str();
    ServerDiscoverySentinelInfoSPtr sent(new RedisDiscoverySentinelInfo(sinf));
    infos->push_back(sent);
    /* Send the GET SLAVES command. */
    redisReply* reply = (redisReply*)redisCommand(context, GET_SENTINEL_SLAVES_PATTERN_1ARGS_S, master_name);
    if (!reply) {
      freeReplyObject(masters_reply);
      redisFree(context);
      return common::make_error_value("I/O error", common::Value::E_ERROR);
    }

    if (reply->type == REDIS_REPLY_ARRAY) {
      for (size_t j = 0; j < reply->elements; ++j) {
        redisReply* server_info = reply->element[j];
        ServerCommonInfo slsinf;
        lerr = makeServerCommonInfo(server_info, &slsinf);
        if (lerr && lerr->isError()) {
          continue;
        }
        ServerDiscoverySentinelInfoSPtr lsent(new RedisDiscoverySentinelInfo(slsinf));
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

RedisRaw::RedisRaw(IRedisRawOwner* observer)
  : context_(nullptr), isAuth_(false), observer_(observer) {
}

RedisRaw::~RedisRaw() {
  if (context_) {
    redisFree(context_);
    context_ = nullptr;
  }
}

const char* RedisRaw::versionApi() {
  return HIREDIS_VERSION;
}

bool RedisRaw::isConnected() const {
  return context_;
}

bool RedisRaw::isAuthenticated() const {
  if (!isConnected()) {
    return false;
  }

  return isAuth_;
}

common::Error RedisRaw::connect(bool force) {
  if (context_ == nullptr || force) {
    if (context_) {
      redisFree(context_);
      context_ = nullptr;
    }

    redisContext* context = nullptr;
    common::Error er = createConnection(config_, sinfo_, &context);
    if (er && er->isError()) {
      return er;
    }

    context_ = context;

    /* Set aggressive KEEP_ALIVE socket option in the Redis context socket
     * in order to prevent timeouts caused by the execution of long
     * commands. At the same time this improves the detection of real
     * errors. */
    anetKeepAlive(NULL, context->fd, REDIS_CLI_KEEPALIVE_INTERVAL);

    /*struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    int res = redisSetTimeout(context, timeout);
    if (res == REDIS_ERR) {
        char buff[512] = {0};
        common::SNPrintf(buff, sizeof(buff), "Redis connection set timeout failed error is: %s.", context->errstr);
        LOG_MSG(buff, common::logging::L_WARNING, true);
    }*/

    /* Do AUTH and select the right DB. */
    er = cliAuth();
    if (er && er->isError()) {
      return er;
    }

    er = select(config_.dbnum, nullptr);
    if (er && er->isError()) {
      return er;
    }
  }

  return common::Error();
}

common::Error RedisRaw::disconnect() {
  if (!isConnected()) {
    return common::Error();
  }

  if (context_) {
    redisFree(context_);
    context_ = nullptr;
  }

  return common::Error();
}

bool RedisRaw::isInterrupted() const {
 if (!observer_) {
   return false;
 }

 return observer_->isInterrupted();
}

/*------------------------------------------------------------------------------
 * Latency and latency history modes
 *--------------------------------------------------------------------------- */

common::Error RedisRaw::latencyMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  FastoObjectCommand* cmd = createCommand<Command>(out, "PING", common::Value::C_INNER);
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_value("Invalid createCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  common::time64_t start;
  uint64_t min = 0, max = 0, tot = 0, count = 0;
  uint64_t history_interval =
          config_.interval ? config_.interval/1000 :
                            LATENCY_HISTORY_DEFAULT_INTERVAL;
  double avg;
  common::time64_t history_start = common::time::current_mstime();

  if (!context_) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  FastoObject* child = nullptr;
  std::string command = cmd->inputCommand();

  while (!isInterrupted()) {
    start = common::time::current_mstime();
    redisReply *reply = (redisReply*)redisCommand(context_, command.c_str());
    if (!reply) {
      return common::make_error_value("I/O error", common::Value::E_ERROR);
    }

    common::time64_t curTime = common::time::current_mstime();

    uint64_t latency = curTime - start;
    freeReplyObject(reply);
    count++;
    if (count == 1) {
      min = max = tot = latency;
      avg = (double) latency;
    } else {
      if (latency < min) min = latency;
      if (latency > max) max = latency;
      tot += latency;
      avg = (double) tot/count;
    }

    std::string buff = common::MemSPrintf("min: %lld, max: %lld, avg: %.2f (%lld samples)",
                                          min, max, avg, count);
    common::Value *val = common::Value::createStringValue(buff);

    if (!child) {
      child = new FastoObject(cmd, val, config_.delimiter, config_.ns_separator);
      cmd->addChildren(child);
      continue;
    }

    if (config_.latency_history && curTime - history_start > history_interval) {
      child = new FastoObject(cmd, val, config_.delimiter, config_.ns_separator);
      cmd->addChildren(child);
      history_start = curTime;
      min = max = tot = count = 0;
    } else {
      child->setValue(val);
    }

    common::utils::msleep(LATENCY_SAMPLE_RATE);
  }

  return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
}

/*------------------------------------------------------------------------------
 * Slave mode
 *--------------------------------------------------------------------------- */

/* Sends SYNC and reads the number of bytes in the payload. Used both by
 * slaveMode() and getRDB(). */
common::Error RedisRaw::sendSync(unsigned long long* payload) {
  if (!payload) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }
  /* To start we need to send the SYNC command and return the payload.
   * The hiredis client lib does not understand this part of the protocol
   * and we don't want to mess with its buffers, so everything is performed
   * using direct low-level I/O. */
  char buf[4096], *p;

  /* Send the SYNC command. */
  ssize_t nwrite = 0;
  if (redisWriteFromBuffer(context_, "SYNC\r\n", &nwrite) == REDIS_ERR) {
      return common::make_error_value("Error writing to master", common::ErrorValue::E_ERROR);
  }

  /* Read $<payload>\r\n, making sure to read just up to "\n" */
  p = buf;
  while (1) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(context_, p, 1, &nread);
    if (res == REDIS_ERR) {
      return common::make_error_value("Error reading bulk length while SYNCing",
                                      common::ErrorValue::E_ERROR);
    }

    if (!nread) {
      continue;
    }

    if (*p == '\n' && p != buf) break;
    if (*p != '\n') p++;
  }
  *p = '\0';
  if (buf[0] == '-') {
    std::string buf2 = common::MemSPrintf("SYNC with master failed: %s", buf);
    return common::make_error_value(buf2, common::ErrorValue::E_ERROR);
  }

  *payload = strtoull(buf + 1, NULL, 10);
  return common::Error();
}

common::Error RedisRaw::slaveMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  unsigned long long payload = 0;
  common::Error er = sendSync(&payload);
  if (er && er->isError()) {
    return er;
  }

  FastoObjectCommand* cmd = createCommand<Command>(out, SYNC_REQUEST,
                                                        common::Value::C_INNER);
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_value("Invalid createCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  char buf[1024];
  /* Discard the payload. */
  while (payload) {
    ssize_t nread = 0;
    int res = redisReadToBuffer(context_, buf, (payload > sizeof(buf)) ? sizeof(buf) :
                                                                         payload, &nread);
    if (res == REDIS_ERR) {
      return common::make_error_value("Error reading RDB payload while SYNCing",
                                      common::ErrorValue::E_ERROR);
    }
    payload -= nread;
  }

  /* Now we can use hiredis to read the incoming protocol. */
  while (1) {
    er = cliReadReply(cmd);
    if (er && er->isError()) {
      break;
    }

    if (isInterrupted()) {
      return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
    }
  }

  return er;
}

/*------------------------------------------------------------------------------
 * RDB transfer mode
 *--------------------------------------------------------------------------- */

/* This function implements --rdb, so it uses the replication protocol in order
 * to fetch the RDB file from a remote server. */
common::Error RedisRaw::getRDB(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  unsigned long long payload = 0;
  common::Error er = sendSync(&payload);
  if (er && er->isError()) {
    return er;
  }

  common::ArrayValue* val = NULL;
  FastoObjectCommand* cmd = createCommand<Command>(out, RDM_REQUEST, common::Value::C_INNER);
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_value("Invalid createCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  int fd = INVALID_DESCRIPTOR;
  /* Write to file. */
  if (config_.rdb_filename == "-") {
    val = new common::ArrayValue;
    FastoObject* child = new FastoObject(cmd, val, config_.delimiter, config_.ns_separator);
    cmd->addChildren(child);
  } else {
    fd = open(config_.rdb_filename.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd == INVALID_DESCRIPTOR) {
      std::string bufeEr = common::MemSPrintf("Error opening '%s': %s", config_.rdb_filename,
                                              strerror(errno));
      return common::make_error_value(bufeEr, common::ErrorValue::E_ERROR);
    }
  }

  char buf[4096];

  while (payload) {
    ssize_t nread = 0, nwritten = 0;

    int res = redisReadToBuffer(context_, buf,(payload > sizeof(buf)) ? sizeof(buf) :
                                                                        payload, &nread);
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

/*------------------------------------------------------------------------------
 * Find big keys
 *--------------------------------------------------------------------------- */

common::Error RedisRaw::sendScan(unsigned long long* it, redisReply** out){
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisReply* reply = (redisReply*)redisCommand(context_, "SCAN %llu", *it);

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
  *it = atoi(reply->element[0]->str);
  *out = reply;
  return common::Error();
}

common::Error RedisRaw::dbsize(size_t* size) {
  if (!size) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  redisReply* reply = (redisReply*)redisCommand(context_, DBSIZE);

  if (!reply || reply->type != REDIS_REPLY_INTEGER) {
    return common::make_error_value("Couldn't determine DBSIZE!", common::Value::E_ERROR);
  }

  /* Grab the number of keys and free our reply */
  *size = reply->integer;
  freeReplyObject(reply);
  return common::Error();
}

common::Error RedisRaw::getKeyTypes(redisReply* keys, int* types) {
  if (!types) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  /* Pipeline TYPE commands */
  for (size_t i = 0; i < keys->elements; i++) {
    redisAppendCommand(context_, "TYPE %s", keys->element[i]->str);
  }

  redisReply* reply = NULL;
  /* Retrieve types */
  for (size_t i = 0; i < keys->elements; i++) {
    if (redisGetReply(context_, (void**)&reply)!=REDIS_OK) {
      std::string buff = common::MemSPrintf("Error getting type for key '%s' (%d: %s)",
                                            keys->element[i]->str, context_->err, context_->errstr);
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

common::Error RedisRaw::getKeySizes(redisReply* keys, int* types, unsigned long long* sizes) {
  redisReply* reply;
  const char* sizecmds[] = {"STRLEN", "LLEN", "SCARD", "HLEN", "ZCARD"};

  /* Pipeline size commands */
  for (size_t i = 0; i < keys->elements; i++) {
    /* Skip keys that were deleted */
    if (types[i] == RTYPE_NONE)
      continue;

    redisAppendCommand(context_, "%s %s", sizecmds[types[i]], keys->element[i]->str);
  }

  /* Retreive sizes */
  for (size_t i = 0; i < keys->elements; i++) {
    /* Skip keys that dissapeared between SCAN and TYPE */
    if (types[i] == RTYPE_NONE) {
      sizes[i] = 0;
      continue;
    }

    /* Retreive size */
    if (redisGetReply(context_, (void**)&reply)!=REDIS_OK) {
      std::string buff = common::MemSPrintf("Error getting size for key '%s' (%d: %s)",
                                            keys->element[i]->str, context_->err, context_->errstr);
      return common::make_error_value(buff, common::Value::E_ERROR);
    } else if (reply->type != REDIS_REPLY_INTEGER) {
      /* Theoretically the key could have been removed and
      * added as a different type between TYPE and SIZE */
      std::string buff = common::MemSPrintf("Warning:  %s on '%s' failed (may have changed type)",
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

common::Error RedisRaw::findBigKeys(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  FastoObjectCommand* cmd = createCommand<Command>(out, FIND_BIG_KEYS_REQUEST,
                                                        common::Value::C_INNER);
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_value("Invalid createCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  unsigned long long biggest[5] = {0}, counts[5] = {0}, totalsize[5] = {0};
  unsigned long long sampled = 0, totlen=0, *sizes = nullptr, it = 0;
  size_t total_keys = 0;
  sds maxkeys[5] = {0};
  const char* typeName[] = {"string","list","set","hash","zset"};
  const char* typeunit[] = {"bytes","items","members","fields","members"};
  redisReply* reply, *keys;
  unsigned int arrsize = 0, i;
  int type, *types = nullptr;

  /* Total keys pre scanning */
  common::Error er = dbsize(&total_keys);
  if (er && er->isError()) {
    return er;
  }

  /* Status message */
  LOG_MSG("# Scanning the entire keyspace to find biggest keys as well as",
          common::logging::L_INFO, true);
  LOG_MSG("# average sizes per key type.  You can use -i 0.1 to sleep 0.1 sec",
          common::logging::L_INFO, true);
  LOG_MSG("# per 100 SCAN commands (not usually needed).", common::logging::L_INFO, true);

  /* New up sds strings to keep track of overall biggest per type */
  for (i=0;i<RTYPE_NONE; i++) {
    maxkeys[i] = sdsempty();
    if (!maxkeys[i]) {
      return common::make_error_value("Failed to allocate memory for largest key names!",
                                      common::Value::E_ERROR);
    }
  }

  /* SCAN loop */
  do {
    /* Calculate approximate percentage completion */
    double pct = 100 * (double)sampled/total_keys;

    /* Grab some keys and point to the keys array */
    er = sendScan(&it, &reply);
    if (er && er->isError()) {
      return er;
    }

    keys = reply->element[1];

    /* Reallocate our type and size array if we need to */
    if (keys->elements > arrsize) {
      int* ltypes = (int*)realloc(types, sizeof(int)*keys->elements);
      if (!ltypes) {
        free(types);
        return common::make_error_value("Failed to allocate storage for keys!",
                                        common::Value::E_ERROR);
      }
      types = ltypes;
      unsigned long long* lsizes = (unsigned long long*)realloc(sizes, sizeof(unsigned long long)*keys->elements);
      if (!lsizes) {
        free(sizes);
        return common::make_error_value("Failed to allocate storage for keys!",
                                        common::Value::E_ERROR);
      }
      sizes = lsizes;
      arrsize = keys->elements;
    }

    /* Retreive types and then sizes */
    er = getKeyTypes(keys, types);
    if (er && er->isError()) {
      return er;
    }

    er = getKeySizes(keys, types, sizes);
    if (er && er->isError()) {
      return er;
    }

    /* Now update our stats */
    for (i = 0; i < keys->elements; i++) {
      if ((type = types[i]) == RTYPE_NONE)
        continue;

      totalsize[type] += sizes[i];
      counts[type]++;
      totlen += keys->element[i]->len;
      sampled++;

      if (biggest[type]<sizes[i]) {
        std::string buff = common::MemSPrintf("[%05.2f%%] Biggest %-6s found so far '%s' with %llu %s",
                                              pct, typeName[type], keys->element[i]->str,
                                              sizes[i], typeunit[type]);
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
    if (sampled && (sampled %100) == 0 && config_.interval) {
      common::utils::usleep(config_.interval);
    }

    freeReplyObject(reply);
  } while (it != 0);

  common::utils::freeifnotnull(types);
  common::utils::freeifnotnull(sizes);

  /* We're done */
  std::string buff = common::MemSPrintf("Sampled %llu keys in the keyspace!", sampled);
  LOG_MSG(buff, common::logging::L_INFO, true);

  buff = common::MemSPrintf("Total key length in bytes is %llu (avg len %.2f)", totlen,
                            totlen ? (double)totlen/sampled : 0);
  LOG_MSG(buff, common::logging::L_INFO, true);

  /* Output the biggest keys we found, for types we did find */
  for (i = 0; i < RTYPE_NONE; i++) {
    if (sdslen(maxkeys[i])>0) {
      memset(&buff, 0, sizeof(buff));
      buff = common::MemSPrintf("Biggest %6s found '%s' has %llu %s", typeName[i], maxkeys[i],
                                biggest[i], typeunit[i]);
      common::StringValue *val = common::Value::createStringValue(buff);
      FastoObject* obj = new FastoObject(cmd, val, config_.delimiter, config_.ns_separator);
      cmd->addChildren(obj);
    }
  }

  for (i = 0; i < RTYPE_NONE; i++) {
    memset(&buff, 0, sizeof(buff));
    buff = common::MemSPrintf("%llu %ss with %llu %s (%05.2f%% of keys, avg size %.2f)",
                              counts[i], typeName[i], totalsize[i], typeunit[i],
                              sampled ? 100 * (double)counts[i]/sampled : 0,
                              counts[i] ? (double)totalsize[i]/counts[i] : 0);
    common::StringValue *val = common::Value::createStringValue(buff);
    FastoObject* obj = new FastoObject(cmd, val, config_.delimiter, config_.ns_separator);
    cmd->addChildren(obj);
  }

  /* Free sds strings containing max keys */
  for (i = 0; i < RTYPE_NONE; i++) {
    sdsfree(maxkeys[i]);
  }

  /* Success! */
  return common::Error();
}

/*------------------------------------------------------------------------------
 * Stats mode
 *--------------------------------------------------------------------------- */

common::Error RedisRaw::statMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  FastoObjectCommand* cmd = createCommand<Command>(out, INFO_REQUEST,
                                                        common::Value::C_INNER);
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_value("Invalid createCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  std::string command = cmd->inputCommand();
  long requests = 0;

  while (!isInterrupted()) {
    redisReply *reply = NULL;
    while (!reply) {
      reply = (redisReply*)redisCommand(context_, command.c_str());
      if (context_->err && !(context_->err & (REDIS_ERR_IO | REDIS_ERR_EOF))) {
        std::string buff = common::MemSPrintf("ERROR: %s", context_->errstr);
        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
      }
    }

    if (reply->type == REDIS_REPLY_ERROR) {
      std::string buff = common::MemSPrintf("ERROR: %s", reply->str);
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    char buf[64];
    int j;
    /* Keys */
    long aux = 0;
    for (j = 0; j < 20; j++) {
      common::SNPrintf(buf, sizeof(buf), "db%d:keys", j);
      long k = getLongInfoField(reply->str, buf);
      if (k == LONG_MIN) continue;
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
    common::SNPrintf(buf, sizeof(buf), " blocked_clients: %ld",aux);
    result += buf;

    /* Requets */
    aux = getLongInfoField(reply->str, "total_commands_processed");
    common::SNPrintf(buf, sizeof(buf), " total_commands_processed: %ld (+%ld)", aux,
                     requests == 0 ? 0 : aux-requests);
    result += buf;
    requests = aux;

    /* Connections */
    aux = getLongInfoField(reply->str, "total_connections_received");
    common::SNPrintf(buf, sizeof(buf), " total_connections_received: %ld",aux);
    result += buf;

    /* Children */
    aux = getLongInfoField(reply->str, "bgsave_in_progress");
    aux |= getLongInfoField(reply->str, "aof_rewrite_in_progress") << 1;
    switch(aux) {
    case 0: break;
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

    common::StringValue *val = common::Value::createStringValue(result);
    FastoObject* obj = new FastoObject(cmd, val, config_.delimiter, config_.ns_separator);
    cmd->addChildren(obj);

    freeReplyObject(reply);

    common::utils::usleep(config_.interval);
  }

  return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
}

/*------------------------------------------------------------------------------
 * Scan mode
 *--------------------------------------------------------------------------- */

common::Error RedisRaw::scanMode(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  FastoObjectCommand* cmd = createCommand<Command>(out, SCAN_MODE_REQUEST,
                                                        common::Value::C_INNER);
  if (!cmd) {
    DNOTREACHED();
    return common::make_error_value("Invalid createCommand input argument",
                                    common::ErrorValue::E_ERROR);
  }

  redisReply* reply = NULL;
  unsigned long long cur = 0;
  const char* pattern = common::utils::c_strornull(config_.pattern);

  do {
    if (pattern)
      reply = (redisReply*)redisCommand(context_, "SCAN %llu MATCH %s", cur, pattern);
    else
      reply = (redisReply*)redisCommand(context_, "SCAN %llu", cur);
    if (reply == NULL) {
      return common::make_error_value("I/O error", common::ErrorValue::E_ERROR);
    } else if (reply->type == REDIS_REPLY_ERROR) {
      std::string buff = common::MemSPrintf("ERROR: %s", reply->str);
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    } else {
      unsigned int j;

      cur = strtoull(reply->element[0]->str,NULL,10);
      for (j = 0; j < reply->element[1]->elements; j++) {
        common::StringValue* val = common::Value::createStringValue(reply->element[1]->element[j]->str);
        FastoObject* obj = new FastoObject(cmd, val, config_.delimiter, config_.ns_separator);
        cmd->addChildren(obj);
      }
    }
    freeReplyObject(reply);
  } while (cur != 0);

  return common::Error();
}

common::Error RedisRaw::cliAuth() {
  common::Error err = authContext(config_, context_);
  if (err && err->isError()) {
    isAuth_ = false;
    return common::Error();
  }

  isAuth_ = true;
  return common::Error();
}

common::Error RedisRaw::select(int num, IDataBaseInfo** info) {
  redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(context_, "SELECT %d", num));
  if (reply) {
    size_t sz = 0;
    dbsize(&sz);
    DataBaseInfo* linfo = new DataBaseInfo(common::convertToString(num), true, sz);
    if (observer_) {
      observer_->currentDataBaseChanged(linfo);
    }

    if (info) {
      *info = linfo;
    }
    freeReplyObject(reply);
    return common::Error();
  }

  return cliPrintContextError(context_);
}

common::Error RedisRaw::cliFormatReplyRaw(FastoObjectArray* ar, redisReply* r) {
  if (!ar) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  switch (r->type) {
    case REDIS_REPLY_NIL: {
      common::Value* val = common::Value::createNullValue();
      ar->append(val);
      break;
    }
    case REDIS_REPLY_ERROR: {
      std::string str(r->str, r->len);
      common::ErrorValue* val = common::Value::createErrorValue(str,
                                                                common::ErrorValue::E_NONE,
                                                                common::logging::L_WARNING);
      ar->append(val);
      break;
    }
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING: {
      common::StringValue* val = common::Value::createStringValue(std::string(r->str, r->len));
      ar->append(val);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      common::FundamentalValue* val = common::Value::createIntegerValue(r->integer);
      ar->append(val);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::createArrayValue();
      FastoObjectArray* child = new FastoObjectArray(ar, arv, config_.delimiter, config_.ns_separator);
      ar->addChildren(child);

      for (size_t i = 0; i < r->elements; ++i) {
        common::Error er = cliFormatReplyRaw(child, r->element[i]);
        if (er && er->isError()) {
          return er;
        }
      }
      break;
    }
    default: {
      char tmp2[128] = {0};
      common::SNPrintf(tmp2, sizeof(tmp2), "Unknown reply type: %d", r->type);
      common::ErrorValue* val = common::Value::createErrorValue(tmp2, common::ErrorValue::E_NONE,
                                                                common::logging::L_WARNING);
      ar->append(val);
    }
  }

  return common::Error();
}

common::Error RedisRaw::cliFormatReplyRaw(FastoObject* out, redisReply* r) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  FastoObject* obj = nullptr;
  switch (r->type) {
    case REDIS_REPLY_NIL: {
      common::Value* val = common::Value::createNullValue();
      obj = new FastoObject(out, val, config_.delimiter, config_.ns_separator);
      out->addChildren(obj);
      break;
    }
    case REDIS_REPLY_ERROR: {
      if (strcasestr(r->str, "NOAUTH")) { //"NOAUTH Authentication required."
        isAuth_ = false;
      }
      std::string str(r->str, r->len);
      return common::make_error_value(str, common::ErrorValue::E_ERROR);
    }
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING: {
      std::string str(r->str, r->len);
      common::StringValue* val = common::Value::createStringValue(str);
      obj = new FastoObject(out, val, config_.delimiter, config_.ns_separator);
      out->addChildren(obj);
      break;
    }
    case REDIS_REPLY_INTEGER: {
      common::FundamentalValue* val = common::Value::createIntegerValue(r->integer);
      obj = new FastoObject(out, val, config_.delimiter, config_.ns_separator);
      out->addChildren(obj);
      break;
    }
    case REDIS_REPLY_ARRAY: {
      common::ArrayValue* arv = common::Value::createArrayValue();
      FastoObjectArray* child = new FastoObjectArray(out, arv, config_.delimiter,
                                                     config_.ns_separator);
      out->addChildren(child);

      for (size_t i = 0; i < r->elements; ++i) {
        common::Error er = cliFormatReplyRaw(child, r->element[i]);
        if (er && er->isError()) {
          return er;
        }
      }
      break;
    }
    default: {
      char tmp2[128] = {0};
      common::SNPrintf(tmp2, sizeof(tmp2), "Unknown reply type: %d", r->type);
      return common::make_error_value(tmp2, common::ErrorValue::E_ERROR);
    }
  }

  return common::Error();
}

common::Error RedisRaw::cliOutputGenericHelp(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  common::StringValue* val = common::Value::createStringValue(PROJECT_NAME_TITLE " based on hiredis " HIREDIS_VERSION "\r\n"
                                                              "Type: \"help @<group>\" to get a list of commands in <group>\r\n"
                                                              "      \"help <command>\" for help on <command>\r\n"
                                                              "      \"help <tab>\" to get a list of possible help topics\r\n"
                                                              "      \"quit\" to exit");
  FastoObject* child = new FastoObject(out, val, config_.delimiter, config_.ns_separator);
  out->addChildren(child);

  return common::Error();
}

common::Error RedisRaw::cliOutputHelp(FastoObject* out, int argc, char** argv) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  int i, j, len;
  int group = -1;
  const helpEntry* entry;
  struct commandHelp* help;

  if (argc == 0) {
    return cliOutputGenericHelp(out);
  } else if (argc > 0 && argv[0][0] == '@') {
    len = sizeof(commandGroups)/sizeof(char*);
    for (i = 0; i < len; i++) {
      if (strcasecmp(argv[0]+1,commandGroups[i]) == 0) {
        group = i;
        break;
      }
    }
  }

  DCHECK(argc > 0);
  for (i = 0; i < helpEntriesLen; i++) {
    entry = &rInit.helpEntries[i];
    if (entry->type != CLI_HELP_COMMAND) continue;

    help = entry->org;
    if (group == -1) {
      /* Compare all arguments */
      if (argc == entry->argc) {
          for (j = 0; j < argc; j++) {
            if (strcasecmp(argv[j],entry->argv[j]) != 0) break;
          }
          if (j == argc) {
            common::Error er = cliOutputCommandHelp(out, help, 1, config_.delimiter,
                                                    config_.ns_separator);
            if (er && er->isError()) {
              return er;
            }
          }
      }
    } else {
      if (group == help->group) {
        common::Error er = cliOutputCommandHelp(out, help, 0, config_.delimiter,
                                                config_.ns_separator);
        if (er && er->isError()) {
            return er;
        }
      }
    }
  }

  return common::Error();
}

common::Error RedisRaw::cliReadReply(FastoObject* out) {
  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  void* _reply = NULL;
  if (redisGetReply(context_, &_reply) != REDIS_OK) {
    /* Filter cases where we should reconnect */
    if (context_->err == REDIS_ERR_IO && errno == ECONNRESET) {
      return common::make_error_value("Needed reconnect.", common::ErrorValue::E_ERROR);
    }
    if (context_->err == REDIS_ERR_EOF) {
      return common::make_error_value("Needed reconnect.", common::ErrorValue::E_ERROR);
    }

    return cliPrintContextError(context_); /* avoid compiler warning */
  }

  redisReply* reply = static_cast<redisReply*>(_reply);
  config_.last_cmd_type = reply->type;

  if (config_.cluster_mode && reply->type == REDIS_REPLY_ERROR &&
    (!strncmp(reply->str, "MOVED", 5) || !strcmp(reply->str, "ASK"))) {
    char* p = reply->str, *s;
    int slot;

    s = strchr(p,' ');      /* MOVED[S]3999 127.0.0.1:6381 */
    p = strchr(s+1,' ');    /* MOVED[S]3999[P]127.0.0.1:6381 */
    *p = '\0';
    slot = atoi(s+1);
    s = strchr(p+1,':');    /* MOVED 3999[P]127.0.0.1[S]6381 */
    *s = '\0';
    config_.host = common::net::hostAndPort(p + 1, atoi(s + 1));
    std::string host_str = common::convertToString(config_.host);
    std::string redir = common::MemSPrintf("-> Redirected to slot [%d] located at %s",
                                           slot, host_str);
    common::StringValue* val = common::Value::createStringValue(redir);
    FastoObject* child = new FastoObject(out, val, config_.delimiter, config_.ns_separator);
    out->addChildren(child);
    config_.cluster_reissue_command = 1;

    freeReplyObject(reply);
    return common::Error();
  }

  common::Error er = cliFormatReplyRaw(out, reply);
  freeReplyObject(reply);
  return er;
}

common::Error RedisRaw::execute(int argc, char** argv, FastoObject* out) {
  CHECK(context_);

  if (!out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  char* command = argv[0];

  if (argc == 3 && strcasecmp(command, "connect") == 0) {
    config_.host.host = argv[1];
    config_.host.port = atoi(argv[2]);
    sdsfreesplitres(argv, argc);
    return connect(true);
  }

  if (strcasecmp(command, "help") == 0 || strcasecmp(command, "?") == 0) {
    return cliOutputHelp(out, --argc, ++argv);
  }

  if (strcasecmp(command, "monitor") == 0) config_.monitor_mode = 1;
  if (strcasecmp(command, "subscribe") == 0 || strcasecmp(command, "psubscribe") == 0) config_.pubsub_mode = 1;
  if (strcasecmp(command, "sync") == 0 || strcasecmp(command, "psync") == 0) config_.slave_mode = 1;

  size_t* argvlen = (size_t*)malloc(argc * sizeof(size_t));
  for (int j = 0; j < argc; j++) {
    size_t len =  sdslen(argv[j]);
    argvlen[j] = len;
  }

  redisAppendCommandArgv(context_, argc, (const char**)argv, argvlen);
  free(argvlen);
  while (config_.monitor_mode) {
    common::Error er = cliReadReply(out);
    if (er && er->isError()) {
      config_.monitor_mode = 0;
      return er;
    }

    if (isInterrupted()) {
      config_.monitor_mode = 0;
      return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
    }
  }

  if (config_.pubsub_mode) {
    while (1) {
      common::Error er = cliReadReply(out);
      if (er && er->isError()) {
        config_.pubsub_mode = 0;
        return er;
      }

      if (isInterrupted()) {
        config_.pubsub_mode = 0;
        return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
      }
    }
  }

  if (config_.slave_mode) {
    common::Error er = slaveMode(out);
    config_.slave_mode = 0;
    return er;  /* Error = slaveMode lost connection to master */
  }

  common::Error er = cliReadReply(out);
  if (er && er->isError()) {
    return er;
  } else {
    /* Store database number when SELECT was successfully executed. */
    if (strcasecmp(command, "select") == 0 && argc == 2) {
      config_.dbnum = atoi(argv[1]);
      size_t sz = 0;
      dbsize(&sz);
      DataBaseInfo* info = new DataBaseInfo(common::convertToString(config_.dbnum), true, sz);
      if (observer_) {
        observer_->currentDataBaseChanged(info);
      }
    } else if (strcasecmp(command, "auth") == 0) {
      auto rchildrens = out->childrens();
      if (rchildrens.size() == 1) {
        FastoObject* obj = rchildrens[0];
        isAuth_ = obj && obj->toString() == "OK";
      } else {
        isAuth_ = false;
      }
    }
  }

  if (config_.interval) {
    common::utils::usleep(config_.interval);
  }

  return common::Error();
}

common::Error RedisRaw::executeAsPipeline(std::vector<FastoObjectCommandIPtr> cmds) {
  if (cmds.empty()) {
    return common::make_error_value("Invalid input command", common::ErrorValue::E_ERROR);
  }

  if (!context_) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  //start piplene mode
  std::vector<FastoObjectCommandIPtr> valid_cmds;
  for (size_t i = 0; i < cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    common::CommandValue* cmdc = cmd->cmd();

    std::string command = cmdc->inputCommand();
    const char* ccommand = common::utils::c_strornull(command);
    if (!ccommand) {
      continue;
    }

    LOG_COMMAND(REDIS, fastonosql::Command(cmdc));
    int argc = 0;
    sds* argv = sdssplitargs(ccommand, &argc);

    if (argv) {
      if (isPipeLineCommand(argv[0])) {
        valid_cmds.push_back(cmd);
        redisAppendCommandArgv(context_, argc, (const char**)argv, NULL);
      }
      sdsfreesplitres(argv, argc);
    }
  }

  for (size_t i = 0; i < valid_cmds.size(); ++i) {
    FastoObjectCommandIPtr cmd = cmds[i];
    common::Error er = cliReadReply(cmd.get());
    if (er && er->isError()) {
      return er;
    }
  }
  //end piplene

  return common::Error();
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
