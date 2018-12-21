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

#pragma once

#include "gui/dialogs/base_dialog.h"

#include "proxy/proxy_fwd.h"

class QTextEdit;

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace core {
class IServerInfo;
}
namespace proxy {
namespace events_info {
class ServerInfoResponce;
struct ServerInfoRequest;
}  // namespace events_info
}  // namespace proxy
namespace gui {

class InfoServerDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  enum { min_width = 420, min_height = 560 };

 private Q_SLOTS:
  void startServerInfo(const proxy::events_info::ServerInfoRequest& req);
  void finishServerInfo(const proxy::events_info::ServerInfoResponce& res);

 protected:
  explicit InfoServerDialog(const QString& title, proxy::IServerSPtr server, QWidget* parent = Q_NULLPTR);

  void showEvent(QShowEvent* e) override;

 private:
  void updateText(core::IServerInfo* serv);

  QTextEdit* server_text_info_;
  common::qt::gui::GlassWidget* glass_widget_;
  const proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
