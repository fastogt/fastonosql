/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include "core/db/memcached/db_connection.h"

#include <string.h>  // for strcasecmp

#include <memory>  // for __shared_ptr
#include <string>  // for string, operator<, etc

#include <string.h>

#include <libmemcached/memcached.h>
#include <libmemcached/util.h>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/net/types.h>       // for HostAndPort
#include <common/sprintf.h>         // for MemSPrintf
#include <common/utils.h>           // for c_strornull
#include <common/value.h>           // for Value::ErrorsType::E_ERROR, etc

#include "core/db/memcached/command_translator.h"
#include "core/db/memcached/config.h"  // for Config
#include "core/db/memcached/database_info.h"
#include "core/db/memcached/internal/commands_api.h"

// hacked
struct hacked_memcached_instance_st {
  struct {
    bool is_allocated;
    bool is_initialized;
    bool is_shutting_down;
    bool is_dead;
    bool ready;
  } options;

  short _events;
  short _revents;

  uint32_t cursor_active_;
  in_port_t port_;
  memcached_socket_t fd;
  uint32_t io_bytes_sent; /* # bytes sent since last read */
  uint32_t request_id;
  uint32_t server_failure_counter;
  uint64_t server_failure_counter_query_id;
  uint32_t server_timeout_counter;
  uint32_t server_timeout_counter_query_id;
  uint32_t weight;
  uint32_t version;
  enum memcached_server_state_t state;
  struct {
    uint32_t read;
    uint32_t write;
    uint32_t timeouts;
    size_t _bytes_read;
  } io_wait_count;
  uint8_t major_version;  // Default definition of UINT8_MAX means that it has not been set.
  uint8_t micro_version;  // ditto, and note that this is the third, not second version bit
  uint8_t minor_version;  // ditto
  memcached_connection_t type;
  char* read_ptr;
  size_t read_buffer_length;
  size_t read_data_length;
  size_t write_buffer_offset;
  struct addrinfo* address_info;
  struct addrinfo* address_info_next;
  time_t next_retry;
  struct memcached_st* root;
  uint64_t limit_maxbytes;
  struct memcached_error_t* error_messages;
  char read_buffer[MEMCACHED_MAX_BUFFER];
  char write_buffer[MEMCACHED_MAX_BUFFER];
  char _hostname[MEMCACHED_NI_MAXHOST];
};

namespace {

struct KeysHolder {
  KeysHolder(const std::string& key_start, const std::string& key_end, uint64_t limit, std::vector<std::string>* r)
      : key_start(key_start), key_end(key_end), limit(limit), r(r) {}

  const std::string key_start;
  const std::string key_end;
  const uint64_t limit;
  std::vector<std::string>* r;

  memcached_return_t addKey(const char* key, size_t key_length, time_t exp) {
    UNUSED(exp);
    if (r->size() < limit) {
      std::string received_key(key, key_length);
      if (key_start < received_key && key_end > received_key) {
        r->push_back(received_key);
        return MEMCACHED_SUCCESS;
      }

      return MEMCACHED_SUCCESS;
    }

    return MEMCACHED_END;
  }
};

memcached_return_t memcached_dump_keys_callback(const memcached_st* ptr,
                                                const char* key,
                                                size_t key_length,
                                                time_t exp,
                                                void* context) {
  UNUSED(ptr);

  KeysHolder* holder = static_cast<KeysHolder*>(context);
  return holder->addKey(key, key_length, exp);
}

struct ScanHolder {
  ScanHolder(uint64_t cursor_in, const std::string& pattern, uint64_t limit)
      : cursor_in(cursor_in), pattern(pattern), limit(limit), r(), cursor_out(0), offset_pos(cursor_in) {}

  const uint64_t cursor_in;
  const std::string pattern;
  const uint64_t limit;
  std::vector<std::string> r;
  uint64_t cursor_out;
  uint64_t offset_pos;

