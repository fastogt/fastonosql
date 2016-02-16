/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QTabWidget>

#include "core/iserver.h"
#include "core/connection_settings.h"

namespace fastonosql {

class QueryWidget;
class MainWidget
        : public QTabWidget {
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

}  // namespace fastonosql
