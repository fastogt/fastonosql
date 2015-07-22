#pragma once

#include <QDialog>

#include "core/events/events_info.h"

class QTableView;

namespace fasto
{
    namespace qt
    {
        namespace gui
        {
            class GlassWidget;
        }
    }
}

namespace fastonosql
{
    class PropertyServerDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        explicit PropertyServerDialog(const QString& title, connectionTypes type, QWidget* parent = 0);

    Q_SIGNALS:
        void showed();
        void changedProperty(const PropertyType& prop);

    public Q_SLOTS:
        void startServerProperty(const EventsInfo::ServerPropertyInfoRequest& req);
        void finishServerProperty(const EventsInfo::ServerPropertyInfoResponce& res);

        void startServerChangeProperty(const EventsInfo::ChangeServerPropertyInfoRequest& req);
        void finishServerChangeProperty(const EventsInfo::ChangeServerPropertyInfoResponce& res);

    protected:
        virtual void showEvent(QShowEvent *e);

    private:
        fasto::qt::gui::GlassWidget *glassWidget_;
        QTableView *propertyes_table_;
        const connectionTypes type_;       
    };
}
