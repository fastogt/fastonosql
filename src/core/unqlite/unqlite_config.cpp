#include "core/unqlite/unqlite_config.h"

#include "common/sprintf.h"
#include "common/utils.h"

#include "fasto/qt/logger.h"

namespace fastonosql
{
    namespace
    {
        int parseOptions(int argc, char **argv, unqliteConfig& cfg) {
            int i;

            for (i = 0; i < argc; i++) {
                int lastarg = i==argc-1;

                if (!strcmp(argv[i],"-h") && !lastarg) {
                    common::utils::freeifnotnull(cfg.hostip_);
                    cfg.hostip_ = strdup(argv[++i]);
                }
                else if (!strcmp(argv[i],"-p") && !lastarg) {
                    cfg.hostport_ = atoi(argv[++i]);
                }
                else if (!strcmp(argv[i],"-d") && !lastarg) {
                    common::utils::freeifnotnull(cfg.mb_delim_);
                    cfg.mb_delim_ = strdup(argv[++i]);
                }
                else if (!strcmp(argv[i], "-f") && !lastarg) {
                    cfg.dbname_ = argv[++i];
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

    unqliteConfig::unqliteConfig()
       : ConnectionConfig("127.0.0.1", 1111)
    {
    }

    unqliteConfig::unqliteConfig(const unqliteConfig &other)
        : ConnectionConfig(other.hostip_, other.hostport_)
    {
        copy(other);
    }

    unqliteConfig& unqliteConfig::operator=(const unqliteConfig &other)
    {
        copy(other);
        return *this;
    }

    void unqliteConfig::copy(const unqliteConfig& other)
    {
        using namespace common::utils;
        dbname_ = other.dbname_;
        ConnectionConfig::copy(other);
    }

    unqliteConfig::~unqliteConfig()
    {
    }
}

namespace common
{
    std::string convertToString(const fastonosql::unqliteConfig &conf)
    {
        std::vector<std::string> argv = conf.args();

        if(!conf.dbname_.empty()){
            argv.push_back("-f");
            argv.push_back(conf.dbname_);
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
    fastonosql::unqliteConfig convertFromString(const std::string& line)
    {
        fastonosql::unqliteConfig cfg;
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
