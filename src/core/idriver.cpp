#include "core/idriver.h"

#ifdef OS_WIN
#include <winsock2.h>
#else
#include <signal.h>
#endif

#include <QThread>
#include <QApplication>

#include "common/file_system.h"
#include "common/time.h"
#include "common/sprintf.h"

namespace
{
#ifdef OS_WIN
struct WinsockInit {
        WinsockInit() {
            WSADATA d;
            if ( WSAStartup(MAKEWORD(2,2), &d) != 0 ) {
                _exit(1);
            }
        }
        ~WinsockInit(){ WSACleanup(); }
    } winsock_init;
#else
    struct SigIgnInit
    {
        SigIgnInit(){
            signal(SIGPIPE, SIG_IGN);
        }
    } sig_init;
#endif

    const char magicNumber = 0x1E;
    std::string createStamp(common::time64_t time)
    {
        return magicNumber + common::convertToString(time) + '\n';
    }

    bool getStamp(const common::buffer_type& stamp, common::time64_t& timeOut)
    {
        if(stamp.empty()){
            return false;
        }

        if(stamp[0] != magicNumber){
            return false;
        }

        common::buffer_type cstamp = stamp;

        if(cstamp[cstamp.size() - 1] == '\n'){
            cstamp.resize(cstamp.size() - 1);
        }

        timeOut = common::convertFromString<common::time64_t>((const char*)(cstamp.c_str() + 1));

        return timeOut != 0;
    }
}

namespace fastonosql
{
    namespace
    {
        void notifyProgressImpl(IDriver* sender, QObject *reciver, int value)
        {
            IDriver::reply(reciver, new events::ProgressResponceEvent(sender, events::ProgressResponceEvent::value_type(value)));
        }

        template<typename event_request_type, typename event_responce_type>
        void replyNotImplementedYet(IDriver* sender, event_request_type* ev, const char* eventCommandText)
        {
            QObject* esender = ev->sender();
            notifyProgressImpl(sender, esender, 0);
            typename event_request_type::value_type res(ev->value());

            char patternResult[1024] = {0};
            common::SNPrintf(patternResult, sizeof(patternResult), "Sorry, but now " PROJECT_NAME_TITLE " not supported %s.", eventCommandText);

            common::ErrorValueSPtr er = common::make_error_value(patternResult, common::ErrorValue::E_ERROR);
            res.setErrorInfo(er);
            event_responce_type* resp = new event_responce_type(sender, res);
            IDriver::reply(esender, resp);
            notifyProgressImpl(sender, esender, 100);
        }
    }

    IDriver::IDriver(IConnectionSettingsBaseSPtr settings)
        : settings_(settings), serverDiscInfo_(), thread_(NULL), timer_info_id_(0), log_file_(NULL)
    {
        thread_ = new QThread(this);
        moveToThread(thread_);

        VERIFY(connect(thread_, &QThread::started, this, &IDriver::init));
        VERIFY(connect(thread_, &QThread::finished, this, &IDriver::clear));
    }

    IDriver::~IDriver()
    {
        delete log_file_;
        log_file_ = NULL;
    }

    void IDriver::reply(QObject *reciver, QEvent *ev)
    {
        qApp->postEvent(reciver, ev);
    }

    connectionTypes IDriver::connectionType() const
    {
        return settings_->connectionType();
    }

    ServerDiscoveryInfoSPtr IDriver::serverDiscoveryInfo() const
    {
        return serverDiscInfo_;
    }

    IConnectionSettingsBaseSPtr IDriver::settings() const
    {
        return settings_;
    }

    ServerInfoSPtr IDriver::serverInfo() const
    {
        return serverInfo_;
    }

    DataBaseInfoSPtr IDriver::currentDatabaseInfo() const
    {
        return currentDatabaseInfo_;
    }

    void IDriver::start()
    {
        thread_->start();
    }

    void IDriver::stop()
    {
        thread_->quit();
        thread_->wait();
    }

