#include "core/connection_confg.h"

#include "common/convert2string.h"

#include "common/utils.h"

namespace fastonosql
{
    ConnectionConfig::ConnectionConfig(const char *hostip, int port)
        : hostip_(strdup(hostip)), hostport_(port), mb_delim_(strdup("\n")), shutdown_(0)
    {
    }

    ConnectionConfig::ConnectionConfig(const ConnectionConfig& other)
        : hostip_(NULL), hostport_(0), mb_delim_(NULL)
    {
        copy(other);
    }

    ConnectionConfig& ConnectionConfig::operator=(const ConnectionConfig &other)
    {
        copy(other);
        return *this;
    }

    void ConnectionConfig::copy(const ConnectionConfig& other)
    {
        using namespace common::utils;
        freeifnotnull(hostip_);
        hostip_ = strdupornull(other.hostip_); //
        hostport_ = other.hostport_;

        freeifnotnull(mb_delim_);
        mb_delim_ = strdupornull(other.mb_delim_); //
        shutdown_ = other.shutdown_;
    }

    std::vector<std::string> ConnectionConfig::args() const
    {
        std::vector<std::string> argv;

        if(hostip_){
            argv.push_back("-h");
            argv.push_back(hostip_);
        }
        if(hostport_){
            argv.push_back("-p");
            argv.push_back(common::convertToString(hostport_));
        }
        if (mb_delim_) {
            argv.push_back("-d");
            argv.push_back(mb_delim_);
        }

        return argv;
    }

    ConnectionConfig::~ConnectionConfig()
    {
        using namespace common::utils;
        freeifnotnull(hostip_);
        freeifnotnull(mb_delim_);
    }
}
