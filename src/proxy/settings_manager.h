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

#include <QFont>
#include <QStringList>

#include <common/patterns/singleton_pattern.h>

#include "proxy/connection_settings/iconnection_settings.h"  // for IClusterSettingsBaseSPtr, etc

#if defined(PRO_VERSION)
#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/isentinel_connection_settings.h"
#include "proxy/user_info.h"
#endif

#include "proxy/types.h"  // for supportedViews

namespace fastonosql {
namespace proxy {

class SettingsManager : public common::patterns::Singleton<SettingsManager> {
 public:
  typedef std::vector<IConnectionSettingsBaseSPtr> connection_settings_t;
#if defined(PRO_VERSION)
  typedef std::vector<IClusterSettingsBaseSPtr> cluster_settings_t;
  typedef std::vector<ISentinelSettingsBaseSPtr> sentinel_settings_t;
#endif
  friend class common::patterns::Singleton<SettingsManager>;

  static std::string GetSettingsDirPath();
  static std::string GetSettingsFilePath();

  uint32_t GetConfigVersion() const;

  bool GetAccpetedEula() const;
  void SetAccpetedEula(bool val);

  void SetDefaultView(supportedViews view);
  supportedViews GetDefaultView() const;

  QString GetCurrentStyle() const;
  void SetCurrentStyle(const QString& style);

  QFont GetCurrentFont() const;
  void SetCurrentFont(const QFont& font);

  QString GetCurrentLanguage() const;
  void SetCurrentLanguage(const QString& lang);

  // connections
  void AddConnection(IConnectionSettingsBaseSPtr connection);
  void RemoveConnection(IConnectionSettingsBaseSPtr connection);

  connection_settings_t GetConnections() const;

  bool GetSendStatistic() const;
  void SetSendStatistic(bool val);

#if defined(PRO_VERSION)
  // sentinels
  void AddSentinel(ISentinelSettingsBaseSPtr sentinel);
  void RemoveSentinel(ISentinelSettingsBaseSPtr sentinel);

  sentinel_settings_t GetSentinels() const;

  // clusters
  void AddCluster(IClusterSettingsBaseSPtr cluster);
  void RemoveCluster(IClusterSettingsBaseSPtr cluster);

  cluster_settings_t GetClusters() const;
#endif

  void AddRConnection(const QString& connection);
  void RemoveRConnection(const QString& connection);
  QStringList GetRecentConnections() const;
  void ClearRConnections();

  void SetLoggingDirectory(const QString& dir);
  QString GetLoggingDirectory() const;

  bool GetAutoCheckUpdates() const;
  void SetAutoCheckUpdates(bool check);

  bool GetAutoCompletion() const;
  void SetAutoCompletion(bool completion);

  bool AutoOpenConsole() const;
  void SetAutoOpenConsole(bool open_console);

  bool GetAutoConnectDB() const;
  void SetAutoConnectDB(bool open_db);

  bool GetFastViewKeys() const;
  void SetFastViewKeys(bool fast_view);

  QByteArray GetWindowSettings() const;
  void SetWindowSettings(const QByteArray& settings);

  QString GetPythonPath() const;
  void SetPythonPath(const QString& path);

  void ReloadFromPath(const std::string& path, bool merge);

  void Load();
  void Save();
#if defined(PRO_VERSION)
  QString GetLastLogin() const;
  void SetLastLogin(const QString& login);

  // runtime
  UserInfo GetUserInfo() const;
  void SetUserInfo(const UserInfo& uinfo);
#endif

 private:
  SettingsManager();
  ~SettingsManager();

  uint32_t config_version_;

  bool accepted_eula_;
  supportedViews views_;
  QString cur_style_;
  QFont cur_font_;
  QString cur_language_;
  bool send_statistic_;
  connection_settings_t connections_;
#if defined(PRO_VERSION)
  sentinel_settings_t sentinels_;
  cluster_settings_t clusters_;
  QString last_login_;
  // runtime settings
  UserInfo user_info_;
#endif
  QStringList recent_connections_;
  QString logging_dir_;
  bool auto_check_updates_;
  bool auto_completion_;
  bool auto_open_console_;
  bool auto_connect_db_;
  bool fast_view_keys_;
  QByteArray window_settings_;
  QString python_path_;
};

}  // namespace proxy
}  // namespace fastonosql
