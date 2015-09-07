#include "core/memcached/memcached_settings.h"

#include "common/utils.h"

namespace fastonosql
{
    MemcachedConnectionSettings::MemcachedConnectionSettings(const std::string& connectionName)
        : IConnectionSettingsBaseRemote(connectionName, MEMCACHED), info_()
    {

    }

    std::string MemcachedConnectionSettings::commandLine() const
    {
        return common::convertToString(info_);
    }

    void MemcachedConnectionSettings::setCommandLine(const std::string& line)
    {
        info_ = common::convertFromString<memcachedConfig>(line);
    }

    void MemcachedConnectionSettings::setHost(const common::net::hostAndPort& host)
    {
        info_.hostip_ = host.host_;
        info_.hostport_ = host.port_;
    }

    common::net::hostAndPort MemcachedConnectionSettings::host() const
    {
        return common::net::hostAndPort(info_.hostip_, info_.hostport_);
    }

    memcachedConfig MemcachedConnectionSettings::info() const
    {
        return info_;
    }

    void MemcachedConnectionSettings::setInfo(const memcachedConfig& info)
    {
        info_ = info;
    }

    IConnectionSettings *MemcachedConnectionSettings::clone() const
    {
        MemcachedConnectionSettings *red = new MemcachedConnectionSettings(*this);
        return red;
    }

    std::string MemcachedConnectionSettings::toCommandLine() const
    {
        std::string result = common::convertToString(info_);
        return result;
    }

    void MemcachedConnectionSettings::initFromCommandLine(const std::string& val)
    {
        info_ = common::convertFromString<memcachedConfig>(val);
    }
}
