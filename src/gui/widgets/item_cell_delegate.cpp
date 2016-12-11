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

#include "gui/widgets/item_cell_delegate.h"

#include <QApplication>
#include <QPainter>
#include <QMouseEvent>

#include "gui/gui_factory.h"

namespace fastonosql {
namespace gui {

ItemCellDelegate::ItemCellDelegate(QObject* parent)
    : QStyledItemDelegate(parent), current_row_(-1) {}

void ItemCellDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  if (!index.isValid()) {
    return;
  }

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  const QRect rect = option.rect;
  const int rect_height = rect.height();

  QRect rect_text = rect;
  rect_text.setWidth(rect.width() - rect_height);
  opt.rect = rect_text;

  QWidget* widget = qobject_cast<QWidget*>(parent());
  QStyle* style = widget ? widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

  QRect rect_button(rect_text.topRight(), QSize(rect_height, rect_height));
  QStyleOptionButton pb_Style;
  pb_Style.state |= QStyle::State_Enabled | option.state;
  pb_Style.rect = rect_button;
  pb_Style.features |= QStyleOptionButton::Flat;
  pb_Style.text = QString();
  pb_Style.iconSize = QSize(16, 16);
  pb_Style.icon = GuiFactory::instance().removeIcon();
  if (current_row_ == index.row()) {
    pb_Style.state |= QStyle::State_Sunken;
  } else {
    pb_Style.state |= QStyle::State_Raised;
  }
  style->drawControl(QStyle::CE_PushButton, &pb_Style, painter, widget);
}

bool ItemCellDelegate::editorEvent(QEvent* event,
                                   QAbstractItemModel* model,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) {
  UNUSED(model);

  if (event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease) {
    return QStyledItemDelegate::editorEvent(event, model, option, index);
  }

  const QRect rect = option.rect;
  const int rect_height = rect.height();
  QRect rect_text = rect;
  rect_text.setWidth(rect.width() - rect_height);
  QRect rect_button(rect_text.topRight(), QSize(rect_height, rect_height));
  QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
  QPoint pos = mouseEvent->pos();
  if (!rect_button.contains(pos)) {
    return QStyledItemDelegate::editorEvent(event, model, option, index);
  }

  if (event->type() == QEvent::MouseButtonPress) {
    current_row_ = index.row();
  } else if (event->type() == QEvent::MouseButtonRelease) {
    emit removeClicked(current_row_);
    current_row_ = -1;
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

}  // namespace gui
}  // namespace fastonosql
