#pragma once

#include <QWidget>

class QTextEdit;
class QAction;

#include "common/log_levels.h"

namespace fastonosql
{
    class LogWidget
            : public QWidget
    {
        Q_OBJECT

    public:
        explicit LogWidget(QWidget* parent = 0);

    public Q_SLOTS:
        void addLogMessage(const QString& message, common::logging::LEVEL_LOG level);

    private Q_SLOTS:
        void showContextMenu(const QPoint& pt);

    protected:
        virtual void changeEvent(QEvent* ev);

    private:
        void retranslateUi();
        QTextEdit *const logTextEdit_;
        QAction *clear_;
    };
}
