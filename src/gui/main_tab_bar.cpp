#include "gui/main_tab_bar.h"

#include <QMenu>
#include <QEvent>

#include "common/macros.h"

#include "gui/shortcuts.h"

#include "translations/global.h"

namespace fastoredis
{
    MainTabBar::MainTabBar(QWidget* parent)
        : QTabBar(parent)
    {
        newShellAction_ = new QAction(this);
        newShellAction_->setShortcut(newTabKey);
        VERIFY(connect(newShellAction_, &QAction::triggered, this , &MainTabBar::createdNewTab));

        nextTabAction_ = new QAction(this);
        nextTabAction_->setShortcut(nextTabKey);
        VERIFY(connect(nextTabAction_, &QAction::triggered, this , &MainTabBar::nextTab));

        prevTabAction_ = new QAction(this);
        prevTabAction_->setShortcut(prevTabKey);
        VERIFY(connect(prevTabAction_, &QAction::triggered, this , &MainTabBar::prevTab));

        reloadShellAction_ = new QAction(this);
        reloadShellAction_->setShortcut(refreshKey);
        VERIFY(connect(reloadShellAction_, &QAction::triggered, this , &MainTabBar::reloadedTab));

        duplicateShellAction_ = new QAction(this);
        VERIFY(connect(duplicateShellAction_, &QAction::triggered, this , &MainTabBar::duplicatedTab));

        closeShellAction_ = new QAction(this);
        closeShellAction_->setShortcut(closeKey);
        VERIFY(connect(closeShellAction_, &QAction::triggered, this , &MainTabBar::closedTab));

        closeOtherShellsAction_ = new QAction(this);
        VERIFY(connect(closeOtherShellsAction_, &QAction::triggered, this , &MainTabBar::closedOtherTabs));

        setContextMenuPolicy(Qt::CustomContextMenu);
        VERIFY(connect(this, &MainTabBar::customContextMenuRequested, this, &MainTabBar::showContextMenu));

        retranslateUi();
    }

    void MainTabBar::showContextMenu(const QPoint& p)
    {
        QMenu menu(this);
        menu.addAction(newShellAction_);
        menu.addAction(nextTabAction_);
        menu.addAction(prevTabAction_);
        menu.addSeparator();
        menu.addAction(reloadShellAction_);
        menu.addAction(duplicateShellAction_);
        menu.addSeparator();
        menu.addAction(closeShellAction_);
        menu.addAction(closeOtherShellsAction_);
        menu.exec(mapToGlobal(p));
    }

    void MainTabBar::changeEvent(QEvent* e)
    {
        if(e->type() == QEvent::LanguageChange){
            retranslateUi();
        }

        QTabBar::changeEvent(e);
    }

    void MainTabBar::retranslateUi()
    {
        using namespace translations;
        newShellAction_->setText(trNewTab);
        nextTabAction_->setText(trNextTab);
        prevTabAction_->setText(trPrevTab);
        reloadShellAction_->setText(trReload);
        duplicateShellAction_->setText(trDuplicate);
        closeShellAction_->setText(trCloseTab);
        closeOtherShellsAction_->setText(trCloseOtherTab);
    }
}
