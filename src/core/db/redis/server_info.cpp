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

#include "core/db/redis/server_info.h"

#include <stddef.h>  // for size_t

#include <sstream>  // for operator<<, basic_ostream, etc
#include <string>   // for operator==, string, etc
#include <utility>  // for make_pair
#include <vector>   // for vector

#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/macros.h>          // for NOTREACHED, UNUSED
#include <common/value.h>           // for FundamentalValue, Value, etc

#include "core/connection_types.h"  // for connectionTypes::REDIS
#include "core/db_traits.h"

namespace fastonosql {
namespace core {
namespace {

const std::vector<Field> g_redis_server_fields = {Field(REDIS_VERSION_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_GIT_SHA1_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_GIT_DIRTY_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_BUILD_ID_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_MODE_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_OS_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_ARCH_BITS_LABEL, common::Value::TYPE_UINTEGER),
                                                  Field(REDIS_MULTIPLEXING_API_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_GCC_VERSION_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_PROCESS_ID_LABEL, common::Value::TYPE_UINTEGER),
                                                  Field(REDIS_RUN_ID_LABEL, common::Value::TYPE_STRING),
                                                  Field(REDIS_TCP_PORT_LABEL, common::Value::TYPE_UINTEGER),
                                                  Field(REDIS_UPTIME_IN_SECONDS_LABEL, common::Value::TYPE_UINTEGER),
                                                  Field(REDIS_UPTIME_IN_DAYS_LABEL, common::Value::TYPE_UINTEGER),
                                                  Field(REDIS_HZ_LABEL, common::Value::TYPE_UINTEGER),
                                                  Field(REDIS_LRU_CLOCK_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_redis_client_fields = {
    Field(REDIS_CONNECTED_CLIENTS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_CLIENT_LONGEST_OUTPUT_LIST_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_CLIENT_BIGGEST_INPUT_BUF_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_BLOCKED_CLIENTS_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_redis_memory_fields = {
    Field(REDIS_USED_MEMORY_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_USED_MEMORY_HUMAN_LABEL, common::Value::TYPE_STRING),
    Field(REDIS_USED_MEMORY_RSS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_USED_MEMORY_PEAK_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_USED_MEMORY_PEAK_HUMAN_LABEL, common::Value::TYPE_STRING),
    Field(REDIS_USED_MEMORY_LUA_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_MEM_FRAGMENTATION_RATIO_LABEL, common::Value::TYPE_DOUBLE),
    Field(REDIS_MEM_ALLOCATOR_LABEL, common::Value::TYPE_STRING)};

const std::vector<Field> g_redis_persistence_fields = {
    Field(REDIS_LOADING_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_RDB_CHANGES_SINCE_LAST_SAVE_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_RDB_DGSAVE_IN_PROGRESS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_RDB_LAST_SAVE_TIME_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_RDB_LAST_DGSAVE_STATUS_LABEL, common::Value::TYPE_STRING),
    Field(REDIS_RDB_LAST_DGSAVE_TIME_SEC_LABEL, common::Value::TYPE_INTEGER),
    Field(REDIS_RDB_CURRENT_DGSAVE_TIME_SEC_LABEL, common::Value::TYPE_INTEGER),
    Field(REDIS_AOF_ENABLED_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_AOF_REWRITE_IN_PROGRESS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_AOF_REWRITE_SHEDULED_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_AOF_LAST_REWRITE_TIME_SEC_LABEL, common::Value::TYPE_INTEGER),
    Field(REDIS_AOF_CURRENT_REWRITE_TIME_SEC_LABEL, common::Value::TYPE_INTEGER),
    Field(REDIS_AOF_LAST_DGREWRITE_STATUS_LABEL, common::Value::TYPE_STRING),
    Field(REDIS_AOF_LAST_WRITE_STATUS_LABEL, common::Value::TYPE_STRING)};

const std::vector<Field> g_redis_stats_fields = {
    Field(REDIS_TOTAL_CONNECTIONS_RECEIVED_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_TOTAL_COMMANDS_PROCESSED_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_INSTANTANEOUS_OPS_PER_SEC_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_REJECTED_CONNECTIONS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_SYNC_FULL_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_SYNC_PARTIAL_OK_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_SYNC_PARTIAL_ERR_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_EXPIRED_KEYS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_EVICTED_KEYS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_KEYSPACE_HITS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_KEYSPACE_MISSES_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_PUBSUB_CHANNELS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_PUBSUB_PATTERNS_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_LATEST_FORK_USEC_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_redis_replication_fields = {
    Field(REDIS_ROLE_LABEL, common::Value::TYPE_STRING),
    Field(REDIS_CONNECTED_SLAVES_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_MASTER_REPL_OFFSET_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_BACKLOG_ACTIVE_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_BACKLOG_SIZE_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_BACKLOG_FIRST_BYTE_OFFSET_LABEL, common::Value::TYPE_UINTEGER),
    Field(REDIS_BACKLOG_HISTEN_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_redis_cpu_fields = {Field(REDIS_USED_CPU_SYS_LABEL, common::Value::TYPE_UINTEGER),
                                               Field(REDIS_USED_CPU_USER_LABEL, common::Value::TYPE_UINTEGER),
                                               Field(REDIS_USED_CPU_SYS_CHILDREN_LABEL, common::Value::TYPE_UINTEGER),
                                               Field(REDIS_USED_CPU_USER_CHILDREN_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_redis_keyspace_fields = {};

}  // namespace

template <>
std::vector<common::Value::Type> DBTraits<REDIS>::GetSupportedTypes() {
  return {common::Value::TYPE_BOOLEAN, common::Value::TYPE_INTEGER, common::Value::TYPE_UINTEGER,
          common::Value::TYPE_DOUBLE,  common::Value::TYPE_STRING,

          common::Value::TYPE_ARRAY,   common::Value::TYPE_SET,     common::Value::TYPE_ZSET,
          common::Value::TYPE_HASH};
}

template <>
std::vector<info_field_t> DBTraits<REDIS>::GetInfoFields() {
  return {std::make_pair(REDIS_SERVER_LABEL, g_redis_server_fields),
          std::make_pair(REDIS_CLIENTS_LABEL, g_redis_client_fields),
          std::make_pair(REDIS_MEMORY_LABEL, g_redis_memory_fields),
          std::make_pair(REDIS_PERSISTENCE_LABEL, g_redis_persistence_fields),
          std::make_pair(REDIS_STATS_LABEL, g_redis_stats_fields),
          std::make_pair(REDIS_REPLICATION_LABEL, g_redis_replication_fields),
          std::make_pair(REDIS_CPU_LABEL, g_redis_cpu_fields),
          std::make_pair(REDIS_KEYSPACE_LABEL, g_redis_keyspace_fields)};
}

namespace redis {

ServerInfo::Server::Server::Server()
    : redis_version_(),
      redis_git_sha1_(),
      redis_git_dirty_(),
      redis_mode_(),
      os_(),
      arch_bits_(0),
      multiplexing_api_(),
      gcc_version_(),
      process_id_(0),
      run_id_(),
      tcp_port_(0),
      uptime_in_seconds_(0),
      uptime_in_days_(0),
      hz_(0),
      lru_clock_(0) {}

ServerInfo::Server::Server(const std::string& server_text)
    : redis_version_(),
      redis_git_sha1_(),
      redis_git_dirty_(),
      redis_mode_(),
      os_(),
      arch_bits_(0),
      multiplexing_api_(),
      gcc_version_(),
      process_id_(0),
      run_id_(),
      tcp_port_(0),
      uptime_in_seconds_(0),
      uptime_in_days_(0),
      hz_(0),
      lru_clock_(0) {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = server_text.find("\r\n", start)) != std::string::npos) {
    std::string line = server_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == REDIS_VERSION_LABEL) {
      redis_version_ = value;
    } else if (field == REDIS_GIT_SHA1_LABEL) {
      redis_git_sha1_ = value;
    } else if (field == REDIS_GIT_DIRTY_LABEL) {
      redis_git_dirty_ = value;
    } else if (field == REDIS_MODE_LABEL) {
      redis_mode_ = value;
    } else if (field == REDIS_OS_LABEL) {
      os_ = value;
    } else if (field == REDIS_ARCH_BITS_LABEL) {
      uint32_t arch_bits;
      if (common::ConvertFromString(value, &arch_bits)) {
        arch_bits_ = arch_bits;
      }
    } else if (field == REDIS_MULTIPLEXING_API_LABEL) {
      multiplexing_api_ = value;
    } else if (field == REDIS_GCC_VERSION_LABEL) {
      gcc_version_ = value;
    } else if (field == REDIS_PROCESS_ID_LABEL) {
      uint32_t process_id;
      if (common::ConvertFromString(value, &process_id)) {
        process_id_ = process_id;
      }
    } else if (field == REDIS_RUN_ID_LABEL) {
      run_id_ = value;
    } else if (field == REDIS_TCP_PORT_LABEL) {
      uint32_t tcp_port;
      if (common::ConvertFromString(value, &tcp_port)) {
        tcp_port_ = tcp_port;
      }
    } else if (field == REDIS_UPTIME_IN_SECONDS_LABEL) {
      uint32_t uptime_in_seconds;
      if (common::ConvertFromString(value, &uptime_in_seconds)) {
        uptime_in_seconds_ = uptime_in_seconds;
      }
    } else if (field == REDIS_UPTIME_IN_DAYS_LABEL) {
      uint32_t uptime_in_days;
      if (common::ConvertFromString(value, &uptime_in_days)) {
        uptime_in_days_ = uptime_in_days;
      }
    } else if (field == REDIS_HZ_LABEL) {
      uint32_t hz;
      if (common::ConvertFromString(value, &hz)) {
        hz_ = hz;
      }
    } else if (field == REDIS_LRU_CLOCK_LABEL) {
      uint32_t lru_clock;
      if (common::ConvertFromString(value, &lru_clock)) {
        lru_clock_ = lru_clock;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Server::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::StringValue(redis_version_);
    case 1:
      return new common::StringValue(redis_git_sha1_);
    case 2:
      return new common::StringValue(redis_git_dirty_);
    case 3:
      return new common::StringValue(redis_build_id_);
    case 4:
      return new common::StringValue(redis_mode_);
    case 5:
      return new common::StringValue(os_);
    case 6:
      return new common::FundamentalValue(arch_bits_);
    case 7:
      return new common::StringValue(multiplexing_api_);
    case 8:
      return new common::StringValue(gcc_version_);
    case 9:
      return new common::FundamentalValue(process_id_);
    case 10:
      return new common::StringValue(run_id_);
    case 11:
      return new common::FundamentalValue(tcp_port_);
    case 12:
      return new common::FundamentalValue(uptime_in_seconds_);
    case 13:
      return new common::FundamentalValue(uptime_in_days_);
    case 14:
      return new common::FundamentalValue(hz_);
    case 15:
      return new common::FundamentalValue(lru_clock_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Clients::Clients()
    : connected_clients_(0), client_longest_output_list_(0), client_biggest_input_buf_(0), blocked_clients_(0) {}

ServerInfo::Clients::Clients(const std::string& client_text)
    : connected_clients_(0), client_longest_output_list_(0), client_biggest_input_buf_(0), blocked_clients_(0) {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = client_text.find(("\r\n"), start)) != std::string::npos) {
    std::string line = client_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == REDIS_CONNECTED_CLIENTS_LABEL) {
      uint32_t connected_clients;
      if (common::ConvertFromString(value, &connected_clients)) {
        connected_clients_ = connected_clients;
      }
    } else if (field == REDIS_CLIENT_LONGEST_OUTPUT_LIST_LABEL) {
      uint32_t client_longest_output_list;
      if (common::ConvertFromString(value, &client_longest_output_list)) {
        client_longest_output_list_ = client_longest_output_list;
      }
    } else if (field == REDIS_CLIENT_BIGGEST_INPUT_BUF_LABEL) {
      uint32_t client_biggest_input_buf;
      if (common::ConvertFromString(value, &client_biggest_input_buf)) {
        client_biggest_input_buf_ = client_biggest_input_buf;
      }
    } else if (field == REDIS_BLOCKED_CLIENTS_LABEL) {
      uint32_t blocked_clients;
      if (common::ConvertFromString(value, &blocked_clients)) {
        blocked_clients_ = blocked_clients;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Clients::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(connected_clients_);
    case 1:
      return new common::FundamentalValue(client_longest_output_list_);
    case 2:
      return new common::FundamentalValue(client_biggest_input_buf_);
    case 3:
      return new common::FundamentalValue(blocked_clients_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Memory::Memory()
    : used_memory_(0),
      used_memory_human_(),
      used_memory_rss_(0),
      used_memory_peak_(0),
      used_memory_peak_human_(),
      used_memory_lua_(0),
      mem_fragmentation_ratio_(0),
      mem_allocator_() {}

ServerInfo::Memory::Memory(const std::string& memory_text)
    : used_memory_(0),
      used_memory_human_(),
      used_memory_rss_(0),
      used_memory_peak_(0),
      used_memory_peak_human_(),
      used_memory_lua_(0),
      mem_fragmentation_ratio_(0),
      mem_allocator_() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = memory_text.find(("\r\n"), start)) != std::string::npos) {
    std::string line = memory_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == REDIS_USED_MEMORY_LABEL) {
      uint32_t used_memory;
      if (common::ConvertFromString(value, &used_memory)) {
        used_memory_ = used_memory;
      }
    } else if (field == REDIS_USED_MEMORY_HUMAN_LABEL) {
      used_memory_human_ = value;
    } else if (field == REDIS_USED_MEMORY_RSS_LABEL) {
      uint32_t used_memory_rss;
      if (common::ConvertFromString(value, &used_memory_rss)) {
        used_memory_rss_ = used_memory_rss;
      }
    } else if (field == REDIS_USED_MEMORY_PEAK_LABEL) {
      uint32_t used_memory_peak;
      if (common::ConvertFromString(value, &used_memory_peak)) {
        used_memory_peak_ = used_memory_peak;
      }
    } else if (field == REDIS_USED_MEMORY_PEAK_HUMAN_LABEL) {
      used_memory_peak_human_ = value;
    } else if (field == REDIS_USED_MEMORY_LUA_LABEL) {
      uint32_t used_memory_lua;
      if (common::ConvertFromString(value, &used_memory_lua)) {
        used_memory_lua_ = used_memory_lua;
      }
    } else if (field == REDIS_MEM_FRAGMENTATION_RATIO_LABEL) {
      float mem_fragmentation_ratio;
      if (common::ConvertFromString(value, &mem_fragmentation_ratio)) {
        mem_fragmentation_ratio_ = mem_fragmentation_ratio;
      }
    } else if (field == REDIS_MEM_ALLOCATOR_LABEL) {
      mem_allocator_ = value;
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Memory::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(used_memory_);
    case 1:
      return new common::StringValue(used_memory_human_);
    case 2:
      return new common::FundamentalValue(used_memory_rss_);
    case 3:
      return new common::FundamentalValue(used_memory_peak_);
    case 4:
      return new common::StringValue(used_memory_peak_human_);
    case 5:
      return new common::FundamentalValue(used_memory_lua_);
    case 6:
      return new common::FundamentalValue(mem_fragmentation_ratio_);
    case 7:
      return new common::StringValue(mem_allocator_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Persistence::Persistence()
    : loading_(0),
      rdb_changes_since_last_save_(0),
      rdb_bgsave_in_progress_(0),
      rdb_last_save_time_(0),
      rdb_last_bgsave_status_(),
      rdb_last_bgsave_time_sec_(0),
      rdb_current_bgsave_time_sec_(0),
      aof_enabled_(0),
      aof_rewrite_in_progress_(0),
      aof_rewrite_scheduled_(0),
      aof_last_rewrite_time_sec_(0),
      aof_current_rewrite_time_sec_(0),
      aof_last_bgrewrite_status_(),
      aof_last_write_status_() {}

ServerInfo::Persistence::Persistence(const std::string& persistence_text)
    : loading_(0),
      rdb_changes_since_last_save_(0),
      rdb_bgsave_in_progress_(0),
      rdb_last_save_time_(0),
      rdb_last_bgsave_status_(),
      rdb_last_bgsave_time_sec_(0),
      rdb_current_bgsave_time_sec_(0),
      aof_enabled_(0),
      aof_rewrite_in_progress_(0),
      aof_rewrite_scheduled_(0),
      aof_last_rewrite_time_sec_(0),
      aof_current_rewrite_time_sec_(0),
      aof_last_bgrewrite_status_(),
      aof_last_write_status_() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = persistence_text.find(("\r\n"), start)) != std::string::npos) {
    std::string line = persistence_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == REDIS_LOADING_LABEL) {
      uint32_t loading;
      if (common::ConvertFromString(value, &loading)) {
        loading_ = loading;
      }
    } else if (field == REDIS_RDB_CHANGES_SINCE_LAST_SAVE_LABEL) {
      uint32_t rdb_changes_since_last_save;
      if (common::ConvertFromString(value, &rdb_changes_since_last_save)) {
        rdb_changes_since_last_save_ = rdb_changes_since_last_save;
      }
    } else if (field == REDIS_RDB_DGSAVE_IN_PROGRESS_LABEL) {
      uint32_t rdb_bgsave_in_progress;
      if (common::ConvertFromString(value, &rdb_bgsave_in_progress)) {
        rdb_bgsave_in_progress_ = rdb_bgsave_in_progress;
      }
    } else if (field == REDIS_RDB_LAST_SAVE_TIME_LABEL) {
      uint32_t rdb_last_save_time;
      if (common::ConvertFromString(value, &rdb_last_save_time)) {
        rdb_last_save_time_ = rdb_last_save_time;
      }
    } else if (field == REDIS_RDB_LAST_DGSAVE_STATUS_LABEL) {
      rdb_last_bgsave_status_ = value;
    } else if (field == REDIS_RDB_LAST_DGSAVE_TIME_SEC_LABEL) {
      int rdb_last_bgsave_time_sec;
      if (common::ConvertFromString(value, &rdb_last_bgsave_time_sec)) {
        rdb_last_bgsave_time_sec_ = rdb_last_bgsave_time_sec;
      }
    } else if (field == REDIS_RDB_CURRENT_DGSAVE_TIME_SEC_LABEL) {
      int rdb_current_bgsave_time_sec;
      if (common::ConvertFromString(value, &rdb_current_bgsave_time_sec)) {
        rdb_current_bgsave_time_sec_ = rdb_current_bgsave_time_sec;
      }
    } else if (field == REDIS_AOF_ENABLED_LABEL) {
      uint32_t aof_enabled;
      if (common::ConvertFromString(value, &aof_enabled)) {
        aof_enabled_ = aof_enabled;
      }
    } else if (field == REDIS_AOF_REWRITE_IN_PROGRESS_LABEL) {
      uint32_t aof_rewrite_in_progress;
      if (common::ConvertFromString(value, &aof_rewrite_in_progress)) {
        aof_rewrite_in_progress_ = aof_rewrite_in_progress;
      }
    } else if (field == REDIS_AOF_REWRITE_SHEDULED_LABEL) {
      uint32_t aof_rewrite_scheduled;
      if (common::ConvertFromString(value, &aof_rewrite_scheduled)) {
        aof_rewrite_scheduled_ = aof_rewrite_scheduled;
      }
    } else if (field == REDIS_AOF_LAST_REWRITE_TIME_SEC_LABEL) {
      int aof_last_rewrite_time_sec;
      if (common::ConvertFromString(value, &aof_last_rewrite_time_sec)) {
        aof_last_rewrite_time_sec_ = aof_last_rewrite_time_sec;
      }
    } else if (field == REDIS_AOF_CURRENT_REWRITE_TIME_SEC_LABEL) {
      int aof_current_rewrite_time_sec;
      if (common::ConvertFromString(value, &aof_current_rewrite_time_sec)) {
        aof_current_rewrite_time_sec_ = aof_current_rewrite_time_sec;
      }
    } else if (field == REDIS_AOF_LAST_DGREWRITE_STATUS_LABEL) {
      aof_last_bgrewrite_status_ = value;
    } else if (field == REDIS_AOF_LAST_WRITE_STATUS_LABEL) {
      aof_last_write_status_ = value;
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Persistence::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(loading_);
    case 1:
      return new common::FundamentalValue(rdb_changes_since_last_save_);
    case 2:
      return new common::FundamentalValue(rdb_bgsave_in_progress_);
    case 3:
      return new common::FundamentalValue(rdb_last_save_time_);
    case 4:
      return new common::StringValue(rdb_last_bgsave_status_);
    case 5:
      return new common::FundamentalValue(rdb_last_bgsave_time_sec_);
    case 6:
      return new common::FundamentalValue(rdb_current_bgsave_time_sec_);
    case 7:
      return new common::FundamentalValue(aof_enabled_);
    case 8:
      return new common::FundamentalValue(aof_rewrite_in_progress_);
    case 9:
      return new common::FundamentalValue(aof_rewrite_scheduled_);
    case 10:
      return new common::FundamentalValue(aof_last_rewrite_time_sec_);
    case 11:
      return new common::FundamentalValue(aof_current_rewrite_time_sec_);
    case 12:
      return new common::StringValue(aof_last_bgrewrite_status_);
    case 13:
      return new common::StringValue(aof_last_write_status_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Stats::Stats()
    : total_connections_received_(0),
      total_commands_processed_(0),
      instantaneous_ops_per_sec_(0),
      rejected_connections_(0),
      sync_full_(0),
      sync_partial_ok_(0),
      sync_partial_err_(0),
      expired_keys_(0),
      evicted_keys_(0),
      keyspace_hits_(0),
      keyspace_misses_(0),
      pubsub_channels_(0),
      pubsub_patterns_(0),
      latest_fork_usec_(0) {}

ServerInfo::Stats::Stats(const std::string& stats_text)
    : total_connections_received_(0),
      total_commands_processed_(0),
      instantaneous_ops_per_sec_(0),
      rejected_connections_(0),
      sync_full_(0),
      sync_partial_ok_(0),
      sync_partial_err_(0),
      expired_keys_(0),
      evicted_keys_(0),
      keyspace_hits_(0),
      keyspace_misses_(0),
      pubsub_channels_(0),
      pubsub_patterns_(0),
      latest_fork_usec_(0) {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = stats_text.find(("\r\n"), start)) != std::string::npos) {
    std::string line = stats_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == REDIS_TOTAL_CONNECTIONS_RECEIVED_LABEL) {
      uint32_t total_connections_received;
      if (common::ConvertFromString(value, &total_connections_received)) {
        total_connections_received_ = total_connections_received;
      }
    } else if (field == REDIS_TOTAL_COMMANDS_PROCESSED_LABEL) {
      uint32_t total_commands_processed;
      if (common::ConvertFromString(value, &total_commands_processed)) {
        total_commands_processed_ = total_commands_processed;
      }
    } else if (field == REDIS_INSTANTANEOUS_OPS_PER_SEC_LABEL) {
      uint32_t instantaneous_ops_per_sec;
      if (common::ConvertFromString(value, &instantaneous_ops_per_sec)) {
        instantaneous_ops_per_sec_ = instantaneous_ops_per_sec;
      }
    } else if (field == REDIS_REJECTED_CONNECTIONS_LABEL) {
      uint32_t rejected_connections;
      if (common::ConvertFromString(value, &rejected_connections)) {
        rejected_connections_ = rejected_connections;
      }
    } else if (field == REDIS_SYNC_FULL_LABEL) {
      uint32_t sync_full;
      if (common::ConvertFromString(value, &sync_full)) {
        sync_full_ = sync_full;
      }
    } else if (field == REDIS_SYNC_PARTIAL_OK_LABEL) {
      uint32_t sync_partial_ok;
      if (common::ConvertFromString(value, &sync_partial_ok)) {
        sync_partial_ok_ = sync_partial_ok;
      }
    } else if (field == REDIS_SYNC_PARTIAL_ERR_LABEL) {
      uint32_t sync_partial_err;
      if (common::ConvertFromString(value, &sync_partial_err)) {
        sync_partial_err_ = sync_partial_err;
      }
    } else if (field == REDIS_EXPIRED_KEYS_LABEL) {
      uint32_t expired_keys;
      if (common::ConvertFromString(value, &expired_keys)) {
        expired_keys_ = expired_keys;
      }
    } else if (field == REDIS_EVICTED_KEYS_LABEL) {
      uint32_t evicted_keys;
      if (common::ConvertFromString(value, &evicted_keys)) {
        evicted_keys_ = evicted_keys;
      }
    } else if (field == REDIS_KEYSPACE_HITS_LABEL) {
      uint32_t keyspace_hits;
      if (common::ConvertFromString(value, &keyspace_hits)) {
        keyspace_hits_ = keyspace_hits;
      }
    } else if (field == REDIS_KEYSPACE_MISSES_LABEL) {
      uint32_t keyspace_misses;
      if (common::ConvertFromString(value, &keyspace_misses)) {
        keyspace_misses_ = keyspace_misses;
      }
    } else if (field == REDIS_PUBSUB_CHANNELS_LABEL) {
      uint32_t pubsub_channels;
      if (common::ConvertFromString(value, &pubsub_channels)) {
        pubsub_channels_ = pubsub_channels;
      }
    } else if (field == REDIS_PUBSUB_PATTERNS_LABEL) {
      uint32_t pubsub_patterns;
      if (common::ConvertFromString(value, &pubsub_patterns)) {
        pubsub_patterns_ = pubsub_patterns;
      }
    } else if (field == REDIS_LATEST_FORK_USEC_LABEL) {
      uint32_t latest_fork_usec;
      if (common::ConvertFromString(value, &latest_fork_usec)) {
        latest_fork_usec_ = latest_fork_usec;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Stats::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(total_connections_received_);
    case 1:
      return new common::FundamentalValue(total_commands_processed_);
    case 2:
      return new common::FundamentalValue(instantaneous_ops_per_sec_);
    case 3:
      return new common::FundamentalValue(rejected_connections_);
    case 4:
      return new common::FundamentalValue(sync_full_);
    case 5:
      return new common::FundamentalValue(sync_partial_ok_);
    case 6:
      return new common::FundamentalValue(sync_partial_err_);
    case 7:
      return new common::FundamentalValue(expired_keys_);
    case 8:
      return new common::FundamentalValue(evicted_keys_);
    case 9:
      return new common::FundamentalValue(keyspace_hits_);
    case 10:
      return new common::FundamentalValue(keyspace_misses_);
    case 11:
      return new common::FundamentalValue(pubsub_channels_);
    case 12:
      return new common::FundamentalValue(pubsub_patterns_);
    case 13:
      return new common::FundamentalValue(latest_fork_usec_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Replication::Replication()
    : role_(),
      connected_slaves_(0),
      master_repl_offset_(0),
      backlog_active_(0),
      backlog_size_(0),
      backlog_first_byte_offset_(0),
      backlog_histen_(0) {}

ServerInfo::Replication::Replication(const std::string& replication_text)
    : role_(),
      connected_slaves_(0),
      master_repl_offset_(0),
      backlog_active_(0),
      backlog_size_(0),
      backlog_first_byte_offset_(0),
      backlog_histen_(0) {
  size_t pos = 0;
  size_t start = 0;

  while ((pos = replication_text.find(("\r\n"), start)) != std::string::npos) {
    std::string line = replication_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == REDIS_ROLE_LABEL) {
      role_ = value;
    } else if (field == REDIS_CONNECTED_SLAVES_LABEL) {
      uint32_t connected_slaves;
      if (common::ConvertFromString(value, &connected_slaves)) {
        connected_slaves_ = connected_slaves;
      }
    } else if (field == REDIS_MASTER_REPL_OFFSET_LABEL) {
      uint32_t master_repl_offset;
      if (common::ConvertFromString(value, &master_repl_offset)) {
        master_repl_offset_ = master_repl_offset;
      }
    } else if (field == REDIS_BACKLOG_ACTIVE_LABEL) {
      uint32_t backlog_active;
      if (common::ConvertFromString(value, &backlog_active)) {
        backlog_active_ = backlog_active;
      }
    } else if (field == REDIS_BACKLOG_SIZE_LABEL) {
      uint32_t backlog_size;
      if (common::ConvertFromString(value, &backlog_size)) {
        backlog_size_ = backlog_size;
      }
    } else if (field == REDIS_BACKLOG_FIRST_BYTE_OFFSET_LABEL) {
      uint32_t backlog_first_byte_offset;
      if (common::ConvertFromString(value, &backlog_first_byte_offset)) {
        backlog_first_byte_offset_ = backlog_first_byte_offset;
      }
    } else if (field == REDIS_BACKLOG_HISTEN_LABEL) {
      uint32_t backlog_histen;
      if (common::ConvertFromString(value, &backlog_histen)) {
        backlog_histen_ = backlog_histen;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Replication::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::StringValue(role_);
    case 1:
      return new common::FundamentalValue(connected_slaves_);
    case 2:
      return new common::FundamentalValue(master_repl_offset_);
    case 3:
      return new common::FundamentalValue(backlog_active_);
    case 4:
      return new common::FundamentalValue(backlog_size_);
    case 5:
      return new common::FundamentalValue(backlog_first_byte_offset_);
    case 6:
      return new common::FundamentalValue(backlog_histen_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Cpu::Cpu() : used_cpu_sys_(0), used_cpu_user_(0), used_cpu_sys_children_(0), used_cpu_user_children_(0) {}

ServerInfo::Cpu::Cpu(const std::string& cpu_text)
    : used_cpu_sys_(0), used_cpu_user_(0), used_cpu_sys_children_(0), used_cpu_user_children_(0) {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = cpu_text.find(("\r\n"), start)) != std::string::npos) {
    std::string line = cpu_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == (REDIS_USED_CPU_SYS_LABEL)) {
      float used_cpu_sys;
      if (common::ConvertFromString(value, &used_cpu_sys)) {
        used_cpu_sys_ = used_cpu_sys;
      }
    } else if (field == REDIS_USED_CPU_USER_LABEL) {
      float used_cpu_user;
      if (common::ConvertFromString(value, &used_cpu_user)) {
        used_cpu_user_ = used_cpu_user;
      }
    } else if (field == REDIS_USED_CPU_SYS_CHILDREN_LABEL) {
      float used_cpu_sys_children;
      if (common::ConvertFromString(value, &used_cpu_sys_children)) {
        used_cpu_sys_children_ = used_cpu_sys_children;
      }
    } else if (field == REDIS_USED_CPU_USER_CHILDREN_LABEL) {
      float used_cpu_user_children;
      if (common::ConvertFromString(value, &used_cpu_user_children)) {
        used_cpu_user_children_ = used_cpu_user_children;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Cpu::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(used_cpu_sys_);
    case 1:
      return new common::FundamentalValue(used_cpu_user_);
    case 2:
      return new common::FundamentalValue(used_cpu_sys_children_);
    case 3:
      return new common::FundamentalValue(used_cpu_user_children_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

common::Value* ServerInfo::Keyspace::GetValueByIndex(unsigned char index) const {
  UNUSED(index);

  return nullptr;
}

ServerInfo::ServerInfo() : IServerInfo(REDIS) {}

ServerInfo::ServerInfo(const Server& serv,
                       const Clients& clients,
                       const Memory& memory,
                       const Persistence& pers,
                       const Stats& stats,
                       const Replication& repl,
                       const Cpu& cpu,
                       const Keyspace& key)
    : IServerInfo(REDIS),
      server_(serv),
      clients_(clients),
      memory_(memory),
      persistence_(pers),
      stats_(stats),
      replication_(repl),
      cpu_(cpu),
      keySp_(key) {}

common::Value* ServerInfo::GetValueByIndexes(unsigned char property, unsigned char field) const {
  switch (property) {
    case 0:
      return server_.GetValueByIndex(field);
    case 1:
      return clients_.GetValueByIndex(field);
    case 2:
      return memory_.GetValueByIndex(field);
    case 3:
      return persistence_.GetValueByIndex(field);
    case 4:
      return stats_.GetValueByIndex(field);
    case 5:
      return replication_.GetValueByIndex(field);
    case 6:
      return cpu_.GetValueByIndex(field);
    case 7:
      return keySp_.GetValueByIndex(field);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Server& value) {
  return out << REDIS_VERSION_LABEL ":" << value.redis_version_ << ("\r\n") << REDIS_GIT_SHA1_LABEL ":"
             << value.redis_git_sha1_ << ("\r\n") << REDIS_GIT_DIRTY_LABEL ":" << value.redis_git_dirty_ << ("\r\n")
             << REDIS_MODE_LABEL ":" << value.redis_mode_ << ("\r\n") << REDIS_OS_LABEL ":" << value.os_ << ("\r\n")
             << REDIS_ARCH_BITS_LABEL ":" << value.arch_bits_ << ("\r\n") << REDIS_MULTIPLEXING_API_LABEL ":"
             << value.multiplexing_api_ << ("\r\n") << REDIS_GCC_VERSION_LABEL ":" << value.gcc_version_ << ("\r\n")
             << REDIS_PROCESS_ID_LABEL ":" << value.process_id_ << ("\r\n") << REDIS_RUN_ID_LABEL ":" << value.run_id_
             << ("\r\n") << REDIS_TCP_PORT_LABEL ":" << value.tcp_port_ << ("\r\n") << REDIS_UPTIME_IN_SECONDS_LABEL ":"
             << value.uptime_in_seconds_ << ("\r\n") << REDIS_UPTIME_IN_DAYS_LABEL ":" << value.uptime_in_days_
             << ("\r\n") << REDIS_HZ_LABEL ":" << value.hz_ << ("\r\n") << REDIS_LRU_CLOCK_LABEL ":" << value.lru_clock_
             << ("\r\n");
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Clients& value) {
  return out << REDIS_CONNECTED_CLIENTS_LABEL ":" << value.connected_clients_ << ("\r\n")
             << REDIS_CLIENT_LONGEST_OUTPUT_LIST_LABEL ":" << value.client_longest_output_list_ << ("\r\n")
             << REDIS_CLIENT_BIGGEST_INPUT_BUF_LABEL ":" << value.client_biggest_input_buf_ << ("\r\n")
             << REDIS_BLOCKED_CLIENTS_LABEL ":" << value.blocked_clients_ << ("\r\n");
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Memory& value) {
  return out << REDIS_USED_MEMORY_LABEL ":" << value.used_memory_ << ("\r\n") << REDIS_USED_MEMORY_HUMAN_LABEL ":"
             << value.used_memory_human_ << ("\r\n") << REDIS_USED_MEMORY_RSS_LABEL ":" << value.used_memory_rss_
             << ("\r\n") << REDIS_USED_MEMORY_PEAK_LABEL ":" << value.used_memory_peak_ << ("\r\n")
             << REDIS_USED_MEMORY_PEAK_HUMAN_LABEL ":" << value.used_memory_peak_human_ << ("\r\n")
             << REDIS_USED_MEMORY_LUA_LABEL ":" << value.used_memory_lua_ << ("\r\n")
             << REDIS_MEM_FRAGMENTATION_RATIO_LABEL ":" << value.mem_fragmentation_ratio_ << ("\r\n")
             << REDIS_MEM_ALLOCATOR_LABEL ":" << value.mem_allocator_ << ("\r\n");
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Persistence& value) {
  return out << REDIS_LOADING_LABEL ":" << value.loading_ << ("\r\n") << REDIS_RDB_CHANGES_SINCE_LAST_SAVE_LABEL ":"
             << value.rdb_changes_since_last_save_ << ("\r\n") << REDIS_RDB_DGSAVE_IN_PROGRESS_LABEL ":"
             << value.rdb_bgsave_in_progress_ << ("\r\n") << REDIS_RDB_LAST_SAVE_TIME_LABEL ":"
             << value.rdb_last_save_time_ << ("\r\n") << REDIS_RDB_LAST_DGSAVE_STATUS_LABEL ":"
             << value.rdb_last_bgsave_status_ << ("\r\n") << REDIS_RDB_LAST_DGSAVE_TIME_SEC_LABEL ":"
             << value.rdb_last_bgsave_time_sec_ << ("\r\n") << REDIS_RDB_CURRENT_DGSAVE_TIME_SEC_LABEL ":"
             << value.rdb_current_bgsave_time_sec_ << ("\r\n") << REDIS_AOF_ENABLED_LABEL ":" << value.aof_enabled_
             << ("\r\n") << REDIS_AOF_REWRITE_IN_PROGRESS_LABEL ":" << value.aof_rewrite_in_progress_ << ("\r\n")
             << REDIS_AOF_REWRITE_SHEDULED_LABEL ":" << value.aof_rewrite_scheduled_ << ("\r\n")
             << REDIS_AOF_LAST_REWRITE_TIME_SEC_LABEL ":" << value.aof_last_rewrite_time_sec_ << ("\r\n")
             << REDIS_AOF_CURRENT_REWRITE_TIME_SEC_LABEL ":" << value.aof_current_rewrite_time_sec_ << ("\r\n")
             << REDIS_AOF_LAST_DGREWRITE_STATUS_LABEL ":" << value.aof_last_bgrewrite_status_ << ("\r\n")
             << REDIS_AOF_LAST_WRITE_STATUS_LABEL ":" << value.aof_last_write_status_ << ("\r\n");
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Stats& value) {
  return out << REDIS_TOTAL_CONNECTIONS_RECEIVED_LABEL ":" << value.total_connections_received_ << ("\r\n")
             << REDIS_TOTAL_COMMANDS_PROCESSED_LABEL ":" << value.total_commands_processed_ << ("\r\n")
             << REDIS_INSTANTANEOUS_OPS_PER_SEC_LABEL ":" << value.instantaneous_ops_per_sec_ << ("\r\n")
             << REDIS_REJECTED_CONNECTIONS_LABEL ":" << value.rejected_connections_ << ("\r\n")
             << REDIS_SYNC_FULL_LABEL ":" << value.sync_full_ << ("\r\n") << REDIS_SYNC_PARTIAL_OK_LABEL ":"
             << value.sync_partial_ok_ << ("\r\n") << REDIS_SYNC_PARTIAL_ERR_LABEL ":" << value.sync_partial_err_
             << ("\r\n") << REDIS_EXPIRED_KEYS_LABEL ":" << value.expired_keys_ << ("\r\n")
             << REDIS_EVICTED_KEYS_LABEL ":" << value.evicted_keys_ << ("\r\n") << REDIS_KEYSPACE_HITS_LABEL ":"
             << value.keyspace_hits_ << ("\r\n") << REDIS_KEYSPACE_MISSES_LABEL ":" << value.keyspace_misses_
             << ("\r\n") << REDIS_PUBSUB_CHANNELS_LABEL ":" << value.pubsub_channels_ << ("\r\n")
             << REDIS_PUBSUB_PATTERNS_LABEL ":" << value.pubsub_patterns_ << ("\r\n")
             << REDIS_LATEST_FORK_USEC_LABEL ":" << value.latest_fork_usec_ << ("\r\n");
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Replication& value) {
  return out << REDIS_ROLE_LABEL ":" << value.role_ << ("\r\n") << REDIS_CONNECTED_SLAVES_LABEL ":"
             << value.connected_slaves_ << ("\r\n") << REDIS_MASTER_REPL_OFFSET_LABEL ":" << value.master_repl_offset_
             << ("\r\n") << REDIS_BACKLOG_ACTIVE_LABEL ":" << value.backlog_active_ << ("\r\n")
             << REDIS_BACKLOG_SIZE_LABEL ":" << value.backlog_size_ << ("\r\n")
             << REDIS_BACKLOG_FIRST_BYTE_OFFSET_LABEL ":" << value.backlog_first_byte_offset_ << ("\r\n")
             << REDIS_BACKLOG_HISTEN_LABEL ":" << value.backlog_histen_ << ("\r\n");
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Cpu& value) {
  return out << REDIS_USED_CPU_SYS_LABEL ":" << value.used_cpu_sys_ << ("\r\n") << REDIS_USED_CPU_USER_LABEL ":"
             << value.used_cpu_user_ << ("\r\n") << REDIS_USED_CPU_SYS_CHILDREN_LABEL ":"
             << value.used_cpu_sys_children_ << ("\r\n") << REDIS_USED_CPU_USER_CHILDREN_LABEL ":"
             << value.used_cpu_user_children_ << ("\r\n");
}

std::string ServerInfo::ToString() const {
  std::stringstream str;
  str << REDIS_SERVER_LABEL "\r\n"
      << server_ << REDIS_CLIENTS_LABEL "\r\n"
      << clients_ << REDIS_MEMORY_LABEL "\r\n"
      << memory_ << REDIS_PERSISTENCE_LABEL "\r\n"
      << persistence_ << REDIS_STATS_LABEL "\r\n"
      << stats_ << REDIS_REPLICATION_LABEL "\r\n"
      << replication_ << REDIS_CPU_LABEL "\r\n"
      << cpu_ << REDIS_KEYSPACE_LABEL "\r\n";
  return str.str();
}

uint32_t ServerInfo::GetVersion() const {
  return common::ConvertVersionNumberFromString(server_.redis_version_);
}

std::ostream& operator<<(std::ostream& out, const ServerInfo& value) {
  // "# Server", "# Clients", "# Memory", "# Persistence",
  // "# Stats", "#
  // Replication", "# CPU", "#
  // Keyspace"
  return out << value.ToString();
}

ServerInfo* MakeRedisServerInfo(const std::string& content) {
  if (content.empty()) {
    return nullptr;
  }

  ServerInfo* result = new ServerInfo;
  size_t j = 0;
  std::string word;
  size_t pos = 0;
  static const std::vector<core::info_field_t> fields = DBTraits<REDIS>::GetInfoFields();
  for (size_t i = 0; i < content.size(); ++i) {
    char ch = content[i];
    word += ch;
    if (word == fields[j].first) {
      if (j + 1 != fields.size()) {
        pos = content.find(fields[j + 1].first, pos);
      } else {
        break;
      }

      if (pos != std::string::npos) {
        std::string part = content.substr(i + 1, pos - i - 1);
        switch (j) {
          case 0:
            result->server_ = ServerInfo::Server(part);
            break;
          case 1:
            result->clients_ = ServerInfo::Clients(part);
            break;
          case 2:
            result->memory_ = ServerInfo::Memory(part);
            break;
          case 3:
            result->persistence_ = ServerInfo::Persistence(part);
            break;
          case 4:
            result->stats_ = ServerInfo::Stats(part);
            break;
          case 5:
            result->replication_ = ServerInfo::Replication(part);
            break;
          case 6:
            result->cpu_ = ServerInfo::Cpu(part);
            break;
          default:
            break;
        }
        i = pos - 1;
        ++j;
      }
      word.clear();
    }
  }

  return result;
}

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
