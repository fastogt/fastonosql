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

#include <common/qt/gui/base/table_model.h>

#include <fastonosql/core/value.h>

namespace fastonosql {
namespace gui {

class StreamTableModel : public common::qt::gui::TableModel {
  Q_OBJECT
 public:
  explicit StreamTableModel(QObject* parent = Q_NULLPTR);
  ~StreamTableModel() override;

  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  int columnCount(const QModelIndex& parent) const override;
  void clear();

  bool getStream(core::StreamValue::stream_id sid, core::StreamValue::Stream* out) const;

  void insertEntry(const QString& key, const QString& value);
  void removeEntry(int row);

 private:
  using TableModel::insertItem;
  using TableModel::removeItem;

  common::qt::gui::TableItem* createEmptyEntry() const;
};

}  // namespace gui
}  // namespace fastonosql
