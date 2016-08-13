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

#include "gui/dialogs/change_password_server_dialog.h"

#include <memory>                       // for __shared_ptr
#include <string>                       // for string

#include <QDialogButtonBox>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

#include "common/error.h"               // for Error
#include "common/macros.h"              // for VERIFY, CHECK, UNUSED
#include "common/qt/convert2string.h"   // for ConvertToString
#include "common/value.h"               // for ErrorValue

#include "core/events/events_info.h"    // for ChangePasswordResponce, etc
#include "core/iserver.h"               // for IServer

#include "fasto/qt/gui/glass_widget.h"  // for GlassWidget

#include "gui/gui_factory.h"            // for GuiFactory

#include "translations/global.h"        // for trError, trInfo, etc

namespace {
  const QString trPassword = QObject::tr("Password:");
  const QString trCPassword = QObject::tr("Confirm Password:");
  const QString trInvalidInput = QObject::tr("Invalid input!");
  const QString trPasswordChS = QObject::tr("Password successfully changed!");
}

namespace fastonosql {
namespace gui {

ChangePasswordServerDialog::ChangePasswordServerDialog(const QString& title,
                                                       core::IServerSPtr server, QWidget* parent)
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
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted,
                 this, &ChangePasswordServerDialog::tryToCreatePassword));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected,
                 this, &ChangePasswordServerDialog::reject));

  VERIFY(connect(server_.get(), &core::IServer::startedChangePassword,
                 this, &ChangePasswordServerDialog::startChangePassword));
  VERIFY(connect(server_.get(), &core::IServer::finishedChangePassword,
                 this, &ChangePasswordServerDialog::finishChangePassword));

  mainLayout->addWidget(buttonBox);
  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainLayout);

  glassWidget_ = new fasto::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(),
                                                 translations::trTryToChangePassword, 0.5,
                                                 QColor(111, 111, 100), this);
}

void ChangePasswordServerDialog::tryToCreatePassword() {
  if (validateInput()) {
    std::string password = common::ConvertToString(passwordLineEdit_->text());
    core::events_info::ChangePasswordRequest req(this, std::string(), password);
    server_->changePassword(req);
  } else {
    QMessageBox::critical(this, translations::trError, trInvalidInput);
  }
}

void ChangePasswordServerDialog::startChangePassword(const core::events_info::ChangePasswordRequest& req) {
  UNUSED(req);

  glassWidget_->start();
}

void ChangePasswordServerDialog::finishChangePassword(const core::events_info::ChangePasswordResponce& res) {
  glassWidget_->stop();
  common::Error err = res.errorInfo();
  if (err && err->isError()) {
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
