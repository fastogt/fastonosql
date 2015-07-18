#pragma  once

#include <QDialog>

#include "core/events/events_info.h"

class QLineEdit;

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

namespace fastoredis
{
    class ChangePasswordServerDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        enum
        {
            fix_height = 160,
            fix_width = 240
        };

        explicit ChangePasswordServerDialog(const QString& title, IServerSPtr server, QWidget* parent);

    private Q_SLOTS:
        void tryToCreatePassword();
        void startChangePassword(const EventsInfo::ChangePasswordRequest& req);
        void finishChangePassword(const EventsInfo::ChangePasswordResponce& res);

    private:
        bool validateInput();
        fasto::qt::gui::GlassWidget *glassWidget_;
        QLineEdit* passwordLineEdit_;
        QLineEdit* confPasswordLineEdit_;
        const IServerSPtr server_;
    };
}
