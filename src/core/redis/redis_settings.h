#pragma once

#include "core/connection_settings.h"

#include "core/redis/redis_config.h"

namespace fastonosql
{
    class RedisConnectionSettings
            : public IConnectionSettingsBaseRemote
    {
    public:
        explicit RedisConnectionSettings(const std::string& connectionName);

        virtual std::string commandLine() const;
        virtual void setCommandLine(const std::string& line);

        virtual void setHost(const common::net::hostAndPort& host);
        virtual common::net::hostAndPort host() const;

        redisConfig info() const;
        void setInfo(const redisConfig& info);

        virtual IConnectionSettings* clone() const;

    private:
        virtual std::string toCommandLine() const;
        virtual void initFromCommandLine(const std::string& val);
        redisConfig info_;
    };
}
