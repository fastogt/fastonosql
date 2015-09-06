#pragma once

#include "common/convert2string.h"

#include "core/connection_confg.h"

namespace fastonosql
{
    struct unqliteConfig
            : public ConnectionConfig
    {
        unqliteConfig();
        unqliteConfig(const unqliteConfig& other);
        unqliteConfig& operator=(const unqliteConfig &other);

        ~unqliteConfig();

        std::string dbname_;

    protected:
        void copy(const unqliteConfig& other);
    };
}

namespace common
{
    std::string convertToString(const fastonosql::unqliteConfig &conf);
}
