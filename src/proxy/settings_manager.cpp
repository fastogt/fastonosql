/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "proxy/settings_manager.h"

#include <string>

#include <QSettings>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>
#include <common/utils.h>

#include <common/file_system/file_system.h>
#include <common/file_system/string_path_utils.h>
#include <common/qt/gui/app_style.h>
#include <common/qt/translations/translations.h>

#include "proxy/connection_settings/iconnection_settings.h"
#include "proxy/connection_settings_factory.h"
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
#include "proxy/cluster_connection_settings_factory.h"
#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/isentinel_connection_settings.h"
#include "proxy/sentinel_connection_settings_factory.h"
#endif

#define SHARE_PATH_RELATIVE "share/resources"
#define MODULE_PATH_RELATIVE "modules"
#define CONVERTERS_PATH_RELATIVE "converters"

#define PREFIX "settings/"

#define LANGUAGE PREFIX "language"
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
#define AUTOCONNECTDB PREFIX "auto_connect_db"
#define WINDOW_SETTINGS PREFIX "window_settings"
#define SEND_STATISTIC PREFIX "send_statistic"
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
#define LAST_LOGIN PREFIX "last_login"
#define LAST_PASSWORD_HASH PREFIX "last_password"
#endif
#define SHOW_WELCOME_PAGE PREFIX "show_welcome_page"
#define PYTHON_PATH PREFIX "python_path"
#define CONFIG_VERSION PREFIX "version"

#if defined(OS_WIN)
#define PYTHON_FILE_NAME "python.exe"
#else
#define PYTHON_FILE_NAME "python"
#endif

namespace {

#if defined(PRO_VERSION)
const char kIniPath[] = "~/.config/" PROJECT_NAME "/config_new_pro.ini";
#elif defined(ENTERPRISE_VERSION)
const char kIniPath[] = "~/.config/" PROJECT_NAME "/config_new_enterprise.ini";
#else
const char kIniPath[] = "~/.config/" PROJECT_NAME "/config_new.ini";
#endif

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
      accepted_eula_(),
      views_(),
      cur_style_(),
      cur_font_(),
      cur_language_(),
      send_statistic_(),
      connections_(),
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
      sentinels_(),
      clusters_(),
      last_login_(),
      last_password_(),
      user_info_(),
#endif
      recent_connections_(),
      show_welcome_page_(),
      auto_check_updates_(),
      auto_completion_(),
      auto_open_console_(),
      auto_connect_db_(),
      window_settings_(),
      python_path_() {
}

SettingsManager::~SettingsManager() {}

std::string SettingsManager::GetSettingsDirPath() {
  return common::file_system::get_dir_path(kIniPath);
}

std::string SettingsManager::GetSettingsFilePath() {
  return common::file_system::prepare_path(kIniPath);
}

std::string SettingsManager::GetSourceDirPath() {
  return common::file_system::absolute_path_from_relative(RELATIVE_SOURCE_DIR, common::file_system::app_pwd());
}

std::string SettingsManager::GetShareDirPath() {
  const std::string absolute_source_dir = GetSourceDirPath();
  return common::file_system::make_path(absolute_source_dir, SHARE_PATH_RELATIVE);
}

std::string SettingsManager::GetModulesPath() {
  const std::string absolute_source_dir = GetShareDirPath();
  return common::file_system::make_path(absolute_source_dir, MODULE_PATH_RELATIVE);
}

std::string SettingsManager::GetConvertersPath() {
  const std::string modules_path = GetModulesPath();
  return common::file_system::make_path(modules_path, CONVERTERS_PATH_RELATIVE);
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

SupportedView SettingsManager::GetDefaultView() const {
  return views_;
}

void SettingsManager::SetDefaultView(SupportedView view) {
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
    DNOTREACHED();
    return;
  }

  auto it = std::find(connections_.begin(), connections_.end(), connection);
  if (it == connections_.end()) {
    connections_.push_back(connection);
  }
}

void SettingsManager::RemoveConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    DNOTREACHED();
    return;
  }

  connections_.erase(std::remove(connections_.begin(), connections_.end(), connection));
}

