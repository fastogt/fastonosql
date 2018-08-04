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

#include "credentials_dialog.h"

#include <QMetaType>  // for qRegisterMetaType
#include <QThread>

#include <common/qt/convert2string.h>
#include <common/qt/gui/glass_widget.h>  // for GlassWidget

#include "gui/gui_factory.h"

#include "iverify_user.h"

namespace {
const QString trSignin =
    QObject::tr("<b>Please sign in (use the same credentials like on <a href=\"" PROJECT_DOMAIN "\">website</a>)</b>");
}  // namespace

namespace fastonosql {

const QSize CredentialsDialog::status_label_icon_size = QSize(24, 24);

CredentialsDialog::CredentialsDialog(QWidget* parent) : base_class(parent), user_info_() {
  glass_widget_ = new common::qt::gui::GlassWidget(gui::GuiFactory::GetInstance().GetPathToLoadingGif(), QString(), 0.5,
                                                   QColor(111, 111, 100), this);
  SetVisibleDescription(false);
  SetVisibleStatus(true);
  SetInforamtion(trSignin);
}

proxy::UserInfo CredentialsDialog::GetUserInfo() const {
  return user_info_;
}

void CredentialsDialog::verifyUserResult(common::Error err, const proxy::UserInfo& user) {
  glass_widget_->stop();
  if (err) {
    const std::string descr_str = err->GetDescription();
    QString descr;
    common::ConvertFromString(descr_str, &descr);
    SetFailed(descr);
    return;
  }

  user_info_ = user;
  base_class::accept();
}

void CredentialsDialog::accept() {
  glass_widget_->start();
  startVerification();
}

void CredentialsDialog::SetInforamtion(const QString& text) {
  SetStatusIcon(gui::GuiFactory::GetInstance().GetInfoIcon(), status_label_icon_size);
  SetStatus(text);
}

void CredentialsDialog::SetFailed(const QString& text) {
  SetStatusIcon(gui::GuiFactory::GetInstance().GetFailIcon(), status_label_icon_size);
  SetStatus(text);
}

void CredentialsDialog::startVerification() {
  qRegisterMetaType<common::Error>("common::Error");
  qRegisterMetaType<proxy::UserInfo>("proxy::UserInfo");

  QThread* th = new QThread;
  IVerifyUser* checker = CreateChecker();
  checker->moveToThread(th);
  VERIFY(connect(th, &QThread::started, checker, &IVerifyUser::routine));
  VERIFY(connect(checker, &IVerifyUser::verifyUserResult, this, &CredentialsDialog::verifyUserResult));
  VERIFY(connect(checker, &IVerifyUser::verifyUserResult, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, checker, &IVerifyUser::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

}  // namespace fastonosql
