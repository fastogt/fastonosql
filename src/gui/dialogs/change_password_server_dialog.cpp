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

#include "gui/dialogs/change_password_server_dialog.h"

#include <memory>  // for __shared_ptr
#include <string>  // for string

#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>

#include <common/error.h>                // for Error
#include <common/macros.h>               // for VERIFY, CHECK, UNUSED
#include <common/qt/convert2string.h>    // for ConvertToString
#include <common/qt/gui/glass_widget.h>  // for GlassWidget
#include <common/qt/logger.h>
#include <common/value.h>  // for ErrorValue

#include "proxy/events/events_info.h"  // for ChangePasswordResponce, etc
#include "proxy/server/iserver.h"      // for IServer

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trError, trInfo, etc

namespace {
const QString trPassword = QObject::tr("Password:");
const QString trCPassword = QObject::tr("Confirm Password:");
const QString trInvalidInput = QObject::tr("Invalid input!");
const QString trPasswordChS = QObject::tr("Password successfully changed!");
}  // namespace

namespace fastonosql {
namespace gui {

ChangePasswordServerDialog::ChangePasswordServerDialog(const QString& title, proxy::IServerSPtr server, QWidget* parent)
    : QDialog(parent), server_(server) {
  CHECK(server_);

  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  QHBoxLayout* passLayout = new QHBoxLayout;
  passLayout->addWidget(new QLabel(trPassword));
  passwordLineEdit_ = new QLineEdit;
  passwordLineEdit_->setEchoMode(QLineEdit::Password);
  passLayout->addWidget(passwordLineEdit_);
  mainLayout->addLayout(passLayout);

  QHBoxLayout* cpassLayout = new QHBoxLayout;
  cpassLayout->addWidget(new QLabel(trCPassword));
  confPasswordLineEdit_ = new QLineEdit;
  confPasswordLineEdit_->setEchoMode(QLineEdit::Password);
  cpassLayout->addWidget(confPasswordLineEdit_);
  mainLayout->addLayout(cpassLayout);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ChangePasswordServerDialog::tryToCreatePassword));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &ChangePasswordServerDialog::reject));

  VERIFY(connect(server_.get(), &proxy::IServer::ChangePasswordStarted, this,
                 &ChangePasswordServerDialog::startChangePassword));
  VERIFY(connect(server_.get(), &proxy::IServer::ChangePasswordFinished, this,
                 &ChangePasswordServerDialog::finishChangePassword));

  mainLayout->addWidget(buttonBox);
  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainLayout);

  glassWidget_ = new common::qt::gui::GlassWidget(
      GuiFactory::Instance().pathToLoadingGif(), translations::trTryToChangePassword, 0.5, QColor(111, 111, 100), this);
}

void ChangePasswordServerDialog::tryToCreatePassword() {
  if (!validateInput()) {
    QMessageBox::critical(this, translations::trError, trInvalidInput);
    return;
  }
  std::string password = common::ConvertToString(passwordLineEdit_->text());
  proxy::events_info::ChangePasswordRequest req(this, std::string(), password);
  server_->ChangePassword(req);
}

void ChangePasswordServerDialog::startChangePassword(const proxy::events_info::ChangePasswordRequest& req) {
  UNUSED(req);

  glassWidget_->start();
}

void ChangePasswordServerDialog::finishChangePassword(const proxy::events_info::ChangePasswordResponce& res) {
  glassWidget_->stop();
  common::Error err = res.errorInfo();
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
    return;
  }

  QMessageBox::information(this, translations::trInfo, trPasswordChS);
  ChangePasswordServerDialog::accept();
}

bool ChangePasswordServerDialog::validateInput() {
  QString pass = passwordLineEdit_->text();
  QString cpass = confPasswordLineEdit_->text();
  if (pass.isEmpty() || cpass.isEmpty()) {
    return false;
  }

  return pass == cpass;
}

}  // namespace gui
}  // namespace fastonosql
