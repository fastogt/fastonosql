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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "gui/dialogs/base_dialog.h"

#include "proxy/connection_settings/icluster_connection_settings.h"
#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr, etc

class QComboBox;
class QLineEdit;
class QDialogButtonBox;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QTreeWidget;
class QToolBar;
class QLabel;

namespace fastonosql {
namespace gui {

class ClusterDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  proxy::IClusterSettingsBaseSPtr connection() const;

 public Q_SLOTS:
  void accept() override;

 private Q_SLOTS:
  void typeConnectionChange(int index);
  void loggingStateChange(int value);
  void testConnection();
  void discoveryCluster();
  void showContextMenu(const QPoint& point);
  void setStartNode();
  void add();
  void remove();
  void edit();
  void itemSelectionChanged();

 protected:
  explicit ClusterDialog(proxy::IClusterSettingsBase* connection, QWidget* parent = Q_NULLPTR);

  void retranslateUi() override;

 private:
  QToolBar* createToolBar();

  bool validateAndApply();  // always return true and init
                            // cluster_connection_
  void addConnection(proxy::IConnectionSettingsBaseSPtr con);

  QLineEdit* connection_name_;
  QLabel* folder_label_;
  QLineEdit* connection_folder_;
  QComboBox* type_connection_;
  QCheckBox* logging_;
  QSpinBox* logging_msec_;

  QAction* add_action_;
  QAction* remove_action_;
  QAction* edit_action_;
  QTreeWidget* list_widget_;

  QPushButton* test_button_;
  QPushButton* discovery_button_;
  QDialogButtonBox* button_box_;

  proxy::IClusterSettingsBaseSPtr cluster_connection_;
};

}  // namespace gui
}  // namespace fastonosql
