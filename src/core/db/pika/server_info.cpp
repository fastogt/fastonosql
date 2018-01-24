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

#include "core/db/pika/server_info.h"

#include <common/convert2string.h>

#include "core/db_traits.h"
#include "core/value.h"

#define PIKA_INFO_MARKER "\r\n"

namespace fastonosql {
namespace core {
namespace {

const std::vector<Field> g_pika_server_fields = {
    Field(PIKA_SERVER_VERSION_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_SERVER_GIT_SHA_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_SERVER_BUILD_COMPILE_DATE_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_SERVER_OS_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_SERVER_ARCH_BITS_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_SERVER_PROCESS_ID_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_SERVER_TCP_PORT_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_SERVER_THREAD_NUM_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_SERVER_SYNC_THREAD_NUM_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_SERVER_UPTIME_IN_SECONDS_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_SERVER_UPTIME_IN_DAYS_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_SERVER_CONFIG_FILE_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_SERVER_SERVER_ID_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_pika_data_fields = {
    Field(PIKA_DATA_DB_SIZE_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_DATA_DB_SIZE_HUMAN_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_DATA_COMPRESSION_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_DATA_USED_MEMORY_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_DATA_USED_MEMORY_HUMAN_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_DATA_DB_MEMTABLE_USAGE_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_DATA_DB_TABLEREADER_USAGE_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_pika_log_fields = {Field(PIKA_LOG_SIZE_LABEL, common::Value::TYPE_UINTEGER),
                                              Field(PIKA_LOG_SIZE_HUMAN_LABEL, common::Value::TYPE_STRING),
                                              Field(PIKA_LOG_SAFETY_PURGE_LABEL, common::Value::TYPE_STRING),
                                              Field(PIKA_LOG_EXPIRE_LOGS_DAYS_LABEL, common::Value::TYPE_UINTEGER),
                                              Field(PIKA_LOG_EXPIRE_LOGS_NUMS_LABEL, common::Value::TYPE_UINTEGER),
                                              Field(PIKA_LOG_BINLOG_OFFSET_LABEL, common::Value::TYPE_STRING)};

const std::vector<Field> g_pika_client_fields = {
    Field(PIKA_CLIENTS_CONNECTED_CLIENTS_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_pika_hub_fields = {};

const std::vector<Field> g_pika_stats_fields = {
    Field(PIKA_STATS_TOTAL_CONNECTIONS_RECEIVED_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_STATS_INSTANTANEOUS_OPS_PER_SEC_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_STATS_TOTAL_COMMANDS_PROCESSED_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_STATS_IS_BGSAVING_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_STATS_IS_SLOTS_RELOADING_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_STATS_IS_SLOTS_CLEANUPING_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_STATS_IS_SCANING_KEYSPACE_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_STATS_IS_COMPACT_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_STATS_COMPACT_CRON_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_STATS_COMPACT_INTERVAL_LABEL, common::Value::TYPE_STRING)};

const std::vector<Field> g_pika_cpu_fields = {
    Field(PIKA_CPU_USED_CPU_SYS_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_CPU_USED_CPU_USER_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_CPU_USED_CPU_SYS_CHILDREN_LABEL, common::Value::TYPE_UINTEGER),
    Field(PIKA_CPU_USED_CPU_USER_CHILDREN_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_redis_replication_fields = {
    Field(PIKA_REPLICATION_ROLE_LABEL, common::Value::TYPE_STRING),
    Field(PIKA_REPLICATION_CONNECTED_SLAVES_LABEL, common::Value::TYPE_UINTEGER)};

const std::vector<Field> g_pika_key_space_fields = {};

const std::vector<Field> g_pika_double_master_fields = {};

}  // namespace

template <>
std::vector<common::Value::Type> DBTraits<PIKA>::GetSupportedValueTypes() {
  return {common::Value::TYPE_BOOLEAN, common::Value::TYPE_INTEGER, common::Value::TYPE_UINTEGER,
          common::Value::TYPE_DOUBLE,  common::Value::TYPE_STRING,

          common::Value::TYPE_ARRAY,   common::Value::TYPE_SET,     common::Value::TYPE_ZSET,
          common::Value::TYPE_HASH,    JsonValue::TYPE_JSON};
}

template <>
std::vector<info_field_t> DBTraits<PIKA>::GetInfoFields() {
  return {std::make_pair(PIKA_SERVER_LABEL, g_pika_server_fields),
          std::make_pair(PIKA_DATA_LABEL, g_pika_data_fields),
          std::make_pair(PIKA_LOG_LABEL, g_pika_log_fields),
          std::make_pair(PIKA_CLIENTS_LABEL, g_pika_client_fields),
          std::make_pair(PIKA_HUB_LABEL, g_pika_hub_fields),
          std::make_pair(PIKA_STATS_LABEL, g_pika_stats_fields),
          std::make_pair(PIKA_CPU_LABEL, g_pika_cpu_fields),
          std::make_pair(PIKA_REPLICATION_LABEL, g_redis_replication_fields),
          std::make_pair(PIKA_KEYSPACE_LABEL, g_pika_key_space_fields),
          std::make_pair(PIKA_DOUBLE_MASTER_LABE, g_pika_double_master_fields)};
}

namespace pika {

ServerInfo::Server::Server::Server()
    : pika_version_(),
      pika_git_sha_(),
      pika_build_compile_date_(),
      os_(),
      arch_bits_(),
      process_id_(),
      tcp_port_(),
      thread_num_(),
      sync_thread_num_(),
      uptime_in_seconds_(),
      uptime_in_days_(),
      config_file_(),
      server_id_() {}

ServerInfo::Server::Server(const std::string& server_text) : Server() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = server_text.find(PIKA_INFO_MARKER, start)) != std::string::npos) {
    std::string line = server_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == PIKA_SERVER_VERSION_LABEL) {
      pika_version_ = value;
    } else if (field == PIKA_SERVER_GIT_SHA_LABEL) {
      pika_git_sha_ = value;
    } else if (field == PIKA_SERVER_BUILD_COMPILE_DATE_LABEL) {
      pika_build_compile_date_ = value;
    } else if (field == PIKA_SERVER_OS_LABEL) {
      os_ = value;
    } else if (field == PIKA_SERVER_ARCH_BITS_LABEL) {
      uint32_t arch_bits;
      if (common::ConvertFromString(value, &arch_bits)) {
        arch_bits_ = arch_bits;
      }
    } else if (field == PIKA_SERVER_PROCESS_ID_LABEL) {
      uint32_t process_id;
      if (common::ConvertFromString(value, &process_id)) {
        process_id_ = process_id;
      }
    } else if (field == PIKA_SERVER_TCP_PORT_LABEL) {
      uint32_t tcp_port;
      if (common::ConvertFromString(value, &tcp_port)) {
        tcp_port_ = tcp_port;
      }
    } else if (field == PIKA_SERVER_THREAD_NUM_LABEL) {
      uint32_t thread_num;
      if (common::ConvertFromString(value, &thread_num)) {
        thread_num_ = thread_num;
      }
    } else if (field == PIKA_SERVER_SYNC_THREAD_NUM_LABEL) {
      uint32_t sync_thread_num;
      if (common::ConvertFromString(value, &sync_thread_num)) {
        sync_thread_num_ = sync_thread_num;
      }
    } else if (field == PIKA_SERVER_UPTIME_IN_SECONDS_LABEL) {
      uint32_t uptime_in_seconds;
      if (common::ConvertFromString(value, &uptime_in_seconds)) {
        uptime_in_seconds_ = uptime_in_seconds;
      }
    } else if (field == PIKA_SERVER_UPTIME_IN_DAYS_LABEL) {
      uint32_t uptime_in_days;
      if (common::ConvertFromString(value, &uptime_in_days)) {
        uptime_in_days_ = uptime_in_days;
      }
    } else if (field == PIKA_SERVER_CONFIG_FILE_LABEL) {
      config_file_ = value;
    } else if (field == PIKA_SERVER_SERVER_ID_LABEL) {
      uint32_t server_id;
      if (common::ConvertFromString(value, &server_id)) {
        server_id_ = server_id;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Server::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::StringValue(pika_version_);
    case 1:
      return new common::StringValue(pika_git_sha_);
    case 2:
      return new common::StringValue(pika_build_compile_date_);
    case 3:
      return new common::StringValue(os_);
    case 4:
      return new common::FundamentalValue(arch_bits_);
    case 5:
      return new common::FundamentalValue(process_id_);
    case 6:
      return new common::FundamentalValue(tcp_port_);
    case 7:
      return new common::FundamentalValue(thread_num_);
    case 8:
      return new common::FundamentalValue(sync_thread_num_);
    case 9:
      return new common::FundamentalValue(uptime_in_seconds_);
    case 10:
      return new common::FundamentalValue(uptime_in_days_);
    case 11:
      return new common::StringValue(config_file_);
    case 12:
      return new common::FundamentalValue(server_id_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Data::Data()
    : db_size_(),
      db_size_human_(),
      compression_(),
      used_memory_(),
      used_memory_human_(),
      db_memtable_usage_(),
      db_tablereader_usage_() {}

ServerInfo::Data::Data(const std::string& data_text) : Data() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = data_text.find((PIKA_INFO_MARKER), start)) != std::string::npos) {
    std::string line = data_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == PIKA_DATA_DB_SIZE_LABEL) {
      uint32_t db_size;
      if (common::ConvertFromString(value, &db_size)) {
        db_size_ = db_size;
      }
    } else if (field == PIKA_DATA_DB_SIZE_HUMAN_LABEL) {
      db_size_human_ = value;
    } else if (field == PIKA_DATA_COMPRESSION_LABEL) {
      compression_ = value;
    } else if (field == PIKA_DATA_USED_MEMORY_LABEL) {
      uint32_t used_memory;
      if (common::ConvertFromString(value, &used_memory)) {
        used_memory_ = used_memory;
      }
    } else if (field == PIKA_DATA_USED_MEMORY_HUMAN_LABEL) {
      used_memory_human_ = value;
    } else if (field == PIKA_DATA_DB_MEMTABLE_USAGE_LABEL) {
      uint32_t db_memtable_usage;
      if (common::ConvertFromString(value, &db_memtable_usage)) {
        db_memtable_usage_ = db_memtable_usage;
      }
    } else if (field == PIKA_DATA_DB_TABLEREADER_USAGE_LABEL) {
      uint32_t db_tablereader_usage;
      if (common::ConvertFromString(value, &db_tablereader_usage)) {
        db_tablereader_usage_ = db_tablereader_usage;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Data::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(db_size_);
    case 1:
      return new common::StringValue(db_size_human_);
    case 2:
      return new common::StringValue(compression_);
    case 3:
      return new common::FundamentalValue(used_memory_);
    case 4:
      return new common::StringValue(used_memory_human_);
    case 5:
      return new common::FundamentalValue(db_memtable_usage_);
    case 6:
      return new common::FundamentalValue(db_tablereader_usage_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Log::Log()
    : log_size_(), log_size_human_(), safety_purge_(), expire_logs_days_(), expire_logs_nums_(), binlog_offset_() {}

ServerInfo::Log::Log(const std::string& log_text) : Log() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = log_text.find((PIKA_INFO_MARKER), start)) != std::string::npos) {
    std::string line = log_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == PIKA_LOG_SIZE_LABEL) {
      uint32_t log_size;
      if (common::ConvertFromString(value, &log_size)) {
        log_size_ = log_size;
      }
    } else if (field == PIKA_LOG_SIZE_HUMAN_LABEL) {
      log_size_human_ = value;
    } else if (field == PIKA_LOG_SAFETY_PURGE_LABEL) {
      safety_purge_ = value;
    } else if (field == PIKA_LOG_EXPIRE_LOGS_DAYS_LABEL) {
      uint32_t expire_logs_days;
      if (common::ConvertFromString(value, &expire_logs_days)) {
        expire_logs_days_ = expire_logs_days;
      }
    } else if (field == PIKA_LOG_EXPIRE_LOGS_NUMS_LABEL) {
      uint32_t expire_logs_nums;
      if (common::ConvertFromString(value, &expire_logs_nums)) {
        expire_logs_nums_ = expire_logs_nums;
      }
    } else if (field == PIKA_LOG_BINLOG_OFFSET_LABEL) {
      binlog_offset_ = value;
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Log::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(log_size_);
    case 1:
      return new common::StringValue(log_size_human_);
    case 2:
      return new common::StringValue(safety_purge_);
    case 3:
      return new common::FundamentalValue(expire_logs_days_);
    case 4:
      return new common::FundamentalValue(expire_logs_nums_);
    case 5:
      return new common::StringValue(binlog_offset_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Clients::Clients() : connected_clients_(0) {}

ServerInfo::Clients::Clients(const std::string& client_text) : Clients() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = client_text.find((PIKA_INFO_MARKER), start)) != std::string::npos) {
    std::string line = client_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == PIKA_CLIENTS_CONNECTED_CLIENTS_LABEL) {
      uint32_t connected_clients;
      if (common::ConvertFromString(value, &connected_clients)) {
        connected_clients_ = connected_clients;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Clients::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(connected_clients_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Hub::Hub() {}

ServerInfo::Hub::Hub(const std::string& hub_text) : Hub() {
  UNUSED(hub_text);
}

common::Value* ServerInfo::Hub::GetValueByIndex(unsigned char index) const {
  UNUSED(index);
  return nullptr;
}

ServerInfo::Stats::Stats()
    : total_connections_received_(),
      instantaneous_ops_per_sec_(),
      total_commands_processed_(),
      is_bgsaving_(),
      is_slots_reloading_(),
      is_slots_cleanuping_(),
      is_scaning_keyspace_(),
      is_compact_(),
      compact_cron_(),
      compact_interval_() {}

ServerInfo::Stats::Stats(const std::string& stats_text) : Stats() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = stats_text.find((PIKA_INFO_MARKER), start)) != std::string::npos) {
    std::string line = stats_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == PIKA_STATS_TOTAL_CONNECTIONS_RECEIVED_LABEL) {
      uint32_t total_connections_received;
      if (common::ConvertFromString(value, &total_connections_received)) {
        total_connections_received_ = total_connections_received;
      }
    } else if (field == PIKA_STATS_INSTANTANEOUS_OPS_PER_SEC_LABEL) {
      uint32_t instantaneous_ops_per_sec;
      if (common::ConvertFromString(value, &instantaneous_ops_per_sec)) {
        instantaneous_ops_per_sec_ = instantaneous_ops_per_sec;
      }
    } else if (field == PIKA_STATS_TOTAL_COMMANDS_PROCESSED_LABEL) {
      uint32_t total_commands_processed;
      if (common::ConvertFromString(value, &total_commands_processed)) {
        total_commands_processed_ = total_commands_processed;
      }
    } else if (field == PIKA_STATS_IS_BGSAVING_LABEL) {
      is_bgsaving_ = value;
    } else if (field == PIKA_STATS_IS_SLOTS_RELOADING_LABEL) {
      is_slots_reloading_ = value;
    } else if (field == PIKA_STATS_IS_SLOTS_CLEANUPING_LABEL) {
      is_slots_cleanuping_ = value;
    } else if (field == PIKA_STATS_IS_SCANING_KEYSPACE_LABEL) {
      is_scaning_keyspace_ = value;
    } else if (field == PIKA_STATS_IS_COMPACT_LABEL) {
      is_compact_ = value;
    } else if (field == PIKA_STATS_COMPACT_CRON_LABEL) {
      compact_cron_ = value;
    } else if (field == PIKA_STATS_COMPACT_INTERVAL_LABEL) {
      compact_interval_ = value;
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Stats::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(total_connections_received_);
    case 1:
      return new common::FundamentalValue(instantaneous_ops_per_sec_);
    case 2:
      return new common::FundamentalValue(instantaneous_ops_per_sec_);
    case 3:
      return new common::FundamentalValue(total_commands_processed_);
    case 4:
      return new common::StringValue(is_bgsaving_);
    case 5:
      return new common::StringValue(is_slots_reloading_);
    case 6:
      return new common::StringValue(is_slots_cleanuping_);
    case 7:
      return new common::StringValue(is_scaning_keyspace_);
    case 8:
      return new common::StringValue(is_compact_);
    case 9:
      return new common::StringValue(compact_cron_);
    case 10:
      return new common::StringValue(compact_interval_);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::Cpu::Cpu() : used_cpu_sys_(0), used_cpu_user_(0), used_cpu_sys_children_(0), used_cpu_user_children_(0) {}

ServerInfo::Cpu::Cpu(const std::string& cpu_text) : Cpu() {
  size_t pos = 0;
  size_t start = 0;
  while ((pos = cpu_text.find((PIKA_INFO_MARKER), start)) != std::string::npos) {
    std::string line = cpu_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == (PIKA_CPU_USED_CPU_SYS_LABEL)) {
      float used_cpu_sys;
      if (common::ConvertFromString(value, &used_cpu_sys)) {
        used_cpu_sys_ = used_cpu_sys;
      }
    } else if (field == PIKA_CPU_USED_CPU_USER_LABEL) {
      float used_cpu_user;
      if (common::ConvertFromString(value, &used_cpu_user)) {
        used_cpu_user_ = used_cpu_user;
      }
    } else if (field == PIKA_CPU_USED_CPU_SYS_CHILDREN_LABEL) {
      float used_cpu_sys_children;
      if (common::ConvertFromString(value, &used_cpu_sys_children)) {
        used_cpu_sys_children_ = used_cpu_sys_children;
      }
    } else if (field == PIKA_CPU_USED_CPU_USER_CHILDREN_LABEL) {
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

ServerInfo::Replication::Replication() : role_(), connected_slaves_(0) {}

ServerInfo::Replication::Replication(const std::string& replication_text) : Replication() {
  size_t pos = 0;
  size_t start = 0;

  while ((pos = replication_text.find((PIKA_INFO_MARKER), start)) != std::string::npos) {
    std::string line = replication_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == PIKA_REPLICATION_ROLE_LABEL) {
      role_ = value;
    } else if (field == PIKA_REPLICATION_CONNECTED_SLAVES_LABEL) {
      uint32_t connected_slaves;
      if (common::ConvertFromString(value, &connected_slaves)) {
        connected_slaves_ = connected_slaves;
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
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::KeySpace::KeySpace() {}

ServerInfo::KeySpace::KeySpace(const std::string& ks_text) : KeySpace() {
  UNUSED(ks_text);
}

common::Value* ServerInfo::KeySpace::GetValueByIndex(unsigned char index) const {
  UNUSED(index);
  return nullptr;
}

ServerInfo::DoubleMaster::DoubleMaster() {}

ServerInfo::DoubleMaster::DoubleMaster(const std::string& dm_text) : DoubleMaster() {
  UNUSED(dm_text);
}

common::Value* ServerInfo::DoubleMaster::GetValueByIndex(unsigned char index) const {
  UNUSED(index);
  return nullptr;
}

ServerInfo::ServerInfo() : IServerInfo(PIKA) {}

ServerInfo::ServerInfo(const Server& serv,
                       const Data& data,
                       const Log& log,
                       const Clients& clients,
                       struct Hub& hub,
                       const Stats& stats,
                       const Cpu& cpu,
                       const Replication& repl,
                       const KeySpace& key_space,
                       const DoubleMaster& double_master)
    : IServerInfo(PIKA),
      server_(serv),
      data_(data),
      log_(log),
      clients_(clients),
      hub_(hub),
      stats_(stats),
      cpu_(cpu),
      replication_(repl),
      key_space_(key_space),
      double_master_(double_master) {}

common::Value* ServerInfo::GetValueByIndexes(unsigned char property, unsigned char field) const {
  switch (property) {
    case 0:
      return server_.GetValueByIndex(field);
    case 1:
      return data_.GetValueByIndex(field);
    case 2:
      return log_.GetValueByIndex(field);
    case 3:
      return clients_.GetValueByIndex(field);
    case 4:
      return hub_.GetValueByIndex(field);
    case 5:
      return stats_.GetValueByIndex(field);
    case 6:
      return cpu_.GetValueByIndex(field);
    case 7:
      return replication_.GetValueByIndex(field);
    case 8:
      return key_space_.GetValueByIndex(field);
    case 9:
      return double_master_.GetValueByIndex(field);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Server& value) {
  return out << PIKA_SERVER_VERSION_LABEL ":" << value.pika_version_ << PIKA_INFO_MARKER
             << PIKA_SERVER_GIT_SHA_LABEL ":" << value.pika_git_sha_ << PIKA_INFO_MARKER
             << PIKA_SERVER_BUILD_COMPILE_DATE_LABEL ":" << value.pika_build_compile_date_ << PIKA_INFO_MARKER
             << PIKA_SERVER_OS_LABEL ":" << value.os_ << PIKA_INFO_MARKER << PIKA_SERVER_ARCH_BITS_LABEL ":"
             << value.arch_bits_ << PIKA_INFO_MARKER << PIKA_SERVER_PROCESS_ID_LABEL ":" << value.process_id_
             << PIKA_INFO_MARKER << PIKA_SERVER_TCP_PORT_LABEL ":" << value.tcp_port_ << PIKA_INFO_MARKER
             << PIKA_SERVER_THREAD_NUM_LABEL ":" << value.thread_num_ << PIKA_INFO_MARKER
             << PIKA_SERVER_SYNC_THREAD_NUM_LABEL ":" << value.sync_thread_num_ << PIKA_INFO_MARKER
             << PIKA_SERVER_UPTIME_IN_SECONDS_LABEL ":" << value.uptime_in_seconds_ << PIKA_INFO_MARKER
             << PIKA_SERVER_UPTIME_IN_DAYS_LABEL ":" << value.uptime_in_days_ << PIKA_INFO_MARKER
             << PIKA_SERVER_CONFIG_FILE_LABEL ":" << value.config_file_ << PIKA_INFO_MARKER
             << PIKA_SERVER_SERVER_ID_LABEL ":" << value.server_id_ << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Data& value) {
  return out << PIKA_DATA_DB_SIZE_LABEL ":" << value.db_size_ << PIKA_INFO_MARKER << PIKA_DATA_DB_SIZE_HUMAN_LABEL ":"
             << value.db_size_human_ << PIKA_INFO_MARKER << PIKA_DATA_COMPRESSION_LABEL ":" << value.compression_
             << PIKA_INFO_MARKER << PIKA_DATA_USED_MEMORY_LABEL ":" << value.used_memory_ << PIKA_INFO_MARKER
             << PIKA_DATA_USED_MEMORY_HUMAN_LABEL ":" << value.used_memory_human_ << PIKA_INFO_MARKER
             << PIKA_DATA_DB_MEMTABLE_USAGE_LABEL ":" << value.db_memtable_usage_ << PIKA_INFO_MARKER
             << PIKA_DATA_DB_TABLEREADER_USAGE_LABEL ":" << value.db_tablereader_usage_ << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Log& value) {
  return out << PIKA_LOG_SIZE_LABEL ":" << value.log_size_ << PIKA_INFO_MARKER << PIKA_LOG_SIZE_HUMAN_LABEL ":"
             << value.log_size_human_ << PIKA_INFO_MARKER << PIKA_LOG_SAFETY_PURGE_LABEL ":" << value.safety_purge_
             << PIKA_INFO_MARKER << PIKA_LOG_EXPIRE_LOGS_DAYS_LABEL ":" << value.expire_logs_days_ << PIKA_INFO_MARKER
             << PIKA_LOG_EXPIRE_LOGS_NUMS_LABEL ":" << value.expire_logs_nums_ << PIKA_INFO_MARKER
             << PIKA_LOG_BINLOG_OFFSET_LABEL ":" << value.binlog_offset_ << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Clients& value) {
  return out << PIKA_CLIENTS_CONNECTED_CLIENTS_LABEL ":" << value.connected_clients_ << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Hub& value) {
  UNUSED(value);
  return out << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Stats& value) {
  return out << PIKA_STATS_TOTAL_CONNECTIONS_RECEIVED_LABEL ":" << value.total_connections_received_ << PIKA_INFO_MARKER
             << PIKA_STATS_INSTANTANEOUS_OPS_PER_SEC_LABEL ":" << value.instantaneous_ops_per_sec_ << PIKA_INFO_MARKER
             << PIKA_STATS_TOTAL_COMMANDS_PROCESSED_LABEL ":" << value.total_commands_processed_ << PIKA_INFO_MARKER
             << PIKA_STATS_IS_BGSAVING_LABEL ":" << value.is_bgsaving_ << PIKA_INFO_MARKER
             << PIKA_STATS_IS_SLOTS_RELOADING_LABEL ":" << value.is_slots_reloading_ << PIKA_INFO_MARKER
             << PIKA_STATS_IS_SLOTS_CLEANUPING_LABEL ":" << value.is_slots_cleanuping_ << PIKA_INFO_MARKER
             << PIKA_STATS_IS_SCANING_KEYSPACE_LABEL ":" << value.is_scaning_keyspace_ << PIKA_INFO_MARKER
             << PIKA_STATS_IS_COMPACT_LABEL ":" << value.is_compact_ << PIKA_INFO_MARKER
             << PIKA_STATS_COMPACT_CRON_LABEL ":" << value.compact_cron_ << PIKA_INFO_MARKER
             << PIKA_STATS_COMPACT_INTERVAL_LABEL ":" << value.compact_interval_ << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Cpu& value) {
  return out << PIKA_CPU_USED_CPU_SYS_LABEL ":" << value.used_cpu_sys_ << PIKA_INFO_MARKER
             << PIKA_CPU_USED_CPU_USER_LABEL ":" << value.used_cpu_user_ << PIKA_INFO_MARKER
             << PIKA_CPU_USED_CPU_SYS_CHILDREN_LABEL ":" << value.used_cpu_sys_children_ << PIKA_INFO_MARKER
             << PIKA_CPU_USED_CPU_USER_CHILDREN_LABEL ":" << value.used_cpu_user_children_ << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Replication& value) {
  return out << PIKA_REPLICATION_ROLE_LABEL ":" << value.role_ << PIKA_INFO_MARKER
             << PIKA_REPLICATION_CONNECTED_SLAVES_LABEL ":" << value.connected_slaves_ << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::KeySpace& value) {
  UNUSED(value);
  return out << PIKA_INFO_MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::DoubleMaster& value) {
  UNUSED(value);
  return out << PIKA_INFO_MARKER;
}

std::string ServerInfo::ToString() const {
  std::stringstream str;
  str << PIKA_SERVER_LABEL PIKA_INFO_MARKER << server_ << PIKA_DATA_LABEL PIKA_INFO_MARKER << data_
      << PIKA_LOG_LABEL PIKA_INFO_MARKER << log_ << PIKA_CLIENTS_LABEL PIKA_INFO_MARKER << clients_
      << PIKA_HUB_LABEL PIKA_INFO_MARKER << hub_ << PIKA_STATS_LABEL PIKA_INFO_MARKER << stats_
      << PIKA_CPU_LABEL PIKA_INFO_MARKER << cpu_ << PIKA_REPLICATION_LABEL PIKA_INFO_MARKER << replication_
      << PIKA_KEYSPACE_LABEL PIKA_INFO_MARKER << key_space_ << PIKA_DOUBLE_MASTER_LABE PIKA_INFO_MARKER
      << double_master_ << PIKA_INFO_MARKER;
  return str.str();
}

uint32_t ServerInfo::GetVersion() const {
  return common::ConvertVersionNumberFromString(server_.pika_version_);
}

std::ostream& operator<<(std::ostream& out, const ServerInfo& value) {
  return out << value.ToString();
}

ServerInfo* MakePikaServerInfo(const std::string& content) {
  if (content.empty()) {
    return nullptr;
  }

  ServerInfo* result = new ServerInfo;
  size_t j = 0;
  std::string word;
  size_t pos = 0;
  static const std::vector<core::info_field_t> fields = DBTraits<PIKA>::GetInfoFields();
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
            result->data_ = ServerInfo::Data(part);
            break;
          case 2:
            result->log_ = ServerInfo::Log(part);
            break;
          case 3:
            result->clients_ = ServerInfo::Clients(part);
            break;
          case 4:
            result->hub_ = ServerInfo::Hub(part);
            break;
          case 5:
            result->stats_ = ServerInfo::Stats(part);
            break;
          case 6:
            result->cpu_ = ServerInfo::Cpu(part);
          case 7:
            result->replication_ = ServerInfo::Replication(part);
          case 8:
            result->key_space_ = ServerInfo::KeySpace(part);
          case 9:
            result->double_master_ = ServerInfo::DoubleMaster(part);
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

}  // namespace pika
}  // namespace core
}  // namespace fastonosql
