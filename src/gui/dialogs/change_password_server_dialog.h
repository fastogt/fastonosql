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

#pragma once

#include <QDialog>

#include "proxy/proxy_fwd.h"

class QLineEdit;

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace proxy {
namespace events_info {
struct ChangePasswordRequest;
struct ChangePasswordResponce;
}  // namespace events_info
}  // namespace proxy
namespace gui {

class ChangePasswordServerDialog : public QDialog {
  Q_OBJECT
 public:
  enum { min_width = 240, min_height = 120 };

  explicit ChangePasswordServerDialog(const QString& title, proxy::IServerSPtr server, QWidget* parent = 0);

 private Q_SLOTS:
  void tryToCreatePassword();
  void startChangePassword(const proxy::events_info::ChangePasswordRequest& req);
  void finishChangePassword(const proxy::events_info::ChangePasswordResponce& res);

 private:
  bool validateInput();

  common::qt::gui::GlassWidget* glassWidget_;
  QLineEdit* passwordLineEdit_;
  QLineEdit* confPasswordLineEdit_;
  const proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