  memcached_return_t addKey(const char* key, size_t key_length, time_t exp) {
    UNUSED(exp);
    if (r.size() < limit) {
      std::string received_key(key, key_length);
      if (common::MatchPattern(received_key, pattern)) {
        if (offset_pos == 0) {
          r.push_back(received_key);
        } else {
          offset_pos--;
        }
      }
      return MEMCACHED_SUCCESS;
    } else {
      cursor_out = cursor_in + limit;
    }

    return MEMCACHED_END;
  }
};

memcached_return_t memcached_dump_scan_callback(const memcached_st* ptr,
                                                const char* key,
                                                size_t key_length,
                                                time_t exp,
                                                void* context) {
  UNUSED(ptr);

  ScanHolder* holder = static_cast<ScanHolder*>(context);
  return holder->addKey(key, key_length, exp);
}

struct TTLHolder {
  TTLHolder(fastonosql::core::key_t key, time_t* exp) : looked_key(key), exp_out(exp) {}
  memcached_return_t CheckKey(const char* key, size_t key_length, time_t exp) {
    fastonosql::core::key_t received_key(std::string(key, key_length));
    if (received_key == looked_key) {
      *exp_out = exp;
      return MEMCACHED_END;
    }

    return MEMCACHED_SUCCESS;
  }

  const fastonosql::core::key_t looked_key;
  time_t* exp_out;
};

memcached_return_t memcached_dump_ttl_callback(const memcached_st* ptr,
                                               const char* key,
                                               size_t key_length,
                                               time_t exp,
                                               void* context) {
  UNUSED(ptr);

  TTLHolder* holder = static_cast<TTLHolder*>(context);
  return holder->CheckKey(key, key_length, exp);
}

}  // namespace

