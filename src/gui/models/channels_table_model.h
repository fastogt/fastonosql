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

#include <common/qt/gui/base/table_model.h>  // for TableModel

namespace fastonosql {
namespace gui {

class ChannelsTableModel : public common::qt::gui::TableModel {
  Q_OBJECT

 public:
  enum eColumn { kName = 0, kNOS = 1, kCountColumns = 2 };
  explicit ChannelsTableModel(QObject* parent = Q_NULLPTR);
  ~ChannelsTableModel() override;

  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  int columnCount(const QModelIndex& parent) const override;
  void clear();
};

}  // namespace gui
}  // namespace fastonosql
