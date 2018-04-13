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

#include "gui/widgets/output_widget.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSplitter>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/qt/convert2string.h>
#include <common/qt/gui/icon_label.h>  // for IconLabel
#include <common/qt/logger.h>

#include "proxy/server/iserver.h"    // for IServer
#include "proxy/settings_manager.h"  // for SettingsManager

#include "gui/fasto_common_item.h"   // for FastoCommonItem
#include "gui/fasto_common_model.h"  // for FastoCommonModel
#include "gui/fasto_table_view.h"    // for FastoTableView
#include "gui/fasto_text_view.h"     // for FastoTextView
#include "gui/fasto_tree_view.h"     // for FastoTreeView
#include "gui/gui_factory.h"         // for GuiFactory
#include "gui/widgets/type_delegate.h"

namespace fastonosql {
namespace gui {
namespace {

core::FastoObjectCommand* FindCommand(core::FastoObject* obj) {
  if (!obj) {
    return nullptr;
  }

  core::FastoObjectCommand* command = dynamic_cast<core::FastoObjectCommand*>(obj);
  if (command) {
    return command;
  }

  return FindCommand(obj->GetParent());
}

FastoCommonItem* CreateItem(common::qt::gui::TreeItem* parent,
                            core::readable_string_t key,
                            bool readOnly,
                            core::FastoObject* item) {
  core::NValue value = item->GetValue();
  core::key_t raw_key(key);
  core::NDbKValue nkey(core::NKey(raw_key), value);
  return new FastoCommonItem(nkey, item->GetDelimiter(), readOnly, parent, item);
}

FastoCommonItem* CreateRootItem(core::FastoObject* item) {
  core::NValue value = item->GetValue();
  core::key_t raw_key;
  core::NKey nk(raw_key);
  core::NDbKValue nkey(nk, value);
  return new FastoCommonItem(nkey, item->GetDelimiter(), true, nullptr, item);
}

}  // namespace

OutputWidget::OutputWidget(proxy::IServerSPtr server, QWidget* parent) : QWidget(parent), server_(server) {
  CHECK(server_);

  common_model_ = new FastoCommonModel(this);
  VERIFY(connect(common_model_, &FastoCommonModel::changedValue, this, &OutputWidget::createKey, Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteStarted, this, &OutputWidget::startExecuteCommand,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteFinished, this, &OutputWidget::finishExecuteCommand,
                 Qt::DirectConnection));

  VERIFY(connect(server_.get(), &proxy::IServer::KeyAdded, this, &OutputWidget::addKey, Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::KeyLoaded, this, &OutputWidget::updateKey, Qt::DirectConnection));

  VERIFY(connect(server_.get(), &proxy::IServer::RootCreated, this, &OutputWidget::rootCreate, Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::RootCompleated, this, &OutputWidget::rootCompleate,
                 Qt::DirectConnection));

  VERIFY(connect(server_.get(), &proxy::IServer::ChildAdded, this, &OutputWidget::addChild, Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::ItemUpdated, this, &OutputWidget::updateItem, Qt::DirectConnection));

  tree_view_ = new QTreeView;
  tree_view_->setModel(common_model_);
  tree_view_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  tree_view_->header()->setSectionResizeMode(1, QHeaderView::Stretch);
  tree_view_->header()->setStretchLastSection(false);
  tree_view_->setItemDelegateForColumn(FastoCommonItem::eValue, new TypeDelegate(this));

  table_view_ = new QTableView;
  table_view_->setModel(common_model_);
  table_view_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_view_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_view_->horizontalHeader()->setStretchLastSection(false);
  table_view_->setItemDelegateForColumn(FastoCommonItem::eValue, new TypeDelegate(this));

  text_view_ = new FastoTextView;
  text_view_->setModel(common_model_);

  time_label_ = new common::qt::gui::IconLabel(GuiFactory::GetInstance().GetTimeIcon(), QSize(32, 32), "0");

  QVBoxLayout* mainL = new QVBoxLayout;
  QHBoxLayout* topL = new QHBoxLayout;

  tree_button_ = new QPushButton;
  table_button_ = new QPushButton;
  text_button_ = new QPushButton;
  tree_button_->setIcon(GuiFactory::GetInstance().GetTreeIcon());
  VERIFY(connect(tree_button_, &QPushButton::clicked, this, &OutputWidget::setTreeView));
  table_button_->setIcon(GuiFactory::GetInstance().GetTableIcon());
  VERIFY(connect(table_button_, &QPushButton::clicked, this, &OutputWidget::setTableView));
  text_button_->setIcon(GuiFactory::GetInstance().GetTextIcon());
  VERIFY(connect(text_button_, &QPushButton::clicked, this, &OutputWidget::setTextView));

  topL->addWidget(tree_button_);
  topL->addWidget(table_button_);
  topL->addWidget(text_button_);
  topL->addWidget(new QSplitter(Qt::Horizontal));
  topL->addWidget(time_label_);

  mainL->addLayout(topL);
  mainL->addWidget(tree_view_);
  mainL->addWidget(table_view_);
  mainL->addWidget(text_view_);
  setLayout(mainL);
  syncWithSettings();
}

void OutputWidget::rootCreate(const proxy::events_info::CommandRootCreatedInfo& res) {
  core::FastoObject* root_obj = res.root.get();
  fastonosql::gui::FastoCommonItem* root = CreateRootItem(root_obj);
  common_model_->setRoot(root);
}

void OutputWidget::rootCompleate(const proxy::events_info::CommandRootCompleatedInfo& res) {
  updateTimeLabel(res);
}

void OutputWidget::addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key) {
  UNUSED(db);
  common_model_->changeValue(key);
}

void OutputWidget::updateKey(core::IDataBaseInfoSPtr db, core::NDbKValue key) {
  UNUSED(db);
  common_model_->changeValue(key);
}

void OutputWidget::startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void OutputWidget::finishExecuteCommand(const proxy::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);
}

