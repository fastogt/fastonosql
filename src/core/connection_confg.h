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
    struct BaseConfig
    {
        explicit BaseConfig(ConfigType type);
        ConfigType type() const;
        std::vector<std::string> args() const;

        std::string mb_delim_;
        bool shutdown_;

    private:
        ConfigType type_;
    };

    // -f
    struct LocalConfig
            : public BaseConfig
    {
        explicit LocalConfig(const std::string& dbname);

        std::vector<std::string> args() const;

        std::string dbname_;
    };

    // -h -p
    struct ConnectionConfig
            : public BaseConfig
    {
        ConnectionConfig(const std::string& hostip, uint16_t port);

        std::vector<std::string> args() const;

        std::string hostip_;
        uint16_t hostport_;
    };
}
