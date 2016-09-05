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

#include <stdint.h>  // for uint32_t

#include <string>  // for string
#include <vector>  // for vector

#include <QStringList>

#include "common/patterns/singleton_pattern.h"  // for LazySingleton

#include "core/cluster_connection_settings.h"
#include "core/connection_settings.h"  // for IClusterSettingsBaseSPtr, etc
#include "core/sentinel_connection_settings.h"

#include "global/types.h"  // for supportedViews

namespace fastonosql {
namespace core {

class SettingsManager : public common::patterns::LazySingleton<SettingsManager> {
 public:
  typedef std::vector<IConnectionSettingsBaseSPtr> connection_settings_t;
  typedef std::vector<IClusterSettingsBaseSPtr> cluster_settings_t;
  typedef std::vector<ISentinelSettingsBaseSPtr> sentinel_settings_t;
  friend class common::patterns::LazySingleton<SettingsManager>;

  static std::string settingsDirPath();
  static std::string settingsFilePath();

  uint32_t configVersion() const;

  bool isSendedStatistic() const;
  void setIsSendedStatistic(bool val);

  void setDefaultView(supportedViews view);
  supportedViews defaultView() const;

  QString currentStyle() const;
  void setCurrentStyle(const QString& style);

  QString currentFontName() const;
  void setCurrentFontName(const QString& font);

  QString currentLanguage() const;
  void setCurrentLanguage(const QString& lang);

  // connections
  void addConnection(IConnectionSettingsBaseSPtr connection);
  void removeConnection(IConnectionSettingsBaseSPtr connection);

  connection_settings_t connections() const;

  // sentinels
  void addSentinel(ISentinelSettingsBaseSPtr sentinel);
  void removeSentinel(ISentinelSettingsBaseSPtr sentinel);

  sentinel_settings_t sentinels() const;

  // clusters
  void addCluster(IClusterSettingsBaseSPtr cluster);
  void removeCluster(IClusterSettingsBaseSPtr cluster);

  cluster_settings_t clusters() const;

  void addRConnection(const QString& connection);
  void removeRConnection(const QString& connection);
  QStringList recentConnections() const;
  void clearRConnections();

  void setLoggingDirectory(const QString& dir);
  QString loggingDirectory() const;

  bool autoCheckUpdates() const;
  void setAutoCheckUpdates(bool isCheck);

  bool autoCompletion() const;
  void setAutoCompletion(bool enableAuto);

  bool autoOpenConsole() const;
  void setAutoOpenConsole(bool enableAuto);

  bool fastViewKeys() const;
  void setFastViewKeys(bool fastView);

  void reloadFromPath(const std::string& path, bool merge);

 private:
  void load();
  void save();

  SettingsManager();
  ~SettingsManager();

  uint32_t config_version_;
  bool sended_statistic_;
  supportedViews views_;
  QString cur_style_;
  QString cur_font_name_;
  QString cur_language_;
  connection_settings_t connections_;
  sentinel_settings_t sentinels_;
  cluster_settings_t clusters_;
  QStringList recent_connections_;
  QString logging_dir_;
  bool auto_check_update_;
  bool auto_completion_;
  bool auto_open_console_;
  bool fast_view_keys_;
};

}  // namespace core
}  // namespace fastonosql
