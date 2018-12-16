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

#pragma once

#include <QWidget>

#include <fastonosql/core/database/idatabase_info.h>
#include <fastonosql/core/global.h>  // for FastoObject, etc

#include "proxy/proxy_fwd.h"  // for IServerSPtr
#include "proxy/types.h"

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
namespace gui {

class FastoTextView;
class FastoCommonModel;
class SaveKeyEditWidget;

class OutputWidget : public QWidget {
  Q_OBJECT
 public:
  typedef QWidget base_class;
  static const QSize kIconSize;

  explicit OutputWidget(proxy::IServerSPtr server, QWidget* parent = Q_NULLPTR);

 private Q_SLOTS:
  void createKey(const core::NDbKValue& dbv);
  void createKeyFromEditor(const core::NDbKValue& dbv);

  void startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req);
  void finishExecuteCommand(const proxy::events_info::ExecuteInfoResponce& res);

  void rootCreate(const proxy::events_info::CommandRootCreatedInfo& res);
  void rootCompleate(const proxy::events_info::CommandRootCompleatedInfo& res);

  void addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);
  void updateKey(core::IDataBaseInfoSPtr db, core::NDbKValue key);

  void addChild(core::FastoObjectIPtr child);
  void addCommand(core::FastoObjectCommand* command, core::FastoObject* child);
  void updateItem(core::FastoObject* item, common::ValueSPtr new_value);

  void setTreeView();
  void setTableView();
  void setTextView();
  void setEditKeyView();

 protected:
  void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  void createKeyImpl(const core::NDbKValue& dbv, void* initiator);

  void syncWithView(proxy::SupportedView view);
  void updateTimeLabel(const proxy::events_info::EventInfoBase& evinfo);
  common::qt::gui::IconLabel* time_label_;
  QPushButton* tree_button_;
  QPushButton* table_button_;
  QPushButton* text_button_;
  QPushButton* key_button_;

  FastoCommonModel* common_model_;
  QTreeView* tree_view_;
  QTableView* table_view_;
  FastoTextView* text_view_;
  SaveKeyEditWidget* key_editor_;
  const proxy::IServerSPtr server_;
  proxy::SupportedView current_view_;
};

}  // namespace gui
}  // namespace fastonosql
