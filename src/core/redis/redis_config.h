#pragma once

#include "common/convert2string.h"

#include "core/connection_confg.h"

namespace fastonosql
{
    struct redisConfig
            : public ConnectionConfig
    {
        redisConfig();
        redisConfig(const redisConfig& other);
        redisConfig& operator=(const redisConfig &other);

        ~redisConfig();

        char *hostsocket;
        long repeat;
        long interval;
        int dbnum;
        int interactive;
        int monitor_mode;
        int pubsub_mode;
        int latency_mode;
        int latency_history;
        int cluster_mode;
        int cluster_reissue_command;
        int slave_mode;
        int getrdb_mode;
        int stat_mode;
        int scan_mode;
        int intrinsic_latency_mode;
        int intrinsic_latency_duration;
        char *pattern;
        char *rdb_filename;
        int bigkeys;
        char *auth;
        char *eval;
        int last_cmd_type;

    protected:
        void copy(const redisConfig& other);
        void init();
    };
}

namespace common
{
    std::string convertToString(const fastonosql::redisConfig &conf);
}
