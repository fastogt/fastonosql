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

#include "gui/dialogs/password_dialog.h"

#include <QDialogButtonBox>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <common/macros.h>

#include <common/qt/gui/icon_label.h>  // for IconLabel

#include "translations/global.h"

namespace {
const QString trSignIn = QObject::tr("Sign in");
}

namespace fastonosql {
namespace gui {

PasswordDialog::PasswordDialog(QWidget* parent) : PasswordDialog(QString(), parent) {}

PasswordDialog::PasswordDialog(const QString& description, QWidget* parent)
    : QDialog(parent),
      description_(nullptr),
      login_label_(nullptr),
      login_box_(nullptr),
      password_label_(nullptr),
      password_box_(nullptr),
      password_echo_mode_button_(nullptr) {
  Qt::WindowFlags flags = windowFlags();
  setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &PasswordDialog::accept));
  buttonBox->button(QDialogButtonBox::Ok)->setText(trSignIn);

  description_ = new QLabel;
  description_->setText(description);
  description_->setOpenExternalLinks(true);

  QHBoxLayout* profile_layout = new QHBoxLayout;
  login_label_ = new QLabel;
  login_box_ = new QLineEdit;
  profile_layout->addWidget(login_label_);
  profile_layout->addWidget(login_box_);

  QHBoxLayout* password_layout = new QHBoxLayout;
  password_label_ = new QLabel;
  password_box_ = new QLineEdit;
  password_box_->setEchoMode(QLineEdit::Password);
  password_echo_mode_button_ = new QPushButton;
  VERIFY(connect(password_echo_mode_button_, &QPushButton::clicked, this, &PasswordDialog::togglePasswordEchoMode));
  password_layout->addWidget(password_label_);
  password_layout->addWidget(password_box_);
  password_layout->addWidget(password_echo_mode_button_);

  status_label_ = new common::qt::gui::IconLabel;
  status_label_->setOpenExternalLinks(true);
  status_label_->setVisible(false);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(description_);
  mainLayout->addLayout(profile_layout);
  mainLayout->addLayout(password_layout);
  mainLayout->addWidget(status_label_, 0, Qt::AlignCenter);
  mainLayout->addWidget(buttonBox);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(mainLayout);
  retranslateUi();
}

QString PasswordDialog::GetLogin() const {
  return login_box_->text();
}

void PasswordDialog::SetLogin(const QString& login) {
  login_box_->setText(login);
}

QString PasswordDialog::GetPassword() const {
  return password_box_->text();
}

void PasswordDialog::SetPassword(const QString& password) {
  password_box_->setText(password);
}

void PasswordDialog::SetLoginEnabled(bool en) {
  login_box_->setEnabled(en);
}

void PasswordDialog::SetDescription(const QString& description) {
  description_->setText(description);
}

QString PasswordDialog::GetDescription() const {
  return description_->text();
}

void PasswordDialog::SetVisibleDescription(bool visible) {
  return description_->setVisible(visible);
}

bool PasswordDialog::IsVisibleDescription() const {
  return description_->isVisible();
}

bool PasswordDialog::IsVisibleStatus() const {
  return description_->isVisible();
}

void PasswordDialog::SetFocusInPassword() {
  password_box_->setFocus();
}

void PasswordDialog::SetFocusInLogin() {
  login_box_->setFocus();
}

void PasswordDialog::accept() {
  const QString pass = GetPassword();
  if (pass.isEmpty()) {
    password_box_->setFocus();
    return;
  }

  QDialog::accept();
}

void PasswordDialog::SetVisibleStatus(bool visible) {
  status_label_->setVisible(visible);
}

void PasswordDialog::SetStatusIcon(const QIcon& icon, const QSize& icon_size) {
  status_label_->setIcon(icon, icon_size);
}

void PasswordDialog::SetStatus(const QString& status) {
  status_label_->setText(status);
}

void PasswordDialog::togglePasswordEchoMode() {
  bool isPassword = password_box_->echoMode() == QLineEdit::Password;
  password_box_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  syncShowButton();
}

void PasswordDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QDialog::changeEvent(e);
}

void PasswordDialog::syncShowButton() {
  bool isPassword = password_box_->echoMode() == QLineEdit::Password;
  password_echo_mode_button_->setText(isPassword ? translations::trShow : translations::trHide);
}

void PasswordDialog::retranslateUi() {
  password_label_->setText(translations::trPassword + ":");
  login_label_->setText(translations::trLogin + ":");
  syncShowButton();
}

}  // namespace gui
}  // namespace fastonosql
