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

#include "gui/widgets/user_password_widget.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <common/macros.h>

#include "translations/global.h"

namespace fastonosql {
namespace gui {

UserPasswordWidget::UserPasswordWidget(const QString& userTitle, const QString& passwordTitle, QWidget* parent)
    : QWidget(parent), userTitle_(userTitle), passwordTitle_(passwordTitle) {
  QVBoxLayout* userPasswordLayout = new QVBoxLayout;
  QHBoxLayout* userLayout = new QHBoxLayout;
  userNameLabel_ = new QLabel;
  userNameBox_ = new QLineEdit;
  userLayout->addWidget(userNameLabel_);
  userLayout->addWidget(userNameBox_);
  userPasswordLayout->addLayout(userLayout);

  QHBoxLayout* passwordLayout = new QHBoxLayout;
  passwordLabel_ = new QLabel;
  passwordBox_ = new QLineEdit;
  passwordBox_->setEchoMode(QLineEdit::Password);
  passwordEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passwordEchoModeButton_, &QPushButton::clicked, this, &UserPasswordWidget::togglePasswordEchoMode));
  passwordLayout->addWidget(passwordLabel_);
  passwordLayout->addWidget(passwordBox_);
  passwordLayout->addWidget(passwordEchoModeButton_);
  userPasswordLayout->addLayout(passwordLayout);
  setLayout(userPasswordLayout);

  retranslateUi();
}

QString UserPasswordWidget::userName() const {
  return userNameBox_->text();
}

void UserPasswordWidget::setUserName(const QString& user) const {
  userNameBox_->setText(user);
}

QString UserPasswordWidget::password() const {
  return passwordBox_->text();
}

void UserPasswordWidget::setPassword(const QString& pass) {
  passwordBox_->setText(pass);
}

bool UserPasswordWidget::isValidCredential() const {
  QString uname = userNameBox_->text();
  QString pass = passwordBox_->text();
  return !uname.isEmpty() && !pass.isEmpty();
}

void UserPasswordWidget::togglePasswordEchoMode() {
  bool isPassword = passwordBox_->echoMode() == QLineEdit::Password;
  passwordBox_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  passwordEchoModeButton_->setText(isPassword ? translations::trHide : translations::trShow);
}

void UserPasswordWidget::retranslateUi() {
  userNameLabel_->setText(userTitle_);
  passwordLabel_->setText(passwordTitle_);
}

void UserPasswordWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

}  // namespace gui
}  // namespace fastonosql
