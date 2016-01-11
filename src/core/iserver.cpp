#include "core/iserver.h"

#include <QApplication>

#include "common/qt/convert_string.h"
#include "fasto/qt/logger.h"
#include "common/net/net.h"

#include "core/idatabase.h"
#include "core/idriver.h"

namespace
{
    using namespace fastonosql;

    template<bool isConnect>
    struct connectFunct
    {
        template<typename t1, typename t2>
        bool operator()(const IServer* sender, t1 signal, const IServer* receiver, t2 member, Qt::ConnectionType type) const
        {
            return QObject::disconnect(sender, signal, receiver, member);
        }
    };

    template<>
    struct connectFunct<true>
    {
        template<typename t1, typename t2>
        bool operator()(const IServer* sender, t1 signal, const IServer* receiver, t2 member, Qt::ConnectionType type) const
        {
            return QObject::connect(sender, signal, receiver, member, type);
        }
    };

    template<bool con>
    void syncServersFunct(IServer *src, IServer *dsc)
    {
        if(!src || !dsc){
            return;
        }

        connectFunct<con> func;
        func(src, &IServer::startedConnect, dsc, &IServer::startedConnect, Qt::UniqueConnection);
        func(src, &IServer::finishedConnect, dsc, &IServer::finishedConnect, Qt::UniqueConnection);

        func(src, &IServer::startedDisconnect, dsc, &IServer::startedDisconnect, Qt::UniqueConnection);
        func(src, &IServer::finishedDisconnect, dsc, &IServer::finishedDisconnect, Qt::UniqueConnection);

        func(src, &IServer::startedExecute, dsc, &IServer::startedExecute, Qt::UniqueConnection);

        func(src, &IServer::rootCreated, dsc, &IServer::rootCreated, Qt::UniqueConnection);
        func(src, &IServer::rootCompleated, dsc, &IServer::rootCompleated, Qt::UniqueConnection);

        func(src, &IServer::addedChild, dsc, &IServer::addedChild, Qt::UniqueConnection);
        func(src, &IServer::itemUpdated, dsc, &IServer::itemUpdated, Qt::UniqueConnection);
        func(src, &IServer::serverInfoSnapShoot, dsc, &IServer::serverInfoSnapShoot, Qt::UniqueConnection);
   }
}

namespace fastonosql
{
    IServerBase::~IServerBase()
    {

    }

    IServer::IServer(IDriverSPtr drv, bool isSuperServer)
        : drv_(drv), isSuperServer_(isSuperServer)
    {
        if(isSuperServer_){
            VERIFY(QObject::connect(drv_.get(), &IDriver::addedChild, this, &IServer::addedChild));
            VERIFY(QObject::connect(drv_.get(), &IDriver::itemUpdated, this, &IServer::itemUpdated));
            VERIFY(QObject::connect(drv_.get(), &IDriver::serverInfoSnapShoot, this, &IServer::serverInfoSnapShoot));
        }
    }

    IServer::~IServer()
    {
    }

    void IServer::stopCurrentEvent()
    {
        drv_->interrupt();
    }

    bool IServer::isConnected() const
    {
        return drv_->isConnected();
    }

    bool IServer::isAuthenticated() const
    {
        return drv_->isAuthenticated();
    }

    bool IServer::isSuperServer() const
    {
        return isSuperServer_;
    }

    bool IServer::isLocalHost() const
    {
        return drv_->address().isLocalHost();
    }

    connectionTypes IServer::type() const
    {
        return drv_->connectionType();
    }

    QString IServer::name() const
    {
        return common::convertFromString<QString>(drv_->settings()->connectionName());
    }

    IDriverSPtr IServer::driver() const
    {
        return drv_;
    }

    ServerDiscoveryInfoSPtr IServer::discoveryInfo() const
    {
        return drv_->serverDiscoveryInfo();
    }

    ServerInfoSPtr IServer::serverInfo() const
    {
        return drv_->serverInfo();
    }

    DataBaseInfoSPtr IServer::currentDatabaseInfo() const
    {
        return drv_->currentDatabaseInfo();
    }

    QString IServer::address() const
    {
        std::string shost = common::convertToString(drv_->address());
        return common::convertFromString<QString>(shost);
    }

