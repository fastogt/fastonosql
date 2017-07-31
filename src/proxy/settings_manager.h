/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <stdint.h>  // for uint32_t

#include <string>  // for string
#include <vector>  // for vector

#include <QFont>
#include <QStringList>

#include <common/net/types.h>
#include <common/patterns/singleton_pattern.h>  // for LazySingleton

#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/iconnection_settings.h"  // for IClusterSettingsBaseSPtr, etc
#include "proxy/connection_settings/isentinel_connection_settings.h"

#include "proxy/types.h"  // for supportedViews

namespace fastonosql {
namespace proxy {

class SettingsManager : public common::patterns::LazySingleton<SettingsManager> {
 public:
  typedef std::vector<IConnectionSettingsBaseSPtr> connection_settings_t;
  typedef std::vector<IClusterSettingsBaseSPtr> cluster_settings_t;
  typedef std::vector<ISentinelSettingsBaseSPtr> sentinel_settings_t;
  friend class common::patterns::LazySingleton<SettingsManager>;

  static std::string SettingsDirPath();
  static std::string SettingsFilePath();

  uint32_t ConfigVersion() const;

  bool IsSendedStatistic() const;
  void SetIsSendedStatistic(bool val);

  void SetDefaultView(supportedViews view);
  supportedViews DefaultView() const;

  QString CurrentStyle() const;
  void SetCurrentStyle(const QString& style);

  QFont CurrentFont() const;
  void SetCurrentFont(const QFont& font);

  QString CurrentLanguage() const;
  void SetCurrentLanguage(const QString& lang);

  // connections
  void AddConnection(IConnectionSettingsBaseSPtr connection);
  void RemoveConnection(IConnectionSettingsBaseSPtr connection);

  connection_settings_t Connections() const;

  // sentinels
  void AddSentinel(ISentinelSettingsBaseSPtr sentinel);
  void RemoveSentinel(ISentinelSettingsBaseSPtr sentinel);

  sentinel_settings_t Sentinels() const;

  // clusters
  void AddCluster(IClusterSettingsBaseSPtr cluster);
  void RemoveCluster(IClusterSettingsBaseSPtr cluster);

  cluster_settings_t Clusters() const;

  void AddRConnection(const QString& connection);
  void RemoveRConnection(const QString& connection);
  QStringList RecentConnections() const;
  void ClearRConnections();

  void SetLoggingDirectory(const QString& dir);
  QString LoggingDirectory() const;

  bool AutoCheckUpdates() const;
  void SetAutoCheckUpdates(bool check);

  bool AutoCompletion() const;
  void SetAutoCompletion(bool completion);

  bool AutoOpenConsole() const;
  void SetAutoOpenConsole(bool open_console);

  bool AutoConnectDB() const;
  void SetAutoConnectDB(bool open_db);

  bool FastViewKeys() const;
  void SetFastViewKeys(bool fast_view);

  void ReloadFromPath(const std::string& path, bool merge);

 private:
  void Load();
  void Save();

  SettingsManager();
  ~SettingsManager();

  uint32_t config_version_;
  bool sended_statistic_;
  supportedViews views_;
  QString cur_style_;
  QFont cur_font_;
  QString cur_language_;
  connection_settings_t connections_;
  sentinel_settings_t sentinels_;
  cluster_settings_t clusters_;
  QStringList recent_connections_;
  QString logging_dir_;
  bool auto_check_update_;
  bool auto_completion_;
  bool auto_open_console_;
  bool auto_connect_db_;
  bool fast_view_keys_;
};

}  // namespace proxy
}  // namespace fastonosql
