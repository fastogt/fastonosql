/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

class QCheckBox;
class QComboBox;
class QFontComboBox;
class QSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;

namespace fastonosql {
namespace gui {

class IPathWidget;

class PreferencesDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  enum { min_width = 640, min_height = 480 };

 public Q_SLOTS:
  void accept() override;

 private:
  void syncWithSettings();

 protected:
  explicit PreferencesDialog(QWidget* parent = Q_NULLPTR);

  void retranslateUi() override;

 private:
  QWidget* createMainTab();
  QWidget* createExternalTab();

#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  // controls in profile_box
  QGroupBox* profile_box_;
  QLabel* first_name_label_;
  QLineEdit* first_name_text_;
  QLabel* last_name_label_;
  QLineEdit* last_name_text_;
  QLabel* login_label_;
  QLineEdit* login_text_;
  QLabel* type_label_;
  QLineEdit* type_text_;
#endif

  // controls in general_box
  QGroupBox* general_box_;
  QCheckBox* send_statitsic_;
  QCheckBox* auto_check_updates_;
  QCheckBox* auto_comletion_;
  QLabel* languages_label_;
  QComboBox* languages_combo_box_;

  QLabel* styles_label_;
  QComboBox* styles_combo_box_;
  QLabel* font_label_;
  QFontComboBox* font_combo_box_;
  QSpinBox* font_size_spin_box_;
  QLabel* default_view_label_;
  QComboBox* default_view_combo_box_;
  QLabel* log_dir_label_;
  QLineEdit* log_dir_path_;
  QCheckBox* auto_open_console_;
  QCheckBox* auto_connect_db_;
  QCheckBox* show_welcome_page_;

  QGroupBox* external_box_;
  IPathWidget* python_path_widget_;
  IPathWidget* modules_path_widget_;
};

}  // namespace gui
}  // namespace fastonosql
