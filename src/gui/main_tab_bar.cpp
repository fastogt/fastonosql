/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <common/macros.h>  // for VERIFY

#include "gui/shortcuts.h"  // for g_close_key, g_new_tab_key, etc

#include "translations/global.h"  // for trCloseOtherTabs, trCloseTab, etc

namespace fastonosql {
namespace gui {

MainTabBar::MainTabBar(QWidget* parent) : QTabBar(parent) {
  setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(this, &MainTabBar::customContextMenuRequested, this, &MainTabBar::showContextMenu));

  retranslateUi();
}

void MainTabBar::showContextMenu(const QPoint& p) {
  QMenu menu(this);
  QAction* newShellAction = new QAction(translations::trNewTab, this);
  newShellAction->setShortcut(g_new_tab_key);
  VERIFY(connect(newShellAction, &QAction::triggered, this, &MainTabBar::createdNewTab));

  QAction* nextTabAction = new QAction(translations::trNextTab, this);
  nextTabAction->setShortcut(g_next_tab_key);
  VERIFY(connect(nextTabAction, &QAction::triggered, this, &MainTabBar::nextTab));

  QAction* prevTabAction = new QAction(translations::trPrevTab, this);
  prevTabAction->setShortcut(g_prev_tab_key);
  VERIFY(connect(prevTabAction, &QAction::triggered, this, &MainTabBar::prevTab));

  QAction* reloadShellAction = new QAction(translations::trReload, this);
  reloadShellAction->setShortcut(g_refresh_key);
  VERIFY(connect(reloadShellAction, &QAction::triggered, this, &MainTabBar::reloadedTab));

  QAction* duplicateShellAction = new QAction(translations::trDuplicate, this);
  VERIFY(connect(duplicateShellAction, &QAction::triggered, this, &MainTabBar::duplicatedTab));

  QAction* closeShellAction = new QAction(translations::trCloseTab, this);
  closeShellAction->setShortcut(g_close_key);
  VERIFY(connect(closeShellAction, &QAction::triggered, this, &MainTabBar::closedTab));

  QAction* closeOtherShellsAction = new QAction(translations::trCloseOtherTabs, this);
  VERIFY(connect(closeOtherShellsAction, &QAction::triggered, this, &MainTabBar::closedOtherTabs));

  menu.addAction(newShellAction);
  menu.addAction(nextTabAction);
  menu.addAction(prevTabAction);
  menu.addSeparator();
  menu.addAction(reloadShellAction);
  menu.addAction(duplicateShellAction);
  menu.addSeparator();
  menu.addAction(closeShellAction);
  menu.addAction(closeOtherShellsAction);
  menu.exec(mapToGlobal(p));
}

void MainTabBar::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QTabBar::changeEvent(e);
}

void MainTabBar::retranslateUi() {}

}  // namespace gui
}  // namespace fastonosql
