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

#include <vector>

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSplitter>

#include <common/qt/gui/icon_label.h>  // for IconLabel
#include <common/qt/logger.h>

#include "proxy/server/iserver.h"    // for IServer
#include "proxy/settings_manager.h"  // for SettingsManager

#include "gui/gui_factory.h"                     // for GuiFactory
#include "gui/models/fasto_common_model.h"       // for FastoCommonModel
#include "gui/models/items/fasto_common_item.h"  // for FastoCommonItem
#include "gui/views/fasto_table_view.h"          // for FastoTableView
#include "gui/views/fasto_text_view.h"           // for FastoTextView
#include "gui/views/fasto_tree_view.h"           // for FastoTreeView
#include "gui/widgets/delegate/type_delegate.h"
#include "gui/widgets/save_key_edit_widget.h"

namespace {
const QString trTreeViewTooltip = QObject::tr("Tree view");
const QString trTableViewTooltip = QObject::tr("Table view");
const QString trTextViewTooltip = QObject::tr("Text view");
const QString trKeyViewTooltip = QObject::tr("Key view");
}  // namespace

namespace fastonosql {
namespace gui {
namespace {

FastoCommonItem* CreateItem(common::qt::gui::TreeItem* parent,
                            core::command_buffer_t key,
                            bool read_only,
                            core::FastoObject* item) {
  const core::NValue value(item->GetValue());
  const core::nkey_t raw_key(key);
  const core::NDbKValue nkey(core::NKey(raw_key), value);
  return new FastoCommonItem(nkey, item->GetDelimiter(), read_only, parent, item);
}

FastoCommonItem* CreateRootItem(core::FastoObject* item) {
  const core::NValue value(item->GetValue());
  const core::nkey_t raw_key;
  const core::NKey nk(raw_key);
  const core::NDbKValue nkey(nk, value);
  return new FastoCommonItem(nkey, item->GetDelimiter(), true, nullptr, item);
}

}  // namespace

const QSize OutputWidget::icon_size = QSize(24, 24);

OutputWidget::OutputWidget(proxy::IServerSPtr server, QWidget* parent) : base_class(parent), server_(server) {
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
  tree_view_->setItemDelegateForColumn(FastoCommonModel::eValue, new TypeDelegate(this));

  table_view_ = new QTableView;
  table_view_->setModel(common_model_);
  table_view_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_view_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_view_->horizontalHeader()->setStretchLastSection(false);
  table_view_->setItemDelegateForColumn(FastoCommonModel::eValue, new TypeDelegate(this));

  text_view_ = new FastoTextView;
  text_view_->setModel(common_model_);

  key_editor_ = new SaveKeyEditWidget;
  key_editor_->setEnableKeyEdit(false);
  VERIFY(connect(key_editor_, &SaveKeyEditWidget::keyReadyToSave, this, &OutputWidget::createKeyFromEditor,
                 Qt::DirectConnection));

  QHBoxLayout* top_layout = new QHBoxLayout;
  tree_button_ = new QPushButton;
  table_button_ = new QPushButton;
  text_button_ = new QPushButton;
  key_button_ = new QPushButton;
  time_label_ = new common::qt::gui::IconLabel(GuiFactory::GetInstance().timeIcon(), icon_size, "0");
  tree_button_->setIcon(GuiFactory::GetInstance().treeIcon());
  VERIFY(connect(tree_button_, &QPushButton::clicked, this, &OutputWidget::setTreeView));
  table_button_->setIcon(GuiFactory::GetInstance().tableIcon());
  VERIFY(connect(table_button_, &QPushButton::clicked, this, &OutputWidget::setTableView));
  text_button_->setIcon(GuiFactory::GetInstance().textIcon());
  VERIFY(connect(text_button_, &QPushButton::clicked, this, &OutputWidget::setTextView));
  key_button_->setIcon(GuiFactory::GetInstance().keyIcon());
  VERIFY(connect(key_button_, &QPushButton::clicked, this, &OutputWidget::setEditKeyView));
  top_layout->addWidget(tree_button_);
  top_layout->addWidget(table_button_);
  top_layout->addWidget(text_button_);
  top_layout->addWidget(key_button_);
  top_layout->addWidget(new QSplitter(Qt::Horizontal));
  top_layout->addWidget(time_label_);
  top_layout->setContentsMargins(0, 0, 0, 0);

  QSplitter* splitter = new QSplitter(Qt::Vertical);
  splitter->addWidget(tree_view_);
  splitter->addWidget(table_view_);
  splitter->addWidget(text_view_);
  splitter->addWidget(key_editor_);

  current_view_ = proxy::SettingsManager::GetInstance()->GetDefaultView();

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addLayout(top_layout, 0);
  main_layout->addWidget(splitter, 1);
  setLayout(main_layout);

  syncWithView(current_view_);
  retranslateUi();
}

void OutputWidget::rootCreate(const proxy::events_info::CommandRootCreatedInfo& res) {
  core::FastoObject* root_obj = res.root.get();
  FastoCommonItem* root = CreateRootItem(root_obj);
  common_model_->setRoot(root);
}

void OutputWidget::rootCompleate(const proxy::events_info::CommandRootCompleatedInfo& res) {
  core::FastoObject* root_obj = res.root.get();
  auto childs = root_obj->GetChildrens();
  if (!childs.empty()) {
    core::FastoObjectIPtr last_child = childs.back();
    core::FastoObjectCommand* command = dynamic_cast<core::FastoObjectCommand*>(last_child.get());
    if (command) {
      core::translator_t tr = server_->GetTranslator();
      core::command_buffer_t input_cmd = command->GetInputCommand();
      core::command_buffer_t key;
      if (tr->IsLoadKeyCommand(input_cmd, &key)) {
        setEditKeyView();
      } else {
        syncWithView(current_view_);
      }
    }
  }
  updateTimeLabel(res);
}

void OutputWidget::addKey(core::IDataBaseInfoSPtr db, core::NDbKValue key) {
  UNUSED(db);
  const auto inf = server_->GetCurrentServerInfo();
  key_editor_->initialize(server_->GetSupportedValueTypes(inf->GetVersion()), key);
  common_model_->changeValue(key);
}

void OutputWidget::updateKey(core::IDataBaseInfoSPtr db, core::NDbKValue key) {
  UNUSED(db);
  const auto inf = server_->GetCurrentServerInfo();
  key_editor_->initialize(server_->GetSupportedValueTypes(inf->GetVersion()), key);
  common_model_->changeValue(key);
}

void OutputWidget::startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req) {
  if (req.initiator() == key_editor_) {
    key_editor_->startSaveKey();
  }
}

void OutputWidget::finishExecuteCommand(const proxy::events_info::ExecuteInfoResponce& res) {
  if (res.initiator() == key_editor_) {
    key_editor_->finishSaveKey();
  }
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

  FastoCommonItem* par = nullptr;
  if (!parent.isValid()) {
    par = static_cast<FastoCommonItem*>(common_model_->root());
  } else {
    par = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(parent);
  }

  if (!par) {
    DNOTREACHED();
    return;
  }

  FastoCommonItem* com_child = CreateItem(par, core::command_buffer_t(), true, child.get());
  common_model_->insertItem(parent, com_child);
}

void OutputWidget::addCommand(core::FastoObjectCommand* command, core::FastoObject* child) {
  void* parentinner = command->GetParent();

  QModelIndex parent;
  bool is_found = common_model_->findItem(parentinner, &parent);
  if (!is_found) {
    return;
  }

  FastoCommonItem* par = nullptr;
  if (!parent.isValid()) {
    par = static_cast<FastoCommonItem*>(common_model_->root());
  } else {
    par = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(parent);
  }

  if (!par) {
    DNOTREACHED();
    return;
  }

  FastoCommonItem* common_child = nullptr;
  core::translator_t tr = server_->GetTranslator();
  core::command_buffer_t input_cmd = command->GetInputCommand();
  core::command_buffer_t key;
  if (tr->IsLoadKeyCommand(input_cmd, &key)) {
    common_child = CreateItem(par, key, false, child);
  } else {
    common_child = CreateItem(par, input_cmd, true, child);
  }
  common_model_->insertItem(parent, common_child);
}

void OutputWidget::updateItem(core::FastoObject* item, common::ValueSPtr new_value) {
  QModelIndex index;
  bool isFound = common_model_->findItem(item, &index);
  if (!isFound) {
    return;
  }

  FastoCommonItem* it = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!it) {
    return;
  }

