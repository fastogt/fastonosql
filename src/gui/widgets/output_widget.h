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

#include <QWidget>

#include "core/events/events_info.h"

class QAction;
class QPushButton;

namespace fasto {
namespace qt {
namespace gui {
  class IconLabel;
}
}
}

namespace fastonosql {

class FastoTextView;
class FastoTreeView;
class FastoTableView;
class FastoCommonModel;

class OutputWidget
  : public QWidget {
  Q_OBJECT
 public:
  explicit OutputWidget(IServerSPtr server, QWidget* parent = 0);

 public Q_SLOTS:
  void rootCreate(const events_info::CommandRootCreatedInfo& res);
  void rootCompleate(const events_info::CommandRootCompleatedInfo& res);

  void startExecuteCommand(const events_info::CommandRequest& req);
  void finishExecuteCommand(const events_info::CommandResponce& res);

  void addChild(FastoObject* child);
  void itemUpdate(FastoObject* item, common::Value* newValue);

 private Q_SLOTS:
  void executeCommand(CommandKeySPtr cmd);

  void setTreeView();
  void setTableView();
  void setTextView();

 private:
  void syncWithSettings();
  void updateTimeLabel(const events_info::EventInfoBase& evinfo);
  fasto::qt::gui::IconLabel* timeLabel_;
  QPushButton* treeButton_;
  QPushButton* tableButton_;
  QPushButton* textButton_;

  FastoCommonModel* commonModel_;
  FastoTreeView* treeView_;
  FastoTableView* tableView_;
  FastoTextView* textView_;
  IServerSPtr server_;
};

}  // namespace fastonosql
