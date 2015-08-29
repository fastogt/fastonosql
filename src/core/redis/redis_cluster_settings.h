#pragma once

#include "core/connection_settings.h"

#include "core/redis/redis_config.h"

namespace fastonosql
{
    class RedisClusterSettings
            : public IClusterSettingsBase
    {
    public:
        explicit RedisClusterSettings(const std::string& connectionName);
        virtual IConnectionSettings* clone() const;
    };
}
