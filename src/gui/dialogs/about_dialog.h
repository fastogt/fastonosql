#pragma  once

#include <QDialog>

namespace fastoredis
{
    class AboutDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        explicit AboutDialog(QWidget* parent);
    };
}