void OutputWidget::addChild(core::FastoObjectIPtr child) {
  DCHECK(child->GetParent());

  core::FastoObjectCommand* command = dynamic_cast<core::FastoObjectCommand*>(child.get());  // +
  if (command) {
    return;
  }

  command = dynamic_cast<core::FastoObjectCommand*>(child->GetParent());  // +
  if (command) {
    addCommand(command, child.get());
    return;
  }

  core::FastoObject* arr = dynamic_cast<core::FastoObject*>(child->GetParent());  // +
  CHECK(arr);

  QModelIndex parent;
  bool isFound = common_model_->findItem(arr, &parent);
  if (!isFound) {
    return;
  }

  fastonosql::gui::FastoCommonItem* par = nullptr;
  if (!parent.isValid()) {
    par = static_cast<fastonosql::gui::FastoCommonItem*>(common_model_->root());
  } else {
    par = common::qt::item<common::qt::gui::TreeItem*, fastonosql::gui::FastoCommonItem*>(parent);
  }

  if (!par) {
    DNOTREACHED();
    return;
  }

  fastonosql::gui::FastoCommonItem* comChild = CreateItem(par, core::command_buffer_t(), true, child.get());
  common_model_->insertItem(parent, comChild);
}

void OutputWidget::addCommand(core::FastoObjectCommand* command, core::FastoObject* child) {
  void* parentinner = command->GetParent();

  QModelIndex parent;
  bool isFound = common_model_->findItem(parentinner, &parent);
  if (!isFound) {
    return;
  }

  fastonosql::gui::FastoCommonItem* par = nullptr;
  if (!parent.isValid()) {
    par = static_cast<fastonosql::gui::FastoCommonItem*>(common_model_->root());
  } else {
    par = common::qt::item<common::qt::gui::TreeItem*, fastonosql::gui::FastoCommonItem*>(parent);
  }

  if (!par) {
    DNOTREACHED();
    return;
  }

  fastonosql::gui::FastoCommonItem* common_child = nullptr;
  core::translator_t tr = server_->GetTranslator();
  core::command_buffer_t input_cmd = command->GetInputCommand();
  core::readable_string_t key;
  if (tr->IsLoadKeyCommand(input_cmd, &key)) {
    common_child = CreateItem(par, key, false, child);
  } else {
    common_child = CreateItem(par, input_cmd, true, child);
  }
  common_model_->insertItem(parent, common_child);
}

void OutputWidget::updateItem(core::FastoObject* item, common::ValueSPtr newValue) {
  QModelIndex index;
  bool isFound = common_model_->findItem(item, &index);
  if (!isFound) {
    return;
  }

  FastoCommonItem* it = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!it) {
    return;
  }

  core::NValue nval = newValue;
  it->setValue(nval);
  common_model_->updateItem(index.parent(), index);
}

void OutputWidget::createKey(const core::NDbKValue& dbv) {
  core::translator_t tran = server_->GetTranslator();
  core::command_buffer_t cmd_text;
  common::Error err = tran->CreateKeyCommand(dbv, &cmd_text);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_text, 0, 0, true, true);
  server_->Execute(req);
}

void OutputWidget::setTreeView() {
  tree_view_->setVisible(true);
  table_view_->setVisible(false);
  text_view_->setVisible(false);
}

void OutputWidget::setTableView() {
  tree_view_->setVisible(false);
  table_view_->setVisible(true);
  text_view_->setVisible(false);
}

void OutputWidget::setTextView() {
  tree_view_->setVisible(false);
  table_view_->setVisible(false);
  text_view_->setVisible(true);
}

void OutputWidget::syncWithSettings() {
  proxy::supportedViews current_view = proxy::SettingsManager::GetInstance()->GetDefaultView();
  if (current_view == proxy::kTree) {
    setTreeView();
  } else if (current_view == proxy::kTable) {
    setTableView();
  } else if (current_view == proxy::kText) {
    setTextView();
  } else {
    NOTREACHED();
  }
}

void OutputWidget::updateTimeLabel(const proxy::events_info::EventInfoBase& evinfo) {
  static const QString msec_template("%1 msec");
  time_label_->setText(msec_template.arg(evinfo.ElapsedTime()));
}

}  // namespace gui
}  // namespace fastonosql
