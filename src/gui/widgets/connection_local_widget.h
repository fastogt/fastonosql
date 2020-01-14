/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#pragma once

#include "gui/widgets/connection_base_widget.h"

#include "proxy/connection_settings/iconnection_settings_local.h"

namespace {
const QString trCreateDBIfMissing = QObject::tr("Create database");
const QString trComparator = QObject::tr("Comparator");
const QString trCompression = QObject::tr("Compression");
}  // namespace

namespace fastonosql {
namespace gui {

class IPathWidget;

class ConnectionLocalWidget : public ConnectionBaseWidget {
 public:
  typedef ConnectionBaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  void syncControls(proxy::IConnectionSettingsBase* connection) override;
  bool validated() const override;

 protected:
  explicit ConnectionLocalWidget(IPathWidget* path_widget, QWidget* parent = Q_NULLPTR);

  virtual proxy::IConnectionSettingsLocal* createConnectionLocalImpl(const proxy::connection_path_t& path) const = 0;

 private:
  proxy::IConnectionSettingsBase* createConnectionImpl(const proxy::connection_path_t& path) const override final;
  IPathWidget* const path_widget_;
};

class ConnectionLocalWidgetDirectoryPath : public ConnectionLocalWidget {
 public:
  typedef ConnectionLocalWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

 protected:
  ConnectionLocalWidgetDirectoryPath(const QString& path_title, const QString& caption, QWidget* parent = Q_NULLPTR);
  proxy::IConnectionSettingsLocal* createConnectionLocalImpl(const proxy::connection_path_t& path) const override = 0;
};

class ConnectionLocalWidgetFilePath : public ConnectionLocalWidget {
 public:
  typedef ConnectionLocalWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

 protected:
  ConnectionLocalWidgetFilePath(const QString& path_title,
                                const QString& filter,
                                const QString& caption,
                                QWidget* parent = Q_NULLPTR);
  proxy::IConnectionSettingsLocal* createConnectionLocalImpl(const proxy::connection_path_t& path) const override = 0;
};

}  // namespace gui
}  // namespace fastonosql
