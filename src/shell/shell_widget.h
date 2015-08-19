#pragma once

#include <QWidget>

#include "core/iserver.h"
#include "core/events/events_info.h"

class QAction;
class QProgressBar;
class QToolButton;
class QComboBox;

namespace fasto
{
    namespace qt
    {
        namespace gui
        {
            class IconLabel;
        }
    }
}

namespace fastonosql
{
    class BaseShell;

    class BaseShellWidget
            : public QWidget
    {
        Q_OBJECT
    public:
        BaseShellWidget(IServerSPtr server, const QString& filePath = QString(), QWidget* parent = 0);
        virtual ~BaseShellWidget();

        IServerSPtr server() const;
        QString text() const;

    Q_SIGNALS:
        void startedExecute(const EventsInfo::ExecuteInfoRequest& req);
        void rootCreated(const EventsInfo::CommandRootCreatedInfo& res);
        void rootCompleated(const EventsInfo::CommandRootCompleatedInfo& res);

        void addedChild(FastoObject* child);
        void itemUpdated(FastoObject* item, const QString& value);

    public Q_SLOTS:
        void setText(const QString& text);
        void executeText(const QString& text);

    private Q_SLOTS:
        void execute();
        void stop();
        void connectToServer();
        void disconnectFromServer();
        void loadFromFile();
        bool loadFromFile(const QString& path);
        void saveToFileAs();
        void saveToFile();

        void changeVersionApi(int index);

        void startConnect(const EventsInfo::ConnectInfoRequest& req);
        void finishConnect(const EventsInfo::ConnectInfoResponce& res);
        void startDisconnect(const EventsInfo::DisConnectInfoRequest& req);
        void finishDisconnect(const EventsInfo::DisConnectInfoResponce& res);

        void startSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseRequest& req);
        void finishSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseResponce& res);

        void progressChange(const EventsInfo::ProgressInfoResponce& res);

        void enterMode(const EventsInfo::EnterModeInfo& res);
        void leaveMode(const EventsInfo::LeaveModeInfo& res);

        void startLoadDiscoveryInfo(const EventsInfo::DiscoveryInfoRequest& res);
        void finishLoadDiscoveryInfo(const EventsInfo::DiscoveryInfoResponce& res);

    private:
        void syncConnectionActions();
        void syncServerInfo(ServerInfoSPtr inf);
        void updateDefaultDatabase(DataBaseInfoSPtr dbs);
        void initShellByType(connectionTypes type);

        const IServerSPtr server_;
        QAction* executeAction_;
        QAction* connectAction_;
        QAction* disConnectAction_;
        QAction* loadAction_;
        QAction* saveAction_;
        QAction* saveAsAction_;
        QComboBox* commandsVersionApi_;

        BaseShell* input_;

        QProgressBar* workProgressBar_;
        fasto::qt::gui::IconLabel* connectionMode_;
        fasto::qt::gui::IconLabel* dbName_;
        QString filePath_;
    };
}
