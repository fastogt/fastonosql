#include "core/ssdb/ssdb_settings.h"

#include "common/utils.h"

namespace fastonosql
{
    SsdbConnectionSettings::SsdbConnectionSettings(const std::string& connectionName)
        : IConnectionSettingsBase(connectionName, SSDB), info_()
    {

    }

    std::string SsdbConnectionSettings::commandLine() const
    {
        return common::convertToString(info_);
    }

    void SsdbConnectionSettings::setCommandLine(const std::string& line)
    {
        info_ = common::convertFromString<ssdbConfig>(line);
    }

    void SsdbConnectionSettings::setHost(const common::net::hostAndPort& host)
    {
        info_.hostip_ = common::utils::strdupornull(host.host_);
        info_.hostport_ = host.port_;
    }

    common::net::hostAndPort SsdbConnectionSettings::host() const
    {
        return common::net::hostAndPort(info_.hostip_, info_.hostport_);
    }

    ssdbConfig SsdbConnectionSettings::info() const
    {
        return info_;
    }

    void SsdbConnectionSettings::setInfo(const ssdbConfig& info)
    {
        info_ = info;
    }

    IConnectionSettings* SsdbConnectionSettings::clone() const
    {
        SsdbConnectionSettings *red = new SsdbConnectionSettings(*this);
        return red;
    }

    std::string SsdbConnectionSettings::toCommandLine() const
    {
        std::string result = common::convertToString(info_);
        return result;
    }

    void SsdbConnectionSettings::initFromCommandLine(const std::string& val)
    {
        info_ = common::convertFromString<ssdbConfig>(val);
    }

}
