#pragma once

#include <rocksdb/options.h>

#include "common/convert2string.h"

#include "core/connection_confg.h"

namespace fastonosql
{
    struct rocksdbConfig
            : public LocalConfig
    {
        rocksdbConfig();

        rocksdb::Options options_;
    };
}

namespace common
{
    std::string convertToString(const fastonosql::rocksdbConfig &conf);
}
