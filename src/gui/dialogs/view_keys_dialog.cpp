/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/view_keys_dialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>
#include <common/qt/logger.h>

#include "proxy/database/idatabase.h"
#include "proxy/server/iserver.h"

#include "gui/gui_factory.h"
#include "gui/views/keys_table_view.h"
#include "gui/widgets/icon_button.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

const QSize ViewKeysDialog::kIconSize = QSize(24, 24);

ViewKeysDialog::ViewKeysDialog(const QString& title, proxy::IDatabaseSPtr db, QWidget* parent)
    : base_class(title, parent),
      cursor_stack_(),
      cur_pos_(0),
      search_box_(nullptr),
      key_count_label_(nullptr),
      count_spin_edit_(nullptr),
      search_button_(nullptr),
      left_button_list_(nullptr),
      current_key_(nullptr),
      count_key_(nullptr),
      right_button_list_(nullptr),
      keys_table_(nullptr),
      db_(db) {
  CHECK(db_) << "Must be database.";

  proxy::IServerSPtr serv = db_->GetServer();
  VERIFY(connect(serv.get(), &proxy::IServer::LoadDataBaseContentStarted, this,
                 &ViewKeysDialog::startLoadDatabaseContent));
  VERIFY(connect(serv.get(), &proxy::IServer::LoadDatabaseContentFinished, this,
                 &ViewKeysDialog::finishLoadDatabaseContent));

  QHBoxLayout* search_layout = new QHBoxLayout;
  search_box_ = new QLineEdit;
  search_box_->setText(ALL_KEYS_PATTERNS);
  VERIFY(connect(search_box_, &QLineEdit::textChanged, this, &ViewKeysDialog::searchLineChanged));
  search_layout->addWidget(search_box_);

  count_spin_edit_ = new QSpinBox;
  count_spin_edit_->setRange(min_key_on_page, max_key_on_page);
  count_spin_edit_->setSingleStep(step_keys_on_page);
  count_spin_edit_->setValue(defaults_key);

  key_count_label_ = new QLabel;

  search_layout->addWidget(key_count_label_);
  search_layout->addWidget(count_spin_edit_);

  search_button_ = new QPushButton;
  VERIFY(connect(search_button_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));
  search_layout->addWidget(search_button_);

  VERIFY(
      connect(serv.get(), &proxy::IServer::ExecuteStarted, this, &ViewKeysDialog::startExecute, Qt::DirectConnection));
  VERIFY(connect(serv.get(), &proxy::IServer::ExecuteFinished, this, &ViewKeysDialog::finishExecute,
                 Qt::DirectConnection));
  VERIFY(
      connect(serv.get(), &proxy::IServer::KeyTTLChanged, this, &ViewKeysDialog::keyTTLChange, Qt::DirectConnection));

  keys_table_ = new KeysTableView;
  VERIFY(connect(keys_table_, &KeysTableView::changedTTL, this, &ViewKeysDialog::changeTTL, Qt::DirectConnection));

  left_button_list_ = new IconButton(GuiFactory::GetInstance().leftIcon(), kIconSize);
  right_button_list_ = new IconButton(GuiFactory::GetInstance().rightIcon(), kIconSize);
  VERIFY(connect(left_button_list_, &QPushButton::clicked, this, &ViewKeysDialog::leftPageClicked));
  VERIFY(connect(right_button_list_, &QPushButton::clicked, this, &ViewKeysDialog::rightPageClicked));

  QHBoxLayout* paging_layout = new QHBoxLayout;
  paging_layout->addWidget(left_button_list_);
  core::IDataBaseInfoSPtr inf = db_->GetInfo();
  size_t keysCount = inf->GetDBKeysCount();
  current_key_ = new QSpinBox;
  current_key_->setEnabled(false);
  current_key_->setValue(0);
  current_key_->setMinimum(0);
  current_key_->setMaximum(keysCount);
  count_key_ = new QSpinBox;
  count_key_->setEnabled(false);
  count_key_->setValue(keysCount);
  paging_layout->addWidget(new QSplitter(Qt::Horizontal));
  paging_layout->addWidget(current_key_);
  paging_layout->addWidget(count_key_);
  paging_layout->addWidget(new QSplitter(Qt::Horizontal));
  paging_layout->addWidget(right_button_list_);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &ViewKeysDialog::accept));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &ViewKeysDialog::reject));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addLayout(search_layout);
  main_layout->addWidget(keys_table_);
  main_layout->addLayout(paging_layout);
  main_layout->addWidget(button_box);
  setLayout(main_layout);
  setMinimumSize(QSize(min_width, min_height));

  updateControls();
}

void ViewKeysDialog::startLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentRequest& req) {
  UNUSED(req);

  keys_table_->clearItems();
}

void ViewKeysDialog::finishLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentResponse& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  proxy::events_info::LoadDatabaseContentResponse::keys_container_t keys = res.keys;

  size_t size = keys.size();
  for (size_t i = 0; i < size; ++i) {
    core::NDbKValue key = keys[i];
    keys_table_->insertKey(key);
  }

  int curv = current_key_->value();
  if (cursor_stack_.size() == cur_pos_) {
    cursor_stack_.push_back(res.cursor_out);
    current_key_->setValue(curv + size);
  } else {
    current_key_->setValue(curv - size);
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

void ViewKeysDialog::finishExecute(const proxy::events_info::ExecuteInfoResponse& res) {
  UNUSED(res);
}

void ViewKeysDialog::keyTTLChange(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl) {
  UNUSED(db);
  core::NKey new_key = key;
  new_key.SetTTL(ttl);
  keys_table_->updateKey(new_key);
}

void ViewKeysDialog::search(bool forward) {
  QString pattern = search_box_->text();
  if (pattern.isEmpty()) {
    return;
  }

  if (cursor_stack_.empty()) {
    cursor_stack_.push_back(0);
  }

  DCHECK_EQ(cursor_stack_[0], 0);
  if (forward) {
    proxy::events_info::LoadDatabaseContentRequest req(this, db_->GetInfo(), common::ConvertToString(pattern),
                                                       count_spin_edit_->value(), cursor_stack_[cur_pos_]);
    db_->LoadContent(req);
    ++cur_pos_;
  } else {
    if (cur_pos_ > 0) {
      proxy::events_info::LoadDatabaseContentRequest req(this, db_->GetInfo(), common::ConvertToString(pattern),
                                                         count_spin_edit_->value(), cursor_stack_[--cur_pos_]);
      db_->LoadContent(req);
    }
  }
}

void ViewKeysDialog::searchLineChanged(const QString& text) {
  UNUSED(text);

  cursor_stack_.clear();
  cur_pos_ = 0;
  current_key_->setValue(0);
  updateControls();
}

void ViewKeysDialog::leftPageClicked() {
  search(false);
}

void ViewKeysDialog::rightPageClicked() {
  search(true);
}

void ViewKeysDialog::retranslateUi() {
  key_count_label_->setText(translations::trKeyCountOnThePage);
  search_button_->setText(translations::trSearch);
  base_class::retranslateUi();
}

void ViewKeysDialog::updateControls() {
  bool isEmptyDb = keysCount() == 0;
  bool isEndSearch = cur_pos_ ? (cursor_stack_[cur_pos_] == 0) : false;

  left_button_list_->setEnabled(cur_pos_ > 0);
  right_button_list_->setEnabled(!isEmptyDb && !isEndSearch);
  search_button_->setEnabled(!isEmptyDb && !isEndSearch);
}

size_t ViewKeysDialog::keysCount() const {
  return count_key_->value();
}

}  // namespace gui
}  // namespace fastonosql
