#pragma once

#include <QWidget>

#include "core/iserver.h"
#include "core/events/events_info.h"

class QAction;
class QProgressBar;
class QToolButton;

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

namespace fastoredis
{
    class FastoEditorShell;

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

        void startConnect(const EventsInfo::ConnectInfoRequest& req);
        void finishConnect(const EventsInfo::ConnectInfoResponce& res);
        void startDisconnect(const EventsInfo::DisonnectInfoRequest& req);
        void finishDisconnect(const EventsInfo::DisConnectInfoResponce& res);

        void startSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseRequest& req);
        void finishSetDefaultDatabase(const EventsInfo::SetDefaultDatabaseResponce& res);

        void progressChange(const EventsInfo::ProgressInfoResponce& res);

        void enterMode(const EventsInfo::EnterModeInfo& res);
        void leaveMode(const EventsInfo::LeaveModeInfo& res);

    private:
        void syncConnectionActions();
        void updateDefaultDatabase(DataBaseInfoSPtr dbs);

        const IServerSPtr server_;
        QAction* executeAction_;
        QAction* connectAction_;
        QAction* disConnectAction_;
        QAction* loadAction_;
        QAction* saveAction_;
        QAction* saveAsAction_;

        FastoEditorShell* input_;

        QProgressBar* workProgressBar_;
        fasto::qt::gui::IconLabel* connectionMode_;
        fasto::qt::gui::IconLabel* dbName_;
        QString filePath_;
    };
}
