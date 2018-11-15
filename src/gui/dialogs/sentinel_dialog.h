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

#include "gui/dialogs/base_dialog.h"

#include "proxy/connection_settings/iconnection_settings.h"  // for IConnectionSettingsBaseSPtr, etc
#include "proxy/connection_settings/isentinel_connection_settings.h"

class QCheckBox;         // lines 29-29
class QComboBox;         // lines 25-25
class QDialogButtonBox;  // lines 27-27
class QLabel;            // lines 33-33
class QLineEdit;         // lines 26-26
class QPushButton;       // lines 28-28
class QSpinBox;          // lines 30-30
class QToolBar;          // lines 32-32
class QTreeWidget;       // lines 31-31

namespace fastonosql {
namespace gui {

class SentinelDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  proxy::ISentinelSettingsBaseSPtr connection() const;

 public Q_SLOTS:
  void accept() override;

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
  explicit SentinelDialog(proxy::ISentinelSettingsBase* connection,
                          QWidget* parent = Q_NULLPTR);  // take ownerships connection

  void retranslateUi() override;

 private:
  bool validateAndApply();  // always return true and init
                            // sentinel_connection_
  void addSentinel(proxy::SentinelSettings sent);

  proxy::ISentinelSettingsBaseSPtr sentinel_connection_;
  QLineEdit* connection_name_;
  QLabel* folder_label_;
  QLineEdit* connection_folder_;
  QComboBox* type_connection_;
  QCheckBox* logging_;
  QSpinBox* logging_msec_;

  QToolBar* savebar_;
  QTreeWidget* list_widget_;

  QPushButton* test_button_;
  QPushButton* discovery_button_;
  QDialogButtonBox* button_box_;
};

}  // namespace gui
}  // namespace fastonosql
