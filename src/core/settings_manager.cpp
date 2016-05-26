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

#include "core/settings_manager.h"

#include <string>

#include <QSettings>
#include <QFont>

#include "fasto/qt/translations/translations.h"
#include "fasto/qt/gui/app_style.h"

#include "common/file_system.h"
#include "common/qt/convert_string.h"
#include "common/utils.h"

#define PREFIX "settings/"

#define LANGUAGE PREFIX"language"
#define STYLE PREFIX"style"
#define FONT PREFIX"font"
#define CONNECTIONS PREFIX"connections"
#define SENTINELS PREFIX"sentinels"
#define CLUSTERS PREFIX"clusters"
#define VIEW PREFIX"view"
#define LOGGINGDIR PREFIX"loggingdir"
#define CHECKUPDATES PREFIX"checkupdates"
#define AUTOCOMPLETION PREFIX"autocompletion"
#define RCONNECTIONS PREFIX"rconnections"
#define AUTOOPENCONSOLE PREFIX"autoopenconsole"
#define FASTVIEWKEYS PREFIX"fastviewkeys"
#define CONFIG_VERSION PREFIX"version"

namespace {

const std::string iniPath("~/.config/" PROJECT_NAME "/conf.ini");

QString fontName() {
/*#if defined(OS_MACOSX) || defined(OS_FREEBSD)
  return "Monaco";
#elif defined(OS_LINUX) || defined(OS_ANDROID)
  return "Monospace";
#elif defined(OS_WIN)
  return "Courier";
#endif*/
  return QFont().defaultFamily();
}

}  // namespace


namespace fastonosql {
namespace core {

SettingsManager::SettingsManager()
  : config_version_(), views_(), cur_style_(), cur_font_name_(), cur_language_(), connections_(),
    logging_dir_(),
    auto_check_update_(), auto_completion_(), auto_open_console_(), fast_view_keys_() {
  load();
}


SettingsManager::~SettingsManager() {
  save();
}

std::string SettingsManager::settingsDirPath() {
  return common::file_system::get_dir_path(iniPath);
}

std::string SettingsManager::settingsFilePath() {
  return common::file_system::prepare_path(iniPath);
}

uint32_t SettingsManager::configVersion() const {
  return config_version_;
}

supportedViews SettingsManager::defaultView() const {
  return views_;
}

void SettingsManager::setDefaultView(supportedViews view) {
  views_ = view;
}

QString SettingsManager::currentStyle() const {
  return cur_style_;
}

void SettingsManager::setCurrentStyle(const QString& st) {
  cur_style_ = st;
}

QString SettingsManager::currentFontName() const {
  return cur_font_name_;
}

void SettingsManager::setCurrentFontName(const QString& font) {
  cur_font_name_ = font;
}

QString SettingsManager::currentLanguage() const {
  return cur_language_;
}

void SettingsManager::setCurrentLanguage(const QString& lang) {
  cur_language_ = lang;
}

void SettingsManager::addConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    return;
  }

  auto it = std::find(connections_.begin(), connections_.end(), connection);
  if (it == connections_.end()) {
    connections_.push_back(connection);
  }
}

void SettingsManager::removeConnection(IConnectionSettingsBaseSPtr connection) {
  if (!connection) {
    return;
  }

  connections_.erase(std::remove(connections_.begin(), connections_.end(), connection));
}

SettingsManager::ConnectionSettingsContainerType SettingsManager::connections() const {
  return connections_;
}

void SettingsManager::addSentinel(ISentinelSettingsBaseSPtr sentinel) {
  if (!sentinel) {
    return;
  }

  auto it = std::find(sentinels_.begin(), sentinels_.end(), sentinel);
  if (it == sentinels_.end()) {
    sentinels_.push_back(sentinel);
  }
}

void SettingsManager::removeSentinel(ISentinelSettingsBaseSPtr sentinel) {
  if (!sentinel) {
    return;
  }

  sentinels_.erase(std::remove(sentinels_.begin(), sentinels_.end(), sentinel));
}

SettingsManager::SentinelSettingsContainerType SettingsManager::sentinels() const {
  return sentinels_;
}

