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

#pragma once

#include "core/server/iserver_info.h"  // for IStateField, IServerInfo

#define PIKA_SERVER_LABEL "# Server"
#define PIKA_DATA_LABEL "# Data"
#define PIKA_LOG_LABEL "# Log"
#define PIKA_CLIENTS_LABEL "# Clients"
#define PIKA_HUB_LABEL "# Hub"
#define PIKA_STATS_LABEL "# Stats"
#define PIKA_CPU_LABEL "# CPU"
#define PIKA_REPLICATION_LABEL "# Replication"
#define PIKA_KEYSPACE_LABEL "# Keyspace"
#define PIKA_DOUBLE_MASTER_LABE "# DoubleMaster"

// Server
/*
pika_version:2.3.2
pika_git_sha:4fbd39ed7a29da324b6f39ab236a1004d8c1c21c
pika_build_compile_date: Jan 23 2018
os:Linux 4.4.0-72-generic x86_64
arch_bits:64
process_id:4839
tcp_port:9221
thread_num:1
sync_thread_num:6
uptime_in_seconds:421
uptime_in_days:1
config_file:./conf/pika.conf
server_id:1
*/
#define PIKA_SERVER_VERSION_LABEL "pika_version"
#define PIKA_SERVER_GIT_SHA_LABEL "pika_git_sha"
#define PIKA_SERVER_BUILD_COMPILE_DATE_LABEL "pika_build_compile_date"
#define PIKA_SERVER_OS_LABEL "os"
#define PIKA_SERVER_ARCH_BITS_LABEL "arch_bits"
#define PIKA_SERVER_PROCESS_ID_LABEL "process_id"
#define PIKA_SERVER_TCP_PORT_LABEL "tcp_port"
#define PIKA_SERVER_THREAD_NUM_LABEL "thread_num"
#define PIKA_SERVER_SYNC_THREAD_NUM_LABEL "sync_thread_num"
#define PIKA_SERVER_UPTIME_IN_SECONDS_LABEL "uptime_in_seconds"
#define PIKA_SERVER_UPTIME_IN_DAYS_LABEL "uptime_in_days"
#define PIKA_SERVER_CONFIG_FILE_LABEL "config_file"
#define PIKA_SERVER_SERVER_ID_LABEL "server_id"

// Data
/*
db_size:235525
db_size_human:0M
compression:snappy
used_memory:4264
used_memory_human:0M
db_memtable_usage:4144
db_tablereader_usage:120
*/
#define PIKA_DATA_DB_SIZE_LABEL "db_size"
#define PIKA_DATA_DB_SIZE_HUMAN_LABEL "db_size_human"
#define PIKA_DATA_COMPRESSION_LABEL "compression"
#define PIKA_DATA_USED_MEMORY_LABEL "used_memory"
#define PIKA_DATA_USED_MEMORY_HUMAN_LABEL "used_memory_human"
#define PIKA_DATA_DB_MEMTABLE_USAGE_LABEL "db_memtable_usage"
#define PIKA_DATA_DB_TABLEREADER_USAGE_LABEL "db_tablereader_usage"

// Log
/*
log_size:73990
log_size_human:0M
safety_purge:none
expire_logs_days:7
expire_logs_nums:10
binlog_offset:0 200
*/
#define PIKA_LOG_SIZE_LABEL "log_size"
#define PIKA_LOG_SIZE_HUMAN_LABEL "log_size_human"
#define PIKA_LOG_SAFETY_PURGE_LABEL "safety_purge"
#define PIKA_LOG_EXPIRE_LOGS_DAYS_LABEL "expire_logs_days"
#define PIKA_LOG_EXPIRE_LOGS_NUMS_LABEL "expire_logs_nums"
#define PIKA_LOG_BINLOG_OFFSET_LABEL "binlog_offset"

// Clients
/*
connected_clients:1
*/
#define PIKA_CLIENTS_CONNECTED_CLIENTS_LABEL "connected_clients"

// Stats
/*
total_connections_received:5
instantaneous_ops_per_sec:0
total_commands_processed:21
is_bgsaving:No, , 0
is_slots_reloading:No, , 0
is_slots_cleanuping:No, , 0
is_scaning_keyspace:No
is_compact:No
compact_cron:
compact_interval:
*/
#define PIKA_STATS_TOTAL_CONNECTIONS_RECEIVED_LABEL "total_connections_received"
#define PIKA_STATS_INSTANTANEOUS_OPS_PER_SEC_LABEL "instantaneous_ops_per_sec"
#define PIKA_STATS_TOTAL_COMMANDS_PROCESSED_LABEL "total_commands_processed"
#define PIKA_STATS_IS_BGSAVING_LABEL "is_bgsaving"
#define PIKA_STATS_IS_SLOTS_RELOADING_LABEL "is_slots_reloading"
#define PIKA_STATS_IS_SLOTS_CLEANUPING_LABEL "is_slots_cleanuping"
#define PIKA_STATS_IS_SCANING_KEYSPACE_LABEL "is_scaning_keyspace"
#define PIKA_STATS_IS_COMPACT_LABEL "is_compact"
#define PIKA_STATS_COMPACT_CRON_LABEL "compact_cron"
#define PIKA_STATS_COMPACT_INTERVAL_LABEL "compact_interval"

