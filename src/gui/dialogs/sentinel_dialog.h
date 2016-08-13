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

#include <vector>                       // for vector

#include <QDialog>

#include "core/connection_settings.h"   // for IConnectionSettingsBaseSPtr, etc

class QCheckBox;  // lines 29-29
class QComboBox;  // lines 25-25
class QDialogButtonBox;  // lines 27-27
class QEvent;
class QLabel;  // lines 33-33
class QLineEdit;  // lines 26-26
class QPushButton;  // lines 28-28
class QSpinBox;  // lines 30-30
class QToolBar;  // lines 32-32
class QTreeWidget;  // lines 31-31
class QWidget;

namespace fastonosql {
namespace gui {

class SentinelDialog
  : public QDialog {
  Q_OBJECT
 public:
  typedef std::vector<core::IConnectionSettingsBaseSPtr> sentinel_connection_t;
  explicit SentinelDialog(QWidget* parent, core::ISentinelSettingsBase* connection = nullptr);  // get ownerships connection
  core::ISentinelSettingsBaseSPtr connection() const;

 public Q_SLOTS:
  virtual void accept();

 private Q_SLOTS:
  void typeConnectionChange(int index);
  void loggingStateChange(int value);
  void testConnection();
  void discoverySentinel();
  void addConnectionSettings();
  void remove();
  void edit();
  void itemSelectionChanged();

 protected:
  virtual void changeEvent(QEvent* ev);

 private:
  void retranslateUi();
  bool validateAndApply();  // always return true and init sentinel_connection_
  void addSentinel(core::SentinelSettings sent);

  core::ISentinelSettingsBaseSPtr sentinel_connection_;
  QLineEdit* connectionName_;
  QLabel* folderLabel_;
  QLineEdit* connectionFolder_;
  QComboBox* typeConnection_;
  QCheckBox* logging_;
  QSpinBox* loggingMsec_;

  QToolBar* savebar_;
  QTreeWidget* listWidget_;

  QPushButton* testButton_;
  QPushButton* discoveryButton_;
  QDialogButtonBox* buttonBox_;
};

}  // namespace gui
}  // namespace fastonosql
