/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/memcached/memcached_driver.h"

#include <libmemcached/memcached.h>
#include <libmemcached/util.h>

#include <string>

#include "common/utils.h"
#include "common/sprintf.h"
#include "fasto/qt/logger.h"

#include "core/memcached/memcached_config.h"
#include "core/memcached/memcached_infos.h"

#include "core/command_logger.h"

#define INFO_REQUEST "STATS"
#define GET_KEYS "STATS ITEMS"
#define GET_SERVER_TYPE ""

#define DELETE_KEY_PATTERN_1ARGS_S "DELETE %s"
#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define SET_KEY_PATTERN_2ARGS_SS "SET %s 0 0 %s"

namespace fastonosql {

common::Error testConnection(MemcachedConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  memcachedConfig inf = settings->info();
  const char* user = common::utils::c_strornull(inf.user);
  const char* passwd = common::utils::c_strornull(inf.password);
  const char* host = common::utils::c_strornull(inf.host.host);
  uint16_t hostport = inf.host.port;

  memcached_return rc;
  char buff[1024] = {0};

  if (user && passwd) {
    libmemcached_util_ping2(host, hostport, user, passwd, &rc);
    if (rc != MEMCACHED_SUCCESS) {
      common::SNPrintf(buff, sizeof(buff), "Couldn't ping server: %s",
                       memcached_strerror(NULL, rc));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  } else {
    libmemcached_util_ping(host, hostport, &rc);
    if (rc != MEMCACHED_SUCCESS) {
      common::SNPrintf(buff, sizeof(buff), "Couldn't ping server: %s",
                       memcached_strerror(NULL, rc));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  }

  return common::Error();
}

struct MemcachedDriver::pimpl {
  pimpl()
    : memc_(NULL) {
  }

  bool isConnected() const {
    if (!memc_) {
      return false;
    }

    memcached_server_st *server = (memcached_server_st*)memc_->servers;
    if (!server) {
      return false;
    }

    return server->state == MEMCACHED_SERVER_STATE_CONNECTED;
  }

  common::Error connect() {
    if (isConnected()) {
      return common::Error();
    }

    clear();
    init();

    if (!memc_) {
      return common::make_error_value("Init error", common::ErrorValue::E_ERROR);
    }

    memcached_return rc;
    char buff[1024] = {0};

    if (!config_.user.empty() && !config_.password.empty()) {
      const char* user = config_.user.c_str();
      const char* passwd = config_.password.c_str();
      rc = memcached_set_sasl_auth_data(memc_, user, passwd);
      if (rc != MEMCACHED_SUCCESS) {
        common::SNPrintf(buff, sizeof(buff), "Couldn't setup SASL auth: %s",
                         memcached_strerror(memc_, rc));
        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
      }
    }

    /*rc = memcached_behavior_set(memc_, MEMCACHED_BEHAVIOR_CONNECT_TIMEOUT, 10000);
    if (rc != MEMCACHED_SUCCESS) {
        sprintf(buff, "Couldn't set the connect timeout: %s", memcached_strerror(memc_, rc));
        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }*/

    const char* host = common::utils::c_strornull(config_.host.host);
    uint16_t hostport = config_.host.port;

    rc = memcached_server_add(memc_, host, hostport);

    if (rc != MEMCACHED_SUCCESS) {
      common::SNPrintf(buff, sizeof(buff), "Couldn't add server: %s",
                       memcached_strerror(memc_, rc));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    memcached_return_t error = memcached_version(memc_);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Connect to server error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error disconnect() {
    if (!isConnected()) {
      return common::Error();
    }

    clear();
    return common::Error();
  }

  common::Error keys(const char* args) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Not supported command: STATS %s", args);
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  common::Error stats(const char* args, MemcachedServerInfo::Common& statsout) {
    memcached_return_t error;
    memcached_stat_st* st = memcached_stat(memc_, (char*)args, &error);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Stats function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    statsout.pid = st->pid;
    statsout.uptime = st->uptime;
    statsout.time = st->time;
    statsout.version = st->version;
    statsout.pointer_size = st->pointer_size;
    statsout.rusage_user = st->rusage_user_seconds;
    statsout.rusage_system = st->rusage_system_seconds;
    statsout.curr_items = st->curr_items;
    statsout.total_items = st->total_items;
    statsout.bytes = st->bytes;
    statsout.curr_connections = st->curr_connections;
    statsout.total_connections = st->total_connections;
    statsout.connection_structures = st->connection_structures;
    statsout.cmd_get = st->cmd_get;
    statsout.cmd_set = st->cmd_set;
    statsout.get_hits = st->get_hits;
    statsout.get_misses = st->get_misses;
    statsout.evictions = st->evictions;
    statsout.bytes_read = st->bytes_read;
    statsout.bytes_written = st->bytes_written;
    statsout.limit_maxbytes = st->limit_maxbytes;
    statsout.threads = st->threads;

    memcached_stat_free(NULL, st);
    return common::Error();
  }

  ~pimpl() {
    clear();
  }

  memcachedConfig config_;
  SSHInfo sinfo_;

  common::Error execute_impl(int argc, char **argv, FastoObject* out) {
    if (strcasecmp(argv[0], "get") == 0) {
      if (argc != 2) {
        return common::make_error_value("Invalid get input argument", common::ErrorValue::E_ERROR);
      }

      std::string ret;
      common::Error er = get(argv[1], ret);
      if (!er) {
        common::StringValue *val = common::Value::createStringValue(ret);
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "set") == 0) {
      if (argc != 5) {
        return common::make_error_value("Invalid set input argument", common::ErrorValue::E_ERROR);
      }

      common::Error er = set(argv[1], argv[4], atoi(argv[2]), atoi(argv[3]));
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "add") == 0) {
      if (argc != 5) {
        return common::make_error_value("Invalid add input argument", common::ErrorValue::E_ERROR);
      }

      common::Error er = add(argv[1], argv[4], atoi(argv[2]), atoi(argv[3]));
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "replace") == 0) {
      if (argc != 5) {
        return common::make_error_value("Invalid replace input argument",
                                        common::ErrorValue::E_ERROR);
      }

      common::Error er = replace(argv[1], argv[4], atoi(argv[2]), atoi(argv[3]));
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "append") == 0) {
      if (argc != 5) {
        return common::make_error_value("Invalid append input argument",
                                        common::ErrorValue::E_ERROR);
      }

      common::Error er = append(argv[1], argv[4], atoi(argv[2]), atoi(argv[3]));
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "prepend") == 0) {
      if (argc != 4) {
        return common::make_error_value("Invalid prepend input argument",
                                        common::ErrorValue::E_ERROR);
      }

      common::Error er = prepend(argv[1], argv[4], atoi(argv[2]), atoi(argv[3]));
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "incr") == 0) {
      if (argc != 3) {
        return common::make_error_value("Invalid incr input argument", common::ErrorValue::E_ERROR);
      }

      common::Error er = incr(argv[1], common::convertFromString<uint64_t>(argv[2]));
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "decr") == 0) {
      if (argc != 3) {
        return common::make_error_value("Invalid decr input argument", common::ErrorValue::E_ERROR);
      }

      common::Error er = decr(argv[1], common::convertFromString<uint64_t>(argv[2]));
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "delete") == 0) {
      if (!(argc == 2 || argc == 3)) {
        return common::make_error_value("Invalid delete input argument",
                                        common::ErrorValue::E_ERROR);
      }

      common::Error er = del(argv[1], argc == 3 ? atoll(argv[2]) : 0);
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("DELETED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "flush_all") == 0) {
      if (argc > 2) {
        return common::make_error_value("Invalid flush_all input argument",
                                        common::ErrorValue::E_ERROR);
      }

      common::Error er = flush_all(argc == 2 ? common::convertFromString<time_t>(argv[1]) : 0);
      if (!er) {
        common::StringValue *val = common::Value::createStringValue("STORED");
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "stats") == 0) {
      if (argc > 2) {
        return common::make_error_value("Invalid stats input argument",
                                        common::ErrorValue::E_ERROR);
      }

      const char* args = argc == 2 ? argv[1] : NULL;

      if (args && strcasecmp(args, "items") == 0) {
        return keys(args);
      }

      MemcachedServerInfo::Common statsout;
      common::Error er = stats(args, statsout);
      if (!er) {
        MemcachedServerInfo minf(statsout);
        common::StringValue *val = common::Value::createStringValue(minf.toString());
        FastoObject* child = new FastoObject(out, val, config_.delimiter);
        out->addChildren(child);
      }
      return er;
    } else if (strcasecmp(argv[0], "version") == 0) {
      if (argc != 1) {
        return common::make_error_value("Invalid version input argument",
                                        common::ErrorValue::E_ERROR);
      }

      return version_server();
    } else if (strcasecmp(argv[0], "verbosity") == 0) {
      if (argc != 1) {
        return common::make_error_value("Invalid verbosity input argument",
                                        common::ErrorValue::E_ERROR);
      }

      return verbosity();
    } else {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Not supported command: %s", argv[0]);
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  }

private:
  common::Error get(const std::string& key, std::string& ret_val) {
    ret_val.clear();
    uint32_t flags = 0;
    memcached_return error;
    size_t value_length = 0;

    char *value = memcached_get(memc_, key.c_str(), key.length(), &value_length, &flags, &error);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Get function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    if (value != NULL) {
      ret_val.reserve(value_length +1);  // Always provide null
      ret_val.assign(value, value + value_length +1);
      ret_val.resize(value_length);
      free(value);
    }
    return common::Error();
  }

  common::Error set(const std::string& key, const std::string& value,
                    time_t expiration, uint32_t flags) {
    memcached_return_t error = memcached_set(memc_, key.c_str(), key.length(),
                                             value.c_str(), value.length(), expiration, flags);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Set function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error add(const std::string& key, const std::string& value,
                    time_t expiration, uint32_t flags) {
    memcached_return_t error = memcached_add(memc_, key.c_str(), key.length(),
                                             value.c_str(), value.length(), expiration, flags);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Add function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error replace(const std::string& key, const std::string& value,
                        time_t expiration, uint32_t flags) {
    memcached_return_t error = memcached_replace(memc_, key.c_str(), key.length(),
                                                 value.c_str(), value.length(), expiration, flags);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Replace function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error append(const std::string& key, const std::string& value,
                       time_t expiration, uint32_t flags) {
    memcached_return_t error = memcached_append(memc_, key.c_str(), key.length(), value.c_str(),
                                                value.length(), expiration, flags);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Append function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error prepend(const std::string& key, const std::string& value,
                        time_t expiration, uint32_t flags) {
    memcached_return_t error = memcached_prepend(memc_, key.c_str(), key.length(),
                                                 value.c_str(), value.length(), expiration, flags);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Prepend function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error incr(const std::string& key, uint64_t value) {
    memcached_return_t error = memcached_increment(memc_, key.c_str(), key.length(), 0, &value);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Incr function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error decr(const std::string& key, uint64_t value) {
    memcached_return_t error = memcached_decrement(memc_, key.c_str(), key.length(), 0, &value);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Decr function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error del(const std::string& key, time_t expiration) {
    memcached_return_t error = memcached_delete(memc_, key.c_str(), key.length(), expiration);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Delete function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error flush_all(time_t expiration) {
    memcached_return_t error = memcached_flush(memc_, expiration);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Fluss all function error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error version_server() const {
    memcached_return_t error = memcached_version(memc_);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      common::SNPrintf(buff, sizeof(buff), "Get server version error: %s",
                       memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }

    return common::Error();
  }

  common::Error verbosity() const {
    /*memcached_return_t error = memcached_verbosity(memc_, 1);
    if (error != MEMCACHED_SUCCESS) {
      char buff[1024] = {0};
      sprintf(buff, "Verbosity error: %s", memcached_strerror(memc_, error));
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }*/

    return common::make_error_value("Not supported command", common::ErrorValue::E_ERROR);
  }

  void init() {
    DCHECK(!memc_);
    memc_ = memcached(NULL, 0);
    DCHECK(memc_);
  }

  void clear() {
    if (memc_) {
      memcached_free(memc_);
    }
    memc_ = NULL;
  }

  memcached_st* memc_;
};

MemcachedDriver::MemcachedDriver(IConnectionSettingsBaseSPtr settings)
  : IDriver(settings, MEMCACHED), impl_(new pimpl) {
}

MemcachedDriver::~MemcachedDriver() {
  interrupt();
  stop();
  delete impl_;
}

bool MemcachedDriver::isConnected() const {
  return impl_->isConnected();
}

bool MemcachedDriver::isAuthenticated() const {
  return impl_->isConnected();
}

common::net::hostAndPort MemcachedDriver::address() const {
  return impl_->config_.host;
}

std::string MemcachedDriver::outputDelemitr() const {
  return impl_->config_.delimiter;
}

const char* MemcachedDriver::versionApi() {
  return memcached_lib_version();
}

void MemcachedDriver::initImpl() {
}

void MemcachedDriver::clearImpl() {
}

common::Error MemcachedDriver::executeImpl(int argc, char **argv, FastoObject* out) {
  return impl_->execute_impl(argc, argv, out);
}

common::Error MemcachedDriver::serverInfo(ServerInfo **info) {
  LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
  MemcachedServerInfo::Common cm;
  common::Error err = impl_->stats(NULL, cm);
  if (!err) {
    *info = new MemcachedServerInfo(cm);
  }

  return err;
}

common::Error MemcachedDriver::serverDiscoveryInfo(ServerInfo **sinfo,
                                                   ServerDiscoveryInfo** dinfo,
                                                   IDataBaseInfo** dbinfo) {
  ServerInfo *lsinfo = NULL;
  common::Error er = serverInfo(&lsinfo);
  if (er && er->isError()) {
    return er;
  }

  FastoObjectIPtr root = FastoObject::createRoot(GET_SERVER_TYPE);
  FastoObjectCommand* cmd = createCommand<MemcachedCommand>(root, GET_SERVER_TYPE,
                                                            common::Value::C_INNER);
  er = execute(cmd);

  if (!er) {
    FastoObject::child_container_type ch = root->childrens();
    if (ch.size()) {
      // *dinfo = makeOwnRedisDiscoveryInfo(ch[0]);
    }
  }

  IDataBaseInfo* ldbinfo = NULL;
  er = currentDataBaseInfo(&ldbinfo);
  if (er && er->isError()) {
    delete lsinfo;
    return er;
  }

  *sinfo = lsinfo;
  *dbinfo = ldbinfo;
  return er;
}

common::Error MemcachedDriver::currentDataBaseInfo(IDataBaseInfo** info) {
  *info = new MemcachedDataBaseInfo("0", true, 0);
  return common::Error();
}

void MemcachedDriver::handleConnectEvent(events::ConnectRequestEvent *ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::ConnectResponceEvent::value_type res(ev->value());
  MemcachedConnectionSettings *set = dynamic_cast<MemcachedConnectionSettings*>(settings_.get());
  if (set) {
    impl_->config_ = set->info();
    impl_->sinfo_ = set->sshInfo();
  notifyProgress(sender, 25);
    common::Error er = impl_->connect();
    if (er && er->isError()) {
      res.setErrorInfo(er);
    }
  notifyProgress(sender, 75);
  }
  reply(sender, new events::ConnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void MemcachedDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::DisconnectResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);

  common::Error er = impl_->disconnect();
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }

  reply(sender, new events::DisconnectResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void MemcachedDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::ExecuteRequestEvent::value_type res(ev->value());
  const char *inputLine = common::utils::c_strornull(res.text);

  common::Error er;
  if (inputLine) {
    size_t length = strlen(inputLine);
    int offset = 0;
    RootLocker lock = make_locker(sender, inputLine);
    FastoObjectIPtr outRoot = lock.root_;
    double step = 100.0f/length;
    for (size_t n = 0; n < length; ++n) {
      if (interrupt_) {
        er.reset(new common::ErrorValue("Interrupted exec.", common::ErrorValue::E_INTERRUPTED));
        res.setErrorInfo(er);
        break;
      }
      if (inputLine[n] == '\n' || n == length-1) {
        notifyProgress(sender, step * n);
        char command[128] = {0};
        if (n == length-1) {
          strcpy(command, inputLine + offset);
        } else {
          strncpy(command, inputLine + offset, n - offset);
        }
        offset = n + 1;
        FastoObjectCommand* cmd = createCommand<MemcachedCommand>(outRoot, stableCommand(command),
                                                                  common::Value::C_USER);
        er = execute(cmd);
        if (er && er->isError()) {
          res.setErrorInfo(er);
          break;
        }
      }
    }
  } else {
    er.reset(new common::ErrorValue("Empty command line.", common::ErrorValue::E_ERROR));
  }

  if (er) {  // E_INTERRUPTED
    LOG_ERROR(er, true);
  }
  notifyProgress(sender, 100);
}

void MemcachedDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::CommandResponceEvent::value_type res(ev->value());
  std::string cmdtext;
  common::Error er = commandByType(res.cmd, &cmdtext);
  if (er && er->isError()) {
    res.setErrorInfo(er);
    reply(sender, new events::CommandResponceEvent(this, res));
    notifyProgress(sender, 100);
    return;
  }

  RootLocker lock = make_locker(sender, cmdtext);
  FastoObjectIPtr root = lock.root_;
  FastoObjectCommand* cmd = createCommand<MemcachedCommand>(root, cmdtext, common::Value::C_INNER);
  notifyProgress(sender, 50);
  er = execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  }
  reply(sender, new events::CommandResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void MemcachedDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  res.databases.push_back(currentDatabaseInfo());
  reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void MemcachedDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent *ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
  FastoObjectIPtr root = FastoObject::createRoot(GET_KEYS);
  notifyProgress(sender, 50);
  FastoObjectCommand* cmd = createCommand<MemcachedCommand>(root, GET_KEYS, common::Value::C_INNER);
  common::Error er = execute(cmd);
  if (er && er->isError()) {
    res.setErrorInfo(er);
  } else {
    FastoObject::child_container_type rchildrens = cmd->childrens();
    if (rchildrens.size()) {
      DCHECK_EQ(rchildrens.size(), 1);
      FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0]);
      if (!array) {
        goto done;
      }
      common::ArrayValue* ar = array->array();
      if (!ar) {
        goto done;
      }

      for (size_t i = 0; i < ar->size(); ++i) {
        std::string key;
        bool isok = ar->getString(i, &key);
        if (isok) {
          NKey k(key);
          NDbKValue ress(k, NValue());
          res.keys.push_back(ress);
        }
      }
    }
  }
done:
  notifyProgress(sender, 75);
  reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void MemcachedDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::SetDefaultDatabaseResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  reply(sender, new events::SetDefaultDatabaseResponceEvent(this, res));
  notifyProgress(sender, 100);
}

void MemcachedDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) {
  QObject *sender = ev->sender();
  notifyProgress(sender, 0);
  events::ServerInfoResponceEvent::value_type res(ev->value());
  notifyProgress(sender, 50);
  LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
  MemcachedServerInfo::Common cm;
  common::Error err = impl_->stats(NULL, cm);
  if (err) {
    res.setErrorInfo(err);
  } else {
    ServerInfoSPtr mem(new MemcachedServerInfo(cm));
    res.setInfo(mem);
  }
  notifyProgress(sender, 75);
  reply(sender, new events::ServerInfoResponceEvent(this, res));
  notifyProgress(sender, 100);
}

// ============== commands =============//
common::Error MemcachedDriver::commandDeleteImpl(CommandDeleteKey* command,
                                                 std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  common::SNPrintf(patternResult, sizeof(patternResult),
                   DELETE_KEY_PATTERN_1ARGS_S, key.keyString());

  *cmdstring = patternResult;
  return common::Error();
}

common::Error MemcachedDriver::commandLoadImpl(CommandLoadKey* command,
                                               std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_PATTERN_1ARGS_S, key.keyString());

  *cmdstring = patternResult;
  return common::Error();
}

common::Error MemcachedDriver::commandCreateImpl(CommandCreateKey* command,
                                                 std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char patternResult[1024] = {0};
  NDbKValue key = command->key();
  NValue val = command->value();
  common::Value* rval = val.get();
  std::string key_str = key.keyString();
  std::string value_str = common::convertToString(rval, " ");
  common::SNPrintf(patternResult, sizeof(patternResult),
                   SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);

  *cmdstring = patternResult;
  return common::Error();
}

common::Error MemcachedDriver::commandChangeTTLImpl(CommandChangeTTL* command,
                                                    std::string* cmdstring) const {
  if (!command || !cmdstring) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  char errorMsg[1024] = {0};
  common::SNPrintf(errorMsg, sizeof(errorMsg), "Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.", common::convertToString(connectionType()));
  return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
}

// ============== commands =============//

void MemcachedDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) {
}

ServerInfoSPtr MemcachedDriver::makeServerInfoFromString(const std::string& val) {
  ServerInfoSPtr res(makeMemcachedServerInfo(val));
  return res;
}

}  // namespace fastonosql
