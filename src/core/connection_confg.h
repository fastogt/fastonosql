#pragma once

#include <vector>
#include <string>

namespace fastonosql
{
    enum ConfigType
    {
        LOCAL,
        REMOTE
    };

    // -d
    template<ConfigType ctype>
    struct BaseConfig
    {
        BaseConfig()
            : mb_delim_("\n"), shutdown_(false)
        {

        }

        ConfigType type() const
        {
            return ctype;
        }

        std::string mb_delim_;
        bool shutdown_;
    };

    // -f
    struct LocalConfig
            : public BaseConfig<LOCAL>
    {
        explicit LocalConfig(const std::string& dbname);

        std::vector<std::string> args() const;

        std::string dbname_;
    };

    // -h -p
    struct RemoteConfig
            : public BaseConfig<REMOTE>
    {
        RemoteConfig(const std::string& hostip, uint16_t port);

        std::vector<std::string> args() const;

        std::string hostip_;
        uint16_t hostport_;
    };
}