    QString IServer::outputDelemitr() const
    {
        return common::convertFromString<QString>(drv_->outputDelemitr());
    }

    IDatabaseSPtr IServer::findDatabaseByInfo(DataBaseInfoSPtr inf) const
    {
        DCHECK(inf);
        if(!inf){
            return IDatabaseSPtr();
        }

        DCHECK(type() == inf->type());
        if(type() != inf->type()){
            return IDatabaseSPtr();
        }

        for(int i = 0; i < databases_.size(); ++i){
            DataBaseInfoSPtr db = databases_[i]->info();
            if(db->name() == inf->name()){
                return databases_[i];
            }
        }

        return IDatabaseSPtr();
    }

    IDatabaseSPtr IServer::findDatabaseByName(const std::string& name) const
    {
        for(int i = 0; i < databases_.size(); ++i){
            DataBaseInfoSPtr db = databases_[i]->info();
            if(db->name() == name){
                return databases_[i];
            }
        }

        return IDatabaseSPtr();
    }

    void IServer::syncWithServer(IServer *src)
    {
        DCHECK(src != this);
        syncServersFunct<true>(src, this);
        //syncServersFunct<true>(this, src);
    }

    void IServer::unSyncFromServer(IServer *src)
    {
        DCHECK(src != this);
        syncServersFunct<false>(src, this);
        //syncServersFunct<false>(this, src);
    }

    void IServer::connect(const EventsInfo::ConnectInfoRequest& req)
    {
        emit startedConnect(req);
        QEvent *ev = new events::ConnectRequestEvent(this, req);
        notify(ev);
    }

    void IServer::disconnect(const EventsInfo::DisConnectInfoRequest& req)
    {
        emit startedDisconnect(req);
        QEvent *ev = new events::DisconnectRequestEvent(this, req);
        notify(ev);
    }

    void IServer::loadDatabases(const EventsInfo::LoadDatabasesInfoRequest& req)
    {
        emit startedLoadDatabases(req);
        QEvent *ev = new events::LoadDatabasesInfoRequestEvent(this, req);
        notify(ev);
    }

    void IServer::loadDatabaseContent(const EventsInfo::LoadDatabaseContentRequest& req)
    {
        emit startedLoadDataBaseContent(req);
        QEvent *ev = new events::LoadDatabaseContentRequestEvent(this, req);
        notify(ev);
    }

    void IServer::setDefaultDb(const EventsInfo::SetDefaultDatabaseRequest& req)
    {
        emit startedSetDefaultDatabase(req);
        QEvent *ev = new events::SetDefaultDatabaseRequestEvent(this, req);
        notify(ev);
    }

    void IServer::execute(const EventsInfo::ExecuteInfoRequest& req)
    {
        emit startedExecute(req);
        QEvent *ev = new events::ExecuteRequestEvent(this, req);
        notify(ev);
    }

    void IServer::executeCommand(const EventsInfo::CommandRequest& req)
    {
        emit startedExecuteCommand(req);
        QEvent *ev = new events::CommandRequestEvent(this, req);
        notify(ev);
    }

    void IServer::shutDown(const EventsInfo::ShutDownInfoRequest& req)
    {
        emit startedShutdown(req);
        QEvent *ev = new events::ShutDownRequestEvent(this, req);
        notify(ev);
    }

    void IServer::backupToPath(const EventsInfo::BackupInfoRequest& req)
    {
        emit startedBackup(req);
        QEvent *ev = new events::BackupRequestEvent(this, req);
        notify(ev);
    }

    void IServer::exportFromPath(const EventsInfo::ExportInfoRequest& req)
    {
        emit startedExport(req);
        QEvent *ev = new events::ExportRequestEvent(this, req);
        notify(ev);
    }

    void IServer::changePassword(const EventsInfo::ChangePasswordRequest& req)
    {
        emit startedChangePassword(req);
        QEvent *ev = new events::ChangePasswordRequestEvent(this, req);
        notify(ev);
    }

