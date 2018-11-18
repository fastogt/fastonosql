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

#include <fastonosql/core/db_key.h>

#include "gui/views/fasto_table_view.h"

class QSortFilterProxyModel;

namespace fastonosql {
namespace gui {

class KeysTableModel;

class KeysTableView : public FastoTableView {
  Q_OBJECT

 public:
  explicit KeysTableView(QWidget* parent = Q_NULLPTR);

  void insertKey(const core::NDbKValue& key);
  void updateKey(const core::NKey& key);

  void clearItems();

 Q_SIGNALS:
  void changedTTL(const core::NDbKValue& value, int ttl);

 private Q_SLOTS:
  void showContextMenu(const QPoint& point);

 private:
  QModelIndex selectedIndex() const;

  KeysTableModel* source_model_;
  QSortFilterProxyModel* proxy_model_;
};

}  // namespace gui
}  // namespace fastonosql
