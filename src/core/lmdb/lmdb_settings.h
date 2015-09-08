#pragma once

#include "core/connection_settings.h"

#include "core/lmdb/lmdb_config.h"

namespace fastonosql
{
    class LmdbConnectionSettings
            : public IConnectionSettingsBase
    {
    public:
        explicit LmdbConnectionSettings(const std::string& connectionName);

        virtual std::string commandLine() const;
        virtual void setCommandLine(const std::string& line);

        lmdbConfig info() const;
        void setInfo(const lmdbConfig &info);

        virtual std::string fullAddress() const;

        virtual IConnectionSettings* clone() const;

    private:
        virtual std::string toCommandLine() const;
        virtual void initFromCommandLine(const std::string& val);
        lmdbConfig info_;
    };
}
