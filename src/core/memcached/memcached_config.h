#pragma once

#include "common/convert2string.h"

#include "core/connection_confg.h"

namespace fastonosql
{
    struct memcachedConfig
            : public ConnectionConfig
    {
        memcachedConfig();
        memcachedConfig(const memcachedConfig& other);
        memcachedConfig& operator=(const memcachedConfig &other);

        ~memcachedConfig();

        char* user_;
        char* password_;

    protected:
        void copy(const memcachedConfig& other);
    };
}

namespace common
{
    std::string convertToString(const fastonosql::memcachedConfig &conf);
}
