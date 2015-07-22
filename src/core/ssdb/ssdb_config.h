#pragma once

#include <vector>

#include "common/convert2string.h"

#define REDIS_CLI_DEFAULT_PIPE_TIMEOUT 30 /* seconds */

namespace fastonosql
{
    struct ssdbConfig
    {
        ssdbConfig();
        ssdbConfig(const ssdbConfig& other);
        ssdbConfig& operator=(const ssdbConfig &other);

        ~ssdbConfig();

        char *hostip_;
        int hostport_;

        char* user_;
        char* password_;

        char *mb_delim_;
        int shutdown_;

    private:
        void copy(const ssdbConfig& other);
        void init();
    };
}

namespace common
{
    std::string convertToString(const fastonosql::ssdbConfig &conf);
}
