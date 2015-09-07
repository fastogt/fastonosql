#pragma once

#include "core/connection_settings.h"

#include "core/memcached/memcached_config.h"

namespace fastonosql
{
    class MemcachedConnectionSettings
            : public IConnectionSettingsBaseRemote
    {
    public:
        explicit MemcachedConnectionSettings(const std::string& connectionName);

        virtual std::string commandLine() const;
        virtual void setCommandLine(const std::string& line);

        virtual void setHost(const common::net::hostAndPort& host);
        virtual common::net::hostAndPort host() const;

        memcachedConfig info() const;
        void setInfo(const memcachedConfig& info);

        virtual IConnectionSettings* clone() const;

    private:
        virtual std::string toCommandLine() const;
        virtual void initFromCommandLine(const std::string& val);
        memcachedConfig info_;
    };
}
