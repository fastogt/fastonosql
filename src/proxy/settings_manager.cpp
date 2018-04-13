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

#include "proxy/settings_manager.h"

#include <QSettings>

#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/utils.h>              // for decode64, encode64

#include <common/file_system/file_system.h>
#include <common/qt/gui/app_style.h>              // for defStyle
#include <common/qt/translations/translations.h>  // for defLanguage

#include "proxy/cluster_connection_settings_factory.h"
#include "proxy/connection_settings_factory.h"
#include "proxy/sentinel_connection_settings_factory.h"

#define PREFIX "settings/"

#define LANGUAGE PREFIX "language"
#define SEND_STATISTIC PREFIX "send_statistic"
#define ACCEPTED_EULA PREFIX "accepted_eula"
#define STYLE PREFIX "style"
#define FONT PREFIX "font"
#define CONNECTIONS PREFIX "connections"
#define SENTINELS PREFIX "sentinels"
#define CLUSTERS PREFIX "clusters"
#define VIEW PREFIX "view"
#define LOGGINGDIR PREFIX "logging_dir"
#define CHECKUPDATES PREFIX "auto_check_updates"
#define AUTOCOMPLETION PREFIX "auto_completion"
#define RCONNECTIONS PREFIX "rconnections"
#define AUTOOPENCONSOLE PREFIX "auto_open_console"
#define AUTOCONNECTDB "auto_connect_db"
#define FASTVIEWKEYS PREFIX "fast_view_keys"
#define WINDOW_SETTINGS PREFIX "window_settings"
#define PYTHON_PATH PREFIX "python_path"
#define CONFIG_VERSION PREFIX "version"

#ifdef OS_WIN
#define PYTHON_FILE_NAME "python.exe"
#else
#define PYTHON_FILE_NAME "python"
#endif

namespace {

const std::string ini_path("~/.config/" PROJECT_NAME "/config.ini");

QFont default_font() {
  /*#if defined(OS_MACOSX) || defined(OS_FREEBSD)
    return "Monaco";
  #elif defined(OS_LINUX) || defined(OS_ANDROID)
    return "Monospace";
  #elif defined(OS_WIN)
    return "Courier";
  #endif*/
  return QFont();
}

}  // namespace

namespace fastonosql {
namespace proxy {

SettingsManager::SettingsManager()
    : config_version_(),
      send_statistic_(),
      accepted_eula_(),
      views_(),
      cur_style_(),
      cur_font_(),
      cur_language_(),
      connections_(),
      sentinels_(),
      clusters_(),
      recent_connections_(),
      logging_dir_(),
      auto_check_updates_(),
      auto_completion_(),
      auto_open_console_(),
      fast_view_keys_(),
      window_settings_(),
      python_path_(),
      user_info_() {}

SettingsManager::~SettingsManager() {}

std::string SettingsManager::GetSettingsDirPath() {
  return common::file_system::get_dir_path(ini_path);
}

std::string SettingsManager::GetSettingsFilePath() {
  return common::file_system::prepare_path(ini_path);
}

uint32_t SettingsManager::GetConfigVersion() const {
  return config_version_;
}

bool SettingsManager::GetAccpetedEula() const {
  return accepted_eula_;
}

void SettingsManager::SetAccpetedEula(bool val) {
  accepted_eula_ = val;
}

bool SettingsManager::GetSendStatistic() const {
  return send_statistic_;
}
void SettingsManager::SetSendStatistic(bool val) {
  send_statistic_ = val;
}

supportedViews SettingsManager::GetDefaultView() const {
  return views_;
}

void SettingsManager::SetDefaultView(supportedViews view) {
  views_ = view;
}

QString SettingsManager::GetCurrentStyle() const {
  return cur_style_;
}

void SettingsManager::SetCurrentStyle(const QString& st) {
  cur_style_ = st;
}

QFont SettingsManager::GetCurrentFont() const {
  return cur_font_;
}

void SettingsManager::SetCurrentFont(const QFont& font) {
  cur_font_ = font;
}

QString SettingsManager::GetCurrentLanguage() const {
  return cur_language_;
}

void SettingsManager::SetCurrentLanguage(const QString& lang) {
  cur_language_ = lang;
}

void SettingsManager::AddConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    return;
  }

  auto it = std::find(connections_.begin(), connections_.end(), connection);
  if (it == connections_.end()) {
    connections_.push_back(connection);
  }
}

void SettingsManager::RemoveConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    return;
  }

  connections_.erase(std::remove(connections_.begin(), connections_.end(), connection));
}

SettingsManager::connection_settings_t SettingsManager::GetConnections() const {
  return connections_;
}

void SettingsManager::AddSentinel(ISentinelSettingsBaseSPtr sentinel) {
  if (!sentinel) {
    return;
  }

  auto it = std::find(sentinels_.begin(), sentinels_.end(), sentinel);
  if (it == sentinels_.end()) {
    sentinels_.push_back(sentinel);
  }
}

void SettingsManager::RemoveSentinel(ISentinelSettingsBaseSPtr sentinel) {
  if (!sentinel) {
    return;
  }

  sentinels_.erase(std::remove(sentinels_.begin(), sentinels_.end(), sentinel));
}

