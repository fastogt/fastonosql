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

#include "proxy/core_fwd.h"  // for IServerSPtr
#include "core/database/idatabase_info.h"

#include "global/global.h"  // for FastoObject, etc

class QPushButton;  // lines 27-27

namespace common {
namespace qt {
namespace gui {
class IconLabel;
}
}
}  // lines 32-32
namespace fastonosql {
namespace core {
namespace events_info {
class EventInfoBase;
struct ExecuteInfoRequest;
struct ExecuteInfoResponce;
struct CommandRootCompleatedInfo;
struct CommandRootCreatedInfo;
}
}
}
namespace fastonosql {
namespace gui {
class FastoTableView;
class FastoTextView;
class FastoTreeView;
class FastoCommonModel;
}
}

namespace fastonosql {
namespace gui {

class OutputWidget : public QWidget {
  Q_OBJECT
 public:
  explicit OutputWidget(core::IServerSPtr server, QWidget* parent = 0);

 private Q_SLOTS:
  void createKey(const core::NDbKValue& dbv);

  void startExecuteCommand(const core::events_info::ExecuteInfoRequest& req);
  void finishExecuteCommand(const core::events_info::ExecuteInfoResponce& res);

  void rootCreate(const core::events_info::CommandRootCreatedInfo& res);
  void rootCompleate(const core::events_info::CommandRootCompleatedInfo& res);

  void addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void updateKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);

  void addChild(FastoObjectIPtr child);
  void updateItem(FastoObject* item, common::ValueSPtr newValue);

  void setTreeView();
  void setTableView();
  void setTextView();

 private:
  void syncWithSettings();
  void updateTimeLabel(const core::events_info::EventInfoBase& evinfo);
  common::qt::gui::IconLabel* timeLabel_;
  QPushButton* treeButton_;
  QPushButton* tableButton_;
  QPushButton* textButton_;

  FastoCommonModel* commonModel_;
  FastoTreeView* treeView_;
  FastoTableView* tableView_;
  FastoTextView* textView_;
  const core::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
