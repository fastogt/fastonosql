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
        explicit PropertyServerDialog(IServerSPtr server, QWidget* parent = 0);

    Q_SIGNALS:
        void showed();

    private Q_SLOTS:
        void startServerProperty(const EventsInfo::ServerPropertyInfoRequest& req);
        void finishServerProperty(const EventsInfo::ServerPropertyInfoResponce& res);

        void startServerChangeProperty(const EventsInfo::ChangeServerPropertyInfoRequest& req);
        void finishServerChangeProperty(const EventsInfo::ChangeServerPropertyInfoResponce& res);

        void changedProperty(const PropertyType& prop);
    protected:
        virtual void changeEvent(QEvent* e);
        virtual void showEvent(QShowEvent *e);

    private:
        void retranslateUi();

        fasto::qt::gui::GlassWidget *glassWidget_;
        QTableView *propertyes_table_;
        const IServerSPtr server_;
    };
}
