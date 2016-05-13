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
#include <libmemcached/instance.hpp>

#include <string>

#include "common/utils.h"
#include "common/sprintf.h"

namespace fastonosql {
namespace core {
template<>
common::Error DBAllocatorTraits<memcached::MemcachedConnection, memcached::Config>::connect(const memcached::Config& config, memcached::MemcachedConnection** hout) {
  memcached::MemcachedConnection* context = nullptr;
  common::Error er = memcached::createConnection(config, &context);
  if (er && er->isError()) {
    return er;
  }

  *hout = context;
  return common::Error();
}
template<>
common::Error DBAllocatorTraits<memcached::MemcachedConnection, memcached::Config>::disconnect(memcached::MemcachedConnection** handle) {
  memcached::MemcachedConnection* lhandle = *handle;
  if (lhandle) {
    memcached_free(lhandle);
  }
  lhandle = nullptr;
  return common::Error();
}
template<>
bool DBAllocatorTraits<memcached::MemcachedConnection, memcached::Config>::isConnected(memcached::MemcachedConnection* handle) {
  if (!handle) {
    return false;
  }

  memcached_instance_st* servers = handle->servers;
  if (!servers) {
    return false;
  }

  return servers->state == MEMCACHED_SERVER_STATE_CONNECTED;
}
namespace memcached {

common::Error createConnection(const Config& config, MemcachedConnection** context) {
  if (!context) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  DCHECK(*context == nullptr);
  memcached_st* memc = ::memcached(NULL, 0);
  if (!memc) {
    return common::make_error_value("Init error", common::ErrorValue::E_ERROR);
  }

  memcached_return rc;
  if (!config.user.empty() && !config.password.empty()) {
    const char* user = common::utils::c_strornull(config.user);
    const char* passwd = common::utils::c_strornull(config.password);
    rc = memcached_set_sasl_auth_data(memc, user, passwd);
    if (rc != MEMCACHED_SUCCESS) {
      memcached_free(memc);
      return common::make_error_value(common::MemSPrintf("Couldn't setup SASL auth: %s",
                                                         memcached_strerror(memc, rc)),
                                      common::ErrorValue::E_ERROR);
    }
  }

  const char* host = common::utils::c_strornull(config.host.host);
  uint16_t hostport = config.host.port;

  rc = memcached_server_add(memc, host, hostport);

  if (rc != MEMCACHED_SUCCESS) {
    memcached_free(memc);
    return common::make_error_value(common::MemSPrintf("Couldn't add server: %s",
                                                       memcached_strerror(memc, rc)),
                                    common::ErrorValue::E_ERROR);
  }

  memcached_return_t error = memcached_version(memc);
  if (error != MEMCACHED_SUCCESS) {
    memcached_free(memc);
    return common::make_error_value(common::MemSPrintf("Connect to server error: %s",
                                                       memcached_strerror(memc, error)),
                                    common::ErrorValue::E_ERROR);
  }

  *context = memc;
  return common::Error();
}

common::Error createConnection(MemcachedConnectionSettings* settings, MemcachedConnection** context) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  Config config = settings->info();
  return createConnection(config, context);
}

common::Error testConnection(MemcachedConnectionSettings* settings) {
  if (!settings) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  Config inf = settings->info();
  const char* user = common::utils::c_strornull(inf.user);
  const char* passwd = common::utils::c_strornull(inf.password);
  const char* host = common::utils::c_strornull(inf.host.host);
  uint16_t hostport = inf.host.port;

  memcached_return rc;
  if (user && passwd) {
    libmemcached_util_ping2(host, hostport, user, passwd, &rc);
    if (rc != MEMCACHED_SUCCESS) {
      return common::make_error_value(common::MemSPrintf("Couldn't ping server: %s",
                                                         memcached_strerror(NULL, rc)),
                                      common::ErrorValue::E_ERROR);
    }
  } else {
    libmemcached_util_ping(host, hostport, &rc);
    if (rc != MEMCACHED_SUCCESS) {
      return common::make_error_value(common::MemSPrintf("Couldn't ping server: %s",
                                                         memcached_strerror(NULL, rc)),
                                      common::ErrorValue::E_ERROR);
    }
  }

  return common::Error();
}

MemcachedRaw::MemcachedRaw()
  : CommandHandler(memcachedCommands), connection_() {
}

common::Error MemcachedRaw::connect(const config_t& config) {
  return connection_.connect(config);
}

common::Error MemcachedRaw::disconnect() {
  return connection_.disconnect();
}

bool MemcachedRaw::isConnected() const {
  return connection_.isConnected();
}

std::string MemcachedRaw::delimiter() const {
  return connection_.config_.delimiter;
}

std::string MemcachedRaw::nsSeparator() const {
  return connection_.config_.ns_separator;
}

MemcachedRaw::config_t MemcachedRaw::config() const {
  return connection_.config_;
}

const char* MemcachedRaw::versionApi() {
  return memcached_lib_version();
}

common::Error MemcachedRaw::keys(const char* args) {
  return notSupported("keys");
}

common::Error MemcachedRaw::info(const char* args, ServerInfo::Common* statsout) {
  CHECK(isConnected());

  if (!statsout) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  memcached_return_t error;
  memcached_stat_st* st = memcached_stat(connection_.handle_, (char*)args, &error);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Stats function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  ServerInfo::Common lstatsout;
  lstatsout.pid = st->pid;
  lstatsout.uptime = st->uptime;
  lstatsout.time = st->time;
  lstatsout.version = st->version;
  lstatsout.pointer_size = st->pointer_size;
  lstatsout.rusage_user = st->rusage_user_seconds;
  lstatsout.rusage_system = st->rusage_system_seconds;
  lstatsout.curr_items = st->curr_items;
  lstatsout.total_items = st->total_items;
  lstatsout.bytes = st->bytes;
  lstatsout.curr_connections = st->curr_connections;
  lstatsout.total_connections = st->total_connections;
  lstatsout.connection_structures = st->connection_structures;
  lstatsout.cmd_get = st->cmd_get;
  lstatsout.cmd_set = st->cmd_set;
  lstatsout.get_hits = st->get_hits;
  lstatsout.get_misses = st->get_misses;
  lstatsout.evictions = st->evictions;
  lstatsout.bytes_read = st->bytes_read;
  lstatsout.bytes_written = st->bytes_written;
  lstatsout.limit_maxbytes = st->limit_maxbytes;
  lstatsout.threads = st->threads;

  *statsout = lstatsout;
  memcached_stat_free(NULL, st);
  return common::Error();
}

common::Error MemcachedRaw::dbsize(size_t* size) {
  *size = 0;
  return common::Error();
}

common::Error MemcachedRaw::get(const std::string& key, std::string* ret_val) {
  CHECK(isConnected());

  if (!ret_val) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  uint32_t flags = 0;
  memcached_return error;
  size_t value_length = 0;

  char* value = memcached_get(connection_.handle_, key.c_str(), key.length(), &value_length, &flags, &error);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Get function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  CHECK(value);
  *ret_val = std::string(value, value + value_length);
  free(value);
  return common::Error();
}

common::Error MemcachedRaw::set(const std::string& key, const std::string& value,
                  time_t expiration, uint32_t flags) {
  CHECK(isConnected());

  memcached_return_t error = memcached_set(connection_.handle_, key.c_str(), key.length(),
                                           value.c_str(), value.length(), expiration, flags);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Set function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::add(const std::string& key, const std::string& value,
                  time_t expiration, uint32_t flags) {
  CHECK(isConnected());

  memcached_return_t error = memcached_add(connection_.handle_, key.c_str(), key.length(),
                                           value.c_str(), value.length(), expiration, flags);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Add function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::replace(const std::string& key, const std::string& value,
                      time_t expiration, uint32_t flags) {
  CHECK(isConnected());

  memcached_return_t error = memcached_replace(connection_.handle_, key.c_str(), key.length(),
                                               value.c_str(), value.length(), expiration, flags);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Replace function error: %s", memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::append(const std::string& key, const std::string& value,
                     time_t expiration, uint32_t flags) {
  CHECK(isConnected());

  memcached_return_t error = memcached_append(connection_.handle_, key.c_str(), key.length(), value.c_str(),
                                              value.length(), expiration, flags);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Append function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::prepend(const std::string& key, const std::string& value,
                      time_t expiration, uint32_t flags) {
  CHECK(isConnected());

  memcached_return_t error = memcached_prepend(connection_.handle_, key.c_str(), key.length(),
                                               value.c_str(), value.length(), expiration, flags);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Prepend function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::incr(const std::string& key, uint64_t value) {
  CHECK(isConnected());

  memcached_return_t error = memcached_increment(connection_.handle_, key.c_str(), key.length(), 0, &value);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Incr function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::decr(const std::string& key, uint64_t value) {
  CHECK(isConnected());

  memcached_return_t error = memcached_decrement(connection_.handle_, key.c_str(), key.length(), 0, &value);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Decr function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::del(const std::string& key, time_t expiration) {
  CHECK(isConnected());

  memcached_return_t error = memcached_delete(connection_.handle_, key.c_str(), key.length(), expiration);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Delete function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::flush_all(time_t expiration) {
  CHECK(isConnected());

  memcached_return_t error = memcached_flush(connection_.handle_, expiration);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Fluss all function error: %s",
                                          memcached_strerror(connection_.handle_, error));
    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
  }

  return common::Error();
}

common::Error MemcachedRaw::version_server() const {
  CHECK(isConnected());

  memcached_return_t error = memcached_version(connection_.handle_);
  if (error != MEMCACHED_SUCCESS) {
    std::string buff = common::MemSPrintf("Get server version error: %s",
                                          memcached_strerror(connection_.handle_, error));
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

  ServerInfo::Common statsout;
  common::Error er = mem->info(args, &statsout);
  if (!er) {
    ServerInfo minf(statsout);
    common::StringValue* val = common::Value::createStringValue(minf.toString());
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error get(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  std::string ret;
  common::Error er = mem->get(argv[0], &ret);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue(ret);
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error set(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->set(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error add(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->add(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error replace(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->replace(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error append(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->append(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error prepend(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->prepend(argv[0], argv[3], atoi(argv[1]), atoi(argv[2]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error incr(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->incr(argv[0], common::convertFromString<uint64_t>(argv[1]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error decr(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->decr(argv[0], common::convertFromString<uint64_t>(argv[1]));
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error del(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->del(argv[0], argc == 2 ? atoll(argv[1]) : 0);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("DELETED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error flush_all(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  common::Error er = mem->flush_all(argc == 1 ? common::convertFromString<time_t>(argv[0]) : 0);
  if (!er) {
    common::StringValue* val = common::Value::createStringValue("STORED");
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
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
    common::FundamentalValue* val = common::Value::createUIntegerValue(dbsize);
    FastoObject* child = new FastoObject(out, val, mem->delimiter(), mem->nsSeparator());
    out->addChildren(child);
  }

  return er;
}

common::Error help(CommandHandler* handler, int argc, char** argv, FastoObject* out) {
  MemcachedRaw* mem = static_cast<MemcachedRaw*>(handler);
  return mem->help(argc - 1, argv + 1);
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
