#include "core/ssdb/ssdb_config.h"

#include "common/sprintf.h"
#include "common/utils.h"

#include "fasto/qt/logger.h"

namespace fastonosql
{
    namespace
    {
        int parseOptions(int argc, char **argv, ssdbConfig& cfg) {
            int i;

            for (i = 0; i < argc; i++) {
                int lastarg = i==argc-1;

                if (!strcmp(argv[i],"-h") && !lastarg) {
                    free(cfg.hostip_);
                    cfg.hostip_ = strdup(argv[++i]);
                }
                else if (!strcmp(argv[i],"-p") && !lastarg) {
                    cfg.hostport_ = atoi(argv[++i]);
                }
                else if (!strcmp(argv[i],"-u") && !lastarg) {
                    cfg.user_ = strdup(argv[++i]);
                }
                else if (!strcmp(argv[i],"-a") && !lastarg) {
                    cfg.password_ = strdup(argv[++i]);
                }
                else if (!strcmp(argv[i],"-d") && !lastarg) {
                    free(cfg.mb_delim_);
                    cfg.mb_delim_ = strdup(argv[++i]);
                }
                else {
                    if (argv[i][0] == '-') {
                        const uint16_t size_buff = 256;
                        char buff[size_buff] = {0};
                        common::SNPrintf(buff, sizeof(buff), "Unrecognized option or bad number of args for: '%s'", argv[i]);
                        LOG_MSG(buff, common::logging::L_WARNING, true);
                        break;
                    } else {
                        /* Likely the command name, stop here. */
                        break;
                    }
                }
            }
            return i;
        }
    }

    ssdbConfig::ssdbConfig()
        : ConnectionConfig("127.0.0.1", 8888), user_(NULL), password_(NULL)
    {
    }

    ssdbConfig::ssdbConfig(const ssdbConfig &other)
        : ConnectionConfig(other.hostip_, other.hostport_), user_(NULL), password_(NULL)
    {
        copy(other);
    }

    ssdbConfig& ssdbConfig::operator=(const ssdbConfig &other)
    {
        copy(other);
        return *this;
    }

    void ssdbConfig::copy(const ssdbConfig& other)
    {
        using namespace common::utils;
        freeifnotnull(user_);
        user_ = strdupornull(other.user_); //
        freeifnotnull(password_);
        password_ = strdupornull(other.password_); //

        ConnectionConfig::copy(other);
    }


    ssdbConfig::~ssdbConfig()
    {
        using namespace common::utils;
        freeifnotnull(user_);
        freeifnotnull(password_);
    }
}

namespace common
{
    std::string convertToString(const fastonosql::ssdbConfig &conf)
    {
        std::vector<std::string> argv = conf.args();

        if(conf.user_){
            argv.push_back("-u");
            argv.push_back(conf.user_);
        }

        if(conf.password_){
            argv.push_back("-a");
            argv.push_back(conf.password_);
        }

        std::string result;
        for(int i = 0; i < argv.size(); ++i){
            result+= argv[i];
            if(i != argv.size()-1){
                result+=" ";
            }
        }

        return result;
    }

    template<>
    fastonosql::ssdbConfig convertFromString(const std::string& line)
    {
        fastonosql::ssdbConfig cfg;
        enum { kMaxArgs = 64 };
        int argc = 0;
        char *argv[kMaxArgs] = {0};

        char* p2 = strtok((char*)line.c_str(), " ");
        while(p2){
            argv[argc++] = p2;
            p2 = strtok(0, " ");
        }

        fastonosql::parseOptions(argc, argv, cfg);
        return cfg;
    }
}
