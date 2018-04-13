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

#define REDIS_SERVER_LABEL "# Server"
#define REDIS_CLIENTS_LABEL "# Clients"
#define REDIS_MEMORY_LABEL "# Memory"
#define REDIS_PERSISTENCE_LABEL "# Persistence"
#define REDIS_STATS_LABEL "# Stats"
#define REDIS_REPLICATION_LABEL "# Replication"
#define REDIS_CPU_LABEL "# CPU"
#define REDIS_KEYSPACE_LABEL "# Keyspace"

// Server
#define REDIS_SERVER_VERSION_LABEL "redis_version"
#define REDIS_SERVER_GIT_SHA1_LABEL "redis_git_sha1"
#define REDIS_SERVER_GIT_DIRTY_LABEL "redis_git_dirty"
#define REDIS_SERVER_BUILD_ID_LABEL "redis_build_id"
#define REDIS_SERVER_MODE_LABEL "redis_mode"
#define REDIS_SERVER_OS_LABEL "os"
#define REDIS_SERVER_ARCH_BITS_LABEL "arch_bits"
#define REDIS_SERVER_MULTIPLEXING_API_LABEL "multiplexing_api"
#define REDIS_SERVER_GCC_VERSION_LABEL "gcc_version"
#define REDIS_SERVER_PROCESS_ID_LABEL "process_id"
#define REDIS_SERVER_RUN_ID_LABEL "run_id"
#define REDIS_SERVER_TCP_PORT_LABEL "tcp_port"
#define REDIS_SERVER_UPTIME_IN_SECONDS_LABEL "uptime_in_seconds"
#define REDIS_SERVER_UPTIME_IN_DAYS_LABEL "uptime_in_days"
#define REDIS_SERVER_HZ_LABEL "hz"
#define REDIS_SERVER_LRU_CLOCK_LABEL "lru_clock"

// Clients
#define REDIS_CLIENTS_CONNECTED_CLIENTS_LABEL "connected_clients"
#define REDIS_CLIENTS_CLIENT_LONGEST_OUTPUT_LIST_LABEL "client_longest_output_list"
#define REDIS_CLIENTS_CLIENT_BIGGEST_INPUT_BUF_LABEL "client_biggest_input_buf"
#define REDIS_CLIENTS_BLOCKED_CLIENTS_LABEL "blocked_clients"

// Memory
#define REDIS_MEMORY_USED_MEMORY_LABEL "used_memory"
#define REDIS_MEMORY_USED_MEMORY_HUMAN_LABEL "used_memory_human"
#define REDIS_MEMORY_USED_MEMORY_RSS_LABEL "used_memory_rss"
#define REDIS_MEMORY_USED_MEMORY_PEAK_LABEL "used_memory_peak"
#define REDIS_MEMORY_USED_MEMORY_PEAK_HUMAN_LABEL "used_memory_peak_human"
#define REDIS_MEMORY_USED_MEMORY_LUA_LABEL "used_memory_lua"
#define REDIS_MEMORY_MEM_FRAGMENTATION_RATIO_LABEL "mem_fragmentation_ratio"
#define REDIS_MEMORY_MEM_ALLOCATOR_LABEL "mem_allocator"

