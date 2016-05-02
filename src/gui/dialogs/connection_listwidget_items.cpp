/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include "gui/dialogs/connection_listwidget_items.h"

#include <string>

#include "gui/gui_factory.h"

namespace fastonosql {
namespace gui {

DirectoryListWidgetItem::DirectoryListWidgetItem(const core::IConnectionSettings::connection_path_t &path)
  : path_(path) {
  std::string dir_name = path.name();
  setText(0, common::convertFromString<QString>(dir_name));
  setIcon(0, GuiFactory::instance().directoryIcon());
  setText(1, common::convertFromString<QString>(path_.directory()));
}

core::IConnectionSettingsBase::connection_path_t DirectoryListWidgetItem::path() const {
  return path_;
}

ConnectionListWidgetItem::ConnectionListWidgetItem(core::IConnectionSettingsBaseSPtr connection, QTreeWidgetItem *parent)
  : QTreeWidgetItem(parent), connection_() {
  setConnection(connection);
}

void ConnectionListWidgetItem::setConnection(core::IConnectionSettingsBaseSPtr cons) {
  if (!cons) {
    return;
  }

  connection_ = cons;
  core::IConnectionSettingsBase::connection_path_t path = connection_->path();
  QString conName = common::convertFromString<QString>(path.name());

  setText(0, conName);
  core::connectionTypes conType = connection_->type();
  setIcon(0, GuiFactory::instance().icon(conType));
  setText(1, common::convertFromString<QString>(connection_->fullAddress()));
}

core::IConnectionSettingsBaseSPtr ConnectionListWidgetItem::connection() const {
  return connection_;
}

ConnectionListWidgetItemEx::ConnectionListWidgetItemEx(core::IConnectionSettingsBaseSPtr connection,
                                                       core::serverTypes st, QTreeWidgetItem* parent)
  : ConnectionListWidgetItem(connection, parent), server_type_(st) {
  std::string sert = common::convertToString(st);
  setText(2, common::convertFromString<QString>(sert));
}

core::serverTypes ConnectionListWidgetItemEx::serverType() const {
  return server_type_;
}

SentinelConnectionListWidgetItem::SentinelConnectionListWidgetItem(core::ISentinelSettingsBaseSPtr connection, QTreeWidgetItem* parent)
  : QTreeWidgetItem(parent), connection_() {
  setConnection(connection);

  core::IClusterSettingsBase::cluster_connection_t servers = connection_->nodes();
  for (size_t i = 0; i < servers.size(); ++i) {
    core::IConnectionSettingsBaseSPtr con = servers[i];
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(con, this);
    addChild(item);
  }
}

void SentinelConnectionListWidgetItem::setConnection(core::ISentinelSettingsBaseSPtr cons) {
  if (!cons) {
    return;
  }

  connection_ = cons;
  std::string path = connection_->path().toString();
  setText(0, common::convertFromString<QString>(path));
  setIcon(0, GuiFactory::instance().sentinelIcon());
}

core::ISentinelSettingsBaseSPtr SentinelConnectionListWidgetItem::connection() const {
  return connection_;
}

ClusterConnectionListWidgetItem::ClusterConnectionListWidgetItem(core::IClusterSettingsBaseSPtr connection, QTreeWidgetItem* parent)
  : QTreeWidgetItem(parent), connection_() {
  setConnection(connection);

  core::IClusterSettingsBase::cluster_connection_t servers = connection_->nodes();

  for (size_t i = 0; i < servers.size(); ++i) {
    core::IConnectionSettingsBaseSPtr con = servers[i];
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(con, this);
    addChild(item);
  }
}

void ClusterConnectionListWidgetItem::setConnection(core::IClusterSettingsBaseSPtr cons) {
  if (!cons) {
    return;
  }

  connection_ = cons;
  std::string path = connection_->path().toString();
  setText(0, common::convertFromString<QString>(path));
  setIcon(0, GuiFactory::instance().clusterIcon());
}

core::IClusterSettingsBaseSPtr ClusterConnectionListWidgetItem::connection() const {
  return connection_;
}

}  // namespace gui
}  // namespace fastonosql
