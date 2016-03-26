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

class QComboBox;
class QLineEdit;
class QDialogButtonBox;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QTreeWidget;
class QToolBar;

#include "core/connection_settings.h"

namespace fastonosql {
namespace gui {

class ClusterDialog
  : public QDialog {
  Q_OBJECT
 public:
  typedef std::vector<IConnectionSettingsBaseSPtr> cluster_connection_type;
  explicit ClusterDialog(QWidget* parent, IClusterSettingsBase* connection = nullptr);  // get ownerships connection
  IClusterSettingsBaseSPtr connection() const;

 public Q_SLOTS:
  virtual void accept();

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
  virtual void changeEvent(QEvent* ev);

 private:
  void retranslateUi();
  bool validateAndApply();
  void addConnection(IConnectionSettingsBaseSPtr con);

  IClusterSettingsBaseSPtr cluster_connection_;
  QLineEdit* connectionName_;
  QComboBox* typeConnection_;
  QCheckBox* logging_;
  QSpinBox* loggingMsec_;

  QToolBar* savebar_;
  QTreeWidget* listWidget_;

  QPushButton* testButton_;
  QPushButton* discoveryButton_;
  QDialogButtonBox* buttonBox_;
  QAction* setDefault_;
};

}  // namespace gui
}  // namespace fastonosql
