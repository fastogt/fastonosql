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

#include <QTreeWidgetItem>

#include <fastonosql/core/server/iserver_info.h>  // for ServerCommonInfo

#include "proxy/connection_settings/iconnection_settings.h"  // for IClusterSettingsBaseSPtr, etc

#if defined(PRO_VERSION)
#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/isentinel_connection_settings.h"
#endif

namespace fastonosql {
namespace gui {

class DirectoryListWidgetItem  // directory can hold many (common, cluster or sentinel_container)
    : public QTreeWidgetItem {
 public:
  explicit DirectoryListWidgetItem(const proxy::connection_path_t& path);
  proxy::connection_path_t path() const;

 private:
  proxy::connection_path_t path_;
};

class IConnectionListWidgetItem  // base class
    : public QTreeWidgetItem {
 public:
  enum itemConnectionType : uint8_t { Common = 0, Discovered, Sentinel };
  virtual void setConnection(proxy::IConnectionSettingsBaseSPtr cons);
  proxy::IConnectionSettingsBaseSPtr connection() const;
  virtual itemConnectionType type() const = 0;

 protected:
  explicit IConnectionListWidgetItem(QTreeWidgetItem* parent);

 private:
  proxy::IConnectionSettingsBaseSPtr connection_;
};

class ConnectionListWidgetItem  // common connection
    : public IConnectionListWidgetItem {
 public:
  explicit ConnectionListWidgetItem(QTreeWidgetItem* parent);
  virtual void setConnection(proxy::IConnectionSettingsBaseSPtr cons) override;
  virtual itemConnectionType type() const override;
};

class ConnectionListWidgetItemDiscovered  // returned after
                                          // discovered
    : public ConnectionListWidgetItem {
 public:
  ConnectionListWidgetItemDiscovered(const core::ServerCommonInfo& info, QTreeWidgetItem* parent);
  virtual itemConnectionType type() const override;

 private:
  core::ServerCommonInfo info_;
};

#if defined(PRO_VERSION)
class SentinelConnectionListWidgetItemContainer  // can hold
                                                 // many
                                                 // sentinel
    // connections
    : public QTreeWidgetItem {
 public:
  explicit SentinelConnectionListWidgetItemContainer(proxy::ISentinelSettingsBaseSPtr connection,
                                                     QTreeWidgetItem* parent);
  void setConnection(proxy::ISentinelSettingsBaseSPtr cons);
  proxy::ISentinelSettingsBaseSPtr connection() const;

 private:
  proxy::ISentinelSettingsBaseSPtr connection_;
};

class SentinelConnectionWidgetItem  // sentinel connection
    : public ConnectionListWidgetItemDiscovered {
 public:
  SentinelConnectionWidgetItem(const core::ServerCommonInfo& info, SentinelConnectionListWidgetItemContainer* parent);
  virtual itemConnectionType type() const override;
};

class ClusterConnectionListWidgetItemContainer  // can hold
                                                // many
                                                // connections
    : public QTreeWidgetItem {
 public:
  explicit ClusterConnectionListWidgetItemContainer(proxy::IClusterSettingsBaseSPtr connection,
                                                    QTreeWidgetItem* parent);
  void setConnection(proxy::IClusterSettingsBaseSPtr cons);
  proxy::IClusterSettingsBaseSPtr connection() const;

 private:
  proxy::IClusterSettingsBaseSPtr connection_;
};
#endif

}  // namespace gui
}  // namespace fastonosql
