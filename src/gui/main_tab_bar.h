#pragma once

#include <QTabBar>

namespace fastonosql
{
    class MainTabBar
            : public QTabBar
    {
        Q_OBJECT

    public:
        explicit MainTabBar(QWidget* parent = 0);

    Q_SIGNALS:
        void createdNewTab();
        void nextTab();
        void prevTab();
        void reloadedTab();
        void duplicatedTab();
        void closedTab();
        void closedOtherTabs();

    private Q_SLOTS:
        void showContextMenu(const QPoint& p);

    protected:
        virtual void changeEvent(QEvent* );

    private:
        void retranslateUi();

        QAction* newShellAction_;
        QAction* nextTabAction_;
        QAction* prevTabAction_;
        QAction* reloadShellAction_;
        QAction* duplicateShellAction_;
        QAction* closeShellAction_;
        QAction* closeOtherShellsAction_;
    };
}
