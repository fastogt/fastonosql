/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include "gui/dialogs/base_dialog.h"

class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;

namespace common {
namespace qt {
namespace gui {
class IconLabel;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace gui {

class PasswordDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  QString login() const;
  void setLogin(const QString& login);

  QString password() const;
  void setPassword(const QString& password);

  void setLoginEnabled(bool en);

  void setDescription(const QString& description);
  QString description() const;

  bool isVisibleDescription() const;
  bool isVisibleStatus() const;
  bool isSavePassword() const;

  void setSavePassword(bool save);

  void setFocusInPassword();
  void setFocusInLogin();

 public Q_SLOTS:
  void accept() override;

  void setVisibleDescription(bool visible);
  void setVisibleStatus(bool visible);
  void setStatusIcon(const QIcon& icon, const QSize& icon_size);
  void setStatus(const QString& status);

 private Q_SLOTS:
  void togglePasswordEchoMode();

 protected:
  explicit PasswordDialog(const QString& title, QWidget* parent = Q_NULLPTR);

  void retranslateUi() override;

 private:
  void syncShowButton();

 private:
  QLabel* description_;

  QLabel* login_label_;
  QLineEdit* login_box_;

  QLabel* password_label_;
  QLineEdit* password_box_;
  QCheckBox* save_password_;
  QPushButton* password_echo_mode_button_;

  common::qt::gui::IconLabel* status_label_;
};

}  // namespace gui
}  // namespace fastonosql
