/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#pragma once

#include <vector>

#include <fastonosql/core/database/idatabase_info.h>

#include "gui/dialogs/base_dialog.h"

#include "proxy/proxy_fwd.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace fastonosql {
namespace proxy {
namespace events_info {
struct ExecuteInfoRequest;
struct ExecuteInfoResponse;
struct LoadDatabaseContentRequest;
struct LoadDatabaseContentResponse;
}  // namespace events_info
}  // namespace proxy
namespace gui {

class KeysTableView;
class ViewKeysDialog : public BaseDialog {
  Q_OBJECT

 public:
  static const QSize kIconSize;
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  enum {
    min_width = 640,
    min_height = 480,
    min_key_on_page = 1,
    max_key_on_page = 100000,
    defaults_key = 100,
    step_keys_on_page = defaults_key
  };

 private Q_SLOTS:
  void startLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentRequest& req);
  void finishLoadDatabaseContent(const proxy::events_info::LoadDatabaseContentResponse& res);

  void startExecute(const proxy::events_info::ExecuteInfoRequest& req);
  void finishExecute(const proxy::events_info::ExecuteInfoResponse& res);
  void keyTTLChange(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl);

  void changeTTL(const core::NDbKValue& value, core::ttl_t ttl);

  void searchLineChanged(const QString& text);
  void leftPageClicked();
  void rightPageClicked();

 protected:
  explicit ViewKeysDialog(const QString& title, proxy::IDatabaseSPtr db, QWidget* parent = Q_NULLPTR);

  void retranslateUi() override;

 private:
  void search(bool forward);
  void updateControls();
  size_t keysCount() const;

  std::vector<uint64_t> cursor_stack_;
  uint32_t cur_pos_;
  QLineEdit* search_box_;
  QLabel* key_count_label_;
  QSpinBox* count_spin_edit_;

  QPushButton* search_button_;
  QPushButton* left_button_list_;
  QSpinBox* current_key_;
  QSpinBox* count_key_;
  QPushButton* right_button_list_;
  KeysTableView* keys_table_;
  proxy::IDatabaseSPtr db_;
};

}  // namespace gui
}  // namespace fastonosql
