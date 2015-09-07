#pragma once

#include "common/convert2string.h"

#include "core/connection_confg.h"

namespace fastonosql
{
    struct memcachedConfig
            : public RemoteConfig
    {
        memcachedConfig();

        std::string user_;
        std::string password_;
    };
}

namespace common
{
    std::string convertToString(const fastonosql::memcachedConfig &conf);
}
