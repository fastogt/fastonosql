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

#include "gui/widgets/main_widget.h"

#include <common/qt/convert2string.h>

#include "proxy/server/iserver.h"  // for IServer
#include "proxy/settings_manager.h"

#include "gui/gui_factory.h"           // for GuiFactory
#include "gui/main_tab_bar.h"          // for MainTabBar
#include "gui/widgets/query_widget.h"  // for QueryWidget
#include "gui/widgets/welcome_widget.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

MainWidget::MainWidget(QWidget* parent) : QTabWidget(parent) {
  MainTabBar* tab = new MainTabBar(this);

  VERIFY(connect(tab, &MainTabBar::createdNewTab, this, &MainWidget::createNewTab));  //
  VERIFY(connect(tab, &MainTabBar::nextTab, this, &MainWidget::nextTab));
  VERIFY(connect(tab, &MainTabBar::prevTab, this, &MainWidget::previousTab));

  VERIFY(connect(tab, &MainTabBar::reloadedTab, this, &MainWidget::reloadCurrentTab));       //
  VERIFY(connect(tab, &MainTabBar::duplicatedTab, this, &MainWidget::duplicateCurrentTab));  //
  VERIFY(connect(tab, &MainTabBar::closedOtherTabs, this, &MainWidget::closedOtherTabs));
  VERIFY(connect(tab, &MainTabBar::closedTab, this, &MainWidget::closeCurrentTab));
  VERIFY(connect(tab, &MainTabBar::tabCloseRequested, this, &MainWidget::closeTab));

  setTabBar(tab);
  setTabsClosable(true);
  setElideMode(Qt::ElideRight);
  setMovable(true);

  if (proxy::SettingsManager::GetInstance()->GetShowWelcomePage()) {
    createWelcomeTab();
  }
}

QueryWidget* MainWidget::queryWidget(int index) const {
  return qobject_cast<QueryWidget*>(QTabWidget::widget(index));
}

void MainWidget::openConsole(proxy::IServerSPtr server, const QString& text) {
  if (!server) {
    DNOTREACHED();
    return;
  }

  QueryWidget* query_widget = createWidget<QueryWidget>(server);
  QString name;
  common::ConvertFromString(server->GetName(), &name);
  addWidgetToTab(query_widget, name);
  query_widget->setInputText(text);
}

void MainWidget::openConsoleAndExecute(proxy::IServerSPtr server, const QString& text) {
  if (!server) {
    DNOTREACHED();
    return;
  }

  QueryWidget* queryWidget = createWidget<QueryWidget>(server);
  QString name;
  common::ConvertFromString(server->GetName(), &name);
  addWidgetToTab(queryWidget, name);
  queryWidget->execute(text);
}

void MainWidget::createNewTab() {
  int current_index = currentIndex();
  QueryWidget* shw = queryWidget(current_index);
  if (shw) {
    openNewTab(shw, tabText(current_index), QString());
  }
}

void MainWidget::nextTab() {
  int index = currentIndex();
  int tabs_count = count();
  if (index == tabs_count - 1) {
    setCurrentIndex(0);
    return;
  }

  if (index >= 0 && index < tabs_count - 1) {
    setCurrentIndex(index + 1);
    return;
  }
}

void MainWidget::previousTab() {
  int index = currentIndex();
  if (index == 0) {
    setCurrentIndex(count() - 1);
    return;
  }

  if (index > 0) {
    setCurrentIndex(index - 1);
    return;
  }
}

void MainWidget::reloadCurrentTab() {
  int current_index = currentIndex();
  QueryWidget* shw = queryWidget(current_index);
  if (shw) {
    shw->reload();
  }
}

void MainWidget::duplicateCurrentTab() {
  int current_index = currentIndex();
  QueryWidget* shw = queryWidget(current_index);
  if (shw) {
    openNewTab(shw, tabText(current_index), shw->inputText());
  }
}

void MainWidget::closeTab(int index) {
  QWidget* wid = widget(index);
  if (wid) {
    removeTab(index);
    delete wid;
  }
}

void MainWidget::closeCurrentTab() {
  int current_index = currentIndex();
  closeTab(current_index);
}

void MainWidget::closedOtherTabs() {
  int current_index = currentIndex();
  tabBar()->moveTab(current_index, 0);
  while (count() > 1) {
    closeTab(1);
  }
}

void MainWidget::addWidgetToTab(QueryWidget* wid, const QString& title) {
  if (!wid) {
    return;
  }

  addTab(wid, GuiFactory::GetInstance().icon(wid->connectionType()), title);
  setCurrentWidget(wid);
}

void MainWidget::openNewTab(QueryWidget* src, const QString& title, const QString& text) {
  QueryWidget* new_widget = src->clone(text);
  addWidgetToTab(new_widget, title);
}

void MainWidget::createWelcomeTab() {
  WelcomeWidget* welcome_tab = createWidget<WelcomeWidget>();
  addTab(welcome_tab, GuiFactory::GetInstance().welcomeTabIcon(), translations::trWelcome);
  setCurrentWidget(welcome_tab);
}

}  // namespace gui
}  // namespace fastonosql
