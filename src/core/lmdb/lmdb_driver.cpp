#include "core/lmdb/lmdb_driver.h"

extern "C" {
    #include "sds.h"
    #include <lmdb.h>
}

#include "common/sprintf.h"
#include "common/utils.h"
#include "common/file_system.h"
#include "fasto/qt/logger.h"

#include "core/command_logger.h"

#include "core/lmdb/lmdb_config.h"
#include "core/lmdb/lmdb_infos.h"

#define INFO_REQUEST "INFO"
#define GET_KEY_PATTERN_1ARGS_S "FETCH %s"
#define SET_KEY_PATTERN_2ARGS_SS "STORE %s %s"

#define GET_KEYS_PATTERN_1ARGS_I "KEYS a z %d"
#define DELETE_KEY_PATTERN_1ARGS_S "DEL %s"
#define GET_SERVER_TYPE ""
#define LMDB_OK 0

namespace
{
    struct lmdb
    {
        MDB_env *env;
        MDB_dbi dbi;
        MDB_txn *txn;
    };

    int lmdb_open(lmdb **context, const char *dbname, bool create_if_missing)
    {
        if(create_if_missing){
            bool res = common::file_system::create_directory(dbname, true);
            UNUSED(res);
            if(common::file_system::is_directory(dbname) != SUCCESS){
                return EACCES;
            }
        }

        lmdb *lcontext = (lmdb*)calloc(1, sizeof(lmdb));
        int rc = mdb_env_create(&lcontext->env);
        if(rc != LMDB_OK){
            free(lcontext);
            return rc;
        }
        rc = mdb_env_open(lcontext->env, dbname, 0, 0664);
        if(rc != LMDB_OK){
            free(lcontext);
            return rc;
        }
        rc = mdb_txn_begin(lcontext->env, NULL, 0, &lcontext->txn);
        if(rc != LMDB_OK){
            free(lcontext);
            return rc;
        }

        rc = mdb_dbi_open(lcontext->txn, NULL, 0, &lcontext->dbi);
        if(rc != LMDB_OK){
            free(lcontext);
            return rc;
        }

        *context = lcontext;
        return rc;
    }

    void lmdb_close(lmdb **context)
    {
        lmdb *lcontext = *context;
        if(!lcontext){
            return;
        }

        mdb_dbi_close(lcontext->env, lcontext->dbi);
        mdb_env_close(lcontext->env);
        free(lcontext);
        *context = NULL;
    }
}

