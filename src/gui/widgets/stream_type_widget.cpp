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

#include "gui/widgets/stream_type_widget.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QLineEdit>

#include <common/qt/utils_qt.h>
#include <common/qt/convert2string.h>

#include "gui/action_cell_delegate.h"
#include "gui/key_value_table_item.h"
#include "gui/stream_table_model.h"

#define DEFAILT_ID "*"

namespace fastonosql {
namespace gui {

StreamTypeWidget::StreamTypeWidget(QWidget* parent) : QWidget(parent), model_(nullptr) {
  QVBoxLayout* bl = new QVBoxLayout;

  QHBoxLayout* id_layout = new QHBoxLayout;
  entry_label_ = new QLabel("ID");
  id_layout->addWidget(entry_label_);

  id_edit_ = new QLineEdit(DEFAILT_ID, this);
  QRegExp rx(".+");
  id_edit_->setValidator(new QRegExpValidator(rx, this));
  id_layout->addWidget(id_edit_);
  bl->addLayout(id_layout);

  table_ = new QTableView(this);
  table_->horizontalHeader()->hide();
  table_->verticalHeader()->hide();
  model_ = new StreamTableModel(this);
  table_->setModel(model_);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &StreamTypeWidget::insertRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &StreamTypeWidget::removeRow));

  table_->setItemDelegateForColumn(KeyValueTableItem::kAction, del);
  table_->setContextMenuPolicy(Qt::ActionsContextMenu);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  bl->addWidget(table_);

  setLayout(bl);
}

StreamTypeWidget::~StreamTypeWidget() {}

void StreamTypeWidget::insertEntry(const QString& first, const QString& second) {
  model_->insertEntry(first, second);
}

void StreamTypeWidget::clear() {
  model_->clear();
}

core::StreamValue* StreamTypeWidget::GetStreamValue() const {
  QString id_text = id_edit_->text();
  if (id_text.isEmpty()) {
    return nullptr;
  }

  return model_->GetStreamValue(common::ConvertToString(id_text));
}

void StreamTypeWidget::insertRow(const QModelIndex& index) {
  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  model_->insertEntry(node->GetKey(), node->GetValue());
}

void StreamTypeWidget::removeRow(const QModelIndex& index) {
  model_->removeEntry(index.row());
}

}  // namespace gui
}  // namespace fastonosql
