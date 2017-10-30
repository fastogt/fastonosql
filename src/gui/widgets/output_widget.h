/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "core/database/idatabase_info.h"
#include "proxy/proxy_fwd.h"  // for IServerSPtr

#include "core/global.h"  // for FastoObject, etc

class QPushButton;  // lines 27-27
class QTreeView;
class QTableView;

namespace common {
namespace qt {
namespace gui {
class IconLabel;
}
}  // namespace qt
}  // namespace common
namespace fastonosql {
namespace proxy {
namespace events_info {
class EventInfoBase;
struct ExecuteInfoRequest;
struct ExecuteInfoResponce;
struct CommandRootCompleatedInfo;
struct CommandRootCreatedInfo;
}  // namespace events_info
}  // namespace proxy
}  // namespace fastonosql
namespace fastonosql {
namespace gui {
class FastoTextView;
class FastoCommonModel;
}  // namespace gui
}  // namespace fastonosql

namespace fastonosql {
namespace gui {

class OutputWidget : public QWidget {
  Q_OBJECT
 public:
  explicit OutputWidget(proxy::IServerSPtr server, QWidget* parent = 0);

 private Q_SLOTS:
  void createKey(const core::NDbKValue& dbv);

  void startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req);
  void finishExecuteCommand(const proxy::events_info::ExecuteInfoResponce& res);

  void rootCreate(const proxy::events_info::CommandRootCreatedInfo& res);
  void rootCompleate(const proxy::events_info::CommandRootCompleatedInfo& res);

  void addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void updateKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);

  void addChild(core::FastoObjectIPtr child);
  void addCommand(core::FastoObjectCommand* command, core::FastoObject* child);
  void updateItem(core::FastoObject* item, common::ValueSPtr newValue);

  void setTreeView();
  void setTableView();
  void setTextView();

 private:
  void syncWithSettings();
  void updateTimeLabel(const proxy::events_info::EventInfoBase& evinfo);
  common::qt::gui::IconLabel* timeLabel_;
  QPushButton* treeButton_;
  QPushButton* tableButton_;
  QPushButton* textButton_;

  FastoCommonModel* commonModel_;
  QTreeView* treeView_;
  QTableView* tableView_;
  FastoTextView* textView_;
  const proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