namespace fastonosql
{
    namespace
    {
        common::ErrorValueSPtr createConnection(const lmdbConfig& config, lmdb** context)
        {
            DCHECK(*context == NULL);

            lmdb* lcontext = NULL;
            int st = lmdb_open(&lcontext, config.dbname_.c_str(), config.create_if_missing_);
            if (st != LMDB_OK){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Fail open database: %s", mdb_strerror(st));
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            *context = lcontext;

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr createConnection(LmdbConnectionSettings* settings, lmdb** context)
        {
            if(!settings){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            lmdbConfig config = settings->info();
            return createConnection(config, context);
        }
    }

    common::ErrorValueSPtr testConnection(fastonosql::LmdbConnectionSettings *settings)
    {
        lmdb* ldb = NULL;
        common::ErrorValueSPtr er = createConnection(settings, &ldb);
        if(er){
            return er;
        }

        lmdb_close(&ldb);

        return common::ErrorValueSPtr();
    }

    struct LmdbDriver::pimpl
    {
        pimpl()
            : lmdb_(NULL)
        {

        }

        bool isConnected() const
        {
            if(!lmdb_){
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

            lmdb* context = NULL;
            common::ErrorValueSPtr er = createConnection(config_, &context);
            if(er){
                return er;
            }

            lmdb_ = context;


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

        MDB_dbi curDb() const
        {
            if(lmdb_){
                return lmdb_->dbi;
            }

            return 0;
        }

        common::ErrorValueSPtr info(const char* args, LmdbServerInfo::Stats& statsout)
        {
            /*std::string rets;
            bool isok = rocksdb_->GetProperty("rocksdb.stats", &rets);
            if (!isok){
                return common::make_error_value("info function failed", common::ErrorValue::E_ERROR);
            }

            if(rets.size() > sizeof(ROCKSDB_HEADER_STATS)){
                const char * retsc = rets.c_str() + sizeof(ROCKSDB_HEADER_STATS);
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
            }*/

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr execute(FastoObjectCommand* cmd) WARN_UNUSED_RESULT
        {
            //DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            const std::string command = cmd->cmd()->inputCommand();
            common::Value::CommandLoggingType type = cmd->cmd()->commandLoggingType();

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

        lmdbConfig config_;
        SSHInfo sinfo_;

   private:
        common::ErrorValueSPtr execute(FastoObject* out, int argc, char **argv)
        {
            if(strcasecmp(argv[0], "info") == 0){
                if(argc > 2){
                    return common::make_error_value("Invalid info input argument", common::ErrorValue::E_ERROR);
                }

                LmdbServerInfo::Stats statsout;
                common::ErrorValueSPtr er = info(argc == 2 ? argv[1] : 0, statsout);
                if(!er){
                    common::StringValue *val = common::Value::createStringValue(LmdbServerInfo(statsout).toString());
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
                    return common::make_error_value("Invalid put input argument", common::ErrorValue::E_ERROR);
                }

                common::ErrorValueSPtr er = put(argv[1], argv[2]);
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

        common::ErrorValueSPtr get(const std::string& key, std::string* ret_val)
        {
            MDB_val mkey;
            mkey.mv_size = key.size();
            mkey.mv_data = (void*)key.c_str();
            MDB_val mval;
            int rc = mdb_get(lmdb_->txn, lmdb_->dbi, &mkey, &mval);
            if (rc != LMDB_OK){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "get function error: %s", mdb_strerror(rc));
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            ret_val->assign((const char*)mval.mv_data, mval.mv_size);

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr put(const std::string& key, const std::string& value)
        {
            MDB_val mkey;
            mkey.mv_size = key.size();
            mkey.mv_data = (void*)key.c_str();
            MDB_val mval;
            mval.mv_size = value.size();
            mval.mv_data = (void*)value.c_str();

            int rc = mdb_put(lmdb_->txn, lmdb_->dbi, &mkey, &mval, 0);
            if (rc == LMDB_OK){
                rc = mdb_txn_commit(lmdb_->txn);
            }

            if (rc != LMDB_OK){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "put function error: %s", mdb_strerror(rc));
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr del(const std::string& key)
        {
            MDB_val mkey;
            mkey.mv_size = key.size();
            mkey.mv_data = (void*)key.c_str();
            int rc = mdb_del(lmdb_->txn, lmdb_->dbi, &mkey, NULL);
            if (rc != LMDB_OK){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "delete function error: %s", mdb_strerror(rc));
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr keys(const std::string &key_start, const std::string &key_end, uint64_t limit, std::vector<std::string> *ret)
        {
            int rc = mdb_txn_begin(lmdb_->env, NULL, MDB_RDONLY, &lmdb_->txn);
            MDB_cursor *cursor;
            if(rc == LMDB_OK){
                rc = mdb_cursor_open(lmdb_->txn, lmdb_->dbi, &cursor);
            }

            if(rc != LMDB_OK){
                char buff[1024] = {0};
                common::SNPrintf(buff, sizeof(buff), "Keys function error: %s", mdb_strerror(rc));
                return common::make_error_value(buff, common::ErrorValue::E_ERROR);
            }

            MDB_val key;
            MDB_val data;
            while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0 && limit > ret->size()) {
                std::string skey((const char*)key.mv_data, key.mv_size);
                if(key_start < skey && key_end > skey){
                    ret->push_back(skey);
                }
            }
            mdb_cursor_close(cursor);
            mdb_txn_abort(lmdb_->txn);

            return common::ErrorValueSPtr();
        }

        void init()
        {

        }

        void clear()
        {
            lmdb_close(&lmdb_);
        }

        lmdb* lmdb_;
    };

    LmdbDriver::LmdbDriver(IConnectionSettingsBaseSPtr settings)
        : IDriver(settings, LMDB), impl_(new pimpl)
    {

    }

    LmdbDriver::~LmdbDriver()
    {
        delete impl_;
    }

    bool LmdbDriver::isConnected() const
    {
        return impl_->isConnected();
    }

    bool LmdbDriver::isAuthenticated() const
    {
        return impl_->isConnected();
    }

    void LmdbDriver::interrupt()
    {
        impl_->config_.shutdown_ = 1;
    }

    // ============== commands =============//
    common::ErrorValueSPtr LmdbDriver::commandDeleteImpl(CommandDeleteKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        NDbValue key = command->key();
        common::SNPrintf(patternResult, sizeof(patternResult), DELETE_KEY_PATTERN_1ARGS_S, key.keyString());
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr LmdbDriver::commandLoadImpl(CommandLoadKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        NDbValue key = command->key();
        common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_PATTERN_1ARGS_S, key.keyString());
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr LmdbDriver::commandCreateImpl(CommandCreateKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        NDbValue key = command->key();
        NValue val = command->value();
        common::Value* rval = val.get();
        std::string key_str = key.keyString();
        std::string value_str = common::convertToString(rval, " ");
        common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_PATTERN_2ARGS_SS, key_str, value_str);
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr LmdbDriver::commandChangeTTLImpl(CommandChangeTTL* command, std::string& cmdstring) const
    {
        UNUSED(command);
        UNUSED(cmdstring);
        char errorMsg[1024] = {0};
        common::SNPrintf(errorMsg, sizeof(errorMsg), "Sorry, but now " PROJECT_NAME_TITLE " not supported change ttl command for %s.", common::convertToString(connectionType()));
        return common::make_error_value(errorMsg, common::ErrorValue::E_ERROR);
    }

     // ============== commands =============//

    common::net::hostAndPort LmdbDriver::address() const
    {
        //return common::net::hostAndPort(impl_->config_.hostip_, impl_->config_.hostport_);
        return common::net::hostAndPort();
    }

    std::string LmdbDriver::outputDelemitr() const
    {
        return impl_->config_.mb_delim_;
    }

    const char* LmdbDriver::versionApi()
    {
        return STRINGIZE(MDB_VERSION_MAJOR) "." STRINGIZE(MDB_VERSION_MINOR) "." STRINGIZE(MDB_VERSION_PATCH);
    }

    void LmdbDriver::customEvent(QEvent *event)
    {
        IDriver::customEvent(event);
        impl_->config_.shutdown_ = 0;
    }

    void LmdbDriver::initImpl()
    {
    }

    void LmdbDriver::clearImpl()
    {
    }

    common::ErrorValueSPtr LmdbDriver::serverInfo(ServerInfo **info)
    {
        LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
        LmdbServerInfo::Stats cm;
        common::ErrorValueSPtr err = impl_->info(NULL, cm);
        if(!err){
            *info = new LmdbServerInfo(cm);
        }

        return err;
    }

    common::ErrorValueSPtr LmdbDriver::serverDiscoveryInfo(ServerInfo **sinfo, ServerDiscoveryInfo **dinfo, DataBaseInfo** dbinfo)
    {
        ServerInfo *lsinfo = NULL;
        common::ErrorValueSPtr er = serverInfo(&lsinfo);
        if(er){
            return er;
        }

        FastoObjectIPtr root = FastoObject::createRoot(GET_SERVER_TYPE);
        FastoObjectCommand* cmd = createCommand<LmdbCommand>(root, GET_SERVER_TYPE, common::Value::C_INNER);
        er = impl_->execute(cmd);

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

    common::ErrorValueSPtr LmdbDriver::currentDataBaseInfo(DataBaseInfo** info)
    {
        *info = new LmdbDataBaseInfo(common::convertToString(impl_->curDb()), 0, true);
        return common::ErrorValueSPtr();
    }

    void LmdbDriver::handleConnectEvent(events::ConnectRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ConnectResponceEvent::value_type res(ev->value());
            LmdbConnectionSettings *set = dynamic_cast<LmdbConnectionSettings*>(settings_.get());
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

    void LmdbDriver::handleDisconnectEvent(events::DisconnectRequestEvent* ev)
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

    void LmdbDriver::handleExecuteEvent(events::ExecuteRequestEvent* ev)
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
                        FastoObjectCommand* cmd = createCommand<LmdbCommand>(outRoot, stableCommand(command), common::Value::C_USER);
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

    void LmdbDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev)
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
            FastoObjectCommand* cmd = createCommand<LmdbCommand>(root, cmdtext, common::Value::C_INNER);
        notifyProgress(sender, 50);
            er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            reply(sender, new events::CommandResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void LmdbDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev)
    {
        QObject *sender = ev->sender();
    notifyProgress(sender, 0);
        events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
    notifyProgress(sender, 50);
        res.databases_.push_back(currentDatabaseInfo());
        reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
    notifyProgress(sender, 100);
    }

    void LmdbDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
            char patternResult[1024] = {0};
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEYS_PATTERN_1ARGS_I, res.countKeys_);
            FastoObjectIPtr root = FastoObject::createRoot(patternResult);
        notifyProgress(sender, 50);
            FastoObjectCommand* cmd = createCommand<LmdbCommand>(root, patternResult, common::Value::C_INNER);
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
                            NKey k(key);
                            NDbValue ress(k, NValue());
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

    void LmdbDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::SetDefaultDatabaseResponceEvent::value_type res(ev->value());
        notifyProgress(sender, 50);
            reply(sender, new events::SetDefaultDatabaseResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void LmdbDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ServerInfoResponceEvent::value_type res(ev->value());
        notifyProgress(sender, 50);
            LOG_COMMAND(Command(INFO_REQUEST, common::Value::C_INNER));
            LmdbServerInfo::Stats cm;
            common::ErrorValueSPtr err = impl_->info(NULL, cm);
            if(err){
                res.setErrorInfo(err);
            }
            else{
                ServerInfoSPtr mem(new LmdbServerInfo(cm));
                res.setInfo(mem);
            }
        notifyProgress(sender, 75);
            reply(sender, new events::ServerInfoResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void LmdbDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev)
    {

    }

    ServerInfoSPtr LmdbDriver::makeServerInfoFromString(const std::string& val)
    {
        ServerInfoSPtr res(makeLmdbServerInfo(val));
        return res;
    }
}
