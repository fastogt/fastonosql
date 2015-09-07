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

        unqliteConfig info() const;
        void setInfo(const unqliteConfig &info);

        virtual std::string fullAddress() const;

        virtual IConnectionSettings* clone() const;

    private:
        virtual std::string toCommandLine() const;
        virtual void initFromCommandLine(const std::string& val);
        unqliteConfig info_;
    };
}
