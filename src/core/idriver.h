#pragma once

#include <QObject>

#include "common/net/net.h"

#include "core/connection_settings.h"
#include "core/events/events.h"

class QThread;

namespace common
{
    namespace file_system
    {
        class File;
    }
}

namespace fastonosql
{
    class IDriver
            : public QObject, private IFastoObjectObserver
    {
        Q_OBJECT
    public:
        IDriver(IConnectionSettingsBaseSPtr settings, connectionTypes type);
        virtual ~IDriver();

        static void reply(QObject* reciver, QEvent* ev);

        // sync methods
        connectionTypes connectionType() const;
        IConnectionSettingsBaseSPtr settings() const;

        ServerDiscoveryInfoSPtr serverDiscoveryInfo() const;
        ServerInfoSPtr serverInfo() const;
        DataBaseInfoSPtr currentDatabaseInfo() const;

        void start();
        void stop();
        common::ErrorValueSPtr commandByType(CommandKeySPtr command, std::string& cmdstring) const WARN_UNUSED_RESULT;

        virtual void interrupt() = 0;
        virtual bool isConnected() const = 0;
        virtual bool isAuthenticated() const = 0;
        virtual common::net::hostAndPort address() const = 0;
        virtual std::string outputDelemitr() const = 0;        

    Q_SIGNALS:
        void addedChild(FastoObject* child);
        void itemUpdated(FastoObject* item, const QString& val);
        void serverInfoSnapShoot(ServerInfoSnapShoot shot);

    private Q_SLOTS:
        void init();
        void clear();

    protected:
        virtual void customEvent(QEvent *event);
        virtual void timerEvent(QTimerEvent* event);        

        void notifyProgress(QObject *reciver, int value);

    protected:
        // handle server events
        virtual void handleConnectEvent(events::ConnectRequestEvent* ev) = 0;
        virtual void handleDisconnectEvent(events::DisconnectRequestEvent* ev) = 0;
        virtual void handleExecuteEvent(events::ExecuteRequestEvent* ev) = 0;
        virtual void handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev) = 0;
        virtual void handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
        virtual void handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
        virtual void handleShutdownEvent(events::ShutDownRequestEvent* ev);
        virtual void handleBackupEvent(events::BackupRequestEvent* ev);
        virtual void handleExportEvent(events::ExportRequestEvent* ev);
        virtual void handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
        virtual void handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);

        // handle database events
        virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev) = 0;
        virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev) = 0;
        virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev) = 0;

        // handle command events
        virtual void handleCommandRequestEvent(events::CommandRequestEvent* ev) = 0;

        const IConnectionSettingsBaseSPtr settings_;

        class RootLocker
        {
        public:
            RootLocker(IDriver* parent, QObject* reciver, const std::string& text);
            ~RootLocker();

            FastoObjectIPtr root_;

        private:
            FastoObjectIPtr createRoot(QObject* reciver, const std::string& text);

            IDriver* parent_;
            QObject* reciver_;
            const common::time64_t tstart_;
        };

        RootLocker make_locker(QObject* reciver, const std::string& text)
        {
            return RootLocker(this, reciver, text);
        }

        void setCurrentDatabaseInfo(DataBaseInfo* inf);

    private:
        // handle info events
        void handleLoadServerInfoHistoryEvent(events::ServerInfoHistoryRequestEvent *ev);
        void handleDiscoveryInfoRequestEvent(events::DiscoveryInfoRequestEvent* ev);

        // notification of execute events
        virtual void addedChildren(FastoObject *child);
        virtual void updated(FastoObject* item, common::Value* val);

        // internal methods
        virtual ServerInfoSPtr makeServerInfoFromString(const std::string& val) = 0;
        virtual common::ErrorValueSPtr serverInfo(ServerInfo** info) = 0;
        virtual common::ErrorValueSPtr serverDiscoveryInfo(ServerInfo** sinfo, ServerDiscoveryInfo** dinfo, DataBaseInfo** dbinfo) = 0;
        virtual common::ErrorValueSPtr currentDataBaseInfo(DataBaseInfo** info) = 0;
        virtual void initImpl() = 0;
        virtual void clearImpl() = 0;

        virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev) = 0;

        // command impl methods
        virtual common::ErrorValueSPtr commandDeleteImpl(CommandDeleteKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT = 0;
        virtual common::ErrorValueSPtr commandLoadImpl(CommandLoadKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT = 0;
        virtual common::ErrorValueSPtr commandCreateImpl(CommandCreateKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT = 0;
        virtual common::ErrorValueSPtr commandChangeTTLImpl(CommandChangeTTL* command, std::string& cmdstring) const WARN_UNUSED_RESULT = 0;

    private:
        ServerInfoSPtr serverInfo_;
        ServerDiscoveryInfoSPtr serverDiscInfo_;
        DataBaseInfoSPtr currentDatabaseInfo_;

        QThread* thread_;
        int timer_info_id_;
        common::file_system::File* log_file_;
        const connectionTypes type_;
    };
}
