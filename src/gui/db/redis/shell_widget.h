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

#include "gui/shell/base_shell_widget.h"

namespace common {
namespace qt {
namespace gui {
class IconComboBox;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace core {
struct ModuleInfo;
}
namespace gui {
namespace redis {

class ShellWidget : public BaseShellWidget {
  Q_OBJECT

 public:
  typedef BaseShellWidget base_class;
  template <typename T, typename... Args>
  friend T* gui::createWidget(Args&&... args);

 protected:
  ShellWidget(proxy::IServerSPtr server, const QString& file_path = QString(), QWidget* parent = Q_NULLPTR);
  void init() override;

  QHBoxLayout* createTopLayout(core::ConnectionType ct) override;
  void OnServerDisconnected() override;
  void OnFinishedLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoResponce& res) override;

 private:
  void updateModules(const std::vector<core::ModuleInfo>& modules);

  common::qt::gui::IconComboBox* modules_;
};
}  // namespace redis
}  // namespace gui
}  // namespace fastonosql