    common::ErrorValueSPtr IDriver::commandByType(CommandKeySPtr command, std::string& cmdstring) const
    {
        if(!command){
            return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
        }

        CommandKey::cmdtype t = command->type();

        if(t == CommandKey::C_DELETE){
            CommandDeleteKey* delc = dynamic_cast<CommandDeleteKey*>(command.get());
            if(!delc){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }
            return commandDeleteImpl(delc, cmdstring);
        }
        else if(t == CommandKey::C_LOAD){
            CommandLoadKey* loadc = dynamic_cast<CommandLoadKey*>(command.get());
            if(!loadc){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }
            return commandLoadImpl(loadc, cmdstring);
        }
        else if(t == CommandKey::C_CREATE){
            CommandCreateKey* createc = dynamic_cast<CommandCreateKey*>(command.get());
            if(!createc || !createc->value().get()){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }
            return commandCreateImpl(createc, cmdstring);
        }
        else{
            NOTREACHED();
            return common::make_error_value("Unknown command", common::ErrorValue::E_ERROR);
        }
    }

    void IDriver::init()
    {
        int interval = settings_->loggingMsTimeInterval();
        timer_info_id_ = startTimer(interval);
        DCHECK(timer_info_id_);
        initImpl();
    }

    void IDriver::clear()
    {
        killTimer(timer_info_id_);
        timer_info_id_ = 0;
    }