namespace fastonosql {
namespace core {
namespace memcached {
namespace {
const ConstantCommandsArray g_commands = {CommandHolder(DB_HELP_COMMAND,
                                                        "[command]",
                                                        "Return how to use command",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Help),
                                          CommandHolder(DB_INFO_COMMAND,
                                                        "[section]",
                                                        "These command return database information.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Info),
                                          CommandHolder(DB_GET_CONFIG_COMMAND,
                                                        "<parameter>",
                                                        "Get the value of a configuration parameter",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::ConfigGet),
                                          CommandHolder("VERSION",
                                                        "-",
                                                        "Return the Memcached server version.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Version),
                                          CommandHolder("INCR",
                                                        "<key> <value>",
                                                        "Increment value associated with key in "
                                                        "Memcached, item must "
                                                        "exist, increment "
                                                        "command will not create it.\n"
                                                        "The limit of increment is the 64 bit mark.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Incr),
                                          CommandHolder("DECR",
                                                        "<key> <value>",
                                                        "Decrement value associated with key "
                                                        "in Memcached, item must "
                                                        "exist, decrement "
                                                        "command will not create it.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Decr),
                                          CommandHolder("PREPEND",
                                                        "<key> <flags> <exptime> <bytes>",
                                                        "Add value to an existing key before "
                                                        "existing data.\n"
                                                        "Prepend does not take <flags> or "
                                                        "<exptime> parameters but "
                                                        "you must provide them!",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        4,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Prepend),
                                          CommandHolder("APPEND",
                                                        "<key> <flags> <exptime> <value>",
                                                        "Add value to an existing key after "
                                                        "existing data.\n"
                                                        "Append does not take <flags> or "
                                                        "<exptime> parameters but "
                                                        "you must provide them!",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        4,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Append),
                                          CommandHolder("REPLACE",
                                                        "<key> <flags> <exptime> <value>",
                                                        "Store key/value pair in Memcached, "
                                                        "but only if the server "
                                                        "already hold data for this key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        4,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Replace),
                                          CommandHolder("ADD",
                                                        "<key> <flags> <exptime> <value>",
                                                        "Store key/value pair in Memcached, "
                                                        "but only if the server "
                                                        "doesn't already hold "
                                                        "data for this key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        4,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Add),
                                          CommandHolder(DB_SCAN_COMMAND,
                                                        "<cursor> [MATCH pattern] [COUNT count]",
                                                        "Incrementally iterate the keys space",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        4,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Scan),
                                          CommandHolder(DB_KEYS_COMMAND,
                                                        "<key_start> <key_end> <limit>",
                                                        "Find all keys matching the given limits.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        3,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Keys),
                                          CommandHolder(DB_DBKCOUNT_COMMAND,
                                                        "-",
                                                        "Return the number of keys in the "
                                                        "selected database",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::DBkcount),
                                          CommandHolder(DB_FLUSHDB_COMMAND,
                                                        "-",
                                                        "Remove all keys from the current database",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        1,
                                                        CommandInfo::Native,
                                                        &CommandsApi::FlushDB),
                                          CommandHolder(DB_SELECTDB_COMMAND,
                                                        "<name>",
                                                        "Change the selected database for the "
                                                        "current connection",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Select),
                                          CommandHolder(DB_SET_KEY_COMMAND,
                                                        "<key> <value>",
                                                        "Set the value of a key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Set),
                                          CommandHolder(DB_GET_KEY_COMMAND,
                                                        "<key>",
                                                        "Get the value of a key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Get),
                                          CommandHolder(DB_RENAME_KEY_COMMAND,
                                                        "<key> <newkey>",
                                                        "Rename a key",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Rename),
                                          CommandHolder(DB_DELETE_KEY_COMMAND,
                                                        "<key> [key ...]",
                                                        "Delete key.",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        INFINITE_COMMAND_ARGS,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Delete),
                                          CommandHolder(DB_SET_TTL_COMMAND,
                                                        "<key> <exptime>",
                                                        "Set a key's time to live in seconds",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        2,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::SetTTL),
                                          CommandHolder(DB_GET_TTL_COMMAND,
                                                        "<key>",
                                                        "Get the time to live for a key",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        1,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::GetTTL),
                                          CommandHolder(DB_QUIT_COMMAND,
                                                        "-",
                                                        "Close the connection",
                                                        UNDEFINED_SINCE,
                                                        UNDEFINED_EXAMPLE_STR,
                                                        0,
                                                        0,
                                                        CommandInfo::Native,
                                                        &CommandsApi::Quit)};
}
}  // namespace memcached
template <>
const char* ConnectionTraits<MEMCACHED>::GetBasedOn() {
  return "libmemcached";
}

template <>
const char* ConnectionTraits<MEMCACHED>::GetVersionApi() {
  return memcached_lib_version();
}
namespace internal {
template <>
common::Error ConnectionAllocatorTraits<memcached::NativeConnection, memcached::Config>::Connect(
    const memcached::Config& config,
    memcached::NativeConnection** hout) {
  memcached::NativeConnection* context = nullptr;
  common::Error err = memcached::CreateConnection(config, &context);
  if (err) {
    return err;
  }

  *hout = context;
  return common::Error();
}

template <>
common::Error ConnectionAllocatorTraits<memcached::NativeConnection, memcached::Config>::Disconnect(
    memcached::NativeConnection** handle) {
  memcached::NativeConnection* lhandle = *handle;
  if (lhandle) {
    memcached_free(lhandle);
  }
  lhandle = nullptr;
  return common::Error();
}

template <>
bool ConnectionAllocatorTraits<memcached::NativeConnection, memcached::Config>::IsConnected(
    memcached::NativeConnection* handle) {
  if (!handle) {
    return false;
  }

  hacked_memcached_instance_st* servers = reinterpret_cast<hacked_memcached_instance_st*>(handle->servers);
  if (!servers) {
    return false;
  }

  return servers->state == MEMCACHED_SERVER_STATE_CONNECTED;
}

template <>
const ConstantCommandsArray& CDBConnection<memcached::NativeConnection, memcached::Config, MEMCACHED>::GetCommands() {
  return memcached::g_commands;
}

}  // namespace internal
namespace memcached {

common::Error CreateConnection(const Config& config, NativeConnection** context) {
  if (!context) {
    return common::make_error_inval();
  }

  DCHECK(*context == nullptr);
  memcached_st* memc = ::memcached(NULL, 0);
  if (!memc) {
    return common::make_error("Init error");
  }

  memcached_return rc;
  if (!config.user.empty() && !config.password.empty()) {
    const char* user = config.user.c_str();
    const char* passwd = config.password.c_str();
    rc = memcached_set_sasl_auth_data(memc, user, passwd);
    if (rc != MEMCACHED_SUCCESS) {
      memcached_free(memc);
      return common::make_error(common::MemSPrintf("Couldn't setup SASL auth: %s", memcached_strerror(memc, rc)));
    }
  }

  std::string host_str = config.host.GetHost();
  const char* host = host_str.empty() ? NULL : host_str.c_str();
  uint16_t hostport = config.host.GetPort();

  rc = memcached_server_add(memc, host, hostport);

  if (rc != MEMCACHED_SUCCESS) {
    memcached_free(memc);
    return common::make_error(common::MemSPrintf("Couldn't add server: %s", memcached_strerror(memc, rc)));
  }

  memcached_return_t error = memcached_version(memc);
  if (error != MEMCACHED_SUCCESS) {
    memcached_free(memc);
    return common::make_error(common::MemSPrintf("Connect to server error: %s", memcached_strerror(memc, error)));
  }

  *context = memc;
  return common::Error();
}

common::Error TestConnection(const Config& config) {
  std::string host_str = config.host.GetHost();
  const char* host = host_str.empty() ? NULL : host_str.c_str();
  uint16_t hostport = config.host.GetPort();

  memcached_return rc;
  if (!config.user.empty() && !config.password.empty()) {
    const char* user = config.user.c_str();
    const char* passwd = config.password.c_str();
    libmemcached_util_ping2(host, hostport, user, passwd, &rc);
    if (rc != MEMCACHED_SUCCESS) {
      return common::make_error(common::MemSPrintf("Couldn't ping server: %s", memcached_strerror(NULL, rc)));
    }
  } else {
    libmemcached_util_ping(host, hostport, &rc);
    if (rc != MEMCACHED_SUCCESS) {
      return common::make_error(common::MemSPrintf("Couldn't ping server: %s", memcached_strerror(NULL, rc)));
    }
  }

  return common::Error();
}

DBConnection::DBConnection(CDBConnectionClient* client)
    : base_class(client, new CommandTranslator(base_class::GetCommands())), current_info_() {}

common::Error DBConnection::Info(const std::string& args, ServerInfo::Stats* statsout) {
  if (!statsout) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  const char* stabled_args = args.empty() ? NULL : args.c_str();
  memcached_return_t error;
  memcached_stat_st* st = memcached_stat(connection_.handle_, const_cast<char*>(stabled_args), &error);
  err = CheckResultCommand(DB_INFO_COMMAND, error);
  if (err) {
    return err;
  }

  ServerInfo::Stats lstatsout;
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
  current_info_ = lstatsout;
  memcached_stat_free(NULL, st);
  return common::Error();
}

common::Error DBConnection::AddIfNotExist(const NKey& key,
                                          const std::string& value,
                                          time_t expiration,
                                          uint32_t flags) {
  if (value.empty()) {
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  const key_t key_str = key.GetKey();
  const string_key_t key_slice = key_str.GetKeyData();
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  err = CheckResultCommand("ADD", memcached_add(connection_.handle_, key_slice_ptr, key_slice.size(), value.c_str(),
                                                value.length(), expiration, flags));
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnAddedKey(NDbKValue(key, NValue(common::Value::CreateStringValue(value))));
  }
  return common::Error();
}

common::Error DBConnection::Replace(const NKey& key, const std::string& value, time_t expiration, uint32_t flags) {
  if (value.empty()) {
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  const key_t key_str = key.GetKey();
  const string_key_t key_slice = key_str.GetKeyData();
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  err = CheckResultCommand("REPLACE", memcached_replace(connection_.handle_, key_slice_ptr, key_slice.size(),
                                                        value.c_str(), value.length(), expiration, flags));
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnLoadedKey(NDbKValue(key, NValue(common::Value::CreateStringValue(value))));
  }
  return common::Error();
}

common::Error DBConnection::Append(const NKey& key, const std::string& value, time_t expiration, uint32_t flags) {
  if (value.empty()) {
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  const key_t key_str = key.GetKey();
  const string_key_t key_slice = key_str.GetKeyData();
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  err = CheckResultCommand("APPEND", memcached_append(connection_.handle_, key_slice_ptr, key_slice.size(),
                                                      value.c_str(), value.length(), expiration, flags));
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnAddedKey(NDbKValue(key, NValue(common::Value::CreateStringValue(value))));
  }
  return common::Error();
}

common::Error DBConnection::Prepend(const NKey& key, const std::string& value, time_t expiration, uint32_t flags) {
  if (value.empty()) {
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  const key_t key_str = key.GetKey();
  const string_key_t key_slice = key_str.GetKeyData();
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  err = CheckResultCommand("PREPEND", memcached_prepend(connection_.handle_, key_slice_ptr, key_slice.size(),
                                                        value.c_str(), value.length(), expiration, flags));
  if (err) {
    return err;
  }

  if (client_) {
    client_->OnAddedKey(NDbKValue(key, NValue(common::Value::CreateStringValue(value))));
  }
  return common::Error();
}

common::Error DBConnection::Incr(const NKey& key, uint32_t value, uint64_t* result) {
  if (!result) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  const key_t key_str = key.GetKey();
  const string_key_t key_slice = key_str.GetKeyData();
  uint64_t local_value = 0;
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  err = CheckResultCommand(
      "INCR", memcached_increment(connection_.handle_, key_slice_ptr, key_slice.size(), value, &local_value));
  if (err) {
    return err;
  }

  if (client_) {
    NValue val(common::Value::CreateULongLongIntegerValue(local_value));
    client_->OnAddedKey(NDbKValue(key, val));
  }
  *result = local_value;
  return common::Error();
}

common::Error DBConnection::Decr(const NKey& key, uint32_t value, uint64_t* result) {
  if (!result) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  const key_t key_str = key.GetKey();
  const string_key_t key_slice = key_str.GetKeyData();
  uint64_t local_value = 0;
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  err = CheckResultCommand(
      "DECR", memcached_decrement(connection_.handle_, key_slice_ptr, key_slice.size(), value, &local_value));
  if (err) {
    return err;
  }

  if (client_) {
    NValue val(common::Value::CreateULongLongIntegerValue(local_value));
    client_->OnAddedKey(NDbKValue(key, val));
  }
  *result = local_value;
  return common::Error();
}

common::Error DBConnection::DelInner(key_t key, time_t expiration) {
  const string_key_t key_slice = key.GetKeyData();
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  return CheckResultCommand(DB_DELETE_KEY_COMMAND,
                            memcached_delete(connection_.handle_, key_slice_ptr, key_slice.size(), expiration));
}

common::Error DBConnection::SetInner(key_t key, const std::string& value, time_t expiration, uint32_t flags) {
  const string_key_t key_slice = key.GetKeyData();
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  return CheckResultCommand(DB_SET_KEY_COMMAND, memcached_set(connection_.handle_, key_slice_ptr, key_slice.size(),
                                                              value.c_str(), value.length(), expiration, flags));
}

common::Error DBConnection::GetInner(key_t key, std::string* ret_val) {
  if (!ret_val) {
    DNOTREACHED();
    return common::make_error_inval();
  }

  uint32_t flags = 0;
  memcached_return error;
  size_t value_length = 0;

  const string_key_t key_slice = key.GetKeyData();
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  char* value = memcached_get(connection_.handle_, key_slice_ptr, key_slice.size(), &value_length, &flags, &error);
  common::Error err = CheckResultCommand(DB_GET_KEY_COMMAND, error);
  if (err) {
    return err;
  }

  CHECK(value);
  *ret_val = std::string(value, value + value_length);
  free(value);
  return common::Error();
}

common::Error DBConnection::ExpireInner(key_t key, ttl_t expiration) {
  uint32_t flags = 0;
  memcached_return error;
  size_t value_length = 0;

  const string_key_t key_slice = key.GetKeyData();
  const char* key_slice_ptr = reinterpret_cast<const char*>(key_slice.data());
  char* value = memcached_get(connection_.handle_, key_slice_ptr, key_slice.size(), &value_length, &flags, &error);
  common::Error err = CheckResultCommand(DB_SET_TTL_COMMAND, error);
  if (err) {
    return err;
  }

  return CheckResultCommand(DB_SET_TTL_COMMAND, memcached_set(connection_.handle_, key_slice_ptr, key_slice.size(),
                                                              value, value_length, expiration, flags));
}

common::Error DBConnection::TTL(key_t key, ttl_t* expiration) {
  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  time_t exp;
  TTLHolder hld(key, &exp);
  memcached_dump_fn func[1] = {0};
  func[0] = memcached_dump_ttl_callback;
  err = CheckResultCommand(DB_GET_TTL_COMMAND, memcached_dump(connection_.handle_, func, &hld, SIZEOFMASS(func)));
  if (err) {
    return err;
  }

  time_t cur_t = time(NULL);
  time_t server_t = current_info_.time;
  if (cur_t > exp) {
    if (server_t > exp) {
      *expiration = NO_TTL;
    } else {
      *expiration = EXPIRED_TTL;
    }
  } else {
    *expiration = exp - cur_t;
  }
  return common::Error();
}

common::Error DBConnection::VersionServer() {
  common::Error err = TestIsAuthenticated();
  if (err) {
    return err;
  }

  return CheckResultCommand("VERSION", memcached_version(connection_.handle_));
}

common::Error DBConnection::ScanImpl(uint64_t cursor_in,
                                     const std::string& pattern,
                                     uint64_t count_keys,
                                     std::vector<std::string>* keys_out,
                                     uint64_t* cursor_out) {
  ScanHolder hld(cursor_in, pattern, count_keys);
  memcached_dump_fn func[1] = {0};
  func[0] = memcached_dump_scan_callback;
  common::Error err =
      CheckResultCommand(DB_SCAN_COMMAND, memcached_dump(connection_.handle_, func, &hld, SIZEOFMASS(func)));
  if (err) {
    return err;
  }

  *keys_out = hld.r;
  *cursor_out = hld.cursor_out;
  return common::Error();
}

common::Error DBConnection::KeysImpl(const std::string& key_start,
                                     const std::string& key_end,
                                     uint64_t limit,
                                     std::vector<std::string>* ret) {
  KeysHolder hld(key_start, key_end, limit, ret);
  memcached_dump_fn func[1] = {0};
  func[0] = memcached_dump_keys_callback;
  return CheckResultCommand(DB_KEYS_COMMAND, memcached_dump(connection_.handle_, func, &hld, SIZEOFMASS(func)));
}

common::Error DBConnection::DBkcountImpl(size_t* size) {
  std::vector<std::string> ret;
  KeysHolder hld("a", "z", UINT64_MAX, &ret);
  memcached_dump_fn func[1] = {0};
  func[0] = memcached_dump_keys_callback;
  common::Error err =
      CheckResultCommand(DB_DBKCOUNT_COMMAND, memcached_dump(connection_.handle_, func, &hld, SIZEOFMASS(func)));
  if (err) {
    return err;
  }

  *size = ret.size();
  return common::Error();
}

common::Error DBConnection::FlushDBImpl() {
  return CheckResultCommand(DB_FLUSHDB_COMMAND, memcached_flush(connection_.handle_, 0));
}

common::Error DBConnection::SelectImpl(const std::string& name, IDataBaseInfo** info) {
  if (name != GetCurrentDBName()) {
    return ICommandTranslator::InvalidInputArguments(DB_SELECTDB_COMMAND);
  }

  size_t kcount = 0;
  common::Error err = DBkcount(&kcount);
  // DCHECK(!err);
  *info = new DataBaseInfo(name, true, kcount);
  return common::Error();
}

common::Error DBConnection::DeleteImpl(const NKeys& keys, NKeys* deleted_keys) {
  for (size_t i = 0; i < keys.size(); ++i) {
    NKey key = keys[i];
    key_t key_str = key.GetKey();
    common::Error err = DelInner(key_str, 0);
    if (err) {
      continue;
    }

    deleted_keys->push_back(key);
  }

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

common::Error DBConnection::SetImpl(const NDbKValue& key, NDbKValue* added_key) {
  const NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  std::string value_str = key.GetValueString();
  common::Error err = SetInner(key_str, value_str, 0, 0);
  if (err) {
    return err;
  }

  *added_key = key;
  return common::Error();
}

common::Error DBConnection::RenameImpl(const NKey& key, string_key_t new_key) {
  key_t key_str = key.GetKey();
  std::string value_str;
  common::Error err = GetInner(key_str, &value_str);
  if (err) {
    return err;
  }

  err = DelInner(key_str, 0);
  if (err) {
    return err;
  }

  err = SetInner(key_t(new_key), value_str, 0, 0);
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::SetTTLImpl(const NKey& key, ttl_t ttl) {
  return ExpireInner(key.GetKey(), ttl);
}

common::Error DBConnection::GetTTLImpl(const NKey& key, ttl_t* ttl) {
  return TTL(key.GetKey(), ttl);
}

common::Error DBConnection::QuitImpl() {
  common::Error err = Disconnect();
  if (err) {
    return err;
  }

  return common::Error();
}

common::Error DBConnection::ConfigGetDatabasesImpl(std::vector<std::string>* dbs) {
  std::vector<std::string> ldbs = {GetCurrentDBName()};
  *dbs = ldbs;
  return common::Error();
}

common::Error DBConnection::CheckResultCommand(const std::string& cmd, int err) {
  memcached_return_t mem_err = static_cast<memcached_return_t>(err);
  if (mem_err != MEMCACHED_SUCCESS) {
    return GenerateError(cmd, memcached_strerror(connection_.handle_, mem_err));
  }

  return common::Error();
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
