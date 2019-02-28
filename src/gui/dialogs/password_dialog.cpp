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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/password_dialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include <common/macros.h>

#include <common/qt/gui/icon_label.h>

#include "translations/global.h"

namespace {
const QString trSignIn = QObject::tr("Sign in");
const QString trSavePassowrd = QObject::tr("Save password");
}  // namespace

namespace fastonosql {
namespace gui {

PasswordDialog::PasswordDialog(const QString& title, QWidget* parent)
    : base_class(title, parent),
      description_(nullptr),
      login_label_(nullptr),
      login_box_(nullptr),
      password_label_(nullptr),
      password_box_(nullptr),
      save_password_(nullptr),
      password_echo_mode_button_(nullptr) {
  description_ = new QLabel;
  description_->setOpenExternalLinks(true);

  QHBoxLayout* profile_layout = new QHBoxLayout;
  login_label_ = new QLabel;
  login_box_ = new QLineEdit;
  profile_layout->addWidget(login_label_);
  profile_layout->addWidget(login_box_);

  QVBoxLayout* password_layout = new QVBoxLayout;
  QHBoxLayout* password_box_layout = new QHBoxLayout;
  password_label_ = new QLabel;
  password_box_ = new QLineEdit;
  password_box_->setEchoMode(QLineEdit::Password);
  password_echo_mode_button_ = new QPushButton;
  VERIFY(connect(password_echo_mode_button_, &QPushButton::clicked, this, &PasswordDialog::togglePasswordEchoMode));
  password_box_layout->addWidget(password_label_);
  password_box_layout->addWidget(password_box_);
  password_box_layout->addWidget(password_echo_mode_button_);

  save_password_ = new QCheckBox;
  QHBoxLayout* save_password_layout = new QHBoxLayout;
  save_password_layout->addWidget(new QSplitter(Qt::Horizontal));
  save_password_layout->addWidget(save_password_);
  password_layout->addLayout(password_box_layout);
  password_layout->addLayout(save_password_layout);

  status_label_ = new common::qt::gui::IconLabel;
  status_label_->setOpenExternalLinks(true);
  status_label_->setVisible(false);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &PasswordDialog::accept));
  button_box->button(QDialogButtonBox::Ok)->setText(trSignIn);

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(description_);
  main_layout->addLayout(profile_layout);
  main_layout->addLayout(password_layout);
  main_layout->addWidget(status_label_, 0, Qt::AlignCenter);
  main_layout->addWidget(button_box);
  main_layout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(main_layout);
}

QString PasswordDialog::login() const {
  return login_box_->text();
}

void PasswordDialog::setLogin(const QString& login) {
  login_box_->setText(login);
}

QString PasswordDialog::password() const {
  return password_box_->text();
}

void PasswordDialog::setPassword(const QString& password) {
  password_box_->setText(password);
}

void PasswordDialog::setLoginEnabled(bool en) {
  login_box_->setEnabled(en);
}

void PasswordDialog::setDescription(const QString& description) {
  description_->setText(description);
}

QString PasswordDialog::description() const {
  return description_->text();
}

void PasswordDialog::setVisibleDescription(bool visible) {
  return description_->setVisible(visible);
}

bool PasswordDialog::isVisibleDescription() const {
  return description_->isVisible();
}

bool PasswordDialog::isVisibleStatus() const {
  return description_->isVisible();
}

bool PasswordDialog::isSavePassword() const {
  return save_password_->isChecked();
}

void PasswordDialog::setSavePassword(bool save) {
  save_password_->setChecked(save);
}

void PasswordDialog::setFocusInPassword() {
  password_box_->setFocus();
}

void PasswordDialog::setFocusInLogin() {
  login_box_->setFocus();
}

void PasswordDialog::accept() {
  const QString& pass = password();
  if (pass.isEmpty()) {
    password_box_->setFocus();
    return;
  }

  base_class::accept();
}

void PasswordDialog::setVisibleStatus(bool visible) {
  status_label_->setVisible(visible);
}

void PasswordDialog::setStatusIcon(const QIcon& icon, const QSize& icon_size) {
  status_label_->setIcon(icon, icon_size);
}

void PasswordDialog::setStatus(const QString& status) {
  status_label_->setText(status);
}

void PasswordDialog::togglePasswordEchoMode() {
  bool isPassword = password_box_->echoMode() == QLineEdit::Password;
  password_box_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  syncShowButton();
}

void PasswordDialog::syncShowButton() {
  bool isPassword = password_box_->echoMode() == QLineEdit::Password;
  password_echo_mode_button_->setText(isPassword ? translations::trShow : translations::trHide);
}

void PasswordDialog::retranslateUi() {
  login_label_->setText(translations::trLogin + ":");
  login_box_->setToolTip(translations::trPleaseEnterYourLogin);
  password_label_->setText(translations::trPassword + ":");
  password_box_->setToolTip(translations::trPleaseEnterYourPassword);
  save_password_->setText(trSavePassowrd);

  syncShowButton();
  base_class::retranslateUi();
}

}  // namespace gui
}  // namespace fastonosql
