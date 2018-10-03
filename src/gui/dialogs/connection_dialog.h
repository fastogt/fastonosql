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

#include <vector>

#include <QDialog>

class QDialogButtonBox;

#include <fastonosql/core/connection_types.h>  // for connectionTypes

#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr, etc

namespace fastonosql {
namespace gui {

class ConnectionBaseWidget;

class ConnectionDialog : public QDialog {
  Q_OBJECT
 public:
  ConnectionDialog(core::connectionTypes type, const QString& connectionName, QWidget* parent = Q_NULLPTR);
  ConnectionDialog(proxy::IConnectionSettingsBase* connection, QWidget* parent = Q_NULLPTR);

  void setFolderEnabled(bool val);
  proxy::IConnectionSettingsBaseSPtr connection() const;

 public Q_SLOTS:
  virtual void accept() override;

 private Q_SLOTS:
  void testConnection();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void init(proxy::IConnectionSettingsBase* connection);

  void retranslateUi();
  bool validateAndApply();

  proxy::IConnectionSettingsBaseSPtr connection_;
  ConnectionBaseWidget* connection_widget_;

  QPushButton* test_button_;
  QDialogButtonBox* button_box_;
};

}  // namespace gui
}  // namespace fastonosql
