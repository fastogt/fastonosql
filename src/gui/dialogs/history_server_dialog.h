#pragma once

#include <QDialog>

class QComboBox;

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
        explicit ServerHistoryDialog(const QString& title, connectionTypes type, QWidget* parent = 0);

    Q_SIGNALS:
        void showed();

    public Q_SLOTS:
        void startLoadServerHistoryInfo(const EventsInfo::ServerInfoHistoryRequest& req);
        void finishLoadServerHistoryInfo(const EventsInfo::ServerInfoHistoryResponce& res);
        void snapShotAdd(ServerInfoSnapShoot snapshot);

    private Q_SLOTS:
        void refreshInfoFields(int index);
        void refreshGraph(int index);

    protected:
        virtual void showEvent(QShowEvent* e);

    private:
        void reset();
        QWidget* settingsGraph_;
        QComboBox* serverInfoGroupsNames_;
        QComboBox* serverInfoFields_;

        fasto::qt::gui::GraphWidget* graphWidget_;

        fasto::qt::gui::GlassWidget* glassWidget_;
        EventsInfo::ServerInfoHistoryResponce::infos_container_type infos_;
        const connectionTypes type_;
    };
}