// Persistence
#define REDIS_PERSISTENCE_LOADING_LABEL "loading"
#define REDIS_PERSISTENCE_RDB_CHANGES_SINCE_LAST_SAVE_LABEL "rdb_changes_since_last_save"
#define REDIS_PERSISTENCE_RDB_DGSAVE_IN_PROGRESS_LABEL "rdb_bgsave_in_progress"
#define REDIS_PERSISTENCE_RDB_LAST_SAVE_TIME_LABEL "rdb_last_save_time"
#define REDIS_PERSISTENCE_RDB_LAST_DGSAVE_STATUS_LABEL "rdb_last_bgsave_status"
#define REDIS_PERSISTENCE_RDB_LAST_DGSAVE_TIME_SEC_LABEL "rdb_last_bgsave_time_sec"
#define REDIS_PERSISTENCE_RDB_CURRENT_DGSAVE_TIME_SEC_LABEL "rdb_current_bgsave_time_sec"
#define REDIS_PERSISTENCE_AOF_ENABLED_LABEL "aof_enabled"
#define REDIS_PERSISTENCE_AOF_REWRITE_IN_PROGRESS_LABEL "aof_rewrite_in_progress"
#define REDIS_PERSISTENCE_AOF_REWRITE_SHEDULED_LABEL "aof_rewrite_scheduled"
#define REDIS_PERSISTENCE_AOF_LAST_REWRITE_TIME_SEC_LABEL "aof_last_rewrite_time_sec"
#define REDIS_PERSISTENCE_AOF_CURRENT_REWRITE_TIME_SEC_LABEL "aof_current_rewrite_time_sec"
#define REDIS_PERSISTENCE_AOF_LAST_DGREWRITE_STATUS_LABEL "aof_last_bgrewrite_status"
#define REDIS_PERSISTENCE_AOF_LAST_WRITE_STATUS_LABEL "aof_last_write_status"

// Stats
#define REDIS_STATS_TOTAL_CONNECTIONS_RECEIVED_LABEL "total_connections_received"
#define REDIS_STATS_TOTAL_COMMANDS_PROCESSED_LABEL "total_commands_processed"
#define REDIS_STATS_INSTANTANEOUS_OPS_PER_SEC_LABEL "instantaneous_ops_per_sec"
#define REDIS_STATS_REJECTED_CONNECTIONS_LABEL "rejected_connections"
#define REDIS_STATS_SYNC_FULL_LABEL "sync_full"
#define REDIS_STATS_SYNC_PARTIAL_OK_LABEL "sync_partial_ok"
#define REDIS_STATS_SYNC_PARTIAL_ERR_LABEL "sync_partial_err"
#define REDIS_STATS_EXPIRED_KEYS_LABEL "expired_keys"
#define REDIS_STATS_EVICTED_KEYS_LABEL "evicted_keys"
#define REDIS_STATS_KEYSPACE_HITS_LABEL "keyspace_hits"
#define REDIS_STATS_KEYSPACE_MISSES_LABEL "keyspace_misses"
#define REDIS_STATS_PUBSUB_CHANNELS_LABEL "pubsub_channels"
#define REDIS_STATS_PUBSUB_PATTERNS_LABEL "pubsub_patterns"
#define REDIS_STATS_LATEST_FORK_USEC_LABEL "latest_fork_usec"

// Replication
#define REDIS_REPLICATION_ROLE_LABEL "role"
#define REDIS_REPLICATION_CONNECTED_SLAVES_LABEL "connected_slaves"
#define REDIS_REPLICATION_MASTER_REPL_OFFSET_LABEL "master_repl_offset"
#define REDIS_REPLICATION_BACKLOG_ACTIVE_LABEL "repl_backlog_active"
#define REDIS_REPLICATION_BACKLOG_SIZE_LABEL "repl_backlog_size"
#define REDIS_REPLICATION_BACKLOG_FIRST_BYTE_OFFSET_LABEL "repl_backlog_first_byte_offset"
#define REDIS_REPLICATION_BACKLOG_HISTEN_LABEL "repl_backlog_histlen"

// CPU
#define REDIS_CPU_USED_CPU_SYS_LABEL "used_cpu_sys"
#define REDIS_CPU_USED_CPU_USER_LABEL "used_cpu_user"
#define REDIS_CPU_USED_CPU_SYS_CHILDREN_LABEL "used_cpu_sys_children"
#define REDIS_CPU_USED_CPU_USER_CHILDREN_LABEL "used_cpu_user_children"

