/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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
#include <QLabel>
#include <QTableView>

#include "core/value.h"

#define DEFAILT_ID "*"

namespace fastonosql {
namespace gui {

class StreamTableModel;

class StreamEntryDialog : public QDialog {
  Q_OBJECT
 public:
  explicit StreamEntryDialog(const QString& sid = DEFAILT_ID, QWidget* parent = Q_NULLPTR);
  virtual ~StreamEntryDialog();

  bool GetStream(core::StreamValue::Stream* stream) const;

  void insertEntry(const QString& first, const QString& second);
  void clear();

 private Q_SLOTS:
  void insertRow(const QModelIndex& index);
  void removeRow(const QModelIndex& index);

 private:
  QLabel* entry_label_;
  QLineEdit* id_edit_;
  QTableView* table_;
  StreamTableModel* model_;
};

}  // namespace gui
}  // namespace fastonosql
