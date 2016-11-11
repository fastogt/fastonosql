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

#include <QWidget>

#include "core/connection_settings/iconnection_settings.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;

namespace fastonosql {
namespace gui {

class ConnectionBaseWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ConnectionBaseWidget(QWidget* parent = 0);

  virtual void syncControls(core::IConnectionSettingsBase* connection);
  virtual void retranslateUi();
  virtual bool validated() const;

  core::IConnectionSettingsBase* createConnection() const;

  QString connectionName() const;
  void setConnectionName(const QString& name);

  QString commandLine() const;
  void setCommandLine(const QString& line);

  QString connectionFolderText() const;
  void setConnectionFolderText(const QString& text);

  void setFolderEnabled(bool val);

  bool isLogging() const;
  void setLogging(bool logging);

  int loggingInterval() const;
  void setLoggingInterval(int val);

 private Q_SLOTS:
  void loggingStateChange(int value);

 protected:
  virtual void changeEvent(QEvent* ev);

  virtual core::IConnectionSettingsBase* createConnectionImpl(
      const core::connection_path_t& path) const = 0;

  QLabel* connectionNameLabel_;
  QLineEdit* connectionName_;

  QLabel* commandLineLabel_;
  QLineEdit* commandLine_;

  QLabel* folderLabel_;
  QLineEdit* connectionFolder_;

  QCheckBox* logging_;
  QSpinBox* loggingMsec_;
};

}  // namespace gui
}  // namespace fastonosql
