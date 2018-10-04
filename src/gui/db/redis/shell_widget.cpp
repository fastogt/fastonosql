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

#include "gui/db/redis/shell_widget.h"

#include <QHBoxLayout>

#include <common/qt/convert2string.h>
#include <common/qt/gui/icon_combobox.h>

#include <fastonosql/core/module_info.h>

#include "proxy/events/events_info.h"

#include "gui/gui_factory.h"

namespace fastonosql {
namespace gui {
namespace redis {

ShellWidget::ShellWidget(proxy::IServerSPtr server, const QString& filePath, QWidget* parent)
    : base_class(server, filePath, parent), modules_(nullptr) {}

void ShellWidget::init() {
  base_class::init();
  updateModules(std::vector<core::ModuleInfo>());
}

QHBoxLayout* ShellWidget::createTopLayout(core::ConnectionTypes ct) {
  QHBoxLayout* top_layout = base_class::createTopLayout(ct);
  modules_ = new common::qt::gui::IconComboBox(gui::GuiFactory::GetInstance().moduleIcon(), top_bar_icon_size);
  top_layout->addWidget(modules_);
  return top_layout;
}

void ShellWidget::updateModules(const std::vector<core::ModuleInfo>& modules) {
  modules_->clearCombo();
  if (modules.empty()) {
    modules_->setEnabled(false);
    return;
  }

  for (size_t i = 0; i < modules.size(); ++i) {
    QString qname;
    common::ConvertFromString(modules[i].name, &qname);
    modules_->addComboItem(qname);
  }
  modules_->setEnabled(true);
}

void ShellWidget::OnServerDisconnected() {
  base_class::OnServerDisconnected();
  updateModules(std::vector<core::ModuleInfo>());
}

void ShellWidget::OnFinishedLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoResponce& res) {
  base_class::OnFinishedLoadDiscoveryInfo(res);

  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  updateModules(res.loaded_modules);
}
}  // namespace redis
}  // namespace gui
}  // namespace fastonosql
