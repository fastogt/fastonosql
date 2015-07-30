#include "core/ssdb/ssdb_driver.h"

extern "C" {
    #include "sds.h"
}

#include "common/sprintf.h"
#include "common/utils.h"
#include "fasto/qt/logger.h"

#include "core/command_logger.h"

#include "core/ssdb/ssdb_config.h"
#include "core/ssdb/ssdb_infos.h"

#include <SSDB.h>

#define INFO_REQUEST "INFO"
#define GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define DELETE_KEY_PATTERN_1ARGS_S "DEL %s"
#define GET_SERVER_TYPE ""

#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define GET_KEY_LIST_PATTERN_1ARGS_S "LRANGE %s 0 -1"
#define GET_KEY_SET_PATTERN_1ARGS_S "SMEMBERS %s"
#define GET_KEY_ZSET_PATTERN_1ARGS_S "ZRANGE %s 0 -1"
#define GET_KEY_HASH_PATTERN_1ARGS_S "HGET %s"

#define SET_KEY_PATTERN_2ARGS_SS "SET %s %s"
#define SET_KEY_LIST_PATTERN_2ARGS_SS "LPUSH %s %s"
#define SET_KEY_SET_PATTERN_2ARGS_SS "SADD %s %s"
#define SET_KEY_ZSET_PATTERN_2ARGS_SS "ZADD %s %s"
#define SET_KEY_HASH_PATTERN_2ARGS_SS "HMSET %s %s"

namespace
{
    std::vector<std::pair<std::string, std::string > > oppositeCommands = { {"GET", "SET"} };
}

namespace fastonosql
{
    namespace
    {
        class SsdbCommand
                : public FastoObjectCommand
        {
        public:
            SsdbCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr)
                : FastoObjectCommand(parent, cmd, delemitr)
            {

            }

            virtual std::string inputCmd() const
            {
                common::CommandValue* command = cmd();
                if(command){
                    std::pair<std::string, std::string> kv = getKeyValueFromLine(command->inputCommand());
                    return kv.first;
                }

                return std::string();
            }

            virtual std::string inputArgs() const
            {
                common::CommandValue* command = cmd();
                if(command){
                    std::pair<std::string, std::string> kv = getKeyValueFromLine(command->inputCommand());
                    return kv.second;
                }

                return std::string();
            }
        };

        SsdbCommand* createCommand(FastoObject* parent, const std::string& input, common::Value::CommandType ct)
        {
            if(input.empty()){
                return NULL;
            }

            DCHECK(parent);
            std::pair<std::string, std::string> kv = getKeyValueFromLine(input);
            std::string opposite = getOppositeCommand(kv.first, oppositeCommands);
            if(!opposite.empty()){
                opposite += " " + kv.second;
            }
            common::CommandValue* cmd = common::Value::createCommand(input, opposite, ct);
            SsdbCommand* fs = new SsdbCommand(parent, cmd, "");
            parent->addChildren(fs);
            return fs;
        }

