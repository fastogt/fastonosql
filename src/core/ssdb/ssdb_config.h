#pragma once

#include "core/connection_confg.h"

#include "common/convert2string.h"

#define REDIS_CLI_DEFAULT_PIPE_TIMEOUT 30 /* seconds */

namespace fastonosql
{
    struct ssdbConfig
            : public ConnectionConfig
    {
        ssdbConfig();
        ssdbConfig(const ssdbConfig& other);
        ssdbConfig& operator=(const ssdbConfig &other);

        ~ssdbConfig();

        char* user_;
        char* password_;

    private:
        void copy(const ssdbConfig& other);
    };
}

namespace common
{
    std::string convertToString(const fastonosql::ssdbConfig &conf);
}
