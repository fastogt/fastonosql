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

#pragma once

#include <QTabBar>

class QAction;
class QEvent;
class QPoint;
class QWidget;

namespace fastonosql {
namespace gui {

class MainTabBar : public QTabBar {
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
  virtual void changeEvent(QEvent*);

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

}  // namespace gui
}  // namespace fastonosql