void SettingsManager::addCluster(IClusterSettingsBaseSPtr cluster) {
  if (!cluster) {
    return;
  }

  auto it = std::find(clusters_.begin(), clusters_.end(), cluster);
  if (it == clusters_.end()) {
    clusters_.push_back(cluster);
  }
}

void SettingsManager::removeCluster(IClusterSettingsBaseSPtr cluster) {
  if (!cluster) {
    return;
  }

  clusters_.erase(std::remove(clusters_.begin(), clusters_.end(), cluster));
}

SettingsManager::ClusterSettingsContainerType SettingsManager::clusters() const {
  return clusters_;
}

void SettingsManager::addRConnection(const QString& connection) {
  if (!connection.isEmpty()) {
    QStringList::iterator it = std::find(recent_connections_.begin(),
                                         recent_connections_.end(), connection);
    if (it == recent_connections_.end()) {
      recent_connections_.push_front(connection);
    }
  }
}

void SettingsManager::removeRConnection(const QString& connection) {
  if (!connection.isEmpty()) {
    QStringList::iterator it = std::find(recent_connections_.begin(),
                                         recent_connections_.end(), connection);
    if (it != recent_connections_.end()) {
      recent_connections_.erase(it);
    }
  }
}

QStringList SettingsManager::recentConnections() const {
  return recent_connections_;
}

void SettingsManager::clearRConnections() {
  recent_connections_.clear();
}

QString SettingsManager::loggingDirectory() const {
  return logging_dir_;
}

void SettingsManager::setLoggingDirectory(const QString& dir) {
  logging_dir_ = dir;
}

bool SettingsManager::autoCheckUpdates() const {
  return auto_check_update_;
}

void SettingsManager::setAutoCheckUpdates(bool isCheck) {
  auto_check_update_ = isCheck;
}

bool SettingsManager::autoCompletion() const {
  return auto_completion_;
}

void SettingsManager::setAutoCompletion(bool enableAuto) {
  auto_completion_ = enableAuto;
}

bool SettingsManager::autoOpenConsole() const {
  return auto_open_console_;
}

void SettingsManager::setAutoOpenConsole(bool enableAuto) {
  auto_open_console_ = enableAuto;
}

bool SettingsManager::fastViewKeys() const {
  return fast_view_keys_;
}

void SettingsManager::setFastViewKeys(bool fastView) {
  fast_view_keys_ = fastView;
}

void SettingsManager::reloadFromPath(const std::string& path, bool merge) {
  if (path.empty()) {
    return;
  }

  if (!merge) {
    clusters_.clear();
    connections_.clear();
    recent_connections_.clear();
  }

  QString inip = common::convertFromString<QString>(common::file_system::prepare_path(path));
  QSettings settings(inip, QSettings::IniFormat);
  DCHECK(settings.status() == QSettings::NoError);

  cur_style_ = settings.value(STYLE, fasto::qt::gui::defStyle).toString();
  cur_font_name_ = settings.value(FONT, fontName()).toString();
  cur_language_ = settings.value(LANGUAGE, fasto::qt::translations::defLanguage).toString();

  int view = settings.value(VIEW, Tree).toInt();
  views_ = static_cast<supportedViews>(view);

  auto clusters = settings.value(CLUSTERS).toList();
  for (auto it = clusters.begin(); it != clusters.end(); ++it) {
    QVariant var = *it;
    QString string = var.toString();
    std::string encoded = common::convertToString(string);
    std::string raw = common::utils::base64::decode64(encoded);

    IClusterSettingsBaseSPtr sett(IClusterSettingsBase::fromString(raw));
    if (sett) {
      clusters_.push_back(sett);
    }
  }

  auto sentinels = settings.value(SENTINELS).toList();
  for (auto it = sentinels.begin(); it != sentinels.end(); ++it) {
    QVariant var = *it;
    QString string = var.toString();
    std::string encoded = common::convertToString(string);
    std::string raw = common::utils::base64::decode64(encoded);

    ISentinelSettingsBaseSPtr sett(ISentinelSettingsBase::fromString(raw));
    if (sett) {
      sentinels_.push_back(sett);
    }
  }

  auto connections = settings.value(CONNECTIONS).toList();
  for (auto it = connections.begin(); it != connections.end(); ++it) {
    QVariant var = *it;
    QString string = var.toString();
    std::string encoded = common::convertToString(string);
    std::string raw = common::utils::base64::decode64(encoded);

    IConnectionSettingsBaseSPtr sett(IConnectionSettingsBase::fromString(raw));
    if (sett) {
      connections_.push_back(sett);
    }
  }

  QStringList rconnections = settings.value(RCONNECTIONS).toStringList();
  for (auto it = rconnections.begin(); it != rconnections.end(); ++it) {
    QString string = *it;
    std::string encoded = common::convertToString(string);
    std::string raw = common::utils::base64::decode64(encoded);

    QString qdata = common::convertFromString<QString>(raw);
    if (!qdata.isEmpty()) {
      recent_connections_.push_back(qdata);
    }
  }

  std::string dir_path = settingsDirPath();
  logging_dir_ = settings.value(LOGGINGDIR, common::convertFromString<QString>(dir_path)).toString();
  auto_check_update_ = settings.value(CHECKUPDATES, true).toBool();
  auto_completion_ = settings.value(AUTOCOMPLETION, true).toBool();
  auto_open_console_ = settings.value(AUTOOPENCONSOLE, true).toBool();
  fast_view_keys_ = settings.value(FASTVIEWKEYS, true).toBool();
  config_version_ = settings.value(CONFIG_VERSION, PROJECT_VERSION_NUMBER).toUInt();
}

