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

#include <fastonosql/core/connection_types.h>

#include "gui/dialogs/base_dialog.h"

class QComboBox;
class QLabel;
class QDialogButtonBox;

namespace fastonosql {
namespace gui {

class ConnectionSelectTypeDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  core::ConnectionType connectionType() const;

 protected:
  explicit ConnectionSelectTypeDialog(const QString& title, QWidget* parent = Q_NULLPTR);
  void retranslateUi() override;

  QLabel* type_connection_label_;
  QComboBox* type_connection_;
  QDialogButtonBox* button_box_;
};

}  // namespace gui
}  // namespace fastonosql
