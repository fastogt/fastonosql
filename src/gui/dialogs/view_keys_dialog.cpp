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

#include "gui/dialogs/view_keys_dialog.h"

#include <memory>  // for __shared_ptr

#include <QDialogButtonBox>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

#include "common/error.h"              // for Error
#include "common/log_levels.h"         // for LEVEL_LOG::L_DEBUG
#include "common/logger.h"             // for DEBUG_MSG_FORMAT
#include "common/macros.h"             // for VERIFY, UNUSED, CHECK, etc
#include "common/qt/convert2string.h"  // for ConvertToString
#include "common/value.h"              // for ErrorValue

#include "core/db_key.h"              // for NDbKValue
#include "core/events/events_info.h"  // for CommandResponce, etc
#include "core/idatabase.h"           // for IDatabase
#include "core/iserver.h"             // for IServer
#include "core/types.h"               // for IDataBaseInfoSPtr, etc

#include "gui/fasto_table_view.h"  // for FastoTableView
#include "gui/gui_factory.h"       // for GuiFactory
#include "gui/keys_table_model.h"  // for KeysTableModel, etc

#include "translations/global.h"  // for trKeyCountOnThePage, etc

namespace {

QPushButton* createButtonWithIcon(const QIcon& icon) {
  QPushButton* button = new QPushButton;
  button->setIcon(icon);
  button->setFixedSize(24, 24);
  button->setFlat(true);
  return button;
}

class NumericDelegate : public QStyledItemDelegate {
 public:
  explicit NumericDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const {
    QSpinBox* editor = new QSpinBox(parent);
    editor->setRange(-1, INT32_MAX);
    editor->setValue(-1);
    return editor;
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const {
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
  }

  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
  }

  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex&) const {
    editor->setGeometry(option.rect);
  }
};

}  // namespace

namespace fastonosql {
namespace gui {

ViewKeysDialog::ViewKeysDialog(const QString& title, core::IDatabaseSPtr db, QWidget* parent)
    : QDialog(parent), cursorStack_(), curPos_(0), db_(db) {
  CHECK(db_);
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  core::IServerSPtr serv = db_->server();
  VERIFY(connect(serv.get(), &core::IServer::startedLoadDataBaseContent, this,
                 &ViewKeysDialog::startLoadDatabaseContent));
  VERIFY(connect(serv.get(), &core::IServer::finishedLoadDatabaseContent, this,
                 &ViewKeysDialog::finishLoadDatabaseContent));

  // main layout
  QVBoxLayout* mainlayout = new QVBoxLayout;

  QHBoxLayout* searchLayout = new QHBoxLayout;
  searchBox_ = new QLineEdit;
  searchBox_->setText("*");
  VERIFY(connect(searchBox_, &QLineEdit::textChanged, this, &ViewKeysDialog::searchLineChanged));
  searchLayout->addWidget(searchBox_);

  countSpinEdit_ = new QSpinBox;
  countSpinEdit_->setRange(min_key_on_page, max_key_on_page);
  countSpinEdit_->setSingleStep(step_keys_on_page);
  countSpinEdit_->setValue(defaults_key);

  keyCountLabel_ = new QLabel;

  searchLayout->addWidget(keyCountLabel_);
  searchLayout->addWidget(countSpinEdit_);

  searchButton_ = new QPushButton;
  VERIFY(connect(searchButton_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));
  searchLayout->addWidget(searchButton_);

  keysModel_ = new KeysTableModel(this);
  VERIFY(connect(keysModel_, &KeysTableModel::changedTTL, this, &ViewKeysDialog::changeTTL,
                 Qt::DirectConnection));

  VERIFY(connect(serv.get(), &core::IServer::startedExecute, this, &ViewKeysDialog::startExecute,
                 Qt::DirectConnection));
  VERIFY(connect(serv.get(), &core::IServer::finishedExecute, this, &ViewKeysDialog::finishExecute,
                 Qt::DirectConnection));
  VERIFY(connect(serv.get(), &core::IServer::keyTTLChanged, this, &ViewKeysDialog::keyTTLChange,
                 Qt::DirectConnection));

  keysTable_ = new FastoTableView;
  keysTable_->setModel(keysModel_);
  keysTable_->setItemDelegateForColumn(KeyTableItem::kTTL, new NumericDelegate(this));

  QDialogButtonBox* buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ViewKeysDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ViewKeysDialog::reject));
  mainlayout->addLayout(searchLayout);
  mainlayout->addWidget(keysTable_);

