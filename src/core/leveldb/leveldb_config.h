#pragma once

#include <vector>

#include "common/convert2string.h"

#define REDIS_CLI_DEFAULT_PIPE_TIMEOUT 30 /* seconds */

namespace fastonosql
{
    struct leveldbConfig
    {
        leveldbConfig();
        leveldbConfig(const leveldbConfig& other);
        leveldbConfig& operator=(const leveldbConfig &other);

        ~leveldbConfig();

        char *hostip_;
        int hostport_;

        char* user_;
        char* password_;

        char *mb_delim_;
        int shutdown_;

    private:
        void copy(const leveldbConfig& other);
        void init();
    };
}

namespace common
{
    std::string convertToString(const fastonosql::leveldbConfig &conf);
}
