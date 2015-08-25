#include "core/rocksdb/rocksdb_settings.h"

#include "common/utils.h"

namespace fastonosql
{
    RocksdbConnectionSettings::RocksdbConnectionSettings(const std::string& connectionName)
        : IConnectionSettingsBase(connectionName, LEVELDB), info_()
    {

    }

    std::string RocksdbConnectionSettings::commandLine() const
    {
        return common::convertToString(info_);
    }

    void RocksdbConnectionSettings::setCommandLine(const std::string& line)
    {
        info_ = common::convertFromString<rocksdbConfig>(line);
    }

    void RocksdbConnectionSettings::setHost(const common::net::hostAndPort& host)
    {
        info_.hostip_ = common::utils::strdupornull(host.host_);
        info_.hostport_ = host.port_;
    }

    common::net::hostAndPort RocksdbConnectionSettings::host() const
    {
        return common::net::hostAndPort(info_.hostip_, info_.hostport_);
    }

    rocksdbConfig RocksdbConnectionSettings::info() const
    {
        return info_;
    }

    void RocksdbConnectionSettings::setInfo(const rocksdbConfig &info)
    {
        info_ = info;
    }

    IConnectionSettings* RocksdbConnectionSettings::clone() const
    {
        RocksdbConnectionSettings *red = new RocksdbConnectionSettings(*this);
        return red;
    }

    std::string RocksdbConnectionSettings::toCommandLine() const
    {
        std::string result = common::convertToString(info_);
        return result;
    }

    void RocksdbConnectionSettings::initFromCommandLine(const std::string& val)
    {
        info_ = common::convertFromString<rocksdbConfig>(val);
    }

}