  leftButtonList_ = createButtonWithIcon(GuiFactory::instance().leftIcon());
  rightButtonList_ = createButtonWithIcon(GuiFactory::instance().rightIcon());
  VERIFY(connect(leftButtonList_, &QPushButton::clicked, this, &ViewKeysDialog::leftPageClicked));
  VERIFY(connect(rightButtonList_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));
  QHBoxLayout* pagingLayout = new QHBoxLayout;
  pagingLayout->addWidget(leftButtonList_);
  core::IDataBaseInfoSPtr inf = db_->info();
  size_t keysCount = inf->dbKeysCount();
  currentKey_ = new QSpinBox;
  currentKey_->setEnabled(false);
  currentKey_->setValue(0);
  currentKey_->setMinimum(0);
  currentKey_->setMaximum(keysCount);
  countKey_ = new QSpinBox;
  countKey_->setEnabled(false);
  countKey_->setValue(keysCount);
  pagingLayout->addWidget(new QSplitter(Qt::Horizontal));
  pagingLayout->addWidget(currentKey_);
  pagingLayout->addWidget(countKey_);
  pagingLayout->addWidget(new QSplitter(Qt::Horizontal));
  pagingLayout->addWidget(rightButtonList_);

  mainlayout->addLayout(pagingLayout);
  mainlayout->addWidget(buttonBox);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainlayout);

  updateControls();
  retranslateUi();
}

void ViewKeysDialog::startLoadDatabaseContent(
    const core::events_info::LoadDatabaseContentRequest& req) {
  UNUSED(req);

  keysModel_->clear();
}

void ViewKeysDialog::finishLoadDatabaseContent(
    const core::events_info::LoadDatabaseContentResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  if (!keysModel_) {
    return;
  }

  core::events_info::LoadDatabaseContentResponce::keys_container_t keys = res.keys;

  size_t size = keys.size();
  for (size_t i = 0; i < size; ++i) {
    core::NDbKValue key = keys[i];
    keysModel_->insertItem(new KeyTableItem(key));
  }

  int curv = currentKey_->value();
  if (cursorStack_.size() == curPos_) {
    cursorStack_.push_back(res.cursor_out);
    currentKey_->setValue(curv + size);
  } else {
    currentKey_->setValue(curv - size);
  }

  updateControls();
}

void ViewKeysDialog::changeTTL(const core::NDbKValue& value, core::ttl_t ttl) {
  core::translator_t tran = db_->translator();
  std::string cmd_str;
  common::Error err = tran->changeKeyTTLCommand(value.key(), ttl, &cmd_str);
  if (err && err->isError()) {
    return;
  }

  core::events_info::ExecuteInfoRequest req(this, cmd_str);
  db_->execute(req);
}

void ViewKeysDialog::startExecute(const core::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void ViewKeysDialog::finishExecute(const core::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);
}

void ViewKeysDialog::keyTTLChange(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl) {
  UNUSED(db);
  core::NKey new_key = key;
  new_key.setTTL(ttl);
  keysModel_->updateKey(new_key);
}

void ViewKeysDialog::search(bool forward) {
  QString pattern = searchBox_->text();
  if (pattern.isEmpty()) {
    return;
  }

  if (cursorStack_.empty()) {
    cursorStack_.push_back(0);
  }

  DCHECK_EQ(cursorStack_[0], 0);
  if (forward) {
    core::events_info::LoadDatabaseContentRequest req(
        this, db_->info(), common::ConvertToString(pattern), countSpinEdit_->value(),
        cursorStack_[curPos_]);
    db_->loadContent(req);
    ++curPos_;
  } else {
    if (curPos_ > 0) {
      core::events_info::LoadDatabaseContentRequest req(
          this, db_->info(), common::ConvertToString(pattern), countSpinEdit_->value(),
          cursorStack_[--curPos_]);
      db_->loadContent(req);
    }
  }
}

void ViewKeysDialog::searchLineChanged(const QString& text) {
  UNUSED(text);

  cursorStack_.clear();
  curPos_ = 0;
  currentKey_->setValue(0);
  updateControls();
}

void ViewKeysDialog::leftPageClicked() {
  search(false);
}

void ViewKeysDialog::rightPageClicked() {
  search(true);
}

void ViewKeysDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void ViewKeysDialog::retranslateUi() {
  keyCountLabel_->setText(translations::trKeyCountOnThePage);
  searchButton_->setText(translations::trSearch);
}

void ViewKeysDialog::updateControls() {
  bool isEmptyDb = keysCount() == 0;
  bool isEndSearch = curPos_ ? (cursorStack_[curPos_] == 0) : false;

  leftButtonList_->setEnabled(curPos_ > 0);
  rightButtonList_->setEnabled(!isEmptyDb && !isEndSearch);
  searchButton_->setEnabled(!isEmptyDb && !isEndSearch);
}

size_t ViewKeysDialog::keysCount() const {
  return countKey_->value();
}

}  // namespace gui
}  // namespace fastonosql
