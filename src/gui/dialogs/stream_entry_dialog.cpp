/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include "gui/dialogs/stream_entry_dialog.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/models/items/key_value_table_item.h"
#include "gui/models/stream_table_model.h"
#include "gui/widgets/delegate/action_cell_delegate.h"

namespace {
const QString trTitle = "Create stream entry";
}

namespace fastonosql {
namespace gui {

StreamEntryDialog::StreamEntryDialog(const QString& sid, QWidget* parent)
    : base_class(trTitle, parent), entry_label_(nullptr), id_edit_(nullptr), table_(nullptr), model_(nullptr) {
  QHBoxLayout* id_layout = new QHBoxLayout;
  entry_label_ = new QLabel("ID");
  id_layout->addWidget(entry_label_);

  id_edit_ = new QLineEdit(sid, this);
  QRegExp rx(".+");
  id_edit_->setValidator(new QRegExpValidator(rx, this));
  id_layout->addWidget(id_edit_);

  table_ = new QTableView(this);
  // table_->horizontalHeader()->hide();
  // table_->verticalHeader()->hide();
  model_ = new StreamTableModel(this);
  table_->setModel(model_);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &StreamEntryDialog::insertRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &StreamEntryDialog::removeRow));

  table_->setItemDelegateForColumn(StreamTableModel::kAction, del);
  table_->setContextMenuPolicy(Qt::ActionsContextMenu);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  QHeaderView* header = table_->horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &StreamEntryDialog::accept));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &StreamEntryDialog::reject));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addLayout(id_layout);
  main_layout->addWidget(table_);
  main_layout->addWidget(button_box);
  setLayout(main_layout);
  setMinimumSize(QSize(min_height, min_width));
}

bool StreamEntryDialog::getStream(core::StreamValue::Stream* out) const {
  core::StreamValue::stream_id sid = common::ConvertToCharBytes(id_edit_->text());
  return model_->getStream(sid, out);
}

void StreamEntryDialog::insertEntry(const key_t& key, const value_t& value) {
  model_->insertEntry(key, value);
}

void StreamEntryDialog::clear() {
  model_->clear();
}

void StreamEntryDialog::insertRow(const QModelIndex& index) {
  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  insertEntry(node->key(), node->value());
}

void StreamEntryDialog::removeRow(const QModelIndex& index) {
  model_->removeEntry(index.row());
}

}  // namespace gui
}  // namespace fastonosql