void SettingsManager::load() {
  reloadFromPath(iniPath, false);
}

void SettingsManager::save() {
  QSettings settings(common::convertFromString<QString>(settingsFilePath()), QSettings::IniFormat);
  DCHECK(settings.status() == QSettings::NoError);

  settings.setValue(STYLE, cur_style_);
  settings.setValue(FONT, cur_font_name_);
  settings.setValue(LANGUAGE, cur_language_);
  settings.setValue(VIEW, views_);

  QList<QVariant> clusters;
  for (auto it = clusters_.begin(); it != clusters_.end(); ++it) {
    IClusterSettingsBaseSPtr conn = *it;
    if (conn) {
      std::string raw = conn->toString();
      std::string enc = common::utils::base64::encode64(raw);
      QString qdata = common::convertFromString<QString>(enc);
      clusters.push_back(qdata);
    }
  }
  settings.setValue(CLUSTERS, clusters);

  QList<QVariant> sentinels;
  for (auto it = sentinels_.begin(); it != sentinels_.end(); ++it) {
    ISentinelSettingsBaseSPtr conn = *it;
    if (conn) {
      std::string raw = conn->toString();
      std::string enc = common::utils::base64::encode64(raw);
      QString qdata = common::convertFromString<QString>(enc);
      sentinels.push_back(qdata);
    }
  }
  settings.setValue(SENTINELS, sentinels);

  QList<QVariant> connections;
  for (auto it = connections_.begin(); it != connections_.end(); ++it) {
    IConnectionSettingsBaseSPtr conn = *it;
    if (conn) {
      std::string raw = conn->toString();
      std::string enc = common::utils::base64::encode64(raw);
      QString qdata = common::convertFromString<QString>(enc);
      connections.push_back(qdata);
    }
  }
  settings.setValue(CONNECTIONS, connections);

  QStringList rconnections;
  for (auto it = recent_connections_.begin(); it != recent_connections_.end(); ++it) {
    QString conn = *it;
    if (!conn.isEmpty()) {
      std::string raw = common::convertToString(conn);
      std::string enc = common::utils::base64::encode64(raw);
      QString qdata = common::convertFromString<QString>(enc);
      rconnections.push_back(qdata);
    }
  }
  settings.setValue(RCONNECTIONS, rconnections);

  settings.setValue(LOGGINGDIR, logging_dir_);
  settings.setValue(CHECKUPDATES, auto_check_update_);
  settings.setValue(AUTOCOMPLETION, auto_completion_);
  settings.setValue(AUTOOPENCONSOLE, auto_open_console_);
  settings.setValue(FASTVIEWKEYS, fast_view_keys_);
  settings.setValue(CONFIG_VERSION, config_version_);
}

}  // namespace core
}  // namespace fastonosql
