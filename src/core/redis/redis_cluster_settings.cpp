#include "core/redis/redis_cluster_settings.h"

namespace fastonosql
{
    RedisClusterSettings::RedisClusterSettings(const std::string& connectionName)
        : IClusterSettingsBase(connectionName, REDIS)
    {

    }

    IConnectionSettings* RedisClusterSettings::clone() const
    {
        RedisClusterSettings *red = new RedisClusterSettings(*this);
        return red;
    }
}
