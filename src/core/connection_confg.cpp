#include "core/connection_confg.h"

#include "common/convert2string.h"

namespace fastonosql
{
    LocalConfig::LocalConfig(const std::string& dbname)
        : BaseConfig<LOCAL>(), dbname_(dbname)
    {

    }

    std::vector<std::string> LocalConfig::args() const
    {
        std::vector<std::string> argv;

        if(!dbname_.empty()){
            argv.push_back("-f");
            argv.push_back(dbname_);
        }

        if (!mb_delim_.empty()) {
            argv.push_back("-d");
            argv.push_back(mb_delim_);
        }

        return argv;
    }

    RemoteConfig::RemoteConfig(const std::string &hostip, uint16_t port)
        : BaseConfig<REMOTE>(), hostip_(hostip), hostport_(port)
    {
    }

    std::vector<std::string> RemoteConfig::args() const
    {
        std::vector<std::string> argv;

        if(!hostip_.empty()){
            argv.push_back("-h");
            argv.push_back(hostip_);
        }
        if(hostport_ != 0){
            argv.push_back("-p");
            argv.push_back(common::convertToString(hostport_));
        }

        if (!mb_delim_.empty()) {
            argv.push_back("-d");
            argv.push_back(mb_delim_);
        }

        return argv;
    }
}
