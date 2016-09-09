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

#include "gui/widgets/output_widget.h"

#include <memory>    // for __shared_ptr, operator==, etc
#include <stddef.h>  // for NULL
#include <string>    // for string

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSplitter>

#include "common/convert2string.h"  // for ConvertFromString
#include "common/error.h"           // for Error
#include "common/log_levels.h"      // for LEVEL_LOG::L_DEBUG
#include "common/logger.h"          // for DEBUG_MSG_FORMAT
#include "common/macros.h"          // for VERIFY, CHECK, DNOTREACHED, etc
#include "common/qt/utils_qt.h"     // for item
#include "common/value.h"           // for StringValue, Value, etc

#include "core/db_key.h"              // for NKey, NDbKValue, NValue
#include "core/events/events_info.h"  // for CommandResponce, etc
#include "core/iserver.h"             // for IServer
#include "core/settings_manager.h"    // for SettingsManager

#include "common/qt/gui/base/tree_item.h"  // for TreeItem
#include "common/qt/gui/icon_label.h"      // for IconLabel

#include "global/types.h"  // for supportedViews, etc

#include "gui/fasto_common_item.h"   // for FastoCommonItem
#include "gui/fasto_common_model.h"  // for FastoCommonModel
#include "gui/fasto_table_view.h"    // for FastoTableView
#include "gui/fasto_text_view.h"     // for FastoTextView
#include "gui/fasto_tree_view.h"     // for FastoTreeView
#include "gui/gui_factory.h"         // for GuiFactory

