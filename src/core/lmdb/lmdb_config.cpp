#include "core/lmdb/lmdb_config.h"

#include "common/sprintf.h"
#include "common/file_system.h"

#include "fasto/qt/logger.h"

namespace fastonosql
{
    namespace
    {
        void parseOptions(int argc, char **argv, lmdbConfig& cfg)
        {
            for (int i = 0; i < argc; i++) {
                int lastarg = i==argc-1;

                if (!strcmp(argv[i],"-d") && !lastarg) {
                    cfg.mb_delim_ = argv[++i];
                }
                else if (!strcmp(argv[i], "-f") && !lastarg) {
                    cfg.dbname_ = argv[++i];
                }
                else if (!strcmp(argv[i],"-c")) {
                    cfg.create_if_missing_ = true;
                }
                else {
                    if (argv[i][0] == '-') {
                        const uint16_t size_buff = 256;
                        char buff[size_buff] = {0};
                        common::SNPrintf(buff, sizeof(buff), "Unrecognized option or bad number of args for: '%s'", argv[i]);
                        LOG_MSG(buff, common::logging::L_WARNING, true);
                        break;
                    }
                    else {
                        /* Likely the command name, stop here. */
                        break;
                    }
                }
            }
        }
    }

    lmdbConfig::lmdbConfig()
       : LocalConfig(common::file_system::prepare_path("~/test.lmdb")), create_if_missing_(false)
    {
    }
}

namespace common
{
    std::string convertToString(const fastonosql::lmdbConfig &conf)
    {
        std::vector<std::string> argv = conf.args();

        if(conf.create_if_missing_){
            argv.push_back("-c");
        }

        std::string result;
        for(int i = 0; i < argv.size(); ++i){
            result += argv[i];
            if(i != argv.size()-1){
                result += " ";
            }
        }

        return result;
    }

    template<>
    fastonosql::lmdbConfig convertFromString(const std::string& line)
    {
        fastonosql::lmdbConfig cfg;
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
