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

#include "gui/widgets/type_delegate.h"

#include "gui/fasto_common_item.h"   // for FastoCommonItem
#include "gui/fasto_common_model.h"  // for FastoCommonModel
#include "gui/fasto_table_view.h"    // for FastoTableView
#include "gui/fasto_text_view.h"     // for FastoTextView
#include "gui/fasto_tree_view.h"     // for FastoTreeView
#include "gui/gui_factory.h"         // for GuiFactory

namespace fastonosql {
namespace gui {
namespace {

FastoCommonItem* createItem(common::qt::gui::TreeItem* parent,
                            core::string_key_t key,
                            bool readOnly,
                            core::FastoObject* item) {
  core::NValue value = item->GetValue();
  core::key_t raw_key(key);
  core::NDbKValue nkey(core::NKey(raw_key), value);
  return new FastoCommonItem(nkey, item->GetDelimiter(), readOnly, parent, item);
}

FastoCommonItem* createRootItem(core::FastoObject* item) {
  core::NValue value = item->GetValue();
  core::key_t raw_key;
  core::NKey nk(raw_key);
  core::NDbKValue nkey(nk, value);
  return new FastoCommonItem(nkey, item->GetDelimiter(), true, nullptr, item);
}

}  // namespace

OutputWidget::OutputWidget(proxy::IServerSPtr server, QWidget* parent) : QWidget(parent), server_(server) {
  CHECK(server_);

  commonModel_ = new FastoCommonModel(this);
  VERIFY(connect(commonModel_, &FastoCommonModel::changedValue, this, &OutputWidget::createKey, Qt::DirectConnection));
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

  treeView_ = new QTreeView;
  treeView_->setModel(commonModel_);
  treeView_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  treeView_->header()->setSectionResizeMode(1, QHeaderView::Stretch);
  treeView_->header()->setStretchLastSection(false);
  treeView_->setItemDelegateForColumn(FastoCommonItem::eValue, new TypeDelegate(this));

  tableView_ = new QTableView;
  tableView_->setModel(commonModel_);
  tableView_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  tableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  tableView_->horizontalHeader()->setStretchLastSection(false);
  tableView_->setItemDelegateForColumn(FastoCommonItem::eValue, new TypeDelegate(this));

  textView_ = new FastoTextView;
  textView_->setModel(commonModel_);

  timeLabel_ = new common::qt::gui::IconLabel(GuiFactory::GetInstance().timeIcon(), "0", QSize(32, 32));

  QVBoxLayout* mainL = new QVBoxLayout;
  QHBoxLayout* topL = new QHBoxLayout;

  treeButton_ = new QPushButton;
  tableButton_ = new QPushButton;
  textButton_ = new QPushButton;
  treeButton_->setIcon(GuiFactory::GetInstance().treeIcon());
  VERIFY(connect(treeButton_, &QPushButton::clicked, this, &OutputWidget::setTreeView));
  tableButton_->setIcon(GuiFactory::GetInstance().tableIcon());
  VERIFY(connect(tableButton_, &QPushButton::clicked, this, &OutputWidget::setTableView));
  textButton_->setIcon(GuiFactory::GetInstance().textIcon());
  VERIFY(connect(textButton_, &QPushButton::clicked, this, &OutputWidget::setTextView));

  topL->addWidget(treeButton_);
  topL->addWidget(tableButton_);
  topL->addWidget(textButton_);
  topL->addWidget(new QSplitter(Qt::Horizontal));
  topL->addWidget(timeLabel_);

  mainL->addLayout(topL);
  mainL->addWidget(treeView_);
  mainL->addWidget(tableView_);
  mainL->addWidget(textView_);
  setLayout(mainL);
  syncWithSettings();
}

void OutputWidget::rootCreate(const proxy::events_info::CommandRootCreatedInfo& res) {
  core::FastoObject* rootObj = res.root.get();
  fastonosql::gui::FastoCommonItem* root = createRootItem(rootObj);
  commonModel_->setRoot(root);
}

void OutputWidget::rootCompleate(const proxy::events_info::CommandRootCompleatedInfo& res) {
  updateTimeLabel(res);
}

void OutputWidget::addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key) {
  UNUSED(db);
  commonModel_->changeValue(key);
}

void OutputWidget::updateKey(core::IDataBaseInfoSPtr db, core::NDbKValue key) {
  UNUSED(db);
  commonModel_->changeValue(key);
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
    void* parentinner = command->GetParent();

    QModelIndex parent;
    bool isFound = commonModel_->findItem(parentinner, &parent);
    if (!isFound) {
      return;
    }

    fastonosql::gui::FastoCommonItem* par = nullptr;
    if (!parent.isValid()) {
      par = static_cast<fastonosql::gui::FastoCommonItem*>(commonModel_->root());
    } else {
      par = common::qt::item<common::qt::gui::TreeItem*, fastonosql::gui::FastoCommonItem*>(parent);
    }

    if (!par) {
      DNOTREACHED();
      return;
    }

    fastonosql::gui::FastoCommonItem* comChild = nullptr;
    core::translator_t tr = server_->GetTranslator();
    core::command_buffer_t input_cmd = command->GetInputCommand();
    core::string_key_t key;
    if (tr->IsLoadKeyCommand(input_cmd, &key)) {
      comChild = createItem(par, key, false, child.get());
    } else {
      comChild = createItem(par, input_cmd, true, child.get());
    }
    commonModel_->insertItem(parent, comChild);
  } else {
    core::FastoObjectArray* arr = dynamic_cast<core::FastoObjectArray*>(child->GetParent());  // +
    CHECK(arr);

    QModelIndex parent;
    bool isFound = commonModel_->findItem(arr, &parent);
    if (!isFound) {
      return;
    }

    fastonosql::gui::FastoCommonItem* par = nullptr;
    if (!parent.isValid()) {
      par = static_cast<fastonosql::gui::FastoCommonItem*>(commonModel_->root());
    } else {
      par = common::qt::item<common::qt::gui::TreeItem*, fastonosql::gui::FastoCommonItem*>(parent);
    }

    if (!par) {
      DNOTREACHED();
      return;
    }

    fastonosql::gui::FastoCommonItem* comChild = createItem(par, core::command_buffer_t(), true, child.get());
    commonModel_->insertItem(parent, comChild);
  }
}

void OutputWidget::updateItem(core::FastoObject* item, common::ValueSPtr newValue) {
  QModelIndex index;
  bool isFound = commonModel_->findItem(item, &index);
  if (!isFound) {
    return;
  }

  FastoCommonItem* it = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!it) {
    return;
  }

  core::NValue nval = newValue;
  it->setValue(nval);
  commonModel_->updateItem(index.parent(), index);
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
  treeView_->setVisible(true);
  tableView_->setVisible(false);
  textView_->setVisible(false);
}

void OutputWidget::setTableView() {
  treeView_->setVisible(false);
  tableView_->setVisible(true);
  textView_->setVisible(false);
}

void OutputWidget::setTextView() {
  treeView_->setVisible(false);
  tableView_->setVisible(false);
  textView_->setVisible(true);
}

void OutputWidget::syncWithSettings() {
  proxy::supportedViews curV = proxy::SettingsManager::GetInstance()->GetDefaultView();
  if (curV == proxy::Tree) {
    setTreeView();
  } else if (curV == proxy::Table) {
    setTableView();
  } else {
    setTextView();
  }
}

void OutputWidget::updateTimeLabel(const proxy::events_info::EventInfoBase& evinfo) {
  timeLabel_->setText(QString("%1 msec").arg(evinfo.ElapsedTime()));
}

}  // namespace gui
}  // namespace fastonosql
