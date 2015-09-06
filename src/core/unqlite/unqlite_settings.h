#pragma once

#include "core/connection_settings.h"

#include "core/unqlite/unqlite_config.h"

namespace fastonosql
{
    class UnqliteConnectionSettings
            : public IConnectionSettingsBase
    {
    public:
        explicit UnqliteConnectionSettings(const std::string& connectionName);

        virtual std::string commandLine() const;
        virtual void setCommandLine(const std::string& line);

        virtual void setHost(const common::net::hostAndPort& host);
        virtual common::net::hostAndPort host() const;

        unqliteConfig info() const;
        void setInfo(const unqliteConfig &info);

        virtual IConnectionSettings* clone() const;

    private:
        virtual std::string toCommandLine() const;
        virtual void initFromCommandLine(const std::string& val);
        unqliteConfig info_;
    };
}
