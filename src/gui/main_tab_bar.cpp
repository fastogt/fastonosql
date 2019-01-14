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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/main_tab_bar.h"

#include <QEvent>
#include <QMenu>

#include <common/macros.h>  // for VERIFY

#include "gui/shortcuts.h"  // for g_close_key, g_new_tab_key, etc

#include "translations/global.h"

namespace fastonosql {
namespace gui {

MainTabBar::MainTabBar(QWidget* parent) : QTabBar(parent) {
  setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(this, &MainTabBar::customContextMenuRequested, this, &MainTabBar::showContextMenu));

  retranslateUi();
}

void MainTabBar::showContextMenu(const QPoint& point) {
  QMenu menu(this);
  QAction* new_shell_action = new QAction(translations::trNewTab, this);
  new_shell_action->setShortcut(g_new_tab_key);
  VERIFY(connect(new_shell_action, &QAction::triggered, this, &MainTabBar::createdNewTab));

  QAction* next_tab_action = new QAction(translations::trNextTab, this);
  next_tab_action->setShortcut(g_next_tab_key);
  VERIFY(connect(next_tab_action, &QAction::triggered, this, &MainTabBar::nextTab));

  QAction* prev_tab_action = new QAction(translations::trPrevTab, this);
  prev_tab_action->setShortcut(g_prev_tab_key);
  VERIFY(connect(prev_tab_action, &QAction::triggered, this, &MainTabBar::prevTab));

  QAction* reload_shell_action = new QAction(translations::trReload, this);
  reload_shell_action->setShortcut(g_refresh_key);
  VERIFY(connect(reload_shell_action, &QAction::triggered, this, &MainTabBar::reloadedTab));

  QAction* duplicate_shell_action = new QAction(translations::trDuplicate, this);
  VERIFY(connect(duplicate_shell_action, &QAction::triggered, this, &MainTabBar::duplicatedTab));

  QAction* close_shell_action = new QAction(translations::trClose, this);
  close_shell_action->setShortcut(g_close_key);
  VERIFY(connect(close_shell_action, &QAction::triggered, this, &MainTabBar::closedTab));

  QAction* close_other_shells_action = new QAction(translations::trCloseOthers, this);
  VERIFY(connect(close_other_shells_action, &QAction::triggered, this, &MainTabBar::closedOtherTabs));

  menu.addAction(new_shell_action);
  menu.addAction(next_tab_action);
  menu.addAction(prev_tab_action);
  menu.addSeparator();
  menu.addAction(reload_shell_action);
  menu.addAction(duplicate_shell_action);
  menu.addSeparator();
  menu.addAction(close_shell_action);
  menu.addAction(close_other_shells_action);
  menu.exec(mapToGlobal(point));
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
