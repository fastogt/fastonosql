#pragma once

#include <vector>

#include "common/convert2string.h"

#define REDIS_CLI_DEFAULT_PIPE_TIMEOUT 30 /* seconds */

namespace fastoredis
{
    struct redisConfig
    {
        redisConfig();
        redisConfig(const redisConfig& other);
        redisConfig& operator=(const redisConfig &other);

        ~redisConfig();

        char *hostip;
        int hostport;
        char *hostsocket;
        long repeat;
        long interval;
        int dbnum;
        int interactive;
        int shutdown;
        int monitor_mode;
        int pubsub_mode;
        int latency_mode;
        int latency_history;
        int cluster_mode;
        int cluster_reissue_command;
        int slave_mode;
        //int pipe_mode;
        //int pipe_timeout;
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
        char *mb_delim;
        int last_cmd_type;

    private:
        void copy(const redisConfig& other);
        void init();
    };
}

namespace common
{
    std::string convertToString(const fastoredis::redisConfig &conf);
}
