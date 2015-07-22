#pragma once

#include <QWidget>

#include "core/iserver.h"

class QAction;

namespace fastonosql
{
    class BaseShellWidget;
    class OutputWidget;

    class QueryWidget
            : public QWidget
    {
        Q_OBJECT
    public:
        QueryWidget(IServerSPtr server, QWidget* parent = 0);

        QueryWidget* clone(const QString& text);
        connectionTypes connectionType() const;
        QString inputText() const;
        void setInputText(const QString& text);
        void execute(const QString& text);
        void reload();

    private:
        BaseShellWidget* shellWidget_;
        OutputWidget* outputWidget_;
    };
}
