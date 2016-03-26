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

#include "gui/widgets/main_widget.h"

#include "core/iserver.h"

#include "gui/shortcuts.h"

#include "gui/main_tab_bar.h"
#include "gui/gui_factory.h"
#include "gui/widgets/query_widget.h"

namespace fastonosql {
namespace gui {

MainWidget::MainWidget(QWidget* parent)
  : QTabWidget(parent) {
  MainTabBar* tab = new MainTabBar(this);

  VERIFY(connect(tab, &MainTabBar::createdNewTab, this, &MainWidget::createNewTab));
  VERIFY(connect(tab, &MainTabBar::nextTab, this, &MainWidget::nextTab));
  VERIFY(connect(tab, &MainTabBar::prevTab, this, &MainWidget::previousTab));

  VERIFY(connect(tab, &MainTabBar::reloadedTab, this, &MainWidget::reloadeCurrentTab));
  VERIFY(connect(tab, &MainTabBar::duplicatedTab, this, &MainWidget::duplicateCurrentTab));
  VERIFY(connect(tab, &MainTabBar::closedOtherTabs, this, &MainWidget::closedOtherTabs));
  VERIFY(connect(tab, &MainTabBar::closedTab, this, &MainWidget::closeCurrentTab));
  VERIFY(connect(tab, &MainTabBar::tabCloseRequested, this, &MainWidget::closeTab));

  setTabBar(tab);
  setTabsClosable(true);
  setElideMode(Qt::ElideRight);
  setMovable(true);
  setDocumentMode(true);
}

QueryWidget* MainWidget::currentWidget() const {
  return qobject_cast<QueryWidget*>(QTabWidget::currentWidget());
}

QueryWidget* MainWidget::widget(int index) const {
  return qobject_cast<QueryWidget*>(QTabWidget::widget(index));
}

void MainWidget::openConsole(IServerSPtr server, const QString& text) {
  if (server) {
    QueryWidget* queryWidget = new QueryWidget(server);
    addWidgetToTab(queryWidget, server->name());
    queryWidget->setInputText(text);
  }
}

void MainWidget::executeText(IServerSPtr server, const QString& text) {
  if (server) {
    QueryWidget* queryWidget = new QueryWidget(server);
    addWidgetToTab(queryWidget, server->name());
    queryWidget->execute(text);
  }
}

void MainWidget::createNewTab() {
  int curIndex = currentIndex();
  QueryWidget* shw = widget(curIndex);
  if (shw) {
    openNewTab(shw, tabText(curIndex), QString());
  }
}

void MainWidget::nextTab() {
  int index = currentIndex();
  int tabsCount = count();
  if (index == tabsCount - 1) {
    setCurrentIndex(0);
    return;
  }

  if (index >= 0 && index < tabsCount - 1) {
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

void MainWidget::reloadeCurrentTab() {
  int curIndex = currentIndex();
  QueryWidget* shw = widget(curIndex);
  if (shw) {
    shw->reload();
  }
}

void MainWidget::duplicateCurrentTab() {
  int curIndex = currentIndex();
  QueryWidget* shw = widget(curIndex);
  if (shw) {
    openNewTab(shw, tabText(curIndex), shw->inputText());
  }
}

void MainWidget::closeTab(int index) {
  QueryWidget* shw = widget(index);
  if (shw) {
    removeTab(index);
    delete shw;
  }
}

void MainWidget::closeCurrentTab() {
  int curIndex = currentIndex();
  closeTab(curIndex);
}

void MainWidget::closedOtherTabs() {
  int curIndex = currentIndex();
  tabBar()->moveTab(curIndex, 0);
  while (count() > 1) {
    closeTab(1);
  }
}

void MainWidget::addWidgetToTab(QueryWidget* wid, const QString& title) {
  if (!wid) {
    return;
  }

  addTab(wid, GuiFactory::instance().icon(wid->connectionType()), title);
  setCurrentWidget(wid);
}

void MainWidget::openNewTab(QueryWidget* src, const QString& title, const QString& text) {
  QueryWidget* newWid = src->clone(text);
  addWidgetToTab(newWid, title);
}

}  // namespace gui
}  // namespace fastonosql
