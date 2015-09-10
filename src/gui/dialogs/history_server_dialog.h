#pragma once

#include <QDialog>

class QComboBox;
class QPushButton;

#include "core/events/events_info.h"

namespace fasto
{
    namespace qt
    {
        namespace gui
        {
            class GlassWidget;
            class GraphWidget;
        }
    }
}

namespace fastonosql
{

    class ServerHistoryDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        explicit ServerHistoryDialog(IServerSPtr server, QWidget* parent = 0);

    Q_SIGNALS:
        void showed();

    private Q_SLOTS:
        void startLoadServerHistoryInfo(const EventsInfo::ServerInfoHistoryRequest& req);
        void finishLoadServerHistoryInfo(const EventsInfo::ServerInfoHistoryResponce& res);
        void startClearServerHistory(const EventsInfo::ClearServerHistoryRequest& req);
        void finishClearServerHistory(const EventsInfo::ClearServerHistoryResponce& res);
        void snapShotAdd(ServerInfoSnapShoot snapshot);
        void clearHistory();

        void refreshInfoFields(int index);
        void refreshGraph(int index);

    protected:
        virtual void changeEvent(QEvent* e);
        virtual void showEvent(QShowEvent* e);

    private:
        void reset();
        void retranslateUi();
        void requestHistoryInfo();

        QWidget* settingsGraph_;
        QPushButton* clearHistory_;
        QComboBox* serverInfoGroupsNames_;
        QComboBox* serverInfoFields_;

        fasto::qt::gui::GraphWidget* graphWidget_;

        fasto::qt::gui::GlassWidget* glassWidget_;
        EventsInfo::ServerInfoHistoryResponce::infos_container_type infos_;
        const IServerSPtr server_;
    };
}
