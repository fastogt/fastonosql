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

#include <memory>  // for __shared_ptr, operator==, etc
#include <string>  // for string

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QSpinBox>
#include <QListWidget>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/error.h>           // for Error
#include <common/log_levels.h>      // for LEVEL_LOG::L_DEBUG
#include <common/macros.h>          // for VERIFY, CHECK, DNOTREACHED, etc
#include <common/value.h>           // for StringValue, Value, etc
#include <common/qt/utils_qt.h>     // for item
#include <common/qt/logger.h>
#include <common/qt/gui/base/tree_item.h>  // for TreeItem
#include <common/qt/gui/icon_label.h>      // for IconLabel
#include <common/qt/convert2string.h>

#include "core/db_key.h"              // for NKey, NDbKValue, NValue
#include "core/events/events_info.h"  // for CommandResponce, etc
#include "core/server/iserver.h"      // for IServer
#include "core/settings_manager.h"    // for SettingsManager

#include "global/types.h"  // for supportedViews, etc

#include "gui/fasto_common_item.h"   // for FastoCommonItem
#include "gui/fasto_common_model.h"  // for FastoCommonModel
#include "gui/fasto_table_view.h"    // for FastoTableView
#include "gui/fasto_text_view.h"     // for FastoTextView
#include "gui/fasto_tree_view.h"     // for FastoTreeView
#include "gui/gui_factory.h"         // for GuiFactory

Q_DECLARE_METATYPE(fastonosql::core::NValue)

namespace fastonosql {
namespace gui {
namespace {

class TypeDelegate : public QStyledItemDelegate {
 public:
  explicit TypeDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}

  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override {
    FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
    if (!node) {
      return QStyledItemDelegate::createEditor(parent, option, index);
    }

    common::Value::Type t = node->type();
    if (t == common::Value::TYPE_INTEGER || t == common::Value::TYPE_UINTEGER) {
      QSpinBox* editor = new QSpinBox(parent);
      editor->setRange(INT32_MIN, INT32_MAX);
      return editor;
    } else if (t == common::Value::TYPE_ARRAY) {
      QListWidget* editor = new QListWidget(parent);
      return editor;
    } else {
      return QStyledItemDelegate::createEditor(parent, option, index);
    }
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
    if (!node) {
      return;
    }

    core::NDbKValue dbv = node->dbv();
    core::NValue val = dbv.Value();
    common::Value::Type t = node->type();
    if (t == common::Value::TYPE_INTEGER) {
      int value = 0;
      if (val->getAsInteger(&value)) {
        QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
      }
    } else if (t == common::Value::TYPE_ARRAY) {
      common::ArrayValue* arr = nullptr;
      if (val->getAsList(&arr)) {
        QListWidget* listwidget = static_cast<QListWidget*>(editor);
        for (auto it = arr->begin(); it != arr->end(); ++it) {
          std::string val = (*it)->toString();
          if (val.empty()) {
            continue;
          }

          QListWidgetItem* nitem =
              new QListWidgetItem(common::ConvertFromString<QString>(val), listwidget);
          nitem->setFlags(nitem->flags() | Qt::ItemIsEditable);
          listwidget->addItem(nitem);
        }
      }
    } else if (t == common::Value::TYPE_UINTEGER) {
      unsigned int value = 0;
      if (val->getAsUInteger(&value)) {
        QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
      }
    } else {
      QStyledItemDelegate::setEditorData(editor, index);
    }
  }

  void setModelData(QWidget* editor,
                    QAbstractItemModel* model,
                    const QModelIndex& index) const override {
    FastoCommonItem* node = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
    if (!node) {
      return;
    }

    common::Value::Type t = node->type();
    if (t == common::Value::TYPE_INTEGER) {
      QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
      int value = spinBox->value();
      core::NValue val(common::Value::createIntegerValue(value));
      QVariant var = QVariant::fromValue(val);
      model->setData(index, var, Qt::EditRole);
    } else if (t == common::Value::TYPE_UINTEGER) {
      QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
      int value = spinBox->value();
      core::NValue val(common::Value::createUIntegerValue(value));
      QVariant var = QVariant::fromValue(val);
      model->setData(index, var, Qt::EditRole);
    } else if (t == common::Value::TYPE_ARRAY) {
      QListWidget* listwidget = static_cast<QListWidget*>(editor);
      common::ArrayValue* ar = common::Value::createArrayValue();
      for (int i = 0; i < listwidget->count(); ++i) {
        std::string val = common::ConvertToString(listwidget->item(i)->text());
        ar->appendString(val);
      }
      QVariant var = QVariant::fromValue(core::NValue(ar));
      model->setData(index, var, Qt::EditRole);
    } else {
      QStyledItemDelegate::setModelData(editor, model, index);
    }
  }

  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex&) const override {
    editor->setGeometry(option.rect);
  }
};

FastoCommonItem* createItem(common::qt::gui::TreeItem* parent,
                            const std::string& key,
                            bool readOnly,
                            FastoObject* item) {
  core::NValue val = item->Value();
  core::NDbKValue nkey(core::NKey(key), val);
  return new FastoCommonItem(nkey, item->Delimiter(), readOnly, parent, item);
}

FastoCommonItem* createRootItem(FastoObject* item) {
  auto value = item->Value();
  std::string str;
  bool res = value->getAsString(&str);
  CHECK(res);
  core::NValue val(common::Value::createStringValue(str));
  core::NDbKValue nkey(core::NKey(std::string()), val);
  return new FastoCommonItem(nkey, item->Delimiter(), true, nullptr, item);
}

}  // namespace

