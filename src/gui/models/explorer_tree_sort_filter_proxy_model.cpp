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

#include "gui/models/explorer_tree_sort_filter_proxy_model.h"

#include <common/qt/utils_qt.h>

#include "gui/models/items/explorer_tree_item.h"

namespace fastonosql {
namespace gui {

ExplorerTreeSortFilterProxyModel::ExplorerTreeSortFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {}

bool ExplorerTreeSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
  IExplorerTreeItem* lnode = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(left);
  if (!lnode) {
    DNOTREACHED();
    return true;
  }

  IExplorerTreeItem* rnode = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(right);
  if (!rnode) {
    DNOTREACHED();
    return true;
  }

  if (rnode->type() == IExplorerTreeItem::eDatabase && lnode->type() == IExplorerTreeItem::eDatabase) {
    QString leftString = lnode->name();
    QString rightString = rnode->name();

    bool lok, rok;
    int lint = leftString.toInt(&lok);
    int rint = rightString.toInt(&rok);
    if (lok && rok) {
      return lint < rint;
    }

    return QString::localeAwareCompare(leftString, rightString) < 0;
  }

  QString leftString = lnode->name();
  QString rightString = rnode->name();

  return QString::localeAwareCompare(leftString, rightString) < 0;
}

bool ExplorerTreeSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
  QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
  IExplorerTreeItem* node = common::qt::item<common::qt::gui::TreeItem*, IExplorerTreeItem*>(index);
  if (!node) {
    return true;
  }

  if (node->type() < IExplorerTreeItem::eNamespace) {
    return true;
  }

  QString name = node->name();
  return name.contains(filterRegExp());
}

}  // namespace gui
}  // namespace fastonosql
