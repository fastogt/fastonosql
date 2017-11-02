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

#include "gui/dialogs/view_keys_dialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/qt/logger.h>          // for LOG_ERROR

#include "proxy/database/idatabase.h"  // for IDatabase
#include "proxy/server/iserver.h"      // for IServer

#include "gui/gui_factory.h"  // for GuiFactory
#include "gui/keys_table_view.h"

#include "translations/global.h"  // for trKeyCountOnThePage, etc

namespace {
QPushButton* createButtonWithIcon(const QIcon& icon) {
  QPushButton* button = new QPushButton;
  button->setIcon(icon);
  button->setFixedSize(24, 24);
  button->setFlat(true);
  return button;
}

}  // namespace

namespace fastonosql {
namespace gui {

ViewKeysDialog::ViewKeysDialog(const QString& title, proxy::IDatabaseSPtr db, QWidget* parent)
    : QDialog(parent), cursorStack_(), curPos_(0), db_(db) {
  CHECK(db_);
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  proxy::IServerSPtr serv = db_->GetServer();
  VERIFY(connect(serv.get(), &proxy::IServer::LoadDataBaseContentStarted, this,
                 &ViewKeysDialog::startLoadDatabaseContent));
  VERIFY(connect(serv.get(), &proxy::IServer::LoadDatabaseContentFinished, this,
                 &ViewKeysDialog::finishLoadDatabaseContent));

  // main layout
  QVBoxLayout* mainlayout = new QVBoxLayout;

  QHBoxLayout* searchLayout = new QHBoxLayout;
  searchBox_ = new QLineEdit;
  searchBox_->setText(ALL_KEYS_PATTERNS);
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

  VERIFY(
      connect(serv.get(), &proxy::IServer::ExecuteStarted, this, &ViewKeysDialog::startExecute, Qt::DirectConnection));
  VERIFY(connect(serv.get(), &proxy::IServer::ExecuteFinished, this, &ViewKeysDialog::finishExecute,
                 Qt::DirectConnection));
  VERIFY(
      connect(serv.get(), &proxy::IServer::KeyTTLChanged, this, &ViewKeysDialog::keyTTLChange, Qt::DirectConnection));

  keysTable_ = new KeysTableView;
  VERIFY(connect(keysTable_, &KeysTableView::changedTTL, this, &ViewKeysDialog::changeTTL, Qt::DirectConnection));

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ViewKeysDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ViewKeysDialog::reject));
  mainlayout->addLayout(searchLayout);
  mainlayout->addWidget(keysTable_);

  leftButtonList_ = createButtonWithIcon(GuiFactory::GetInstance().leftIcon());
  rightButtonList_ = createButtonWithIcon(GuiFactory::GetInstance().rightIcon());
  VERIFY(connect(leftButtonList_, &QPushButton::clicked, this, &ViewKeysDialog::leftPageClicked));
  VERIFY(connect(rightButtonList_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));
  QHBoxLayout* pagingLayout = new QHBoxLayout;
  pagingLayout->addWidget(leftButtonList_);
  core::IDataBaseInfoSPtr inf = db_->GetInfo();
  size_t keysCount = inf->GetDBKeysCount();
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

void ViewKeysDialog::startLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentRequest& req) {
  UNUSED(req);

  keysTable_->clearItems();
}

void ViewKeysDialog::finishLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentResponce& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  proxy::events_info::LoadDatabaseContentResponce::keys_container_t keys = res.keys;

  size_t size = keys.size();
  for (size_t i = 0; i < size; ++i) {
    core::NDbKValue key = keys[i];
    keysTable_->insertKey(key);
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
  proxy::IServerSPtr server = db_->GetServer();
  core::translator_t tran = server->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = tran->ChangeKeyTTLCommand(value.GetKey(), ttl, &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
  db_->Execute(req);
}

void ViewKeysDialog::startExecute(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void ViewKeysDialog::finishExecute(const proxy::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);
}

void ViewKeysDialog::keyTTLChange(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl) {
  UNUSED(db);
  core::NKey new_key = key;
  new_key.SetTTL(ttl);
  keysTable_->updateKey(new_key);
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
    proxy::events_info::LoadDatabaseContentRequest req(this, db_->GetInfo(), common::ConvertToString(pattern),
                                                       countSpinEdit_->value(), cursorStack_[curPos_]);
    db_->LoadContent(req);
    ++curPos_;
  } else {
    if (curPos_ > 0) {
      proxy::events_info::LoadDatabaseContentRequest req(this, db_->GetInfo(), common::ConvertToString(pattern),
                                                         countSpinEdit_->value(), cursorStack_[--curPos_]);
      db_->LoadContent(req);
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
