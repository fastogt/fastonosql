#include "core/leveldb/leveldb_settings.h"

#include "common/utils.h"

namespace fastonosql
{
    LeveldbConnectionSettings::LeveldbConnectionSettings(const std::string& connectionName)
        : IConnectionSettingsBase(connectionName, LEVELDB), info_()
    {

    }

    std::string LeveldbConnectionSettings::commandLine() const
    {
        return common::convertToString(info_);
    }

    void LeveldbConnectionSettings::setCommandLine(const std::string& line)
    {
        info_ = common::convertFromString<leveldbConfig>(line);
    }

    void LeveldbConnectionSettings::setHost(const common::net::hostAndPort& host)
    {
        info_.hostip_ = common::utils::strdupornull(host.host_);
        info_.hostport_ = host.port_;
    }

    common::net::hostAndPort LeveldbConnectionSettings::host() const
    {
        return common::net::hostAndPort(info_.hostip_, info_.hostport_);
    }

    leveldbConfig LeveldbConnectionSettings::info() const
    {
        return info_;
    }

    void LeveldbConnectionSettings::setInfo(const leveldbConfig &info)
    {
        info_ = info;
    }

    IConnectionSettings* LeveldbConnectionSettings::clone() const
    {
        LeveldbConnectionSettings *red = new LeveldbConnectionSettings(*this);
        return red;
    }

    std::string LeveldbConnectionSettings::toCommandLine() const
    {
        std::string result = common::convertToString(info_);
        return result;
    }

    void LeveldbConnectionSettings::initFromCommandLine(const std::string& val)
    {
        info_ = common::convertFromString<leveldbConfig>(val);
    }

}
