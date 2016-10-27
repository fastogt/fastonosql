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

#include <stddef.h>  // for size_t

#include <memory>  // for __shared_ptr
#include <string>  // for string
#include <vector>  // for vector

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for DNOTREACHED

#include "core/connection_types.h"  // for ConvertToString, etc

#include "gui/gui_factory.h"  // for GuiFactory

namespace fastonosql {
namespace gui {

DirectoryListWidgetItem::DirectoryListWidgetItem(const core::connection_path_t& path)
    : path_(path) {
  std::string dir_name = path.Name();
  setText(0, common::ConvertFromString<QString>(dir_name));
  setIcon(0, GuiFactory::instance().directoryIcon());
  setText(1, common::ConvertFromString<QString>(path_.Directory()));
}

core::connection_path_t DirectoryListWidgetItem::path() const {
  return path_;
}

IConnectionListWidgetItem::IConnectionListWidgetItem(QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent), connection_() {}

void IConnectionListWidgetItem::setConnection(core::IConnectionSettingsBaseSPtr cons) {
  connection_ = cons;
}

core::IConnectionSettingsBaseSPtr IConnectionListWidgetItem::connection() const {
  return connection_;
}

ConnectionListWidgetItem::ConnectionListWidgetItem(QTreeWidgetItem* parent)
    : IConnectionListWidgetItem(parent) {}

void ConnectionListWidgetItem::setConnection(core::IConnectionSettingsBaseSPtr cons) {
  if (!cons) {
    DNOTREACHED();
    return;
  }

  core::connection_path_t path = cons->Path();
  QString conName = common::ConvertFromString<QString>(path.Name());

  setText(0, conName);
  core::connectionTypes conType = cons->Type();
  setIcon(0, GuiFactory::instance().icon(conType));
  setText(1, common::ConvertFromString<QString>(cons->FullAddress()));
  IConnectionListWidgetItem::setConnection(cons);
}

IConnectionListWidgetItem::itemConnectionType ConnectionListWidgetItem::type() const {
  return Common;
}

SentinelConnectionWidgetItem::SentinelConnectionWidgetItem(
    const core::ServerCommonInfo& info,
    SentinelConnectionListWidgetItemContainer* parent)
    : ConnectionListWidgetItemDiscovered(info,
                                         parent) {  // core::SENTINEL
}

IConnectionListWidgetItem::itemConnectionType SentinelConnectionWidgetItem::type() const {
  return Sentinel;
}

ConnectionListWidgetItemDiscovered::ConnectionListWidgetItemDiscovered(
    const core::ServerCommonInfo& info,
    QTreeWidgetItem* parent)
    : ConnectionListWidgetItem(parent), info_(info) {
  std::string stype = common::ConvertToString(info_.type);
  setText(2, common::ConvertFromString<QString>(stype));
  std::string sstate = common::ConvertToString(info_.state);
  setText(3, common::ConvertFromString<QString>(sstate));
}

IConnectionListWidgetItem::itemConnectionType ConnectionListWidgetItemDiscovered::type() const {
  return Discovered;
}

SentinelConnectionListWidgetItemContainer::SentinelConnectionListWidgetItemContainer(
    core::ISentinelSettingsBaseSPtr connection,
    QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent), connection_() {
  setConnection(connection);

  core::ISentinelSettingsBase::sentinel_connections_t sentinels = connection_->Sentinels();
  for (size_t i = 0; i < sentinels.size(); ++i) {
    core::SentinelSettings sent = sentinels[i];
    SentinelConnectionWidgetItem* item =
        new SentinelConnectionWidgetItem(core::ServerCommonInfo(), this);
    item->setConnection(sent.sentinel);
    addChild(item);
    for (size_t j = 0; j < sent.sentinel_nodes.size(); ++j) {
      core::IConnectionSettingsBaseSPtr con = sent.sentinel_nodes[j];
      ConnectionListWidgetItem* child = new ConnectionListWidgetItem(item);
      child->setConnection(con);
      item->addChild(child);
    }
  }
}

void SentinelConnectionListWidgetItemContainer::setConnection(
    core::ISentinelSettingsBaseSPtr cons) {
  if (!cons) {
    return;
  }

  connection_ = cons;
  std::string path = connection_->Path().ToString();
  setText(0, common::ConvertFromString<QString>(path));
  setIcon(0, GuiFactory::instance().sentinelIcon());
}

core::ISentinelSettingsBaseSPtr SentinelConnectionListWidgetItemContainer::connection() const {
  return connection_;
}

ClusterConnectionListWidgetItemContainer::ClusterConnectionListWidgetItemContainer(
    core::IClusterSettingsBaseSPtr connection,
    QTreeWidgetItem* parent)
    : QTreeWidgetItem(parent), connection_() {
  setConnection(connection);

  auto nodes = connection_->Nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    core::IConnectionSettingsBaseSPtr con = nodes[i];
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(this);
    item->setConnection(con);
    addChild(item);
  }
}

void ClusterConnectionListWidgetItemContainer::setConnection(core::IClusterSettingsBaseSPtr cons) {
  if (!cons) {
    return;
  }

  connection_ = cons;
  std::string path = connection_->Path().ToString();
  setText(0, common::ConvertFromString<QString>(path));
  setIcon(0, GuiFactory::instance().clusterIcon());
}

core::IClusterSettingsBaseSPtr ClusterConnectionListWidgetItemContainer::connection() const {
  return connection_;
}

}  // namespace gui
}  // namespace fastonosql