SettingsManager::connection_settings_t SettingsManager::GetConnections() const {
  return connections_;
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
void SettingsManager::AddSentinel(ISentinelSettingsBaseSPtr sentinel) {
  if (!sentinel) {
    DNOTREACHED();
    return;
  }

  auto it = std::find(sentinels_.begin(), sentinels_.end(), sentinel);
  if (it == sentinels_.end()) {
    sentinels_.push_back(sentinel);
  }
}

void SettingsManager::RemoveSentinel(ISentinelSettingsBaseSPtr sentinel) {
  if (!sentinel) {
    DNOTREACHED();
    return;
  }

  sentinels_.erase(std::remove(sentinels_.begin(), sentinels_.end(), sentinel));
}

SettingsManager::sentinel_settings_t SettingsManager::GetSentinels() const {
  return sentinels_;
}

void SettingsManager::AddCluster(IClusterSettingsBaseSPtr cluster) {
  if (!cluster) {
    DNOTREACHED();
    return;
  }

  auto it = std::find(clusters_.begin(), clusters_.end(), cluster);
  if (it == clusters_.end()) {
    clusters_.push_back(cluster);
  }
}

void SettingsManager::RemoveCluster(IClusterSettingsBaseSPtr cluster) {
  if (!cluster) {
    DNOTREACHED();
    return;
  }

  clusters_.erase(std::remove(clusters_.begin(), clusters_.end(), cluster));
}

SettingsManager::cluster_settings_t SettingsManager::GetClusters() const {
  return clusters_;
}
#endif

void SettingsManager::AddRecentConnection(const QString& connection) {
  if (connection.isEmpty()) {
    return;
  }

  if (!recent_connections_.contains(connection)) {
    recent_connections_.push_front(connection);
  }
}

void SettingsManager::RemoveRecentConnection(const QString& connection) {
  if (connection.isEmpty()) {
    return;
  }

  recent_connections_.removeOne(connection);
}

QStringList SettingsManager::GetRecentConnections() const {
  return recent_connections_;
}

void SettingsManager::ClearRConnections() {
  recent_connections_.clear();
}

QString SettingsManager::GetLoggingDirectory() const {
  QString path;
  common::ConvertFromString(ConnectionSettingsFactory::GetInstance().GetLoggingDirectory(), &path);
  return path;
}

void SettingsManager::SetLoggingDirectory(const QString& dir) {
  std::string dir_str = common::ConvertToString(dir);
  ConnectionSettingsFactory::GetInstance().SetLoggingDirectory(dir_str);
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

QByteArray SettingsManager::GetMainWindowSettings() const {
  return window_settings_;
}

void SettingsManager::SetMainWindowSettings(const QByteArray& settings) {
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
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
    sentinels_.clear();
    clusters_.clear();
#endif
    connections_.clear();
    recent_connections_.clear();
  }

  QString ini_path;
  common::ConvertFromString(common::file_system::prepare_path(path), &ini_path);
  QSettings settings(ini_path, QSettings::IniFormat);
  DCHECK(settings.status() == QSettings::NoError);

  cur_style_ = settings.value(STYLE, common::qt::gui::defStyle).toString();
  accepted_eula_ = settings.value(ACCEPTED_EULA, false).toBool();
  QFont font = default_font();
  cur_font_ = settings.value(FONT, font).value<QFont>();
  cur_language_ = settings.value(LANGUAGE, common::qt::translations::defLanguage).toString();
  send_statistic_ = settings.value(SEND_STATISTIC, true).toBool();

  int view = settings.value(VIEW, kText).toInt();
  views_ = static_cast<SupportedView>(view);

  const std::string dir_path = GetSettingsDirPath();
  QString qdir;
  common::ConvertFromString(dir_path, &qdir);
  const QString logging_dir = settings.value(LOGGINGDIR, qdir).toString();
  SetLoggingDirectory(logging_dir);

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  const QList<QVariant> clusters = settings.value(CLUSTERS).toList();
  for (const auto& cluster : clusters) {
    QString string = cluster.toString();
    common::char_buffer_t raw;
    if (common::utils::base64::decode64(common::ConvertToCharBytes(string), &raw)) {
      IClusterSettingsBaseSPtr sett(ClusterConnectionSettingsFactory::GetInstance().CreateFromStringCluster(raw));
      if (sett) {
        clusters_.push_back(sett);
      }
    }
  }

  const QList<QVariant> sentinels = settings.value(SENTINELS).toList();
  for (const auto& sentinel : sentinels) {
    QString string = sentinel.toString();
    common::char_buffer_t raw;
    if (common::utils::base64::decode64(common::ConvertToCharBytes(string), &raw)) {
      ISentinelSettingsBase* sentinel_settings =
          SentinelConnectionSettingsFactory::GetInstance().CreateFromStringSentinel(raw);
      if (sentinel_settings) {
        sentinels_.push_back(ISentinelSettingsBaseSPtr(sentinel_settings));
      }
    }
  }
  last_login_ = settings.value(LAST_LOGIN, QString()).toString();
  last_password_ = settings.value(LAST_PASSWORD_HASH, QString()).toString();
#endif

  const QList<QVariant> connections = settings.value(CONNECTIONS).toList();
  for (const auto& connection : connections) {
    QString string = connection.toString();
    common::char_buffer_t raw;
    if (common::utils::base64::decode64(common::ConvertToCharBytes(string), &raw)) {
      IConnectionSettingsBase* sett = ConnectionSettingsFactory::GetInstance().CreateSettingsFromString(raw);
      if (sett) {
        connections_.push_back(IConnectionSettingsBaseSPtr(sett));
      }
    }
  }

  const QStringList rconnections = settings.value(RCONNECTIONS).toStringList();
  for (const auto& rconnection : rconnections) {
    recent_connections_.push_back(rconnection);
  }

  show_welcome_page_ = settings.value(SHOW_WELCOME_PAGE, true).toBool();
  auto_check_updates_ = settings.value(CHECKUPDATES, true).toBool();
  auto_completion_ = settings.value(AUTOCOMPLETION, true).toBool();
  auto_open_console_ = settings.value(AUTOOPENCONSOLE, true).toBool();
  auto_connect_db_ = settings.value(AUTOCONNECTDB, true).toBool();
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
  ReloadFromPath(kIniPath, false);
}

void SettingsManager::Save() {
  QString qsave;
  common::ConvertFromString(GetSettingsFilePath(), &qsave);
  QSettings settings(qsave, QSettings::IniFormat);
  DCHECK(settings.status() == QSettings::NoError);

  settings.setValue(STYLE, cur_style_);
  settings.setValue(FONT, cur_font_);
  settings.setValue(ACCEPTED_EULA, accepted_eula_);
  settings.setValue(LANGUAGE, cur_language_);
  settings.setValue(SEND_STATISTIC, send_statistic_);
  settings.setValue(VIEW, views_);

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  QList<QVariant> clusters;
  for (const auto& cluster : clusters_) {
    if (cluster) {
      const serialize_t raw = ClusterConnectionSettingsFactory::GetInstance().ConvertSettingsToString(cluster.get());
      serialize_t enc;
      if (common::utils::base64::encode64(raw, &enc)) {
        QString qdata;
        common::ConvertFromBytes(enc, &qdata);
        clusters.push_back(qdata);
      }
    }
  }
  settings.setValue(CLUSTERS, clusters);

  QList<QVariant> sentinels;
  for (const auto& sentinel : sentinels_) {
    if (sentinel) {
      const serialize_t raw = SentinelConnectionSettingsFactory::GetInstance().ConvertSettingsToString(sentinel.get());
      serialize_t enc;
      if (common::utils::base64::encode64(raw, &enc)) {
        QString qdata;
        common::ConvertFromBytes(enc, &qdata);
        sentinels.push_back(qdata);
      }
    }
  }
  settings.setValue(SENTINELS, sentinels);
#endif

  QList<QVariant> connections;
  for (const auto& connection : connections_) {
    if (connection) {
      const serialize_t raw = ConnectionSettingsFactory::GetInstance().ConvertSettingsToString(connection.get());
      serialize_t enc;
      if (common::utils::base64::encode64(raw, &enc)) {
        QString qdata;
        common::ConvertFromBytes(enc, &qdata);
        connections.push_back(qdata);
      }
    }
  }
  settings.setValue(CONNECTIONS, connections);

  QStringList rconnections;
  for (const auto& rconnection : recent_connections_) {
    if (!rconnection.isEmpty()) {
      rconnections.push_back(rconnection);
    }
  }
  settings.setValue(RCONNECTIONS, rconnections);

  const QString logging_dir = GetLoggingDirectory();
  settings.setValue(LOGGINGDIR, logging_dir);
  settings.setValue(SHOW_WELCOME_PAGE, show_welcome_page_);
  settings.setValue(CHECKUPDATES, auto_check_updates_);
  settings.setValue(AUTOCOMPLETION, auto_completion_);
  settings.setValue(AUTOOPENCONSOLE, auto_open_console_);
  settings.setValue(AUTOCONNECTDB, auto_connect_db_);
  settings.setValue(WINDOW_SETTINGS, window_settings_);
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  settings.setValue(LAST_LOGIN, last_login_);
  settings.setValue(LAST_PASSWORD_HASH, last_password_);
#endif
  settings.setValue(PYTHON_PATH, python_path_);
  settings.setValue(CONFIG_VERSION, config_version_);
}

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
QString SettingsManager::GetLastLogin() const {
  return last_login_;
}

void SettingsManager::SetLastLogin(const QString& login) {
  last_login_ = login;
}

QString SettingsManager::GetLastPassword() const {
  return last_password_;
}

void SettingsManager::SetLastPassword(const QString& hash) {
  last_password_ = hash;
}

UserInfo SettingsManager::GetUserInfo() const {
  return user_info_;
}

void SettingsManager::SetUserInfo(const UserInfo& uinfo) {
  user_info_ = uinfo;
}
#endif

bool SettingsManager::GetShowWelcomePage() const {
  return show_welcome_page_;
}

void SettingsManager::SetShowWelcomePage(bool show) {
  show_welcome_page_ = show;
}

}  // namespace proxy
}  // namespace fastonosql
