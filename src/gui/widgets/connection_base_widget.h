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

#include "gui/widgets/base_widget.h"

#include "proxy/connection_settings/iconnection_settings.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;

namespace {
const QString trDBPath = QObject::tr("Database path:");
const QString trCaption = QObject::tr("Select database path");
const QString trFilter = QObject::tr("Database files (*)");
const QString trReadOnlyDB = QObject::tr("Read only database");
const QString trDBName = QObject::tr("Database name:");
const QString trUseAuth = QObject::tr("Use AUTH");
const QString trDefaultDb = QObject::tr("Default database:");
}  // namespace

namespace fastonosql {
namespace gui {

class ConnectionBaseWidget : public BaseWidget {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  virtual void syncControls(proxy::IConnectionSettingsBase* connection);
  virtual bool validated() const;

  proxy::IConnectionSettingsBase* createConnection() const;

  QString connectionName() const;
  void setConnectionName(const QString& name);

  QString UIFolderText() const;
  void setUIFolderText(const QString& text);

  void setUIFolderEnabled(bool val);

  bool isLogging() const;
  void setLogging(bool logging);

  int loggingInterval() const;
  void setLoggingInterval(int val);

 private Q_SLOTS:
  void loggingStateChange(int value);

 protected:
  explicit ConnectionBaseWidget(QWidget* parent = Q_NULLPTR);

  void retranslateUi() override;

  void addWidget(QWidget* widget);
  void addLayout(QLayout* layout);

  virtual proxy::IConnectionSettingsBase* createConnectionImpl(const proxy::connection_path_t& path) const = 0;

 private:
  QLabel* connection_name_label_;
  QLineEdit* connection_name_;

  QLabel* namespace_separator_label_;
  QComboBox* namespace_separator_;

  QLabel* namespace_displaying_strategy_label_;
  QComboBox* namespace_displaying_strategy_;

  QLabel* delimiter_label_;
  QComboBox* delimiter_;

  QLabel* folder_label_;
  QLineEdit* connection_folder_;

  QCheckBox* logging_;
  QSpinBox* logging_msec_;
};

}  // namespace gui
}  // namespace fastonosql