        SsdbCommand* createCommand(FastoObjectIPtr parent, const std::string& input, common::Value::CommandType ct)
        {
            return createCommand(parent.get(), input, ct);
        }
    }

    common::ErrorValueSPtr testConnection(SsdbConnectionSettings* settings)
    {
        if(!settings){
            return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
        }

        ssdbConfig inf = settings->info();

        ssdb::Client* ssdb = ssdb::Client::connect(inf.hostip_, inf.hostport_);
        if (!ssdb){
            return common::make_error_value("Fail connect to server!", common::ErrorValue::E_ERROR);
        }

        delete ssdb;

        return common::ErrorValueSPtr();
    }

    struct SsdbDriver::pimpl
    {
        pimpl()
            : ssdb_(NULL)
        {

        }

        bool isConnected() const
        {
            if(!ssdb_){
                return false;
            }

            return true;
        }

        common::ErrorValueSPtr connect()
        {
            if(isConnected()){
                return common::ErrorValueSPtr();
            }

            clear();
            init();

            ssdb_ = ssdb::Client::connect(config_.hostip_, config_.hostport_);
            if (!ssdb_){
                return common::make_error_value("Fail connect to server!", common::ErrorValue::E_ERROR);
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr disconnect()
        {
            if(!isConnected()){
                return common::ErrorValueSPtr();
            }

            clear();
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr info(const char* args, SsdbServerInfo::Common& statsout)
        {
            std::vector<std::string> ret;
            ssdb::Status st = ssdb_->info(args ? args : std::string(), &ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "info function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            for(int i = 0; i < ret.size(); i += 2){
                if(ret[i] == SSDB_VERSION_LABEL){
                    statsout.version_ = ret[i + 1];
                }
                else if (ret[i] == SSDB_LINKS_LABEL){
                    statsout.links_ = common::convertFromString<uint32_t>(ret[i + 1]);
                }
                else if(ret[i] == SSDB_TOTAL_CALLS_LABEL){
                    statsout.total_calls_ = common::convertFromString<uint32_t>(ret[i + 1]);
                }
                else if(ret[i] == SSDB_DBSIZE_LABEL){
                    statsout.dbsize_ = common::convertFromString<uint32_t>(ret[i + 1]);
                }
                else if(ret[i] == SSDB_BINLOGS_LABEL){
                    statsout.binlogs_ = ret[i + 1];
                }
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr execute(SsdbCommand* cmd) WARN_UNUSED_RESULT
        {
            //DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            const std::string command = cmd->cmd()->inputCommand();
            common::Value::CommandType type = cmd->cmd()->commandType();

            if(command.empty()){
                return common::make_error_value("Command empty", common::ErrorValue::E_ERROR);
            }

            LOG_COMMAND(Command(command, type));

            common::ErrorValueSPtr er;
            if (command[0] != '\0') {
                int argc;
                sds *argv = sdssplitargs(command.c_str(), &argc);

                if (argv == NULL) {
                    common::StringValue *val = common::Value::createStringValue("Invalid argument(s)");
                    FastoObject* child = new FastoObject(cmd, val, config_.mb_delim_);
                    cmd->addChildren(child);
                }
                else if (argc > 0) {
                    if (strcasecmp(argv[0], "quit") == 0){
                        config_.shutdown_ = 1;
                    }
                    else {
                        er = execute(cmd, argc, argv);
                    }
                }
                sdsfreesplitres(argv,argc);
            }

            return er;
        }

        ~pimpl()
        {
            clear();
        }

        ssdbConfig config_;
        SSHInfo sinfo_;

   private:
        common::ErrorValueSPtr execute(FastoObject* out, int argc, char **argv)
        {
            if(strcasecmp(argv[0], "get") == 0){
                if(argc != 2){
                    return common::make_error_value("Invalid get input argument", common::ErrorValue::E_ERROR);
                }

                std::string ret;
                common::ErrorValueSPtr er = get(argv[1], &ret);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue(ret);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "set") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid set input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = set(argv[1], argv[2]);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("STORED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "setx") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid setx input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = setx(argv[1], argv[2], atoi(argv[3]));
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("STORED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "del") == 0){
                if(argc != 2){
                    return common::make_error_value("Invalid del input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = del(argv[1]);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("DELETED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "incr") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid incr input argument", common::ErrorValue::E_ERROR);
                }

                int64_t ret = 0;
                common::ErrorValueSPtr er = incr(argv[1], atoll(argv[2]), &ret);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(ret);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "keys") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid keys input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysout;
                common::ErrorValueSPtr er = keys(argv[1], argv[2], atoll(argv[3]), &keysout);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < keysout.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(keysout[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "scan") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid scan input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysout;
                common::ErrorValueSPtr er = scan(argv[1], argv[2], atoll(argv[3]), &keysout);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < keysout.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(keysout[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "rscan") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid rscan input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysout;
                common::ErrorValueSPtr er = rscan(argv[1], argv[2], atoll(argv[3]), &keysout);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < keysout.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(keysout[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "multi_get") == 0){
                if(argc < 2){
                    return common::make_error_value("Invalid multi_get input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysget;
                for(int i = 1; i < argc; ++i){
                    keysget.push_back(argv[i]);
                }

                std::vector<std::string> keysout;
                common::ErrorValueSPtr er = multi_get(keysget, &keysout);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < keysout.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(keysout[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "multi_del") == 0){
                if(argc < 2){
                    return common::make_error_value("Invalid multi_del input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysget;
                for(int i = 1; i < argc; ++i){
                    keysget.push_back(argv[i]);
                }

                common::ErrorValueSPtr er = multi_del(keysget);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("DELETED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "multi_set") == 0){
                if(argc < 2 || argc % 2){
                    return common::make_error_value("Invalid multi_del input argument", common::ErrorValue::E_ERROR);
                }

                std::map<std::string, std::string> keysset;
                for(int i = 1; i < argc; i += 2){
                    keysset[argv[i]] = argv[i + 1];
                }

                common::ErrorValueSPtr er = multi_set(keysset);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("STORED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hget") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid hget input argument", common::ErrorValue::E_ERROR);
                }

                std::string ret;
                common::ErrorValueSPtr er = hget(argv[1], argv[2], &ret);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue(ret);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hset") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid hset input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = hset(argv[1], argv[2], argv[3]);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("STORED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hdel") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid hset input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = hdel(argv[1], argv[2]);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("DELETED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hincr") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid hincr input argument", common::ErrorValue::E_ERROR);
                }

                int64_t res = 0;
                common::ErrorValueSPtr er = hincr(argv[1], argv[2], atoll(argv[3]), &res);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(res);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hsize") == 0){
                if(argc != 2){
                    return common::make_error_value("Invalid hsize input argument", common::ErrorValue::E_ERROR);
                }

                int64_t res = 0;
                common::ErrorValueSPtr er = hsize(argv[1], &res);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(res);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hclear") == 0){
                if(argc != 2){
                    return common::make_error_value("Invalid hclear input argument", common::ErrorValue::E_ERROR);
                }

                int64_t res = 0;
                common::ErrorValueSPtr er = hclear(argv[1], &res);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(res);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hkeys") == 0){
                if(argc != 5){
                    return common::make_error_value("Invalid hclear input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysout;
                common::ErrorValueSPtr er = hkeys(argv[1], argv[2], argv[3], atoll(argv[4]), &keysout);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < keysout.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(keysout[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hscan") == 0){
                if(argc != 5){
                    return common::make_error_value("Invalid hscan input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysout;
                common::ErrorValueSPtr er = hscan(argv[1], argv[2], argv[3], atoll(argv[4]), &keysout);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < keysout.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(keysout[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "hrscan") == 0){
                if(argc != 5){
                    return common::make_error_value("Invalid hrscan input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysout;
                common::ErrorValueSPtr er = hrscan(argv[1], argv[2], argv[3], atoll(argv[4]), &keysout);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < keysout.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(keysout[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "multi_hget") == 0){
                if(argc < 2){
                    return common::make_error_value("Invalid multi_get input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysget;
                for(int i = 2; i < argc; ++i){
                    keysget.push_back(argv[i]);
                }

                std::vector<std::string> keysout;
                common::ErrorValueSPtr er = multi_hget(argv[1], keysget, &keysout);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < keysout.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(keysout[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "multi_hset") == 0){
                if(argc < 2 || (argc % 2 == 0)){
                    return common::make_error_value("Invalid multi_hset input argument", common::ErrorValue::E_ERROR);
                }

                std::map<std::string, std::string> keys;
                for(int i = 2; i < argc; i += 2){
                    keys[argv[i]] = argv[i + 1];
                }

                common::ErrorValueSPtr er = multi_hset(argv[1], keys);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("STORED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zget") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid zget input argument", common::ErrorValue::E_ERROR);
                }

                int64_t ret;
                common::ErrorValueSPtr er = zget(argv[1], argv[2], &ret);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(ret);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zset") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid zset input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = zset(argv[1], argv[2], atoll(argv[3]));
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("STORED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zdel") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid zdel input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = zdel(argv[1], argv[2]);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("DELETED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zincr") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid zincr input argument", common::ErrorValue::E_ERROR);
                }

                int64_t ret = 0;
                common::ErrorValueSPtr er = zincr(argv[1], argv[2], atoll(argv[3]), &ret);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(ret);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zsize") == 0){
                if(argc != 2){
                    return common::make_error_value("Invalid zsize input argument", common::ErrorValue::E_ERROR);
                }

                int64_t res = 0;
                common::ErrorValueSPtr er = zsize(argv[1], &res);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(res);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zclear") == 0){
                if(argc != 2){
                    return common::make_error_value("Invalid zclear input argument", common::ErrorValue::E_ERROR);
                }

                int64_t res = 0;
                common::ErrorValueSPtr er = zclear(argv[1], &res);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(res);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zrank") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid zrank input argument", common::ErrorValue::E_ERROR);
                }

                int64_t res = 0;
                common::ErrorValueSPtr er = zrank(argv[1], argv[2], &res);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(res);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zzrank") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid zzrank input argument", common::ErrorValue::E_ERROR);
                }

                int64_t res = 0;
                common::ErrorValueSPtr er = zrrank(argv[1], argv[2], &res);
                if(!er){
                    common::FundamentalValue *val = common::Value::createIntegerValue(res);
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zrange") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid zrange input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> res;
                common::ErrorValueSPtr er = zrange(argv[1], atoll(argv[2]), atoll(argv[3]), &res);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < res.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(res[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zrrange") == 0){
                if(argc != 4){
                    return common::make_error_value("Invalid zrrange input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> res;
                common::ErrorValueSPtr er = zrrange(argv[1], atoll(argv[2]), atoll(argv[3]), &res);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < res.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(res[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zkeys") == 0){
                if(argc != 6){
                    return common::make_error_value("Invalid zkeys input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> res;
                int64_t st = atoll(argv[3]);
                int64_t end = atoll(argv[4]);
                common::ErrorValueSPtr er = zkeys(argv[1], argv[2], &st, &end, atoll(argv[5]), &res);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < res.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(res[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zscan") == 0){
                if(argc != 6){
                    return common::make_error_value("Invalid zscan input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> res;
                int64_t st = atoll(argv[3]);
                int64_t end = atoll(argv[4]);
                common::ErrorValueSPtr er = zscan(argv[1], argv[2], &st, &end, atoll(argv[5]), &res);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < res.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(res[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "zrscan") == 0){
                if(argc != 6){
                    return common::make_error_value("Invalid zrscan input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> res;
                int64_t st = atoll(argv[3]);
                int64_t end = atoll(argv[4]);
                common::ErrorValueSPtr er = zrscan(argv[1], argv[2], &st, &end, atoll(argv[5]), &res);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < res.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(res[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "multi_zget") == 0){
                if(argc < 2){
                    return common::make_error_value("Invalid zrscan input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysget;
                for(int i = 2; i < argc; ++i){
                    keysget.push_back(argv[i]);
                }

                std::vector<std::string> res;
                common::ErrorValueSPtr er = multi_zget(argv[1], keysget, &res);
                if(!er){
                    common::ArrayValue* ar = common::Value::createArrayValue();
                    for(int i = 0; i < res.size(); ++i){
                        common::StringValue *val = common::Value::createStringValue(res[i]);
                        ar->append(val);
                    }
                    FastoObjectArray* child = new FastoObjectArray(out, ar, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "multi_zset") == 0){
                if(argc < 2 || (argc % 2 == 0)){
                    return common::make_error_value("Invalid zrscan input argument", common::ErrorValue::E_ERROR);
                }

                std::map<std::string, int64_t> keysget;
                for(int i = 2; i < argc; i += 2){
                    keysget[argv[i]] = atoll(argv[i+1]);
                }

                common::ErrorValueSPtr er = multi_zset(argv[1], keysget);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("STORED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "multi_zdel") == 0){
                if(argc < 2){
                    return common::make_error_value("Invalid zrscan input argument", common::ErrorValue::E_ERROR);
                }

                std::vector<std::string> keysget;
                for(int i = 2; i < argc; ++i){
                    keysget.push_back(argv[i]);
                }

                common::ErrorValueSPtr er = multi_zdel(argv[1], keysget);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("DELETED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "info") == 0){
                if(argc > 2){
                    return common::make_error_value("Invalid info input argument", common::ErrorValue::E_ERROR);
                }

                SsdbServerInfo::Common statsout;
                common::ErrorValueSPtr er = info(argc == 2 ? argv[1] : 0, statsout);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue(SsdbServerInfo(statsout).toString());
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else{
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Not supported command: %s", argv[0]);
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
        }

        common::ErrorValueSPtr get(const std::string& key, std::string* ret_val)
        {
            ssdb::Status st = ssdb_->get(key, ret_val);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "get function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr set(const std::string& key, const std::string& value)
        {
            ssdb::Status st = ssdb_->set(key, value);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "set function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr setx(const std::string& key, const std::string& value, int ttl)
        {
            ssdb::Status st = ssdb_->setx(key, value, ttl);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "setx function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr del(const std::string& key)
        {
            ssdb::Status st = ssdb_->del(key);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "del function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr incr(const std::string& key, int64_t incrby, int64_t *ret)
        {
            ssdb::Status st = ssdb_->incr(key, incrby, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Incr function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr keys(const std::string &key_start, const std::string &key_end, uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->keys(key_start, key_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Keys function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr scan(const std::string &key_start, const std::string &key_end, uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->scan(key_start, key_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "scan function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr rscan(const std::string &key_start, const std::string &key_end, uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->rscan(key_start, key_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "rscan function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr multi_get(const std::vector<std::string>& keys, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->multi_get(keys, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "multi_get function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr multi_set(const std::map<std::string, std::string> &kvs)
        {
            ssdb::Status st = ssdb_->multi_set(kvs);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "multi_set function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr multi_del(const std::vector<std::string>& keys)
        {
            ssdb::Status st = ssdb_->multi_del(keys);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "multi_del function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        /******************** hash *************************/

        common::ErrorValueSPtr hget(const std::string& name, const std::string& key, std::string *val)
        {
            ssdb::Status st = ssdb_->hget(name, key, val);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hget function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr hset(const std::string& name, const std::string& key, const std::string& val)
        {
            ssdb::Status st = ssdb_->hset(name, key, val);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hset function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr hdel(const std::string& name, const std::string& key)
        {
            ssdb::Status st = ssdb_->hdel(name, key);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hdel function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr hincr(const std::string &name, const std::string &key, int64_t incrby, int64_t *ret)
        {
            ssdb::Status st = ssdb_->hincr(name, key, incrby, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hincr function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr hsize(const std::string &name, int64_t *ret)
        {
            ssdb::Status st = ssdb_->hsize(name, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hset function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr hclear(const std::string &name, int64_t *ret)
        {
            ssdb::Status st = ssdb_->hclear(name, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hclear function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr hkeys(const std::string &name, const std::string &key_start, const std::string &key_end, uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->hkeys(name, key_start, key_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hkeys function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr hscan(const std::string &name, const std::string &key_start, const std::string &key_end, uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->hscan(name, key_start, key_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hscan function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr hrscan(const std::string &name, const std::string &key_start, const std::string &key_end, uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->hrscan(name, key_start, key_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hrscan function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr multi_hget(const std::string &name, const std::vector<std::string> &keys, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->multi_hget(name, keys, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "hrscan function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr multi_hset(const std::string &name, const std::map<std::string, std::string> &keys)
        {
            ssdb::Status st = ssdb_->multi_hset(name, keys);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "multi_hset function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        /******************** zset *************************/

        common::ErrorValueSPtr zget(const std::string &name, const std::string &key, int64_t *ret)
        {
            ssdb::Status st = ssdb_->zget(name, key, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zget function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zset(const std::string &name, const std::string &key, int64_t score)
        {
            ssdb::Status st = ssdb_->zset(name, key, score);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zset function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zdel(const std::string &name, const std::string &key)
        {
            ssdb::Status st = ssdb_->zdel(name, key);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Zdel function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zincr(const std::string &name, const std::string &key, int64_t incrby, int64_t *ret)
        {
            ssdb::Status st = ssdb_->zincr(name, key, incrby, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Zincr function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zsize(const std::string &name, int64_t *ret)
        {
            ssdb::Status st = ssdb_->zsize(name, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zsize function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zclear(const std::string &name, int64_t *ret)
        {
            ssdb::Status st = ssdb_->zclear(name, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zclear function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zrank(const std::string &name, const std::string &key, int64_t *ret)
        {
            ssdb::Status st = ssdb_->zrank(name, key, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zrank function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zrrank(const std::string &name, const std::string &key, int64_t *ret)
        {
            ssdb::Status st = ssdb_->zrrank(name, key, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zrrank function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zrange(const std::string &name,
                uint64_t offset, uint64_t limit,
                std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->zrange(name, offset, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zrange function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zrrange(const std::string &name,
                uint64_t offset, uint64_t limit,
                std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->zrrange(name, offset, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zrrange function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zkeys(const std::string &name, const std::string &key_start,
            int64_t *score_start, int64_t *score_end,
            uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->zkeys(name, key_start, score_start, score_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zkeys function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zscan(const std::string &name, const std::string &key_start,
            int64_t *score_start, int64_t *score_end,
            uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->zscan(name, key_start, score_start, score_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zscan function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr zrscan(const std::string &name, const std::string &key_start,
            int64_t *score_start, int64_t *score_end,
            uint64_t limit, std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->zrscan(name, key_start, score_start, score_end, limit, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "zrscan function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr multi_zget(const std::string &name, const std::vector<std::string> &keys,
            std::vector<std::string> *ret)
        {
            ssdb::Status st = ssdb_->multi_zget(name, keys, ret);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "multi_zget function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr multi_zset(const std::string &name, const std::map<std::string, int64_t> &kss)
        {
            ssdb::Status st = ssdb_->multi_zset(name, kss);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "multi_zset function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr multi_zdel(const std::string &name, const std::vector<std::string> &keys)
        {
            ssdb::Status st = ssdb_->multi_zdel(name, keys);
            if (st.error()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "multi_zdel function error: %s", st.code());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        void init()
        {

        }

        void clear()
        {
            delete ssdb_;
            ssdb_ = NULL;
        }

        ssdb::Client* ssdb_;
    };

    SsdbDriver::SsdbDriver(IConnectionSettingsBaseSPtr settings)
        : IDriver(settings), impl_(new pimpl)
    {

    }

    SsdbDriver::~SsdbDriver()
    {
        delete impl_;
    }

    bool SsdbDriver::isConnected() const
    {
        return impl_->isConnected();
    }

    bool SsdbDriver::isAuthenticated() const
    {
        return impl_->isConnected();
    }

    void SsdbDriver::interrupt()
    {
        impl_->config_.shutdown_ = 1;
    }

    // ============== commands =============//
    common::ErrorValueSPtr SsdbDriver::commandDeleteImpl(CommandDeleteKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        const NKey key = command->key();
        common::SNPrintf(patternResult, sizeof(patternResult), DELETE_KEY_PATTERN_1ARGS_S, key.key_);
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr SsdbDriver::commandLoadImpl(CommandLoadKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        const NKey key = command->key();
        if(key.type_ == common::Value::TYPE_ARRAY){
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_LIST_PATTERN_1ARGS_S, key.key_);
        }
        else if(key.type_ == common::Value::TYPE_SET){
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_SET_PATTERN_1ARGS_S, key.key_);
        }
        else if(key.type_ == common::Value::TYPE_ZSET){
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_ZSET_PATTERN_1ARGS_S, key.key_);
        }
        else if(key.type_ == common::Value::TYPE_HASH){
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_HASH_PATTERN_1ARGS_S, key.key_);
        }
        else{
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_PATTERN_1ARGS_S, key.key_);\
        }
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr SsdbDriver::commandCreateImpl(CommandCreateKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        const NKey key = command->key();
        FastoObjectIPtr val = command->value();
        if(key.type_ == common::Value::TYPE_ARRAY){
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_LIST_PATTERN_2ARGS_SS, key.key_, val->toString());
        }
        else if(key.type_ == common::Value::TYPE_SET){
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_SET_PATTERN_2ARGS_SS, key.key_, val->toString());
        }
        else if(key.type_ == common::Value::TYPE_ZSET){
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_ZSET_PATTERN_2ARGS_SS, key.key_, val->toString());
        }
        else if(key.type_ == common::Value::TYPE_HASH){
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_HASH_PATTERN_2ARGS_SS, key.key_, val->toString());
        }
        else{
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_PATTERN_2ARGS_SS, key.key_, val->toString());\
        }
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }
     // ============== commands =============//

    common::net::hostAndPort SsdbDriver::address() const
    {
        return common::net::hostAndPort(impl_->config_.hostip_, impl_->config_.hostport_);
    }

    std::string SsdbDriver::version() const
    {
        return versionApi();
    }

    std::string SsdbDriver::outputDelemitr() const
    {
        return impl_->config_.mb_delim_;
    }

    const char* SsdbDriver::versionApi()
    {
        return "1.8.0";
    }

    void SsdbDriver::customEvent(QEvent *event)
    {
        IDriver::customEvent(event);
        impl_->config_.shutdown_ = 0;
    }

    void SsdbDriver::initImpl()
    {
        currentDatabaseInfo_.reset(new SsdbDataBaseInfo("0", 0, true));
    }

    void SsdbDriver::clearImpl()
    {

    }

    common::ErrorValueSPtr SsdbDriver::currentLoggingInfo(ServerInfo **info)
    {
        *info = NULL;
        LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
        SsdbServerInfo::Common cm;
        common::ErrorValueSPtr err = impl_->info(NULL, cm);
        if(!err){
            *info = new SsdbServerInfo(cm);
        }

        return err;
    }

    common::ErrorValueSPtr SsdbDriver::serverDiscoveryInfo(ServerDiscoveryInfo** dinfo)
    {
        *dinfo = NULL;
        FastoObjectIPtr root = FastoObject::createRoot(GET_SERVER_TYPE);
        SsdbCommand* cmd = createCommand(root, GET_SERVER_TYPE, common::Value::C_INNER);
        common::ErrorValueSPtr er = impl_->execute(cmd);

        if(!er){
            FastoObject::child_container_type ch = root->childrens();
            if(ch.size()){
                //*dinfo = makeOwnRedisDiscoveryInfo(ch[0]);
            }
        }
        return er;
    }

    void SsdbDriver::handleConnectEvent(events::ConnectRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ConnectResponceEvent::value_type res(ev->value());
            SsdbConnectionSettings *set = dynamic_cast<SsdbConnectionSettings*>(settings_.get());
            if(set){
                impl_->config_ = set->info();
                impl_->sinfo_ = set->sshInfo();
        notifyProgress(sender, 25);
                    common::ErrorValueSPtr er = impl_->connect();
                    if(er){
                        res.setErrorInfo(er);
                    }
        notifyProgress(sender, 75);
            }
            reply(sender, new events::ConnectResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void SsdbDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::DisconnectResponceEvent::value_type res(ev->value());
        notifyProgress(sender, 50);

            common::ErrorValueSPtr er = impl_->disconnect();
            if(er){
                res.setErrorInfo(er);
            }

            reply(sender, new events::DisconnectResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void SsdbDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ExecuteRequestEvent::value_type res(ev->value());
            const char *inputLine = common::utils::c_strornull(res.text_);

            common::ErrorValueSPtr er;
            if(inputLine){
                size_t length = strlen(inputLine);
                int offset = 0;
                RootLocker lock = make_locker(sender, inputLine);
                FastoObjectIPtr outRoot = lock.root_;
                double step = 100.0f/length;
                for(size_t n = 0; n < length; ++n){
                    if(impl_->config_.shutdown_){
                        er.reset(new common::ErrorValue("Interrupted exec.", common::ErrorValue::E_INTERRUPTED));
                        res.setErrorInfo(er);
                        break;
                    }
                    if(inputLine[n] == '\n' || n == length-1){
        notifyProgress(sender, step * n);
                        char command[128] = {0};
                        if(n == length-1){
                            strcpy(command, inputLine + offset);
                        }
                        else{
                            strncpy(command, inputLine + offset, n - offset);
                        }
                        offset = n + 1;
                        SsdbCommand* cmd = createCommand(outRoot, stableCommand(command), common::Value::C_USER);
                        er = impl_->execute(cmd);
                        if(er){
                            res.setErrorInfo(er);
                            break;
                        }
                    }
                }
            }
            else{
                er.reset(new common::ErrorValue("Empty command line.", common::ErrorValue::E_ERROR));
            }

            if(er){
                LOG_ERROR(er, true);
            }
        notifyProgress(sender, 100);
    }

    void SsdbDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::CommandResponceEvent::value_type res(ev->value());
            std::string cmdtext;
            common::ErrorValueSPtr er = commandByType(res.cmd_, cmdtext);
            if(er){
                res.setErrorInfo(er);
                reply(sender, new events::CommandResponceEvent(this, res));
                notifyProgress(sender, 100);
                return;
            }

            RootLocker lock = make_locker(sender, cmdtext);
            FastoObjectIPtr root = lock.root_;
            SsdbCommand* cmd = createCommand(root, cmdtext, common::Value::C_INNER);
        notifyProgress(sender, 50);
            er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            reply(sender, new events::CommandResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void SsdbDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev)
    {
        QObject *sender = ev->sender();
    notifyProgress(sender, 0);
        events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
    notifyProgress(sender, 50);
        res.databases_.push_back(currentDatabaseInfo_);
        reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    notifyProgress(sender, 100);
    }

    void SsdbDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
            char patternResult[1024] = {0};
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEYS_PATTERN_1ARGS_I, res.countKeys_);
            FastoObjectIPtr root = FastoObject::createRoot(patternResult);
        notifyProgress(sender, 50);
            SsdbCommand* cmd = createCommand(root, patternResult, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            else{
                FastoObject::child_container_type rchildrens = cmd->childrens();
                if(rchildrens.size()){
                    DCHECK(rchildrens.size() == 1);
                    FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0]);
                    if(!array){
                        goto done;
                    }
                    common::ArrayValue* ar = array->array();
                    if(!ar){
                        goto done;
                    }

                    for(int i = 0; i < ar->getSize(); ++i)
                    {
                        std::string key;
                        bool isok = ar->getString(i, &key);
                        if(isok){
                            NKey ress(key);
                            res.keys_.push_back(ress);
                        }
                    }
                }
            }
    done:
        notifyProgress(sender, 75);
            reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void SsdbDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::SetDefaultDatabaseResponceEvent::value_type res(ev->value());
        notifyProgress(sender, 50);
            reply(sender, new events::SetDefaultDatabaseResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void SsdbDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ServerInfoResponceEvent::value_type res(ev->value());
        notifyProgress(sender, 50);
            LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
            SsdbServerInfo::Common cm;
            common::ErrorValueSPtr err = impl_->info(NULL, cm);
            if(err){
                res.setErrorInfo(err);
            }
            else{
                ServerInfoSPtr mem(new SsdbServerInfo(cm));
                res.setInfo(mem);
            }
        notifyProgress(sender, 75);
            reply(sender, new events::ServerInfoResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void SsdbDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev)
    {

    }

    ServerInfoSPtr SsdbDriver::makeServerInfoFromString(const std::string& val)
    {
        ServerInfoSPtr res(makeSsdbServerInfo(val));
        return res;
    }
}
