#pragma once

#include "core/connection_settings.h"

#include "core/redis/redis_config.h"

namespace fastoredis
{
    class RedisClusterSettings
            : public IClusterSettingsBase
    {
    public:
        RedisClusterSettings(const std::string& connectionName);
        virtual IConnectionSettings* clone() const;
    };
}