SettingsManager::sentinel_settings_t SettingsManager::GetSentinels() const {
  return sentinels_;
}

void SettingsManager::AddCluster(IClusterSettingsBaseSPtr cluster) {
  if (!cluster) {
    return;
  }

  auto it = std::find(clusters_.begin(), clusters_.end(), cluster);
  if (it == clusters_.end()) {
    clusters_.push_back(cluster);
  }
}

void SettingsManager::RemoveCluster(IClusterSettingsBaseSPtr cluster) {
  if (!cluster) {
    return;
  }

  clusters_.erase(std::remove(clusters_.begin(), clusters_.end(), cluster));
}

SettingsManager::cluster_settings_t SettingsManager::GetClusters() const {
  return clusters_;
}

void SettingsManager::AddRConnection(const QString& connection) {
  if (!connection.isEmpty()) {
    auto it = std::find(recent_connections_.begin(), recent_connections_.end(), connection);
    if (it == recent_connections_.end()) {
      recent_connections_.push_front(connection);
    }
  }
}

void SettingsManager::RemoveRConnection(const QString& connection) {
  if (!connection.isEmpty()) {
    auto it = std::find(recent_connections_.begin(), recent_connections_.end(), connection);
    if (it != recent_connections_.end()) {
      recent_connections_.erase(it);
    }
  }
}

QStringList SettingsManager::GetRecentConnections() const {
  return recent_connections_;
}

void SettingsManager::ClearRConnections() {
  recent_connections_.clear();
}

QString SettingsManager::GetLoggingDirectory() const {
  return logging_dir_;
}

void SettingsManager::SetLoggingDirectory(const QString& dir) {
  logging_dir_ = dir;
}

bool SettingsManager::GetAutoCheckUpdates() const {
  return auto_check_updates_;
}

void SettingsManager::SetAutoCheckUpdates(bool check) {
  auto_check_updates_ = check;
}

bool SettingsManager::GetAutoCompletion() const {
  return auto_completion_;
}

void SettingsManager::SetAutoCompletion(bool completion) {
  auto_completion_ = completion;
}

bool SettingsManager::AutoOpenConsole() const {
  return auto_open_console_;
}

void SettingsManager::SetAutoOpenConsole(bool open_console) {
  auto_open_console_ = open_console;
}

bool SettingsManager::GetAutoConnectDB() const {
  return auto_connect_db_;
}

void SettingsManager::SetAutoConnectDB(bool open_db) {
  auto_connect_db_ = open_db;
}

bool SettingsManager::GetFastViewKeys() const {
  return fast_view_keys_;
}

void SettingsManager::SetFastViewKeys(bool fast_view) {
  fast_view_keys_ = fast_view;
}

QByteArray SettingsManager::GetWindowSettings() const {
  return window_settings_;
}

void SettingsManager::SetWindowSettings(const QByteArray& settings) {
  window_settings_ = settings;
}

QString SettingsManager::GetPythonPath() const {
  return python_path_;
}

void SettingsManager::SetPythonPath(const QString& path) {
  python_path_ = path;
}