// CPU
/*
used_cpu_sys:0.06
used_cpu_user:0.07
used_cpu_sys_children:0.00
used_cpu_user_children:0.00
*/
#define PIKA_CPU_USED_CPU_SYS_LABEL "used_cpu_sys"
#define PIKA_CPU_USED_CPU_USER_LABEL "used_cpu_user"
#define PIKA_CPU_USED_CPU_SYS_CHILDREN_LABEL "used_cpu_sys_children"
#define PIKA_CPU_USED_CPU_USER_CHILDREN_LABEL "used_cpu_user_children"

// Replication
/*
role:master
connected_slaves:0
*/
#define PIKA_REPLICATION_ROLE_LABEL "role"
#define PIKA_REPLICATION_CONNECTED_SLAVES_LABEL "connected_slaves"

// Keyspace
/*
kv keys:2
hash keys:0
list keys:0
zset keys:0
set keys:0
*/
#define PIKA_KEYSPACE_KV_KEYS_LABEL "kv keys"
#define PIKA_KEYSPACE_HASH_KEYS_LABEL "hash keys"
#define PIKA_KEYSPACE_LIST_KEYS_LABEL "list keys"
#define PIKA_KEYSPACE_ZSET_KEYS_LABEL "zset keys"
#define PIKA_KEYSPACE_SET_KEYS_LABEL "set keys"

namespace fastonosql {
namespace core {
namespace pika {

class ServerInfo : public IServerInfo {
 public:
  struct Server : IStateField {
    Server();
    explicit Server(const std::string& server_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    std::string pika_version_;
    std::string pika_git_sha_;
    std::string pika_build_compile_date_;
    std::string os_;
    uint32_t arch_bits_;
    uint32_t process_id_;
    uint32_t tcp_port_;
    uint32_t thread_num_;
    uint32_t sync_thread_num_;
    uint32_t uptime_in_seconds_;
    uint32_t uptime_in_days_;
    std::string config_file_;
    uint32_t server_id_;
  } server_;

  struct Data : IStateField {
    Data();
    explicit Data(const std::string& data_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t db_size_;
    std::string db_size_human_;
    std::string compression_;
    uint32_t used_memory_;
    std::string used_memory_human_;
    uint32_t db_memtable_usage_;
    uint32_t db_tablereader_usage_;
  } data_;

  struct Log : IStateField {
    Log();
    explicit Log(const std::string& client_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t log_size_;
    std::string log_size_human_;
    std::string safety_purge_;
    uint32_t expire_logs_days_;
    uint32_t expire_logs_nums_;
    std::string binlog_offset_;
  } log_;

  struct Clients : IStateField {
    Clients();
    explicit Clients(const std::string& client_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t connected_clients_;
  } clients_;

  struct Hub : IStateField {
    Hub();
    explicit Hub(const std::string& hub_text);
    common::Value* GetValueByIndex(unsigned char index) const override;
  } hub_;

  struct Stats : IStateField {
    Stats();
    explicit Stats(const std::string& stats_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t total_connections_received_;
    uint32_t instantaneous_ops_per_sec_;
    uint32_t total_commands_processed_;
    std::string is_bgsaving_;
    std::string is_slots_reloading_;
    std::string is_slots_cleanuping_;
    std::string is_scaning_keyspace_;
    std::string is_compact_;
    std::string compact_cron_;
    std::string compact_interval_;
  } stats_;

  struct Cpu : IStateField {
    Cpu();
    explicit Cpu(const std::string& cpu_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    float used_cpu_sys_;
    float used_cpu_user_;
    float used_cpu_sys_children_;
    float used_cpu_user_children_;
  } cpu_;

  struct Replication : IStateField {
    Replication();
    explicit Replication(const std::string& replication_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    std::string role_;
    uint32_t connected_slaves_;
  } replication_;

  struct KeySpace : IStateField {
    KeySpace();
    explicit KeySpace(const std::string& ks_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t kv_;
    uint32_t hash_;
    uint32_t list_;
    uint32_t zset_;
    uint32_t set_;
  } key_space_;

  struct DoubleMaster : IStateField {
    DoubleMaster();
    explicit DoubleMaster(const std::string& dm_text);
    common::Value* GetValueByIndex(unsigned char index) const override;
  } double_master_;

  ServerInfo();
  ServerInfo(const Server& serv,
             const Data& data,
             const Log& log,
             const Clients& clients,
             struct Hub& hub,
             const Stats& stats,
             const Cpu& cpu,
             const Replication& repl,
             const KeySpace& key_space,
             const DoubleMaster& double_master);

  virtual common::Value* GetValueByIndexes(unsigned char property, unsigned char field) const override;
  virtual std::string ToString() const override;
  virtual uint32_t GetVersion() const override;
};

std::ostream& operator<<(std::ostream& out, const ServerInfo& value);

ServerInfo* MakePikaServerInfo(const std::string& content);

}  // namespace pika
}  // namespace core
}  // namespace fastonosql
