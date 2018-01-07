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

#include <QDialog>

#include "core/server_property_info.h"  // for property_t
#include "proxy/proxy_fwd.h"            // for IServerSPtr

class QTableView;  // lines 26-26

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
struct ChangeServerPropertyInfoRequest;
struct ServerPropertyInfoRequest;
struct ServerPropertyInfoResponce;
struct ChangeServerPropertyInfoResponce;
}  // namespace events_info
}  // namespace proxy
namespace gui {

class PropertyServerDialog : public QDialog {
  Q_OBJECT
 public:
  explicit PropertyServerDialog(proxy::IServerSPtr server, QWidget* parent = Q_NULLPTR);
  enum { min_width = 240, min_height = 200 };

 private Q_SLOTS:
  void startServerProperty(const proxy::events_info::ServerPropertyInfoRequest& req);
  void finishServerProperty(const proxy::events_info::ServerPropertyInfoResponce& res);

  void startServerChangeProperty(const proxy::events_info::ChangeServerPropertyInfoRequest& req);
  void finishServerChangeProperty(const proxy::events_info::ChangeServerPropertyInfoResponce& res);

  void changedProperty(const core::property_t& prop);

 protected:
  virtual void changeEvent(QEvent* e) override;
  virtual void showEvent(QShowEvent* e) override;

 private:
  void retranslateUi();

  common::qt::gui::GlassWidget* glassWidget_;
  QTableView* properties_table_;
  const proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
