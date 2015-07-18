#pragma once

#include "core/connection_settings.h"

#include "core/ssdb/ssdb_config.h"

namespace fastoredis
{
    class SsdbConnectionSettings
            : public IConnectionSettingsBase
    {
    public:
        SsdbConnectionSettings(const std::string& connectionName);

        virtual std::string commandLine() const;
        virtual void setCommandLine(const std::string& line);

        virtual void setHost(const common::net::hostAndPort& host);
        virtual common::net::hostAndPort host() const;

        ssdbConfig info() const;
        void setInfo(const ssdbConfig &info);

        virtual IConnectionSettings* clone() const;

    private:
        virtual std::string toCommandLine() const;
        virtual void initFromCommandLine(const std::string& val);
        ssdbConfig info_;
    };
}
