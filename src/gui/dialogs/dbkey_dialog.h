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

#include <QDialog>

#include <fastonosql/core/connection_types.h>  // for ConnectionType
#include <fastonosql/core/db_key.h>            // for NDbKValue, NValue

namespace fastonosql {
namespace gui {

class KeyEditWidget;

class DbKeyDialog : public QDialog {
  Q_OBJECT
 public:
  typedef QDialog base_class;
  static const QSize min_dialog_size;

  DbKeyDialog(const QString& title,
              const QIcon& icon,
              core::ConnectionType type,
              const core::NDbKValue& key,
              bool is_edit,
              QWidget* parent = Q_NULLPTR);
  core::NDbKValue key() const;

 public Q_SLOTS:
  virtual void accept() override;

 private Q_SLOTS:
  void changeType(common::Value::Type type);

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  bool validateAndApply();
  void retranslateUi();

  KeyEditWidget* general_box_;
  core::NDbKValue key_;
};

}  // namespace gui
}  // namespace fastonosql
