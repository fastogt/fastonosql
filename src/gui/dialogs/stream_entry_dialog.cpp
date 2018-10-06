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

#include "gui/dialogs/stream_entry_dialog.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/action_cell_delegate.h"
#include "gui/key_value_table_item.h"
#include "gui/stream_table_model.h"

namespace fastonosql {
namespace gui {

const QSize StreamEntryDialog::min_dialog_size = QSize(360, 240);

StreamEntryDialog::StreamEntryDialog(const QString& sid, QWidget* parent) : QDialog(parent), model_(nullptr) {
  QVBoxLayout* layout = new QVBoxLayout;

  QHBoxLayout* id_layout = new QHBoxLayout;
  entry_label_ = new QLabel("ID");
  id_layout->addWidget(entry_label_);

  id_edit_ = new QLineEdit(sid, this);
  QRegExp rx(".+");
  id_edit_->setValidator(new QRegExpValidator(rx, this));
  id_layout->addWidget(id_edit_);
  layout->addLayout(id_layout);

  table_ = new QTableView(this);
  // table_->horizontalHeader()->hide();
  // table_->verticalHeader()->hide();
  model_ = new StreamTableModel(this);
  table_->setModel(model_);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &StreamEntryDialog::insertRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &StreamEntryDialog::removeRow));

  table_->setItemDelegateForColumn(KeyValueTableItem::kAction, del);
  table_->setContextMenuPolicy(Qt::ActionsContextMenu);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  QHeaderView* header = table_->horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);
  layout->addWidget(table_);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &StreamEntryDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &StreamEntryDialog::reject));
  layout->addWidget(buttonBox);

  setMinimumSize(min_dialog_size);
  setLayout(layout);
}

StreamEntryDialog::~StreamEntryDialog() {}

bool StreamEntryDialog::getStream(core::StreamValue::Stream* stream) const {
  core::StreamValue::stream_id sid = common::ConvertToString(id_edit_->text());
  return model_->getStream(sid, stream);
}

void StreamEntryDialog::insertEntry(const QString& first, const QString& second) {
  model_->insertEntry(first, second);
}

void StreamEntryDialog::clear() {
  model_->clear();
}

void StreamEntryDialog::insertRow(const QModelIndex& index) {
  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  model_->insertEntry(node->key(), node->value());
}

void StreamEntryDialog::removeRow(const QModelIndex& index) {
  model_->removeEntry(index.row());
}

}  // namespace gui
}  // namespace fastonosql
