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

#include <QStyledItemDelegate>

namespace fastonosql {
namespace gui {

class ActionDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit ActionDelegate(QObject* parent = Q_NULLPTR);

  virtual QSize sizeHint(const QStyleOptionViewItem& option,
                         const QModelIndex& index) const override;
  virtual void paint(QPainter* painter,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) const override;

 Q_SIGNALS:
  void addClicked(const QModelIndex& index);
  void removeClicked(const QModelIndex& index);

 protected:
  virtual bool editorEvent(QEvent* event,
                           QAbstractItemModel* model,
                           const QStyleOptionViewItem& option,
                           const QModelIndex& index) override;

 private:
  QModelIndex current_index_;
};

}  // namespace gui
}  // namespace fastonosql