namespace fastonosql {
namespace gui {
namespace {

FastoCommonItem* createItem(fasto::qt::gui::TreeItem* parent,
                            const std::string& key,
                            bool readOnly,
                            FastoObject* item) {
  core::NValue val = item->value();
  core::NDbKValue nkey(core::NKey(key), val);
  return new FastoCommonItem(nkey, item->delimiter(), readOnly, parent, item);
}

FastoCommonItem* createRootItem(FastoObject* item) {
  auto value = item->value();
  std::string str;
  bool res = value->getAsString(&str);
  CHECK(res);
  core::NValue val(common::Value::createStringValue(str));
  core::NDbKValue nkey(core::NKey(std::string()), val);
  return new FastoCommonItem(nkey, item->delimiter(), true, NULL, item);
}

}  // namespace

OutputWidget::OutputWidget(core::IServerSPtr server, QWidget* parent)
    : QWidget(parent), server_(server) {
  CHECK(server_);

  commonModel_ = new FastoCommonModel(this);
  VERIFY(connect(commonModel_, &FastoCommonModel::changedValue, this, &OutputWidget::executeCommand,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::startedExecuteCommand, this,
                 &OutputWidget::startExecuteCommand, Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::finishedExecuteCommand, this,
                 &OutputWidget::finishExecuteCommand, Qt::DirectConnection));

  VERIFY(connect(server_.get(), &core::IServer::rootCreated, this, &OutputWidget::rootCreate,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::rootCompleated, this, &OutputWidget::rootCompleate,
                 Qt::DirectConnection));

  VERIFY(connect(server_.get(), &core::IServer::addedChild, this, &OutputWidget::addChild,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::itemUpdated, this, &OutputWidget::itemUpdate,
                 Qt::DirectConnection));

  treeView_ = new FastoTreeView;
  treeView_->setModel(commonModel_);

  tableView_ = new FastoTableView;
  tableView_->setModel(commonModel_);

  QString delimiter = common::ConvertFromString<QString>(server_->delimiter());
  textView_ = new FastoTextView(delimiter);
  textView_->setModel(commonModel_);

  timeLabel_ = new fasto::qt::gui::IconLabel(GuiFactory::instance().timeIcon(), "0", QSize(32, 32));

  QVBoxLayout* mainL = new QVBoxLayout;
  QHBoxLayout* topL = new QHBoxLayout;
  QSplitter* splitter = new QSplitter;
  splitter->setOrientation(Qt::Horizontal);
  splitter->setHandleWidth(1);

  treeButton_ = new QPushButton;
  tableButton_ = new QPushButton;
  textButton_ = new QPushButton;
  treeButton_->setIcon(GuiFactory::instance().treeIcon());
  VERIFY(connect(treeButton_, SIGNAL(clicked()), this, SLOT(setTreeView())));
  tableButton_->setIcon(GuiFactory::instance().tableIcon());
  VERIFY(connect(tableButton_, SIGNAL(clicked()), this, SLOT(setTableView())));
  textButton_->setIcon(GuiFactory::instance().textIcon());
  VERIFY(connect(textButton_, SIGNAL(clicked()), this, SLOT(setTextView())));

  topL->addWidget(treeButton_);
  topL->addWidget(tableButton_);
  topL->addWidget(textButton_);
  topL->addWidget(splitter);
  topL->addWidget(timeLabel_);

  mainL->addLayout(topL);
  mainL->addWidget(treeView_);
  mainL->addWidget(tableView_);
  mainL->addWidget(textView_);
  setLayout(mainL);
  syncWithSettings();
}

void OutputWidget::rootCreate(const core::events_info::CommandRootCreatedInfo& res) {
  FastoObject* rootObj = res.root.get();
  fastonosql::gui::FastoCommonItem* root = createRootItem(rootObj);
  commonModel_->setRoot(root);
}

void OutputWidget::rootCompleate(const core::events_info::CommandRootCompleatedInfo& res) {
  updateTimeLabel(res);
}

void OutputWidget::startExecuteCommand(const core::events_info::CommandRequest& req) {
  UNUSED(req);
}

void OutputWidget::finishExecuteCommand(const core::events_info::CommandResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  if (res.initiator() != this) {
    DEBUG_MSG_FORMAT<512>(common::logging::L_DEBUG, "Skipped event in file: %s, function: %s",
                          __FILE__, __FUNCTION__);
    return;
  }

  core::CommandKeySPtr key = res.cmd;
  if (key->type() == core::CommandKey::C_CREATE) {
    core::NDbKValue dbv = key->key();
    commonModel_->changeValue(dbv);
  }
}

void OutputWidget::addChild(FastoObjectIPtr child) {
  DCHECK(child->parent());

  FastoObjectCommand* command = dynamic_cast<FastoObjectCommand*>(child.get());  // +
  if (command) {
    return;
  }

  command = dynamic_cast<FastoObjectCommand*>(child->parent());  // +
  if (command) {
    void* parentinner = command->parent();

    QModelIndex parent;
    bool isFound = commonModel_->findItem(parentinner, &parent);
    if (!isFound) {
      return;
    }

    fastonosql::gui::FastoCommonItem* par = nullptr;
    if (!parent.isValid()) {
      par = static_cast<fastonosql::gui::FastoCommonItem*>(commonModel_->root());
    } else {
      par = common::utils_qt::item<fasto::qt::gui::TreeItem*, fastonosql::gui::FastoCommonItem*>(
          parent);
    }

    if (!par) {
      DNOTREACHED();
      return;
    }

    std::string inputArgs = command->inputArgs();
    fastonosql::gui::FastoCommonItem* comChild =
        createItem(par, GetFirstWordFromLine(inputArgs), command->isReadOnly(), child.get());
    commonModel_->insertItem(parent, comChild);
  } else {
    FastoObjectArray* arr = dynamic_cast<FastoObjectArray*>(child->parent());  // +
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
      par = common::utils_qt::item<fasto::qt::gui::TreeItem*, fastonosql::gui::FastoCommonItem*>(
          parent);
    }

    if (!par) {
      DNOTREACHED();
      return;
    }

    fastonosql::gui::FastoCommonItem* comChild = createItem(par, std::string(), true, child.get());
    commonModel_->insertItem(parent, comChild);
  }
}

void OutputWidget::itemUpdate(FastoObject* item, common::ValueSPtr newValue) {
  QModelIndex index;
  bool isFound = commonModel_->findItem(item, &index);
  if (!isFound) {
    return;
  }

  FastoCommonItem* it = common::utils_qt::item<fasto::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!it) {
    return;
  }

  core::NValue nval = newValue;
  it->setValue(nval);
  commonModel_->updateItem(index.parent(), index);
}

void OutputWidget::executeCommand(core::CommandKeySPtr cmd) {
  if (server_) {
    core::events_info::CommandRequest req(this, server_->currentDatabaseInfo(), cmd);
    server_->executeCommand(req);
  }
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
  supportedViews curV = core::SettingsManager::instance().defaultView();
  if (curV == Tree) {
    setTreeView();
  } else if (curV == Table) {
    setTableView();
  } else {
    setTextView();
  }
}

void OutputWidget::updateTimeLabel(const core::events_info::EventInfoBase& evinfo) {
  timeLabel_->setText(QString("%1 msec").arg(evinfo.elapsedTime()));
}

}  // namespace gui
}  // namespace fastonosql
