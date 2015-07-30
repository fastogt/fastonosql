#pragma once

#include <leveldb/options.h>

#include "common/convert2string.h"

#include "core/connection_confg.h"


namespace fastonosql
{
    struct leveldbConfig
            : public ConnectionConfig
    {
        leveldbConfig();
        leveldbConfig(const leveldbConfig& other);
        leveldbConfig& operator=(const leveldbConfig &other);

        ~leveldbConfig();

        leveldb::Options options_;
        std::string dbname_;

    protected:
        void copy(const leveldbConfig& other);
    };
}

namespace common
{
    std::string convertToString(const fastonosql::leveldbConfig &conf);
}
