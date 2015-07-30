#pragma once

#include <vector>
#include <string>

namespace fastonosql
{
    // -h -p -d
    struct ConnectionConfig
    {
        ConnectionConfig(const char* hostip, int port);
        ConnectionConfig(const ConnectionConfig& other);
        ConnectionConfig& operator=(const ConnectionConfig &other);
        ~ConnectionConfig();

        char *hostip_;
        int hostport_;

        char *mb_delim_;
        int shutdown_;

        std::vector<std::string> args() const;

    protected:
        void copy(const ConnectionConfig& other);
    };
}
