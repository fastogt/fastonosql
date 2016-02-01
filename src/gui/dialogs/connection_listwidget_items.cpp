/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/connection_listwidget_items.h"

#include <string>

#include "gui/gui_factory.h"

namespace fastonosql {

ConnectionListWidgetItem::ConnectionListWidgetItem(IConnectionSettingsBaseSPtr connection)
  : connection_() {
  setConnection(connection);
}

void ConnectionListWidgetItem::setConnection(IConnectionSettingsBaseSPtr cons) {
  if (!cons) {
    return;
  }

  connection_ = cons;
  setText(0, common::convertFromString<QString>(connection_->connectionName()));
  connectionTypes conType = connection_->connectionType();
  setIcon(0, GuiFactory::instance().icon(conType));
  setText(1, common::convertFromString<QString>(connection_->fullAddress()));
}

IConnectionSettingsBaseSPtr ConnectionListWidgetItem::connection() const {
  return connection_;
}

ConnectionListWidgetItemEx::ConnectionListWidgetItemEx(IConnectionSettingsBaseSPtr connection,
                                                       serverTypes st)
  : ConnectionListWidgetItem(connection) {
  std::string sert = common::convertToString(st);
  setText(2, common::convertFromString<QString>(sert));
}

ClusterConnectionListWidgetItem::ClusterConnectionListWidgetItem(IClusterSettingsBaseSPtr connection)
  : connection_(connection) {
  setText(0, common::convertFromString<QString>(connection_->connectionName()));
  setIcon(0, GuiFactory::instance().clusterIcon());

  IClusterSettingsBase::cluster_connection_type servers = connection_->nodes();

  for (size_t i = 0; i < servers.size(); ++i) {
    IConnectionSettingsBaseSPtr con = servers[i];
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(con);
    addChild(item);
  }
}

void ClusterConnectionListWidgetItem::setConnection(IClusterSettingsBaseSPtr cons) {
  if (!cons) {
    return;
  }

  connection_ = cons;
  setText(0, common::convertFromString<QString>(connection_->connectionName()));
  setIcon(0, GuiFactory::instance().clusterIcon());
}

IClusterSettingsBaseSPtr ClusterConnectionListWidgetItem::connection() const {
  return connection_;
}

}  // namespace fastonosql