    void IServer::setMaxConnection(const EventsInfo::ChangeMaxConnectionRequest& req)
    {
        emit startedChangeMaxConnection(req);
        QEvent *ev = new events::ChangeMaxConnectionRequestEvent(this, req);
        notify(ev);
    }

    void IServer::loadServerInfo(const EventsInfo::ServerInfoRequest& req)
    {
        emit startedLoadServerInfo(req);
        QEvent *ev = new events::ServerInfoRequestEvent(this, req);
        notify(ev);
    }

    void IServer::serverProperty(const EventsInfo::ServerPropertyInfoRequest& req)
    {
        emit startedLoadServerProperty(req);
        QEvent *ev = new events::ServerPropertyInfoRequestEvent(this, req);
        notify(ev);
    }

    void IServer::requestHistoryInfo(const EventsInfo::ServerInfoHistoryRequest& req)
    {
        emit startedLoadServerHistoryInfo(req);
        QEvent *ev = new events::ServerInfoHistoryRequestEvent(this, req);
        notify(ev);
    }

    void IServer::clearHistory(const EventsInfo::ClearServerHistoryRequest &req)
    {
        emit startedClearServerHistory(req);
        QEvent *ev = new events::ClearServerHistoryRequestEvent(this, req);
        notify(ev);
    }

    void IServer::changeProperty(const EventsInfo::ChangeServerPropertyInfoRequest& req)
    {
        emit startedChangeServerProperty(req);
        QEvent *ev = new events::ChangeServerPropertyInfoRequestEvent(this, req);
        notify(ev);
    }

