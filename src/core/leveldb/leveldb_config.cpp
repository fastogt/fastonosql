#include "core/leveldb/leveldb_config.h"

#include "common/sprintf.h"
#include "common/file_system.h"

#include "fasto/qt/logger.h"

namespace fastonosql
{
    namespace
    {
        int parseOptions(int argc, char **argv, leveldbConfig& cfg) {
            int i;

            for (i = 0; i < argc; i++) {
                int lastarg = i==argc-1;

                if (!strcmp(argv[i],"-d") && !lastarg) {
                    cfg.mb_delim_ = argv[++i];
                }
                else if (!strcmp(argv[i], "-f") && !lastarg) {
                    cfg.dbname_ = argv[++i];
                }
                else if (!strcmp(argv[i],"-c")) {
                    cfg.options_.create_if_missing = true;
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

    leveldbConfig::leveldbConfig()
       : LocalConfig(common::file_system::prepare_path("~/test.leveldb"))
    {
        options_.create_if_missing = true;
    }
}

namespace common
{
    std::string convertToString(const fastonosql::leveldbConfig &conf)
    {
        std::vector<std::string> argv = conf.args();

        if(conf.options_.create_if_missing){
            argv.push_back("-c");
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
    fastonosql::leveldbConfig convertFromString(const std::string& line)
    {
        fastonosql::leveldbConfig cfg;
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
