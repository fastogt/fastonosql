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

#include <QDialog>

#include "core/connection_types.h"

class QLineEdit;
class QSpinBox;

namespace fastonosql {
namespace gui {

class LoadContentDbDialog
  : public QDialog {
  Q_OBJECT
 public:
  enum {
    min_height = 120,
    min_width = 240,
    min_key_on_page = 1,
    max_key_on_page = 1000,
    defaults_key = 100,
    step_keys_on_page = defaults_key
  };

  explicit LoadContentDbDialog(const QString& title, core::connectionTypes type, QWidget* parent = 0);
  int count() const;
  QString pattern() const;

 public Q_SLOTS:
  virtual void accept();

 private:
  const core::connectionTypes type_;

  QLineEdit* patternEdit_;
  QSpinBox* countSpinEdit_;
};

}  // namespace gui
}  // namespace fastonosql
