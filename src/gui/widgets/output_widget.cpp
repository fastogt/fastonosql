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

#include <string>

#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QHeaderView>

#include "gui/fasto_text_view.h"
#include "gui/fasto_table_view.h"
#include "gui/fasto_tree_view.h"
#include "gui/fasto_common_model.h"
#include "gui/fasto_common_item.h"

#include "gui/gui_factory.h"
#include "fasto/qt/gui/icon_label.h"

#include "core/settings_manager.h"
#include "core/iserver.h"

#include "common/qt/convert_string.h"
#include "common/utf_string_conversions.h"
#include "common/time.h"
#include "common/logger.h"

namespace fastonosql {
namespace {

FastoCommonItem* createItem(fasto::qt::gui::TreeItem* parent, const std::string& key,
                            bool readOnly, fastonosql::FastoObject* item) {
  NValue val = common::make_value(item->value()->deepCopy());
  return new FastoCommonItem(common::convertFromString<QString>(key), val, readOnly, parent, item);
}

}

OutputWidget::OutputWidget(IServerSPtr server, QWidget* parent)
  : QWidget(parent), server_(server) {
  DCHECK(server);
  commonModel_ = new FastoCommonModel(this);
  VERIFY(connect(commonModel_, &FastoCommonModel::changedValue, this,
                 &OutputWidget::executeCommand, Qt::DirectConnection));
  VERIFY(connect(server.get(), &IServer::startedExecuteCommand, this,
                 &OutputWidget::startExecuteCommand, Qt::DirectConnection));
  VERIFY(connect(server.get(), &IServer::finishedExecuteCommand, this,
                 &OutputWidget::finishExecuteCommand, Qt::DirectConnection));

  treeView_ = new FastoTreeView;
  treeView_->setModel(commonModel_);

  tableView_ = new FastoTableView;
  tableView_->setModel(commonModel_);

  textView_ = new FastoTextView(server->outputDelemitr());
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

void OutputWidget::rootCreate(const events_info::CommandRootCreatedInfo& res) {
  FastoObject* rootObj = res.root.get();
  fastonosql::FastoCommonItem* root = createItem(NULL, std::string(), true, rootObj);
  commonModel_->setRoot(root);
}

void OutputWidget::rootCompleate(const events_info::CommandRootCompleatedInfo& res) {
  updateTimeLabel(res);
}

void OutputWidget::startExecuteCommand(const events_info::CommandRequest& req) {
}

void OutputWidget::finishExecuteCommand(const events_info::CommandResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  if (res.initiator() != this) {
    DEBUG_MSG_FORMAT<512>(common::logging::L_DEBUG,
                          "Skipped event in file: %s, function: %s", __FILE__, __FUNCTION__);
    return;
  }

  CommandKeySPtr key = res.cmd;
  if (key->type() == CommandKey::C_CREATE) {
    NDbKValue dbv = key->key();
    commonModel_->changeValue(dbv);
  }
}

void OutputWidget::addChild(FastoObject* child) {
  DCHECK(child->parent());

  FastoObjectCommand* command = dynamic_cast<FastoObjectCommand*>(child);
  if (command) {
    return;
  }

  command = dynamic_cast<FastoObjectCommand*>(child->parent());
  if (command) {
    void* parentinner = command->parent();

    QModelIndex parent;
    bool isFound = commonModel_->findItem(parentinner, parent);
    if (!isFound) {
      return;
    }

    fastonosql::FastoCommonItem* par = NULL;
    if (!parent.isValid()) {
      par = static_cast<fastonosql::FastoCommonItem*>(commonModel_->root());
    } else {
      par = common::utils_qt::item<fastonosql::FastoCommonItem*>(parent);
    }

    DCHECK(par);
    if (!par) {
      return;
    }

    std::string inputArgs = command->inputArgs();

    fastonosql::FastoCommonItem* comChild = createItem(par, getFirstWordFromLine(inputArgs),
                                                       command->isReadOnly(), child);
    commonModel_->insertItem(parent, comChild);
  } else {
    FastoObjectArray* arr = dynamic_cast<FastoObjectArray*>(child->parent());
    if (arr) {
      QModelIndex parent;
      bool isFound = commonModel_->findItem(arr, parent);
      if (!isFound) {
        return;
      }

      fastonosql::FastoCommonItem* par = NULL;
      if (!parent.isValid()) {
        par = static_cast<fastonosql::FastoCommonItem*>(commonModel_->root());
      } else {
        par = common::utils_qt::item<fastonosql::FastoCommonItem*>(parent);
      }

      DCHECK(par);
      if (!par) {
        return;
      }

      fastonosql::FastoCommonItem* comChild = createItem(par, std::string(), true, child);
      commonModel_->insertItem(parent, comChild);
    } else {
      NOTREACHED();
    }
  }
}

void OutputWidget::itemUpdate(FastoObject* item, common::Value *newValue) {
  QModelIndex index;
  bool isFound = commonModel_->findItem(item, index);
  if (!isFound) {
    return;
  }

  fastonosql::FastoCommonItem* it = common::utils_qt::item<fastonosql::FastoCommonItem*>(index);
  if (!it) {
    return;
  }

  DCHECK(item->value() == newValue);

  NValue nval = common::make_value(newValue->deepCopy());
  it->setValue(nval);
  commonModel_->updateItem(index.parent(), index);
}

void OutputWidget::executeCommand(CommandKeySPtr cmd) {
  if (server_) {
    events_info::CommandRequest req(this, server_->currentDatabaseInfo(), cmd);
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
  supportedViews curV = SettingsManager::instance().defaultView();
  if (curV == Tree) {
    setTreeView();
  } else if (curV == Table) {
    setTableView();
  } else {
    setTextView();
  }
}

void OutputWidget::updateTimeLabel(const events_info::EventInfoBase& evinfo) {
  timeLabel_->setText(QString("%1 msec").arg(evinfo.elapsedTime()));
}

}  // namespace fastonosql
