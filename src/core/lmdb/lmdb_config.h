#pragma once

#include "common/convert2string.h"

#include "core/connection_confg.h"

namespace fastonosql
{
    struct lmdbConfig
            : public LocalConfig
    {
        lmdbConfig();

        bool create_if_missing_;
    };
}

namespace common
{
    std::string convertToString(const fastonosql::lmdbConfig &conf);
}
