#include "core/lmdb/lmdb_settings.h"

namespace fastonosql
{
    LmdbConnectionSettings::LmdbConnectionSettings(const std::string& connectionName)
        : IConnectionSettingsBase(connectionName, LMDB), info_()
    {

    }

    std::string LmdbConnectionSettings::commandLine() const
    {
        return common::convertToString(info_);
    }

    void LmdbConnectionSettings::setCommandLine(const std::string& line)
    {
        info_ = common::convertFromString<lmdbConfig>(line);
    }

    lmdbConfig LmdbConnectionSettings::info() const
    {
        return info_;
    }

    void LmdbConnectionSettings::setInfo(const lmdbConfig &info)
    {
        info_ = info;
    }

    std::string LmdbConnectionSettings::fullAddress() const
    {
        return info_.dbname_;
    }

    IConnectionSettings* LmdbConnectionSettings::clone() const
    {
        LmdbConnectionSettings *red = new LmdbConnectionSettings(*this);
        return red;
    }

    std::string LmdbConnectionSettings::toCommandLine() const
    {
        std::string result = common::convertToString(info_);
        return result;
    }

    void LmdbConnectionSettings::initFromCommandLine(const std::string& val)
    {
        info_ = common::convertFromString<lmdbConfig>(val);
    }
}