  core::NValue nval(new_value);
  it->setValue(nval);
  common_model_->updateItem(index.parent(), index);
}

void OutputWidget::createKey(const core::NDbKValue& dbv) {
  createKeyImpl(dbv, this);
}

void OutputWidget::createKeyFromEditor(const core::NDbKValue& dbv) {
  createKeyImpl(dbv, key_editor_);
}

void OutputWidget::setTreeView() {
  tree_view_->setVisible(true);
  table_view_->setVisible(false);
  text_view_->setVisible(false);
  key_editor_->setVisible(false);
  current_view_ = proxy::kTree;
}

void OutputWidget::setTableView() {
  tree_view_->setVisible(false);
  table_view_->setVisible(true);
  text_view_->setVisible(false);
  key_editor_->setVisible(false);
  current_view_ = proxy::kTable;
}

void OutputWidget::setTextView() {
  tree_view_->setVisible(false);
  table_view_->setVisible(false);
  text_view_->setVisible(true);
  key_editor_->setVisible(false);
  current_view_ = proxy::kText;
}

void OutputWidget::setEditKeyView() {
  tree_view_->setVisible(false);
  table_view_->setVisible(false);
  text_view_->setVisible(false);
  key_editor_->setVisible(true);
}

void OutputWidget::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  base_class::changeEvent(e);
}

void OutputWidget::retranslateUi() {
  tree_button_->setToolTip(trTreeViewTooltip);
  table_button_->setToolTip(trTableViewTooltip);
  text_button_->setToolTip(trTextViewTooltip);
  key_button_->setToolTip(trKeyViewTooltip);
}

void OutputWidget::createKeyImpl(const core::NDbKValue& dbv, void* initiator) {
  core::translator_t tran = server_->GetTranslator();
  core::command_buffer_t cmd_text;
  common::Error err = tran->CreateKeyCommand(dbv, &cmd_text);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(initiator, cmd_text, 0, 0, true, true);
  server_->Execute(req);
}

void OutputWidget::syncWithView(proxy::SupportedView view) {
  if (view == proxy::kTree) {
    setTreeView();
  } else if (view == proxy::kTable) {
    setTableView();
  } else if (view == proxy::kText) {
    setTextView();
  } else {
    NOTREACHED() << "Unknown view: " << view;
  }
}

void OutputWidget::updateTimeLabel(const proxy::events_info::EventInfoBase& evinfo) {
  static const QString msec_template("%1 msec");
  time_label_->setText(msec_template.arg(evinfo.ElapsedTime()));
}

}  // namespace gui
}  // namespace fastonosql
