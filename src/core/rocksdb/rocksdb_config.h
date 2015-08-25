#pragma once

#include "common/convert2string.h"

#include "core/connection_confg.h"

namespace fastonosql
{
    struct rocksdbConfig
            : public ConnectionConfig
    {
        rocksdbConfig();
        rocksdbConfig(const rocksdbConfig& other);
        rocksdbConfig& operator=(const rocksdbConfig &other);

        ~rocksdbConfig();

        std::string dbname_;
        bool create_if_missing_;

    protected:
        void copy(const rocksdbConfig& other);
    };
}

namespace common
{
    std::string convertToString(const fastonosql::rocksdbConfig &conf);
}
