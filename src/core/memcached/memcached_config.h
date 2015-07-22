#pragma once

#include <vector>

#include "common/convert2string.h"

#define REDIS_CLI_DEFAULT_PIPE_TIMEOUT 30 /* seconds */

namespace fastonosql
{
    struct memcachedConfig
    {
        memcachedConfig();
        memcachedConfig(const memcachedConfig& other);
        memcachedConfig& operator=(const memcachedConfig &other);

        ~memcachedConfig();

        char* hostip_;
        int hostport_;

        char* user_;
        char* password_;

        char* mb_delim_;
        int shutdown_;

    private:
        void copy(const memcachedConfig& other);
        void init();
    };
}

namespace common
{
    std::string convertToString(const fastonosql::memcachedConfig &conf);
}