OutputWidget::OutputWidget(core::IServerSPtr server, QWidget* parent)
    : QWidget(parent), server_(server) {
  CHECK(server_);

  commonModel_ = new FastoCommonModel(this);
  VERIFY(connect(commonModel_, &FastoCommonModel::changedValue, this, &OutputWidget::createKey,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::ExecuteStarted, this,
                 &OutputWidget::startExecuteCommand, Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::ExecuteFinished, this,
                 &OutputWidget::finishExecuteCommand, Qt::DirectConnection));

  VERIFY(connect(server_.get(), &core::IServer::KeyAdded, this, &OutputWidget::addKey,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::KeyLoaded, this, &OutputWidget::updateKey,
                 Qt::DirectConnection));

  VERIFY(connect(server_.get(), &core::IServer::RootCreated, this, &OutputWidget::rootCreate,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::RootCompleated, this, &OutputWidget::rootCompleate,
                 Qt::DirectConnection));

  VERIFY(connect(server_.get(), &core::IServer::ChildAdded, this, &OutputWidget::addChild,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &core::IServer::ItemUpdated, this, &OutputWidget::updateItem,
                 Qt::DirectConnection));

  treeView_ = new FastoTreeView;
  treeView_->setModel(commonModel_);
  treeView_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  treeView_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  treeView_->setItemDelegateForColumn(FastoCommonItem::eValue, new TypeDelegate(this));

  tableView_ = new FastoTableView;
  tableView_->setModel(commonModel_);
  tableView_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  tableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  tableView_->setItemDelegateForColumn(FastoCommonItem::eValue, new TypeDelegate(this));

  QString delimiter = common::ConvertFromString<QString>(server_->Delimiter());
  textView_ = new FastoTextView(delimiter);
  textView_->setModel(commonModel_);

  timeLabel_ =
      new common::qt::gui::IconLabel(GuiFactory::instance().timeIcon(), "0", QSize(32, 32));

  QVBoxLayout* mainL = new QVBoxLayout;
  QHBoxLayout* topL = new QHBoxLayout;

  treeButton_ = new QPushButton;
  tableButton_ = new QPushButton;
  textButton_ = new QPushButton;
  treeButton_->setIcon(GuiFactory::instance().treeIcon());
  VERIFY(connect(treeButton_, &QPushButton::clicked, this, &OutputWidget::setTreeView));
  tableButton_->setIcon(GuiFactory::instance().tableIcon());
  VERIFY(connect(tableButton_, &QPushButton::clicked, this, &OutputWidget::setTableView));
  textButton_->setIcon(GuiFactory::instance().textIcon());
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

void OutputWidget::rootCreate(const core::events_info::CommandRootCreatedInfo& res) {
  FastoObject* rootObj = res.root.get();
  fastonosql::gui::FastoCommonItem* root = createRootItem(rootObj);
  commonModel_->setRoot(root);
}

void OutputWidget::rootCompleate(const core::events_info::CommandRootCompleatedInfo& res) {
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

void OutputWidget::startExecuteCommand(const core::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void OutputWidget::finishExecuteCommand(const core::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);
}

void OutputWidget::addChild(FastoObjectIPtr child) {
  DCHECK(child->Parent());

  FastoObjectCommand* command = dynamic_cast<FastoObjectCommand*>(child.get());  // +
  if (command) {
    return;
  }

  command = dynamic_cast<FastoObjectCommand*>(child->Parent());  // +
  if (command) {
    void* parentinner = command->Parent();

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
    core::translator_t tr = server_->Translator();
    std::string inputCmd = command->InputCommand();
    std::string key;
    if (tr->IsLoadKeyCommand(inputCmd, &key)) {
      comChild = createItem(par, key, false, child.get());
    } else {
      comChild = createItem(par, inputCmd, true, child.get());
    }
    commonModel_->insertItem(parent, comChild);
  } else {
    FastoObjectArray* arr = dynamic_cast<FastoObjectArray*>(child->Parent());  // +
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

    fastonosql::gui::FastoCommonItem* comChild = createItem(par, std::string(), true, child.get());
    commonModel_->insertItem(parent, comChild);
  }
}

void OutputWidget::updateItem(FastoObject* item, common::ValueSPtr newValue) {
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
  core::translator_t tran = server_->Translator();
  std::string cmd_text;
  common::Error err = tran->CreateKeyCommand(dbv, &cmd_text);
  if (err && err->isError()) {
    LOG_ERROR(err, true);
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_text, 0, 0, true, true);
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
  supportedViews curV = core::SettingsManager::instance().DefaultView();
  if (curV == Tree) {
    setTreeView();
  } else if (curV == Table) {
    setTableView();
  } else {
    setTextView();
  }
}

void OutputWidget::updateTimeLabel(const core::events_info::EventInfoBase& evinfo) {
  timeLabel_->setText(QString("%1 msec").arg(evinfo.ElapsedTime()));
}

}  // namespace gui
}  // namespace fastonosql
