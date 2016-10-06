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

#pragma once

#include <QTreeWidgetItem>

#include "core/connection_settings/connection_settings.h"  // for IClusterSettingsBaseSPtr, etc
#include "core/connection_settings/cluster_connection_settings.h"
#include "core/connection_settings/sentinel_connection_settings.h"
#include "core/server/iserver_info.h"  // for ServerCommonInfo

namespace fastonosql {
namespace gui {

class DirectoryListWidgetItem  // directory can hold many
                               // (common, cluster or
                               // sentinel_container)
    : public QTreeWidgetItem {
 public:
  DirectoryListWidgetItem(const core::IConnectionSettingsBase::connection_path_t& path);
  core::IConnectionSettingsBase::connection_path_t path() const;

 private:
  core::IConnectionSettingsBase::connection_path_t path_;
};

class IConnectionListWidgetItem  // base class
    : public QTreeWidgetItem {
 public:
  enum itemConnectionType { Common, Discovered, Sentinel };
  virtual void setConnection(core::IConnectionSettingsBaseSPtr cons);
  core::IConnectionSettingsBaseSPtr connection() const;
  virtual itemConnectionType type() const = 0;

 protected:
  explicit IConnectionListWidgetItem(QTreeWidgetItem* parent);

 private:
  core::IConnectionSettingsBaseSPtr connection_;
};

class ConnectionListWidgetItem  // common connection
    : public IConnectionListWidgetItem {
 public:
  explicit ConnectionListWidgetItem(QTreeWidgetItem* parent);
  virtual void setConnection(core::IConnectionSettingsBaseSPtr cons);
  virtual itemConnectionType type() const;
};

class ConnectionListWidgetItemDiscovered  // returned after
                                          // discovered
    : public ConnectionListWidgetItem {
 public:
  ConnectionListWidgetItemDiscovered(const core::ServerCommonInfo& info, QTreeWidgetItem* parent);
  virtual itemConnectionType type() const;

 private:
  core::ServerCommonInfo info_;
};

class SentinelConnectionListWidgetItemContainer  // can hold
                                                 // many
                                                 // sentinel
    // connections
    : public QTreeWidgetItem {
 public:
  explicit SentinelConnectionListWidgetItemContainer(core::ISentinelSettingsBaseSPtr connection,
                                                     QTreeWidgetItem* parent);
  void setConnection(core::ISentinelSettingsBaseSPtr cons);
  core::ISentinelSettingsBaseSPtr connection() const;

 private:
  core::ISentinelSettingsBaseSPtr connection_;
};

class SentinelConnectionWidgetItem  // sentinel connection
    : public ConnectionListWidgetItemDiscovered {
 public:
  SentinelConnectionWidgetItem(const core::ServerCommonInfo& info,
                               SentinelConnectionListWidgetItemContainer* parent);
  virtual itemConnectionType type() const;
};

class ClusterConnectionListWidgetItemContainer  // can hold
                                                // many
                                                // connections
    : public QTreeWidgetItem {
 public:
  explicit ClusterConnectionListWidgetItemContainer(core::IClusterSettingsBaseSPtr connection,
                                                    QTreeWidgetItem* parent);
  void setConnection(core::IClusterSettingsBaseSPtr cons);
  core::IClusterSettingsBaseSPtr connection() const;

 private:
  core::IClusterSettingsBaseSPtr connection_;
};

}  // namespace gui
}  // namespace fastonosql
