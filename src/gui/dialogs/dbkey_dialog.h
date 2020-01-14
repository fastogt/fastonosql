/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <vector>

#include <fastonosql/core/db_key.h>

#include "gui/dialogs/base_dialog.h"

namespace fastonosql {
namespace gui {

class KeyEditWidget;

class DbKeyDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  core::NDbKValue key() const;

 public Q_SLOTS:
  void accept() override;

 private Q_SLOTS:
  void changeType(common::Value::Type type);

 protected:
  DbKeyDialog(const QString& title,
              const QIcon& icon,
              std::vector<common::Value::Type> types,
              const core::NDbKValue& key,
              bool is_edit,
              QWidget* parent = Q_NULLPTR);

  void keyPressEvent(QKeyEvent* event) override;

 private:
  bool validateAndApply();

  KeyEditWidget* editor_;
  core::NDbKValue key_;
};

}  // namespace gui
}  // namespace fastonosql