void SettingsManager::ReloadFromPath(const std::string& path, bool merge) {
  if (path.empty()) {
    return;
  }

  if (!merge) {
    clusters_.clear();
    connections_.clear();
    recent_connections_.clear();
  }

  QString inip;
  common::ConvertFromString(common::file_system::prepare_path(path), &inip);
  QSettings settings(inip, QSettings::IniFormat);
  DCHECK(settings.status() == QSettings::NoError);

  cur_style_ = settings.value(STYLE, common::qt::gui::defStyle).toString();
  send_statistic_ = settings.value(SEND_STATISTIC, true).toBool();
  accepted_eula_ = settings.value(ACCEPTED_EULA, false).toBool();
  QFont font = default_font();
  cur_font_ = settings.value(FONT, font).value<QFont>();
  cur_language_ = settings.value(LANGUAGE, common::qt::translations::defLanguage).toString();

  int view = settings.value(VIEW, kText).toInt();
  views_ = static_cast<supportedViews>(view);

  QList<QVariant> clusters = settings.value(CLUSTERS).toList();
  for (const auto& cluster : clusters) {
    QString string = cluster.toString();
    std::string encoded = common::ConvertToString(string);
    std::string raw = common::utils::base64::decode64(encoded);

    IClusterSettingsBaseSPtr sett(ClusterConnectionSettingsFactory::GetInstance().CreateFromString(raw));
    if (sett) {
      clusters_.push_back(sett);
    }
  }

  QList<QVariant> sentinels = settings.value(SENTINELS).toList();
  for (const auto& sentinel : sentinels) {
    QString string = sentinel.toString();
    std::string encoded = common::ConvertToString(string);
    std::string raw = common::utils::base64::decode64(encoded);

    ISentinelSettingsBaseSPtr sett(SentinelConnectionSettingsFactory::GetInstance().CreateFromString(raw));
    if (sett) {
      sentinels_.push_back(sett);
    }
  }

  QList<QVariant> connections = settings.value(CONNECTIONS).toList();
  for (const auto& connection : connections) {
    QString string = connection.toString();
    std::string encoded = common::ConvertToString(string);
    std::string raw = common::utils::base64::decode64(encoded);

    IConnectionSettingsBaseSPtr sett(ConnectionSettingsFactory::GetInstance().CreateFromString(raw));
    if (sett) {
      connections_.push_back(sett);
    }
  }

  QStringList rconnections = settings.value(RCONNECTIONS).toStringList();
  for (const auto& rconnection : rconnections) {
    std::string encoded = common::ConvertToString(rconnection);
    std::string raw = common::utils::base64::decode64(encoded);

    QString qdata;
    if (common::ConvertFromString(raw, &qdata) && !qdata.isEmpty()) {
      recent_connections_.push_back(qdata);
    }
  }

  std::string dir_path = GetSettingsDirPath();
  QString qdir;
  common::ConvertFromString(dir_path, &qdir);
  logging_dir_ = settings.value(LOGGINGDIR, qdir).toString();
  auto_check_updates_ = settings.value(CHECKUPDATES, true).toBool();
  auto_completion_ = settings.value(AUTOCOMPLETION, true).toBool();
  auto_open_console_ = settings.value(AUTOOPENCONSOLE, true).toBool();
  auto_connect_db_ = settings.value(AUTOCONNECTDB, true).toBool();
  fast_view_keys_ = settings.value(FASTVIEWKEYS, true).toBool();
  window_settings_ = settings.value(WINDOW_SETTINGS, QByteArray()).toByteArray();

  QString qpython_path;
  std::string python_path;
  if (common::file_system::find_file_in_path(PYTHON_FILE_NAME, &python_path) &&
      common::ConvertFromString(python_path, &qpython_path)) {
  }
  python_path_ = settings.value(PYTHON_PATH, qpython_path).toString();
  config_version_ = settings.value(CONFIG_VERSION, PROJECT_VERSION_NUMBER).toUInt();
}

void SettingsManager::Load() {
  ReloadFromPath(ini_path, false);
}

void SettingsManager::Save() {
  QString qsave;
  common::ConvertFromString(GetSettingsFilePath(), &qsave);
  QSettings settings(qsave, QSettings::IniFormat);
  DCHECK(settings.status() == QSettings::NoError);

  settings.setValue(STYLE, cur_style_);
  settings.setValue(FONT, cur_font_);
  settings.setValue(SEND_STATISTIC, send_statistic_);
  settings.setValue(ACCEPTED_EULA, accepted_eula_);
  settings.setValue(LANGUAGE, cur_language_);
  settings.setValue(VIEW, views_);

  QList<QVariant> clusters;
  for (const auto& cluster : clusters_) {
    if (cluster) {
      std::string raw = cluster->ToString();
      std::string enc = common::utils::base64::encode64(raw);
      QString qdata;
      common::ConvertFromString(enc, &qdata);
      clusters.push_back(qdata);
    }
  }
  settings.setValue(CLUSTERS, clusters);

  QList<QVariant> sentinels;
  for (const auto& sentinel : sentinels_) {
    if (sentinel) {
      std::string raw = sentinel->ToString();
      std::string enc = common::utils::base64::encode64(raw);
      QString qdata;
      common::ConvertFromString(enc, &qdata);
      sentinels.push_back(qdata);
    }
  }
  settings.setValue(SENTINELS, sentinels);

  QList<QVariant> connections;
  for (const auto& connection : connections_) {
    if (connection) {
      std::string raw = connection->ToString();
      std::string enc = common::utils::base64::encode64(raw);
      QString qdata;
      common::ConvertFromString(enc, &qdata);
      connections.push_back(qdata);
    }
  }
  settings.setValue(CONNECTIONS, connections);

  QStringList rconnections;
  for (const auto& rconnection : recent_connections_) {
    if (!rconnection.isEmpty()) {
      std::string raw = common::ConvertToString(rconnection);
      std::string enc = common::utils::base64::encode64(raw);
      QString qdata;
      common::ConvertFromString(enc, &qdata);
      rconnections.push_back(qdata);
    }
  }
  settings.setValue(RCONNECTIONS, rconnections);

  settings.setValue(LOGGINGDIR, logging_dir_);
  settings.setValue(CHECKUPDATES, auto_check_updates_);
  settings.setValue(AUTOCOMPLETION, auto_completion_);
  settings.setValue(AUTOOPENCONSOLE, auto_open_console_);
  settings.setValue(AUTOCONNECTDB, auto_connect_db_);
  settings.setValue(FASTVIEWKEYS, fast_view_keys_);
  settings.setValue(WINDOW_SETTINGS, window_settings_);
  settings.setValue(PYTHON_PATH, python_path_);
  settings.setValue(CONFIG_VERSION, config_version_);
}

UserInfo SettingsManager::GetUserInfo() const {
  return user_info_;
}

void SettingsManager::SetUserInfo(const UserInfo& uinfo) {
  user_info_ = uinfo;
}

}  // namespace proxy
}  // namespace fastonosql
