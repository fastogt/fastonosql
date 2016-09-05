/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/main_tab_bar.h"

#include <QEvent>
#include <QMenu>

#include "common/macros.h"  // for VERIFY

#include "fasto/qt/gui/shortcuts.h"  // for FastoQKeySequence

#include "gui/shortcuts.h"  // for closeKey, newTabKey, etc

#include "translations/global.h"  // for trCloseOtherTab, trCloseTab, etc

namespace fastonosql {
namespace gui {

MainTabBar::MainTabBar(QWidget* parent) : QTabBar(parent) {
  newShellAction_ = new QAction(this);
  newShellAction_->setShortcut(newTabKey);
  VERIFY(connect(newShellAction_, &QAction::triggered, this, &MainTabBar::createdNewTab));

  nextTabAction_ = new QAction(this);
  nextTabAction_->setShortcut(nextTabKey);
  VERIFY(connect(nextTabAction_, &QAction::triggered, this, &MainTabBar::nextTab));

  prevTabAction_ = new QAction(this);
  prevTabAction_->setShortcut(prevTabKey);
  VERIFY(connect(prevTabAction_, &QAction::triggered, this, &MainTabBar::prevTab));

  reloadShellAction_ = new QAction(this);
  reloadShellAction_->setShortcut(refreshKey);
  VERIFY(connect(reloadShellAction_, &QAction::triggered, this, &MainTabBar::reloadedTab));

  duplicateShellAction_ = new QAction(this);
  VERIFY(connect(duplicateShellAction_, &QAction::triggered, this, &MainTabBar::duplicatedTab));

  closeShellAction_ = new QAction(this);
  closeShellAction_->setShortcut(closeKey);
  VERIFY(connect(closeShellAction_, &QAction::triggered, this, &MainTabBar::closedTab));

  closeOtherShellsAction_ = new QAction(this);
  VERIFY(connect(closeOtherShellsAction_, &QAction::triggered, this, &MainTabBar::closedOtherTabs));

  setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(
      connect(this, &MainTabBar::customContextMenuRequested, this, &MainTabBar::showContextMenu));

  retranslateUi();
}

void MainTabBar::showContextMenu(const QPoint& p) {
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

void MainTabBar::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QTabBar::changeEvent(e);
}

void MainTabBar::retranslateUi() {
  newShellAction_->setText(translations::trNewTab);
  nextTabAction_->setText(translations::trNextTab);
  prevTabAction_->setText(translations::trPrevTab);
  reloadShellAction_->setText(translations::trReload);
  duplicateShellAction_->setText(translations::trDuplicate);
  closeShellAction_->setText(translations::trCloseTab);
  closeOtherShellsAction_->setText(translations::trCloseOtherTab);
}

}  // namespace gui
}  // namespace fastonosql
