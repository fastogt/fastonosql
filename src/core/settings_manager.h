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

#include <vector>
#include <string>

#include <QStringList>

#include "global/types.h"

#include "common/patterns/singleton_pattern.h"

#include "core/connection_settings.h"

namespace fastonosql {

class SettingsManager
  : public common::patterns::LazySingleton<SettingsManager> {
 public:
  typedef std::vector<IConnectionSettingsBaseSPtr> ConnectionSettingsContainerType;
  typedef std::vector<IClusterSettingsBaseSPtr> ClusterSettingsContainerType;
  friend class common::patterns::LazySingleton<SettingsManager>;

  static QString settingsDirPath();
  static std::string settingsFilePath();

  void setDefaultView(supportedViews view);
  supportedViews defaultView() const;

  QString currentStyle() const;
  void setCurrentStyle(const QString &style);

  QString currentFontName() const;
  void setCurrentFontName(const QString& font);

  QString currentLanguage() const;
  void setCurrentLanguage(const QString &lang);

  // connections
  void addConnection(IConnectionSettingsBaseSPtr connection);
  void removeConnection(IConnectionSettingsBaseSPtr connection);

  ConnectionSettingsContainerType connections() const;

  // clusters
  void addCluster(IClusterSettingsBaseSPtr cluster);
  void removeCluster(IClusterSettingsBaseSPtr cluster);

  ClusterSettingsContainerType clusters() const;

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

  supportedViews views_;
  QString cur_style_;
  QString cur_font_name_;
  QString cur_language_;
  ConnectionSettingsContainerType connections_;
  ClusterSettingsContainerType clusters_;
  QStringList recent_connections_;
  QString logging_dir_;
  bool auto_check_update_;
  bool auto_completion_;
  bool auto_open_console_;
  bool fast_view_keys_;
};

}  // namespace fastonosql
