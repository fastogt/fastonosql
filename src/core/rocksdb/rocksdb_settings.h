#pragma once

#include "core/connection_settings.h"

#include "core/rocksdb/rocksdb_config.h"

namespace fastonosql
{
    class RocksdbConnectionSettings
            : public IConnectionSettingsBase
    {
    public:
        explicit RocksdbConnectionSettings(const std::string& connectionName);

        virtual std::string commandLine() const;
        virtual void setCommandLine(const std::string& line);

        rocksdbConfig info() const;
        void setInfo(const rocksdbConfig &info);

        virtual std::string fullAddress() const;

        virtual IConnectionSettings* clone() const;

    private:
        virtual std::string toCommandLine() const;
        virtual void initFromCommandLine(const std::string& val);
        rocksdbConfig info_;
    };
}
