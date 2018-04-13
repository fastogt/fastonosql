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

#pragma once

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;

namespace fastonosql {
namespace gui {

class UserPasswordWidget : public QWidget {
  Q_OBJECT
 public:
  UserPasswordWidget(const QString& user_title, const QString& password_title, QWidget* parent = Q_NULLPTR);

  QString userName() const;
  void setUserName(const QString& user) const;

  QString password() const;
  void setPassword(const QString& pass);

  bool isValidCredential() const;

 private Q_SLOTS:
  void togglePasswordEchoMode();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  QLabel* user_name_label_;
  QLineEdit* user_name_textbox_;

  QLabel* password_label_;
  QLineEdit* password_textbox_;
  QPushButton* password_echomode_button_;

  QString user_title_;
  QString password_title_;
};

}  // namespace gui
}  // namespace fastonosql
