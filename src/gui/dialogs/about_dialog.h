#pragma  once

#include <QDialog>

namespace fastonosql
{
    class AboutDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        explicit AboutDialog(QWidget* parent);
    };
}
