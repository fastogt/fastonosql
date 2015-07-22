#pragma once

#include <QTabWidget>

#include "core/iserver.h"
#include "core/connection_settings.h"

namespace fastonosql
{
    class QueryWidget;

    class MainWidget
            : public QTabWidget
    {
        Q_OBJECT
    public:
        explicit MainWidget(QWidget* parent = 0);
        QueryWidget* currentWidget() const;
        QueryWidget* widget(int index) const;

    public Q_SLOTS:
        void openConsole(IServerSPtr server, const QString& text);
        void executeText(IServerSPtr server, const QString& text);

    private Q_SLOTS:
        void createNewTab();
        void nextTab();
        void previousTab();

        void reloadeCurrentTab();
        void duplicateCurrentTab();
        void closeTab(int index);
        void closeCurrentTab();
        void closedOtherTabs();

    private:
        void addWidgetToTab(QueryWidget* wid, const QString& title);
        void openNewTab(QueryWidget* src, const QString& title, const QString& text);
    };
}