    void IDriver::customEvent(QEvent *event)
    {
        using namespace events;
        QEvent::Type type = event->type();
        if (type == static_cast<QEvent::Type>(ConnectRequestEvent::EventType)){            
            ConnectRequestEvent *ev = static_cast<ConnectRequestEvent*>(event);
            handleConnectEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ShutDownRequestEvent::EventType)){
            ShutDownRequestEvent *ev = static_cast<ShutDownRequestEvent*>(event);
            handleShutdownEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ProcessConfigArgsRequestEvent::EventType)){
            ProcessConfigArgsRequestEvent *ev = static_cast<ProcessConfigArgsRequestEvent*>(event);
            handleProcessCommandLineArgs(ev);
        }
        else if (type == static_cast<QEvent::Type>(DisconnectRequestEvent::EventType)){
            DisconnectRequestEvent *ev = static_cast<DisconnectRequestEvent*>(event);
            handleDisconnectEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ExecuteRequestEvent::EventType)){
            ExecuteRequestEvent *ev = static_cast<ExecuteRequestEvent*>(event);
            handleExecuteEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(LoadDatabasesInfoRequestEvent::EventType)){
            LoadDatabasesInfoRequestEvent *ev = static_cast<LoadDatabasesInfoRequestEvent*>(event);
            handleLoadDatabaseInfosEvent(ev);
        }        
        else if (type == static_cast<QEvent::Type>(ServerInfoRequestEvent::EventType)){
            ServerInfoRequestEvent *ev = static_cast<ServerInfoRequestEvent*>(event);
            handleLoadServerInfoEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ServerInfoHistoryRequestEvent::EventType)){
            ServerInfoHistoryRequestEvent *ev = static_cast<ServerInfoHistoryRequestEvent*>(event);
            handleLoadServerInfoHistoryEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ServerPropertyInfoRequestEvent::EventType)){
            ServerPropertyInfoRequestEvent *ev = static_cast<ServerPropertyInfoRequestEvent*>(event);
            handleLoadServerPropertyEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ChangeServerPropertyInfoRequestEvent::EventType)){
            ChangeServerPropertyInfoRequestEvent *ev = static_cast<ChangeServerPropertyInfoRequestEvent*>(event);
            handleServerPropertyChangeEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(BackupRequestEvent::EventType)){
            BackupRequestEvent *ev = static_cast<BackupRequestEvent*>(event);
            handleBackupEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ExportRequestEvent::EventType)){
            ExportRequestEvent *ev = static_cast<ExportRequestEvent*>(event);
            handleExportEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ChangePasswordRequestEvent::EventType)){
            ChangePasswordRequestEvent *ev = static_cast<ChangePasswordRequestEvent*>(event);
            handleChangePasswordEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ChangeMaxConnectionRequestEvent::EventType)){
            ChangeMaxConnectionRequestEvent *ev = static_cast<ChangeMaxConnectionRequestEvent*>(event);
            handleChangeMaxConnectionEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(LoadDatabaseContentRequestEvent::EventType)){
            LoadDatabaseContentRequestEvent *ev = static_cast<LoadDatabaseContentRequestEvent*>(event);
            handleLoadDatabaseContentEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(SetDefaultDatabaseRequestEvent::EventType)){
            SetDefaultDatabaseRequestEvent *ev = static_cast<SetDefaultDatabaseRequestEvent*>(event);
            handleSetDefaultDatabaseEvent(ev);
        }
        else if(type == static_cast<QEvent::Type>(CommandRequestEvent::EventType)){
            events::CommandRequestEvent* ev = static_cast<events::CommandRequestEvent*>(event);
            handleCommandRequestEvent(ev);
        }
        else if(type == static_cast<QEvent::Type>(DiscoveryInfoRequestEvent::EventType)){
            events::DiscoveryInfoRequestEvent* ev = static_cast<events::DiscoveryInfoRequestEvent*>(event);
            handleDiscoveryInfoRequestEvent(ev);
        }
        return QObject::customEvent(event);
    }

    void IDriver::timerEvent(QTimerEvent* event)
    {
        if(timer_info_id_ == event->timerId() && isConnected() && settings_->loggingEnabled()){
            if(!log_file_){
                std::string path = settings_->loggingPath();
                std::string dir = common::file_system::get_dir_path(path);
                bool res = common::file_system::create_directory(dir, true);
                UNUSED(res);
                if(common::file_system::is_directory(dir) == SUCCESS){
                    common::file_system::Path p(path);
                    log_file_ = new common::file_system::File(p);
                }
            }

            if(log_file_ && !log_file_->isOpened()){
                bool opened = log_file_->open("ab+");
                DCHECK(opened);
            }

            if(log_file_ && log_file_->isOpened()){
                common::time64_t time = common::time::current_mstime();
                std::string stamp = createStamp(time);
                ServerInfo* info = NULL;
                common::ErrorValueSPtr er = serverInfo(&info);
                if(er && er->isError()){
                    QObject::timerEvent(event);
                    return;
                }

                ServerInfoSnapShoot shot(time, ServerInfoSPtr(info));
                emit serverInfoSnapShoot(shot);

                log_file_->write(stamp);
                log_file_->write(info->toString());
                log_file_->flush();
            }
        }
        QObject::timerEvent(event);
    }

    void IDriver::notifyProgress(QObject *reciver, int value)
    {
        notifyProgressImpl(this, reciver, value);
    }

    void IDriver::handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev)
    {
        replyNotImplementedYet<events::ServerPropertyInfoRequestEvent, events::ServerPropertyInfoResponceEvent>(this, ev, "server property command");
    }

    void IDriver::handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev)
    {
        replyNotImplementedYet<events::ChangeServerPropertyInfoRequestEvent, events::ChangeServerPropertyInfoResponceEvent>(this, ev, "change server property command");
    }

    void IDriver::handleShutdownEvent(events::ShutDownRequestEvent* ev)
    {
        replyNotImplementedYet<events::ShutDownRequestEvent, events::ShutDownResponceEvent>(this, ev, "shutdown command");
    }

    void IDriver::handleBackupEvent(events::BackupRequestEvent* ev)
    {
        replyNotImplementedYet<events::BackupRequestEvent, events::BackupResponceEvent>(this, ev, "backup server command");
    }

    void IDriver::handleExportEvent(events::ExportRequestEvent* ev)
    {
        replyNotImplementedYet<events::ExportRequestEvent, events::ExportResponceEvent>(this, ev, "export server command");
    }

    void IDriver::handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev)
    {
        replyNotImplementedYet<events::ChangePasswordRequestEvent, events::ChangePasswordResponceEvent>(this, ev, "change password command");
    }

    void IDriver::handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev)
    {
        replyNotImplementedYet<events::ChangeMaxConnectionRequestEvent, events::ChangeMaxConnectionResponceEvent>(this, ev, "change maximum connection command");
    }

    IDriver::RootLocker::RootLocker(IDriver* parent, QObject *reciver, const std::string &text)
        : parent_(parent), reciver_(reciver), tstart_(common::time::current_mstime())
    {
        DCHECK(parent_);
        root_ = createRoot(reciver, text);
    }

    IDriver::RootLocker::~RootLocker()
    {
        events::CommandRootCompleatedEvent::value_type res(tstart_, root_);
        reply(reciver_, new events::CommandRootCompleatedEvent(parent_, res));
    }

    FastoObjectIPtr IDriver::RootLocker::createRoot(QObject *reciver, const std::string& text)
    {
        FastoObjectIPtr root = FastoObject::createRoot(text, parent_);
        events::CommandRootCreatedEvent::value_type res(root);
        reply(reciver, new events::CommandRootCreatedEvent(parent_, res));
        return root;
    }

    void IDriver::setCurrentDatabaseInfo(DataBaseInfo *inf)
    {
        currentDatabaseInfo_.reset(inf);
    }

    void IDriver::handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        events::ServerInfoHistoryResponceEvent::value_type res(ev->value());        

        std::string path = settings_->loggingPath();
        common::file_system::Path p(path);

        common::file_system::File readFile(p);
        if(readFile.open("rb")){
            events::ServerInfoHistoryResponceEvent::value_type::infos_container_type tmpInfos;

            common::time64_t curStamp = 0;
            common::buffer_type dataInfo;

            while(!readFile.isEof()){
                common::buffer_type data;
                bool res = readFile.readLine(data);
                if(!res || readFile.isEof()){
                    if(curStamp){
                        tmpInfos.push_back(ServerInfoSnapShoot(curStamp, makeServerInfoFromString(common::convertToString(dataInfo))));
                    }
                    break;
                }

                common::time64_t tmpStamp = 0;
                bool isSt = getStamp(data, tmpStamp);
                if(isSt){
                    if(curStamp){
                        tmpInfos.push_back(ServerInfoSnapShoot(curStamp, makeServerInfoFromString(common::convertToString(dataInfo))));
                    }
                    curStamp = tmpStamp;
                    dataInfo.clear();
                }
                else{
                    dataInfo += data;
                }
            }
            res.setInfos(tmpInfos);
        }
        else{
           common::ErrorValueSPtr er(new common::ErrorValue("Logging file not found", common::ErrorValue::E_ERROR));
           res.setErrorInfo(er);
        }

        reply(sender, new events::ServerInfoHistoryResponceEvent(this, res));
    }

    void IDriver::handleDiscoveryInfoRequestEvent(events::DiscoveryInfoRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        events::DiscoveryInfoResponceEvent::value_type res(ev->value());

        if(isConnected()){
            ServerDiscoveryInfo* disc = NULL;
            ServerInfo* info = NULL;
            DataBaseInfo* db = NULL;
            common::ErrorValueSPtr er = serverDiscoveryInfo(&info, &disc, &db);
            if(!er){
               DCHECK(info);
               DCHECK(db);
               serverInfo_.reset(info);
               serverDiscInfo_.reset(disc);
               currentDatabaseInfo_.reset(db);

               res.sinfo_ = serverInfo_;
               res.dinfo_ = serverDiscInfo_;
               res.dbinfo_ = currentDatabaseInfo_;
            }
            else{
                res.setErrorInfo(er);
            }
        }
        else{
            res.setErrorInfo(common::make_error_value("Not connected to server, impossible to get discovery info!", common::Value::E_ERROR));
        }

        reply(sender, new events::DiscoveryInfoResponceEvent(this, res));
    }

    void IDriver::addedChildren(FastoObject* child)
    {
        DCHECK(child);
        if(!child){
            return;
        }

        emit addedChild(child);
    }

    void IDriver::updated(FastoObject* item, common::Value* val)
    {
        const QString value = common::convertFromString<QString>(val->toString());
        emit itemUpdated(item, value);
    }
}
