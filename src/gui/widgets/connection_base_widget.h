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

#include <QWidget>

#include "proxy/connection_settings/iconnection_settings.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;

namespace {
const QString trDBPath = QObject::tr("Database path:");
}

namespace fastonosql {
namespace gui {

class ConnectionBaseWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ConnectionBaseWidget(QWidget* parent = 0);

  virtual void syncControls(proxy::IConnectionSettingsBase* connection);
  virtual void retranslateUi();
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
  void addWidget(QWidget* widget);
  void addLayout(QLayout* layout);

  virtual void changeEvent(QEvent* ev) override;

  virtual proxy::IConnectionSettingsBase* createConnectionImpl(const proxy::connection_path_t& path) const = 0;

  QLabel* connectionNameLabel_;
  QLineEdit* connectionName_;

  QLabel* namespaceSeparatorLabel_;
  QComboBox* namespaceSeparator_;

  QLabel* delimiterLabel_;
  QComboBox* delimiter_;

  QLabel* folderLabel_;
  QLineEdit* connectionFolder_;

  QCheckBox* logging_;
  QSpinBox* loggingMsec_;
};

}  // namespace gui
}  // namespace fastonosql
