#include "core/redis/redis_settings.h"

#include "common/utils.h"

namespace fastonosql
{
    RedisConnectionSettings::RedisConnectionSettings(const std::string &connectionName)
        : IConnectionSettingsBase(connectionName, REDIS), info_()
    {

    }

    void RedisConnectionSettings::setHost(const common::net::hostAndPort& host)
    {
        info_.hostip_ = host.host_;
        info_.hostport_ = host.port_;
    }

    common::net::hostAndPort RedisConnectionSettings::host() const
    {
        return common::net::hostAndPort(info_.hostip_, info_.hostport_);
    }

    void RedisConnectionSettings::initFromCommandLine(const std::string &val)
    {
        info_ = common::convertFromString<redisConfig>(val);
    }

    std::string RedisConnectionSettings::toCommandLine() const
    {
        std::string result = common::convertToString(info_);
        return result;
    }

    void RedisConnectionSettings::setCommandLine(const std::string &line)
    {
        info_ = common::convertFromString<redisConfig>(line);
    }

    std::string RedisConnectionSettings::commandLine() const
    {
        return common::convertToString(info_);
    }

    redisConfig RedisConnectionSettings::info() const
    {
        return info_;
    }

    void RedisConnectionSettings::setInfo(const redisConfig &info)
    {
        info_ =  info;
    }

    IConnectionSettings *RedisConnectionSettings::clone() const
    {
        RedisConnectionSettings *red = new RedisConnectionSettings(*this);
        return red;
    }
}
