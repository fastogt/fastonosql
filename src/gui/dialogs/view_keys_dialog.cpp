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

#include <QDialogButtonBox>
#include <QEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSpinBox>
#include <QSplitter>
#include <QStyledItemDelegate>

#include "common/qt/convert_string.h"
#include "common/logger.h"

#include "core/iserver.h"
#include "core/idatabase.h"

#include "translations/global.h"

#include "gui/keys_table_model.h"
#include "gui/fasto_table_view.h"
#include "gui/gui_factory.h"

namespace {

QPushButton* createButtonWithIcon(const QIcon& icon) {
  QPushButton* button = new QPushButton;
  button->setIcon(icon);
  button->setFixedSize(24, 24);
  button->setFlat(true);
  return button;
}

class NumericDelegate
  : public QStyledItemDelegate {
 public:
  explicit NumericDelegate(QObject* parent = 0)
  : QStyledItemDelegate(parent) {
}

QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& , const QModelIndex& ) const {
  QSpinBox* editor = new QSpinBox(parent);
  editor->setRange(-1, INT32_MAX);
  editor->setValue(-1);
  return editor;
}

void setEditorData(QWidget* editor, const QModelIndex &index) const {
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

void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                          const QModelIndex& ) const {
  editor->setGeometry(option.rect);
}
};

}  // namespace

namespace fastonosql {
namespace gui {

ViewKeysDialog::ViewKeysDialog(const QString& title, core::IDatabaseSPtr db, QWidget* parent)
  : QDialog(parent), cursorStack_(), curPos_(0), db_(db) {
  CHECK(db_);
  core::IServerSPtr serv = db_->server();
  VERIFY(connect(serv.get(), &core::IServer::startedLoadDataBaseContent,
                 this, &ViewKeysDialog::startLoadDatabaseContent));
  VERIFY(connect(serv.get(), &core::IServer::finishedLoadDatabaseContent,
                 this, &ViewKeysDialog::finishLoadDatabaseContent));

  setWindowTitle(title);

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
  VERIFY(connect(searchButton_, &QPushButton::clicked,
                 this, &ViewKeysDialog::rightPageClicked));
  searchLayout->addWidget(searchButton_);

  keysModel_ = new KeysTableModel(this);
  VERIFY(connect(keysModel_, &KeysTableModel::changedValue,
                 this, &ViewKeysDialog::executeCommand, Qt::DirectConnection));

  VERIFY(connect(serv.get(), &core::IServer::startedExecuteCommand,
                 this, &ViewKeysDialog::startExecuteCommand, Qt::DirectConnection));
  VERIFY(connect(serv.get(), &core::IServer::finishedExecuteCommand,
                 this, &ViewKeysDialog::finishExecuteCommand, Qt::DirectConnection));
  keysTable_ = new FastoTableView;
  keysTable_->setModel(keysModel_);
  keysTable_->setItemDelegateForColumn(KeyTableItem::kTTL, new NumericDelegate(this));

  QDialogButtonBox* buttonBox = new QDialogButtonBox;
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
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

void ViewKeysDialog::startLoadDatabaseContent(const core::events_info::LoadDatabaseContentRequest& req) {
  UNUSED(req);

  keysModel_->clear();
}

void ViewKeysDialog::finishLoadDatabaseContent(const core::events_info::LoadDatabaseContentResponce& res) {
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

void ViewKeysDialog::executeCommand(core::CommandKeySPtr cmd) {
  core::events_info::CommandRequest req(this, db_->info(), cmd);
  db_->executeCommand(req);
}

void ViewKeysDialog::startExecuteCommand(const core::events_info::CommandRequest& req) {
  UNUSED(req);
}

void ViewKeysDialog::finishExecuteCommand(const core::events_info::CommandResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  if (res.initiator() != this) {
    DEBUG_MSG_FORMAT<512>(common::logging::L_DEBUG,
                          "Skipped event in file: %s, function: %s", __FILE__, __FUNCTION__);
    return;
  }

  core::CommandKeySPtr key = res.cmd;
  if (key->type() == core::CommandKey::C_CHANGE_TTL) {
    core::CommandChangeTTL* cttl = static_cast<core::CommandChangeTTL*>(key.get());
    core::NDbKValue dbv = cttl->newKey();
    keysModel_->changeValue(dbv);
  }
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
    core::events_info::LoadDatabaseContentRequest req(this, db_->info(),
                                               common::ConvertToString(pattern),
                                               countSpinEdit_->value(), cursorStack_[curPos_]);
    db_->loadContent(req);
    ++curPos_;
  } else {
    if (curPos_ > 0) {
      core::events_info::LoadDatabaseContentRequest req(this, db_->info(),
                                                 common::ConvertToString(pattern),
                                                 countSpinEdit_->value(), cursorStack_[--curPos_]);
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
