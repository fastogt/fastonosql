/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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

#include <common/qt/gui/base/table_model.h>

namespace fastonosql {
namespace gui {

class ClientsTableModel : public common::qt::gui::TableModel {
  Q_OBJECT

 public:
  enum eColumn {
    kId = 0,
    kAddr = 1,
    kFd = 2,
    kName = 3,
    kAge = 4,
    kIdle = 5,
    kFlags = 6,
    kDb = 7,
    kSub = 8,
    kPsub = 9,
    kMulti = 10,
    kQbuf = 11,
    kQbufFree = 12,
    kOdl = 13,
    kOll = 14,
    kOmem = 15,
    kEvents = 16,
    kCmd = 17,
    kCountColumns = 18
  };

  explicit ClientsTableModel(QObject* parent = Q_NULLPTR);

  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  int columnCount(const QModelIndex& parent) const override;
  void clear();

  common::qt::gui::TableItem* findChildById(int iden) const;
};

}  // namespace gui
}  // namespace fastonosql
