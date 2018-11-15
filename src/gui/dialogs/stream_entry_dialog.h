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

#include <fastonosql/core/value.h>

#define DEFAILT_ID "*"

class QLabel;
class QLineEdit;
class QTableView;

namespace fastonosql {
namespace gui {

class StreamTableModel;

class StreamEntryDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);
  enum { min_width = 360, min_height = 240 };

  virtual ~StreamEntryDialog();

  bool getStream(core::StreamValue::Stream* out) const;

  void insertEntry(const QString& first, const QString& second);
  void clear();

 private Q_SLOTS:
  void insertRow(const QModelIndex& index);
  void removeRow(const QModelIndex& index);

 protected:
  explicit StreamEntryDialog(const QString& sid = DEFAILT_ID, QWidget* parent = Q_NULLPTR);

 private:
  QLabel* entry_label_;
  QLineEdit* id_edit_;
  QTableView* table_;
  StreamTableModel* model_;
};

}  // namespace gui
}  // namespace fastonosql
