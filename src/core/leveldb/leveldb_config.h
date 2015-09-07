#pragma once

#include <leveldb/options.h>

#include "common/convert2string.h"

#include "core/connection_confg.h"

namespace fastonosql
{
    struct leveldbConfig
            : public LocalConfig
    {
        leveldbConfig();

        leveldb::Options options_;
    };
}

namespace common
{
    std::string convertToString(const fastonosql::leveldbConfig &conf);
}