namespace fastonosql {
namespace core {
namespace redis {

class ServerInfo : public IServerInfo {
 public:
  struct Server : IStateField {
    Server();
    explicit Server(const std::string& server_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    std::string redis_version_;
    std::string redis_git_sha1_;
    std::string redis_git_dirty_;
    std::string redis_build_id_;
    std::string redis_mode_;
    std::string os_;
    uint32_t arch_bits_;
    std::string multiplexing_api_;
    std::string gcc_version_;
    uint32_t process_id_;
    std::string run_id_;
    uint32_t tcp_port_;
    uint32_t uptime_in_seconds_;
    uint32_t uptime_in_days_;
    uint32_t hz_;
    uint32_t lru_clock_;
  } server_;

  struct Clients : IStateField {
    Clients();
    explicit Clients(const std::string& client_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t connected_clients_;
    uint32_t client_longest_output_list_;
    uint32_t client_biggest_input_buf_;
    uint32_t blocked_clients_;
  } clients_;

  struct Memory : IStateField {
    Memory();
    explicit Memory(const std::string& memory_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t used_memory_;
    std::string used_memory_human_;
    uint32_t used_memory_rss_;
    uint32_t used_memory_peak_;
    std::string used_memory_peak_human_;
    uint32_t used_memory_lua_;
    float mem_fragmentation_ratio_;
    std::string mem_allocator_;
  } memory_;

  struct Persistence : IStateField {
    Persistence();
    explicit Persistence(const std::string& persistence_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t loading_;
    uint32_t rdb_changes_since_last_save_;
    uint32_t rdb_bgsave_in_progress_;
    uint32_t rdb_last_save_time_;
    std::string rdb_last_bgsave_status_;
    int rdb_last_bgsave_time_sec_;
    int rdb_current_bgsave_time_sec_;
    uint32_t aof_enabled_;
    uint32_t aof_rewrite_in_progress_;
    uint32_t aof_rewrite_scheduled_;
    int aof_last_rewrite_time_sec_;
    int aof_current_rewrite_time_sec_;
    std::string aof_last_bgrewrite_status_;
    std::string aof_last_write_status_;
  } persistence_;

  struct Stats : IStateField {
    Stats();
    explicit Stats(const std::string& stats_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    uint32_t total_connections_received_;
    uint32_t total_commands_processed_;
    uint32_t instantaneous_ops_per_sec_;
    uint32_t rejected_connections_;
    uint32_t sync_full_;
    uint32_t sync_partial_ok_;
    uint32_t sync_partial_err_;
    uint32_t expired_keys_;
    uint32_t evicted_keys_;
    uint32_t keyspace_hits_;
    uint32_t keyspace_misses_;
    uint32_t pubsub_channels_;
    uint32_t pubsub_patterns_;
    uint32_t latest_fork_usec_;
  } stats_;

  struct Replication : IStateField {
    Replication();
    explicit Replication(const std::string& replication_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    std::string role_;
    uint32_t connected_slaves_;
    uint32_t master_repl_offset_;
    uint32_t backlog_active_;
    uint32_t backlog_size_;
    uint32_t backlog_first_byte_offset_;
    uint32_t backlog_histen_;
  } replication_;

  struct Cpu : IStateField {
    Cpu();
    explicit Cpu(const std::string& cpu_text);
    common::Value* GetValueByIndex(unsigned char index) const override;

    float used_cpu_sys_;
    float used_cpu_user_;
    float used_cpu_sys_children_;
    float used_cpu_user_children_;
  } cpu_;

  struct Keyspace : IStateField {
    common::Value* GetValueByIndex(unsigned char index) const override;
  } keySp_;

  ServerInfo();
  ServerInfo(const Server& serv,
             const Clients& clients,
             const Memory& memory,
             const Persistence& pers,
             const Stats& stats,
             const Replication& repl,
             const Cpu& cpu,
             const Keyspace& key);

  virtual common::Value* GetValueByIndexes(unsigned char property, unsigned char field) const override;
  virtual std::string ToString() const override;
  virtual uint32_t GetVersion() const override;
};

std::ostream& operator<<(std::ostream& out, const ServerInfo& value);

ServerInfo* MakeRedisServerInfo(const std::string& content);

}  // namespace redis
}  // namespace core
}  // namespace fastonosql
