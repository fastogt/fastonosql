#pragma once

#include "core/connection_confg.h"

#include "common/convert2string.h"

namespace fastonosql
{
    struct ssdbConfig
            : public ConnectionConfig
    {
        ssdbConfig();

        std::string user_;
        std::string password_;
    };
}

namespace common
{
    std::string convertToString(const fastonosql::ssdbConfig &conf);
}
