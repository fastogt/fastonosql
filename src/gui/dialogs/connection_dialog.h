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

#include <QDialog>

class QDialogButtonBox;

#include "core/connection_types.h"  // for connectionTypes
#include "core/ssh_info.h"          // for SSHInfo, etc

#include "core/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr, etc

namespace fastonosql {
namespace gui {

class ConnectionAdvancedWidget;
class ConnectionBasicWidget;
class ConnectionSSHWidget;

class ConnectionDialog : public QDialog {
  Q_OBJECT
 public:
  ConnectionDialog(QWidget* parent,
                   core::IConnectionSettingsBase* connection = nullptr,
                   const std::vector<core::connectionTypes>& availibleTypes =
                       std::vector<core::connectionTypes>(),
                   const QString& connectionName = "New Connection");  // get ownerships connection

  void setFolderEnabled(bool val);
  core::IConnectionSettingsBaseSPtr connection() const;

 public Q_SLOTS:
  virtual void accept() override;

 private Q_SLOTS:
  void typeConnectionChange(core::connectionTypes type);
  void testConnection();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();
  bool validateAndApply();

  core::IConnectionSettingsBaseSPtr connection_;

  ConnectionAdvancedWidget* advanced_widget_;
  ConnectionBasicWidget* basic_widget_;
  ConnectionSSHWidget* ssh_widget_;

  QPushButton* testButton_;
  QDialogButtonBox* buttonBox_;
};

}  // namespace gui
}  // namespace fastonosql
