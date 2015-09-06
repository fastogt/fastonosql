#include "core/unqlite/unqlite_settings.h"

#include "common/utils.h"

namespace fastonosql
{
    UnqliteConnectionSettings::UnqliteConnectionSettings(const std::string& connectionName)
        : IConnectionSettingsBase(connectionName, UNQLITE), info_()
    {

    }

    std::string UnqliteConnectionSettings::commandLine() const
    {
        return common::convertToString(info_);
    }

    void UnqliteConnectionSettings::setCommandLine(const std::string& line)
    {
        info_ = common::convertFromString<unqliteConfig>(line);
    }

    void UnqliteConnectionSettings::setHost(const common::net::hostAndPort& host)
    {
        info_.hostip_ = common::utils::strdupornull(host.host_);
        info_.hostport_ = host.port_;
    }

    common::net::hostAndPort UnqliteConnectionSettings::host() const
    {
        return common::net::hostAndPort(info_.hostip_, info_.hostport_);
    }

    unqliteConfig UnqliteConnectionSettings::info() const
    {
        return info_;
    }

    void UnqliteConnectionSettings::setInfo(const unqliteConfig &info)
    {
        info_ = info;
    }

    IConnectionSettings* UnqliteConnectionSettings::clone() const
    {
        UnqliteConnectionSettings *red = new UnqliteConnectionSettings(*this);
        return red;
    }

    std::string UnqliteConnectionSettings::toCommandLine() const
    {
        std::string result = common::convertToString(info_);
        return result;
    }

    void UnqliteConnectionSettings::initFromCommandLine(const std::string& val)
    {
        info_ = common::convertFromString<unqliteConfig>(val);
    }

}
