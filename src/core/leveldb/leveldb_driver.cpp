#include "core/leveldb/leveldb_driver.h"

#include <leveldb/db.h>

#include "common/sprintf.h"
#include "common/utils.h"
#include "fasto/qt/logger.h"

#include "core/command_logger.h"
#include "core/leveldb/leveldb_config.h"
#include "core/leveldb/leveldb_infos.h"

#define INFO_REQUEST "INFO"
#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define SET_KEY_PATTERN_2ARGS_SS "PUT %s %s"

#define GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define DELETE_KEY_PATTERN_1ARGS_S "DEL %s"
#define GET_SERVER_TYPE ""
#define LEVELDB_HEADER_STATS    "                               Compactions\n"\
                                "Level  Files Size(MB) Time(sec) Read(MB) Write(MB)\n"\
                                "--------------------------------------------------\n"

namespace
{
    std::once_flag leveldb_version_once;
    static void leveldb_version_startup_function(char * version)
    {
        sprintf(version, "%d.%d", leveldb::kMajorVersion, leveldb::kMinorVersion);
    }
}

namespace fastonosql
{
    namespace
    {
        common::ErrorValueSPtr createConnection(const leveldbConfig& config, leveldb::DB** context)
        {
            DCHECK(*context == NULL);

            leveldb::DB* lcontext = NULL;
            leveldb::Status st = leveldb::DB::Open(config.options_, config.dbname_, &lcontext);
            if (!st.ok()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Fail connect to server: %s!", st.ToString());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            *context = lcontext;

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr createConnection(LeveldbConnectionSettings* settings, leveldb::DB** context)
        {
            if(!settings){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            leveldbConfig config = settings->info();
            return createConnection(config, context);
        }
    }

    common::ErrorValueSPtr testConnection(LeveldbConnectionSettings* settings)
    {
        leveldb::DB* ldb = NULL;
        common::ErrorValueSPtr er = createConnection(settings, &ldb);
        if(er){
            return er;
        }

        delete ldb;

        return common::ErrorValueSPtr();
    }

    struct LeveldbDriver::pimpl
    {
        pimpl()
            : leveldb_(NULL)
        {

        }

        bool isConnected() const
        {
            if(!leveldb_){
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

            leveldb::DB* context = NULL;
            common::ErrorValueSPtr er = createConnection(config_, &context);
            if(er){
                return er;
            }

            leveldb_ = context;


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

        common::ErrorValueSPtr dbsize(size_t& size) WARN_UNUSED_RESULT
        {
            leveldb::ReadOptions ro;
            leveldb::Iterator* it = leveldb_->NewIterator(ro);
            size_t sz = 0;
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                sz++;
            }

            leveldb::Status st = it->status();
            delete it;

            if (!st.ok()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Couldn't determine DBSIZE error: %s", st.ToString());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            size = sz;
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr info(const char* args, LeveldbServerInfo::Stats& statsout)
        {
            //sstables
            //stats
            //char prop[1024] = {0};
            //common::SNPrintf(prop, sizeof(prop), "leveldb.%s", args ? args : "stats");

            std::string rets;
            bool isok = leveldb_->GetProperty("leveldb.stats", &rets);
            if (!isok){
                return common::make_error_value("info function failed", common::ErrorValue::E_ERROR);
            }

            if(rets.size() > sizeof(LEVELDB_HEADER_STATS)){
                const char * retsc = rets.c_str() + sizeof(LEVELDB_HEADER_STATS);
                char* p2 = strtok((char*)retsc, " ");
                int pos = 0;
                while(p2){
                    switch(pos++){
                        case 0:
                            statsout.compactions_level_ = atoi(p2);
                            break;
                        case 1:
                            statsout.file_size_mb_ = atoi(p2);
                            break;
                        case 2:
                            statsout.time_sec_ = atoi(p2);
                            break;
                        case 3:
                            statsout.read_mb_ = atoi(p2);
                            break;
                        case 4:
                            statsout.write_mb_ = atoi(p2);
                            break;
                        default:
                            break;
                    }
                    p2 = strtok(0, " ");
                }
            }

            return common::ErrorValueSPtr();
        }

        ~pimpl()
        {
            clear();
        }

        leveldbConfig config_;

        common::ErrorValueSPtr execute_impl(FastoObject* out, int argc, char **argv)
        {
            if(strcasecmp(argv[0], "info") == 0){
                if(argc > 2){
                    return common::make_error_value("Invalid info input argument", common::ErrorValue::E_ERROR);
                }

                LeveldbServerInfo::Stats statsout;
                common::ErrorValueSPtr er = info(argc == 2 ? argv[1] : 0, statsout);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue(LeveldbServerInfo(statsout).toString());
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "get") == 0){
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
            else if(strcasecmp(argv[0], "put") == 0){
                if(argc != 3){
                    return common::make_error_value("Invalid set input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = put(argv[1], argv[2]);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue("STORED");
                    FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(child);
                }
                return er;
            }
            else if(strcasecmp(argv[0], "dbsize") == 0){
                if(argc != 1){
                    return common::make_error_value("Invalid dbsize input argument", common::ErrorValue::E_ERROR);
                }

                size_t ret = 0;
                common::ErrorValueSPtr er = dbsize(ret);
                if(!er){
                    common::FundamentalValue *val = common::Value::createUIntegerValue(ret);
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
            else{
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Not supported command: %s", argv[0]);
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
        }

    private:
        common::ErrorValueSPtr get(const std::string& key, std::string* ret_val)
        {
            leveldb::ReadOptions ro;
            leveldb::Status st = leveldb_->Get(ro, key, ret_val);
            if (!st.ok()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "get function error: %s", st.ToString());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr put(const std::string& key, const std::string& value)
        {
            leveldb::WriteOptions wo;
            leveldb::Status st = leveldb_->Put(wo, key, value);
            if (!st.ok()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "put function error: %s", st.ToString());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr del(const std::string& key)
        {
            leveldb::WriteOptions wo;
            leveldb::Status st = leveldb_->Delete(wo, key);
            if (!st.ok()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "del function error: %s", st.ToString());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr keys(const std::string &key_start, const std::string &key_end, uint64_t limit, std::vector<std::string> *ret)
        {
            ret->clear();

            leveldb::ReadOptions ro;
            leveldb::Iterator* it = leveldb_->NewIterator(ro); //keys(key_start, key_end, limit, ret);
            for (it->Seek(key_start); it->Valid() && it->key().ToString() < key_end; it->Next()) {
                std::string key = it->key().ToString();
                if(ret->size() <= limit){
                    ret->push_back(key);
                }
                else{
                    break;
                }
            }

            leveldb::Status st = it->status();
            delete it;

            if (!st.ok()){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Keys function error: %s", st.ToString());
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }
            return common::ErrorValueSPtr();
        }

        void init()
        {

        }

        void clear()
        {
            delete leveldb_;
            leveldb_ = NULL;
        }

        leveldb::DB* leveldb_;
    };

    LeveldbDriver::LeveldbDriver(IConnectionSettingsBaseSPtr settings)
        : IDriver(settings, LEVELDB), impl_(new pimpl)
    {

    }

    LeveldbDriver::~LeveldbDriver()
    {
        delete impl_;
    }

    bool LeveldbDriver::isConnected() const
    {
        return impl_->isConnected();
    }

    bool LeveldbDriver::isAuthenticated() const
    {
        return impl_->isConnected();
    }

    // ============== commands =============//
    common::ErrorValueSPtr LeveldbDriver::commandDeleteImpl(CommandDeleteKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        NDbKValue key = command->key();
        common::SNPrintf(patternResult, sizeof(patternResult), DELETE_KEY_PATTERN_1ARGS_S, key.keyString());
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr LeveldbDriver::commandLoadImpl(CommandLoadKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        NDbKValue key = command->key();
        common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_PATTERN_1ARGS_S, key.keyString());
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr LeveldbDriver::commandCreateImpl(CommandCreateKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        NDbKValue key = command->key();
        NValue val = command->value();
        common::Value* rval = val.get();
        std::string key_str = key.keyString();
        std::string value_str = common::convertToString(rval, " ");
        common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr LeveldbDriver::commandChangeTTLImpl(CommandChangeTTL* command, std::string& cmdstring) const
    {
        UNUSED(command);
        UNUSED(cmdstring);
        char errorMsg[1024] = {0};
        common::SNPrintf(errorMsg, sizeof(errorMsg), "Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.", common::convertToString(connectionType()));
        return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
    }

     // ============== commands =============//

    common::net::hostAndPort LeveldbDriver::address() const
    {
        //return common::net::hostAndPort(impl_->config_.hostip_, impl_->config_.hostport_);
        return common::net::hostAndPort();
    }

    std::string LeveldbDriver::outputDelemitr() const
    {
        return impl_->config_.mb_delim_;
    }

    const char* LeveldbDriver::versionApi()
    {
        static char leveldb_version[32] = {0};
        std::call_once(leveldb_version_once, leveldb_version_startup_function, leveldb_version);
        return leveldb_version;
    }

    void LeveldbDriver::initImpl()
    {
    }

    void LeveldbDriver::clearImpl()
    {
    }

    common::ErrorValueSPtr LeveldbDriver::executeImpl(FastoObject* out, int argc, char **argv)
    {
        return impl_->execute_impl(out, argc, argv);
    }

    common::ErrorValueSPtr LeveldbDriver::serverInfo(ServerInfo **info)
    {
        LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
        LeveldbServerInfo::Stats cm;
        common::ErrorValueSPtr err = impl_->info(NULL, cm);
        if(!err){
            *info = new LeveldbServerInfo(cm);
        }

        return err;
    }

    common::ErrorValueSPtr LeveldbDriver::serverDiscoveryInfo(ServerInfo **sinfo, ServerDiscoveryInfo **dinfo, DataBaseInfo** dbinfo)
    {
        ServerInfo *lsinfo = NULL;
        common::ErrorValueSPtr er = serverInfo(&lsinfo);
        if(er){
            return er;
        }

        FastoObjectIPtr root = FastoObject::createRoot(GET_SERVER_TYPE);
        FastoObjectCommand* cmd = createCommand<LeveldbCommand>(root, GET_SERVER_TYPE, common::Value::C_INNER);
        er = execute(cmd);

        if(!er){
            FastoObject::child_container_type ch = root->childrens();
            if(ch.size()){
                //*dinfo = makeOwnRedisDiscoveryInfo(ch[0]);
            }
        }

        DataBaseInfo* ldbinfo = NULL;
        er = currentDataBaseInfo(&ldbinfo);
        if(er){
            delete lsinfo;
            return er;
        }

        *sinfo = lsinfo;
        *dbinfo = ldbinfo;
        return er;
    }

    common::ErrorValueSPtr LeveldbDriver::currentDataBaseInfo(DataBaseInfo** info)
    {
        size_t size = 0;
        impl_->dbsize(size);
        *info = new LeveldbDataBaseInfo("0", true, size);
        return common::ErrorValueSPtr();
    }

    void LeveldbDriver::handleConnectEvent(events::ConnectRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ConnectResponceEvent::value_type res(ev->value());
            LeveldbConnectionSettings *set = dynamic_cast<LeveldbConnectionSettings*>(settings_.get());
            if(set){
                impl_->config_ = set->info();
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

    void LeveldbDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev)
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

    void LeveldbDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev)
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
                    if(interrupt_){
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
                        FastoObjectCommand* cmd = createCommand<LeveldbCommand>(outRoot, stableCommand(command), common::Value::C_USER);
                        er = execute(cmd);
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

    void LeveldbDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev)
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
            FastoObjectCommand* cmd = createCommand<LeveldbCommand>(root, cmdtext, common::Value::C_INNER);
        notifyProgress(sender, 50);
            er = execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            reply(sender, new events::CommandResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void LeveldbDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev)
    {
        QObject *sender = ev->sender();
    notifyProgress(sender, 0);
        events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
    notifyProgress(sender, 50);
        res.databases_.push_back(currentDatabaseInfo());
        reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    notifyProgress(sender, 100);
    }

    void LeveldbDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
            char patternResult[1024] = {0};
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEYS_PATTERN_1ARGS_I, res.countKeys_);
            FastoObjectIPtr root = FastoObject::createRoot(patternResult);
        notifyProgress(sender, 50);
            FastoObjectCommand* cmd = createCommand<LeveldbCommand>(root, patternResult, common::Value::C_INNER);
            common::ErrorValueSPtr er = execute(cmd);
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

                    for(int i = 0; i < ar->size(); ++i)
                    {
                        std::string key;
                        bool isok = ar->getString(i, &key);
                        if(isok){
                            NKey k(key);
                            NDbKValue ress(k, NValue());
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

    void LeveldbDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::SetDefaultDatabaseResponceEvent::value_type res(ev->value());
        notifyProgress(sender, 50);
            reply(sender, new events::SetDefaultDatabaseResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void LeveldbDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ServerInfoResponceEvent::value_type res(ev->value());
        notifyProgress(sender, 50);
            LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
            LeveldbServerInfo::Stats cm;
            common::ErrorValueSPtr err = impl_->info(NULL, cm);
            if(err){
                res.setErrorInfo(err);
            }
            else{
                ServerInfoSPtr mem(new LeveldbServerInfo(cm));
                res.setInfo(mem);
            }
        notifyProgress(sender, 75);
            reply(sender, new events::ServerInfoResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void LeveldbDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev)
    {

    }

    ServerInfoSPtr LeveldbDriver::makeServerInfoFromString(const std::string& val)
    {
        ServerInfoSPtr res(makeLeveldbServerInfo(val));
        return res;
    }
}
