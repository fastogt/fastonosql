#pragma once

#include "core/connection_confg.h"

#include "common/convert2string.h"

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
