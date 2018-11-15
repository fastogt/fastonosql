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

#include "gui/widgets/connection_base_widget.h"

#include "proxy/connection_settings/iconnection_settings_remote.h"

namespace fastonosql {
namespace gui {
class HostPortWidget;

class ConnectionRemoteWidget : public ConnectionBaseWidget {
  Q_OBJECT

 public:
  explicit ConnectionRemoteWidget(QWidget* parent = Q_NULLPTR);

  void syncControls(proxy::IConnectionSettingsBase* connection) override;
  void retranslateUi() override;
  bool validated() const override;

 protected:
  virtual proxy::IConnectionSettingsRemote* createConnectionRemoteImpl(const proxy::connection_path_t& path) const = 0;

 private:
  proxy::IConnectionSettingsBase* createConnectionImpl(const proxy::connection_path_t& path) const override final;
  HostPortWidget* host_widget_;
};

}  // namespace gui
}  // namespace fastonosql
