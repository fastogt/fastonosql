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

#include <common/error.h>

#include "gui/dialogs/password_dialog.h"

#include "proxy/user_info.h"

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {

class IVerifyUser;

class CredentialsDialog : public fastonosql::gui::PasswordDialog {
  Q_OBJECT
 public:
  static const QSize status_label_icon_size;

  typedef fastonosql::gui::PasswordDialog base_class;
  explicit CredentialsDialog(QWidget* parent = Q_NULLPTR);

  proxy::UserInfo userInfo() const;

 public Q_SLOTS:
  virtual void accept() override;

 private Q_SLOTS:
  void verifyUserResult(common::Error err, const proxy::UserInfo& user);

 private:
  virtual IVerifyUser* createChecker() const = 0;

  void setInforamtion(const QString& text);
  void setFailed(const QString& text);

  void startVerification();

  common::qt::gui::GlassWidget* glass_widget_;
  proxy::UserInfo user_info_;
};
}  // namespace fastonosql
