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

#include "core/memcached/memcached_raw.h"

#include <libmemcached/util.h>

#include <string>

#include "common/utils.h"
#include "common/sprintf.h"

namespace fastonosql {
namespace memcached {
namespace {
common::Error createConnection(const MemcachedConfig& config, struct memcached_st** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  memcached_st* memc = ::memcached(NULL, 0);
  if (!memc) {
    return common::make_error_value("Init error", common::ErrorValue::E_ERROR);
  }

  memcached_return rc;
  char buff[1024] = {0};

  if (!config.user.empty() && !config.password.empty()) {
    const char* user = common::utils::c_strornull(config.user);
    const char* passwd = common::utils::c_strornull(config.password);
    rc = memcached_set_sasl_auth_data(memc, user, passwd);
    if (rc != MEMCACHED_SUCCESS) {
      common::SNPrintf(buff, sizeof(buff), "Couldn't setup SASL auth: %s",
                       memcached_strerror(memc, rc));
      memcached_free(memc);
      return common::make_error_value(buff, common::ErrorValue::E_ERROR);
    }
  }

  const char* host = common::utils::c_strornull(config.host.host);
  uint16_t hostport = config.host.port;

  rc = memcached_server_add(memc, host, hostport);

  if (rc != MEMCACHED_SUCCESS) {
    common::SNPrintf(buff, sizeof(buff), "Couldn't add server: %s",
                     memcached_strerror(memc, rc));
    memcached_free(memc);
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  memcached_return_t error = memcached_version(memc);
  if (error != MEMCACHED_SUCCESS) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Connect to server error: %s",
                     memcached_strerror(memc, error));
    memcached_free(memc);
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  *context = memc;
  return common::Error();
}

common::Error createConnection(MemcachedConnectionSettings* settings, struct memcached_st** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  MemcachedConfig config = settings->info();
  return createConnection(config, context);
}
}  // namespace

common::Error testConnection(MemcachedConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  MemcachedConfig inf = settings->info();
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

MemcachedRaw::MemcachedRaw()
  : CommandHandler(memcachedCommands), memc_(nullptr) {
}

MemcachedRaw::~MemcachedRaw() {
  if (memc_) {
    memcached_free(memc_);
  }
  memc_ = nullptr;
}

const char* MemcachedRaw::versionApi() {
  return memcached_lib_version();
}

bool MemcachedRaw::isConnected() const {
  if (!memc_) {
    return false;
  }

  memcached_server_st *server = (memcached_server_st*)memc_->servers;
  if (!server) {
    return false;
  }

  return server->state == MEMCACHED_SERVER_STATE_CONNECTED;
}

common::Error MemcachedRaw::connect() {
  if (isConnected()) {
    return common::Error();
  }

  struct memcached_st* context = NULL;
  common::Error err = createConnection(config_, &context);
  if (err && err->isError()) {
    return err;
  }

  memc_ = context;
  return common::Error();
}

common::Error MemcachedRaw::disconnect() {
  if (!isConnected()) {
    return common::Error();
  }

  if (memc_) {
    memcached_free(memc_);
  }
  memc_ = nullptr;
  return common::Error();
}

common::Error MemcachedRaw::keys(const char* args) {
  return notSupported("keys");
}

common::Error MemcachedRaw::stats(const char* args, MemcachedServerInfo::Common& statsout) {
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

common::Error MemcachedRaw::dbsize(size_t* size) {
  *size = 0;
  return common::Error();
}

common::Error MemcachedRaw::get(const std::string& key, std::string* ret_val) {
  if (!ret_val) {
    return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
  }

  uint32_t flags = 0;
  memcached_return error;
  size_t value_length = 0;

  char* value = memcached_get(memc_, key.c_str(), key.length(), &value_length, &flags, &error);
  if (error != MEMCACHED_SUCCESS) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Get function error: %s",
                     memcached_strerror(memc_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  CHECK(value);
  *ret_val = std::string(value, value + value_length);
  free(value);
  return common::Error();
}

common::Error MemcachedRaw::set(const std::string& key, const std::string& value,
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

common::Error MemcachedRaw::add(const std::string& key, const std::string& value,
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

common::Error MemcachedRaw::replace(const std::string& key, const std::string& value,
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

common::Error MemcachedRaw::append(const std::string& key, const std::string& value,
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

common::Error MemcachedRaw::prepend(const std::string& key, const std::string& value,
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

common::Error MemcachedRaw::incr(const std::string& key, uint64_t value) {
  memcached_return_t error = memcached_increment(memc_, key.c_str(), key.length(), 0, &value);
  if (error != MEMCACHED_SUCCESS) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Incr function error: %s",
                     memcached_strerror(memc_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::decr(const std::string& key, uint64_t value) {
  memcached_return_t error = memcached_decrement(memc_, key.c_str(), key.length(), 0, &value);
  if (error != MEMCACHED_SUCCESS) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Decr function error: %s",
                     memcached_strerror(memc_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::del(const std::string& key, time_t expiration) {
  memcached_return_t error = memcached_delete(memc_, key.c_str(), key.length(), expiration);
  if (error != MEMCACHED_SUCCESS) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Delete function error: %s",
                     memcached_strerror(memc_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::flush_all(time_t expiration) {
  memcached_return_t error = memcached_flush(memc_, expiration);
  if (error != MEMCACHED_SUCCESS) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Fluss all function error: %s",
                     memcached_strerror(memc_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::version_server() const {
  memcached_return_t error = memcached_version(memc_);
  if (error != MEMCACHED_SUCCESS) {
    char buff[1024] = {0};
    common::SNPrintf(buff, sizeof(buff), "Get server version error: %s",
                     memcached_strerror(memc_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::help(int argc, char** argv) {
  return notSupported("HELP");
}

common::Error keys(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  return mem->keys("items");
}

common::Error stats(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  const char* args = argc == 1 ? argv[0] : nullptr;
  if (args && strcasecmp(args, "items") == 0) {
    return mem->keys(args);
  }

  MemcachedServerInfo::Common statsout;
  common::Error er = mem->stats(args, statsout);
  if (!er) {
    MemcachedServerInfo minf(statsout);
    common::StringValue *val = common::Value::createStringValue(minf.toString());
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  std::string ret;
  common::Error er = mem->get(argv[0], &ret);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->set(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error add(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->add(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error replace(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->replace(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error append(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->append(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error prepend(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->prepend(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error incr(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->incr(argv[0], common::convertFromString<uint64_t>(argv[1]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error decr(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->decr(argv[0], common::convertFromString<uint64_t>(argv[1]));
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->del(argv[0], argc == 2 ? atoll(argv[1]) : 0);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error flush_all(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->flush_all(argc == 1 ? common::convertFromString<time_t>(argv[0]) : 0);
  if (!er) {
    common::StringValue *val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error version_server(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  return mem->version_server();
}

common::Error dbsize(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  size_t dbsize = 0;
  common::Error er = mem->dbsize(&dbsize);
  if (!er) {
    common::FundamentalValue *val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, mem->config_.delimiter);
    out->addChildren(child);
  }

  return er;
}

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  return mem->help(argc - 1, argv + 1);
}

}  // namespace memcached
}  // namespace fastonosql
