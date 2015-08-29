#pragma once

#include <QWidget>

class QTextEdit;
class QAction;

#include "global/types.h"

namespace fastonosql
{
    class CommandsWidget
            : public QWidget
    {
        Q_OBJECT
    public:
        explicit CommandsWidget(QWidget* parent = 0);

    public Q_SLOTS:
        void addCommand(const Command& command);

    private Q_SLOTS:
        void showContextMenu(const QPoint& pt);

    protected:
        virtual void changeEvent(QEvent *ev);

    private:
        void retranslateUi();
        QTextEdit* const logTextEdit_;
        QAction* clear_;
    };
}