    void IServer::customEvent(QEvent *event)
    {
        using namespace events;
        QEvent::Type type = event->type();
        if (type == static_cast<QEvent::Type>(ConnectResponceEvent::EventType)){
            ConnectResponceEvent *ev = static_cast<ConnectResponceEvent*>(event);
            handleConnectEvent(ev);

            ConnectResponceEvent::value_type v = ev->value();
            common::Error er(v.errorInfo());
            if(!er){
                EventsInfo::DiscoveryInfoRequest dreq(this);
                processDiscoveryInfo(dreq);

                EventsInfo::ProcessConfigArgsInfoRequest preq(this);
                processConfigArgs(preq);
            }
        }
        else if(type == static_cast<QEvent::Type>(EnterModeEvent::EventType))
        {
            EnterModeEvent *ev = static_cast<EnterModeEvent*>(event);
            EnterModeEvent::value_type v = ev->value();
            emit enteredMode(v);
        }
        else if(type == static_cast<QEvent::Type>(LeaveModeEvent::EventType))
        {
            LeaveModeEvent *ev = static_cast<LeaveModeEvent*>(event);
            LeaveModeEvent::value_type v = ev->value();
            emit leavedMode(v);
        }
        else if(type == static_cast<QEvent::Type>(CommandRootCreatedEvent::EventType)){
            CommandRootCreatedEvent *ev = static_cast<CommandRootCreatedEvent*>(event);
            CommandRootCreatedEvent::value_type v = ev->value();
            emit rootCreated(v);
        }
        else if(type == static_cast<QEvent::Type>(CommandRootCompleatedEvent::EventType)){
            CommandRootCompleatedEvent *ev = static_cast<CommandRootCompleatedEvent*>(event);
            CommandRootCompleatedEvent::value_type v = ev->value();
            emit rootCompleated(v);
        }
        else if(type == static_cast<QEvent::Type>(DisconnectResponceEvent::EventType)){
            DisconnectResponceEvent *ev = static_cast<DisconnectResponceEvent*>(event);
            handleDisconnectEvent(ev);
        }
        else if(type == static_cast<QEvent::Type>(LoadDatabasesInfoResponceEvent::EventType)){
            LoadDatabasesInfoResponceEvent *ev = static_cast<LoadDatabasesInfoResponceEvent*>(event);
            handleLoadDatabaseInfosEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ServerInfoResponceEvent::EventType)){
            ServerInfoResponceEvent *ev = static_cast<ServerInfoResponceEvent*>(event);
            handleLoadServerInfoEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ServerInfoHistoryResponceEvent::EventType)){
            ServerInfoHistoryResponceEvent *ev = static_cast<ServerInfoHistoryResponceEvent*>(event);
            handleLoadServerInfoHistoryEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ClearServerHistoryResponceEvent::EventType)){
            ClearServerHistoryResponceEvent *ev = static_cast<ClearServerHistoryResponceEvent*>(event);
            handleClearServerHistoryResponceEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ServerPropertyInfoResponceEvent::EventType)){
            ServerPropertyInfoResponceEvent *ev = static_cast<ServerPropertyInfoResponceEvent*>(event);
            handleLoadServerPropertyEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ChangeServerPropertyInfoResponceEvent::EventType)){
            ChangeServerPropertyInfoResponceEvent *ev = static_cast<ChangeServerPropertyInfoResponceEvent*>(event);
            handleServerPropertyChangeEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(BackupResponceEvent::EventType)){
            BackupResponceEvent *ev = static_cast<BackupResponceEvent*>(event);
            handleBackupEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ExportResponceEvent::EventType)){
            ExportResponceEvent *ev = static_cast<ExportResponceEvent*>(event);
            handleExportEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ChangePasswordResponceEvent::EventType)){
            ChangePasswordResponceEvent *ev = static_cast<ChangePasswordResponceEvent*>(event);
            handleChangePasswordEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(ChangeMaxConnectionResponceEvent::EventType)){
            ChangeMaxConnectionResponceEvent *ev = static_cast<ChangeMaxConnectionResponceEvent*>(event);
            handleChangeMaxConnection(ev);
        }
        else if (type == static_cast<QEvent::Type>(LoadDatabaseContentResponceEvent::EventType)){
            LoadDatabaseContentResponceEvent *ev = static_cast<LoadDatabaseContentResponceEvent*>(event);
            handleLoadDatabaseContentEvent(ev);
        }
        else if (type == static_cast<QEvent::Type>(SetDefaultDatabaseResponceEvent::EventType)){
            SetDefaultDatabaseResponceEvent *ev = static_cast<SetDefaultDatabaseResponceEvent*>(event);
            handleSetDefaultDatabaseEvent(ev);
        }
        else if(type == static_cast<QEvent::Type>(CommandResponceEvent::EventType)){
            CommandResponceEvent *ev = static_cast<CommandResponceEvent*>(event);
            handleCommandResponceEvent(ev);
        }
        else if(type == static_cast<QEvent::Type>(DiscoveryInfoResponceEvent::EventType)){
            DiscoveryInfoResponceEvent *ev = static_cast<DiscoveryInfoResponceEvent*>(event);
            handleDiscoveryInfoResponceEvent(ev);
        }
        else if(type == static_cast<QEvent::Type>(ProgressResponceEvent::EventType))
        {
            ProgressResponceEvent *ev = static_cast<ProgressResponceEvent*>(event);
            ProgressResponceEvent::value_type v = ev->value();
            emit progressChanged(v);
        }
        return QObject::customEvent(event);
    }

    void IServer::notify(QEvent *ev)
    {
        EventsInfo::ProgressInfoResponce resp(0);
        emit progressChanged(resp);
        qApp->postEvent(drv_.get(), ev);
    }

    void IServer::handleConnectEvent(events::ConnectResponceEvent* ev)
    {
        using namespace events;
        ConnectResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedConnect(v);
    }

    void IServer::handleDisconnectEvent(events::DisconnectResponceEvent* ev)
    {
        using namespace events;
        DisconnectResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedDisconnect(v);
    }

    void IServer::handleLoadServerInfoEvent(events::ServerInfoResponceEvent* ev)
    {
        using namespace events;
        ServerInfoResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedLoadServerInfo(v);
    }

    void IServer::handleLoadServerPropertyEvent(events::ServerPropertyInfoResponceEvent* ev)
    {
        using namespace events;
        ServerPropertyInfoResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedLoadServerProperty(v);
    }

    void IServer::handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoResponceEvent* ev)
    {
        using namespace events;
        ChangeServerPropertyInfoResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedChangeServerProperty(v);
    }

    void IServer::handleShutdownEvent(events::ShutDownResponceEvent* ev)
    {
        using namespace events;
        ShutDownResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedShutdown(v);
    }

    void IServer::handleBackupEvent(events::BackupResponceEvent* ev)
    {
        using namespace events;
        BackupResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedBackup(v);
    }

    void IServer::handleExportEvent(events::ExportResponceEvent* ev)
    {
        using namespace events;
        ExportResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedExport(v);
    }

    void IServer::handleChangePasswordEvent(events::ChangePasswordResponceEvent* ev)
    {
        using namespace events;
        ChangePasswordResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedChangePassword(v);
    }

    void IServer::handleChangeMaxConnection(events::ChangeMaxConnectionResponceEvent* ev)
    {
        using namespace events;
        ChangeMaxConnectionResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedChangeMaxConnection(v);
    }

    void IServer::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoResponceEvent* ev)
    {
        using namespace events;
        LoadDatabasesInfoResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
            databases_.clear();
        }
        else{
            EventsInfo::LoadDatabasesInfoResponce::database_info_cont_type dbs = v.databases_;
            EventsInfo::LoadDatabasesInfoResponce::database_info_cont_type tmp;
            for(int j = 0; j < dbs.size(); ++j){
                DataBaseInfoSPtr db = dbs[j];
                IDatabaseSPtr datab = findDatabaseByInfo(db);
                if(!datab){
                    datab = createDatabase(db);
                    databases_.push_back(datab);
                }
                tmp.push_back(datab->info());
            }
            v.databases_ = tmp;
        }
        emit finishedLoadDatabases(v);
    }

    void IServer::handleLoadDatabaseContentEvent(events::LoadDatabaseContentResponceEvent* ev)
    {
        using namespace events;
        LoadDatabaseContentResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        else{
            IDatabaseSPtr db = findDatabaseByInfo(v.inf_);
            if(db){
                DataBaseInfoSPtr rdb = db->info();
                if(rdb){
                    rdb->setKeys(v.keys_);
                }
            }
        }
        emit finishedLoadDatabaseContent(v);
    }

    void IServer::handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryResponceEvent *ev)
    {
        using namespace events;
        ServerInfoHistoryResponceEvent::value_type v = ev->value();
        common::Error er = v.errorInfo();
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedLoadServerHistoryInfo(v);
    }

    void IServer::handleDiscoveryInfoResponceEvent(events::DiscoveryInfoResponceEvent* ev)
    {
        using namespace events;
        DiscoveryInfoResponceEvent::value_type v = ev->value();
        common::Error er = v.errorInfo();
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedLoadDiscoveryInfo(v);
    }

    void IServer::handleClearServerHistoryResponceEvent(events::ClearServerHistoryResponceEvent* ev)
    {
        using namespace events;
        ClearServerHistoryResponceEvent::value_type v = ev->value();
        common::Error er = v.errorInfo();
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedClearServerHistory(v);
    }

    void IServer::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseResponceEvent* ev)
    {
        using namespace events;
        SetDefaultDatabaseResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        else{
            DataBaseInfoSPtr inf = v.inf_;
            for(int i = 0; i < databases_.size(); ++i){
                IDatabaseSPtr db = databases_[i];
                DataBaseInfoSPtr info = db->info();
                if(info->name() == inf->name()){
                    info->setIsDefault(true);
                }
                else{
                    info->setIsDefault(false);
                }
            }
        }

        emit finishedSetDefaultDatabase(v);
    }

    void IServer::handleCommandResponceEvent(events::CommandResponceEvent* ev)
    {
        using namespace events;
        CommandResponceEvent::value_type v = ev->value();
        common::Error er(v.errorInfo());
        if(er && er->isError()){
            LOG_ERROR(er, true);
        }
        emit finishedExecuteCommand(v);
    }

    void IServer::processConfigArgs(const EventsInfo::ProcessConfigArgsInfoRequest &req)
    {
        QEvent *ev = new events::ProcessConfigArgsRequestEvent(this, req);
        notify(ev);
    }

    void IServer::processDiscoveryInfo(const EventsInfo::DiscoveryInfoRequest& req)
    {
        emit startedLoadDiscoveryInfo(req);
        QEvent *ev = new events::DiscoveryInfoRequestEvent(this, req);
        notify(ev);
    }
}
