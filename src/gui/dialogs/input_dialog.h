#pragma  once

#include <QDialog>

class QLineEdit;

namespace fastonosql
{
    class InputDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        enum InputType { SingleLine, DoubleLine };
        explicit InputDialog(QWidget* parent, const QString& title, InputType type,
                             const QString& firstLabelText, const QString& secondLabelText = QString());

        QString firstText() const;
        QString secondText() const;

    private:
        QLineEdit* firstLine_;
        QLineEdit* secondLine_;
    };
}
