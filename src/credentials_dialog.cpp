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
const QString trSignInInfo =
    QObject::tr("<b>Please sign in (use the same credentials like on <a href=\"" PROJECT_DOMAIN "\">website</a>)</b>");
}  // namespace

namespace fastonosql {

const QSize CredentialsDialog::status_label_icon_size = QSize(24, 24);

CredentialsDialog::CredentialsDialog(QWidget* parent) : base_class(parent), glass_widget_(nullptr), user_info_() {
  qRegisterMetaType<common::Error>("common::Error");
  qRegisterMetaType<proxy::UserInfo>("proxy::UserInfo");

  glass_widget_ = new common::qt::gui::GlassWidget(gui::GuiFactory::GetInstance().pathToLoadingGif(), QString(), 0.5,
                                                   QColor(111, 111, 100), this);
  setVisibleDescription(false);
  setVisibleStatus(true);
  setInforamtion(trSignInInfo);
}

proxy::UserInfo CredentialsDialog::userInfo() const {
  DCHECK(user_info_.IsValid()) << "You can get only valid user info.";
  return user_info_;
}

void CredentialsDialog::verifyUserResult(common::Error err, const proxy::UserInfo& user) {
  glass_widget_->stop();
  if (err) {
    const std::string descr_str = err->GetDescription();
    QString descr;
    common::ConvertFromString(descr_str, &descr);
    setFailed(descr);
    return;
  }

  user_info_ = user;
  base_class::accept();
}

void CredentialsDialog::accept() {
  glass_widget_->start();
  startVerification();
}

void CredentialsDialog::setInforamtion(const QString& text) {
  setStatusIcon(gui::GuiFactory::GetInstance().infoIcon(), status_label_icon_size);
  setStatus(text);
}

void CredentialsDialog::setFailed(const QString& text) {
  setStatusIcon(gui::GuiFactory::GetInstance().failIcon(), status_label_icon_size);
  setStatus(text);
}

void CredentialsDialog::startVerification() {
  QThread* th = new QThread;
  IVerifyUser* checker = createChecker();
  checker->moveToThread(th);
  VERIFY(connect(th, &QThread::started, checker, &IVerifyUser::routine));
  VERIFY(connect(checker, &IVerifyUser::verifyUserResult, this, &CredentialsDialog::verifyUserResult));
  VERIFY(connect(checker, &IVerifyUser::verifyUserResult, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, checker, &IVerifyUser::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

}  // namespace fastonosql
