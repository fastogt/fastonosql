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

#include "gui/widgets/stream_type_widget.h"

#include <QHeaderView>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/dialogs/stream_entry_dialog.h"
#include "gui/models/hash_table_model.h"
#include "gui/models/items/key_value_table_item.h"
#include "gui/widgets/delegate/action_cell_delegate.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

class StreamTypeWidget::StreamTableModel : public HashTableModel {
 public:
  explicit StreamTableModel(QObject* parent = Q_NULLPTR) : HashTableModel(parent) {
    setFirstColumnName("ID");
    setSecondColumnName(translations::trValue);
  }

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (!index.isValid()) {
      return Qt::NoItemFlags;
    }

    return Qt::NoItemFlags;
  }
};

StreamTypeWidget::StreamTypeWidget(QWidget* parent) : QTableView(parent) {
  model_ = new StreamTableModel(this);
  setModel(model_);

  setColumnHidden(HashTableModel::kValue, true);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &StreamTypeWidget::addRow));
  VERIFY(connect(del, &ActionDelegate::editClicked, this, &StreamTypeWidget::editRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &StreamTypeWidget::removeRow));
  QAbstractItemDelegate* default_del = itemDelegate();
  VERIFY(connect(default_del, &QAbstractItemDelegate::closeEditor, this, &StreamTypeWidget::dataChangedSignal));

  setItemDelegateForColumn(HashTableModel::kAction, del);
  setContextMenuPolicy(Qt::ActionsContextMenu);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  QHeaderView* header = horizontalHeader();
  header->setSectionResizeMode(QHeaderView::Stretch);
}

void StreamTypeWidget::insertStream(const core::StreamValue::Stream& stream) {
  streams_.push_back(stream);
  QString qsid;
  common::ConvertFromBytes(stream.sid, &qsid);
  model_->insertRow(qsid, QString());
  emit dataChangedSignal();
}

void StreamTypeWidget::updateStream(const QModelIndex& index, const core::StreamValue::Stream& stream) {
  if (!index.isValid()) {
    return;
  }

  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  int row = index.row();
  size_t stabled_row_index = static_cast<size_t>(row);
  streams_[stabled_row_index] = stream;
  QString qsid;
  common::ConvertFromBytes(stream.sid, &qsid);
  node->setKey(qsid);
  model_->updateItem(model_->index(row, StreamTableModel::kKey, QModelIndex()),
                     model_->index(row, StreamTableModel::kAction, QModelIndex()));
  emit dataChangedSignal();
}

void StreamTypeWidget::clear() {
  model_->clear();
  streams_.clear();
  emit dataChangedSignal();
}

core::StreamValue* StreamTypeWidget::streamValue() const {
  if (streams_.empty()) {
    return nullptr;
  }

  core::StreamValue* str = new core::StreamValue;
  str->SetStreams(streams_);
  return str;
}

void StreamTypeWidget::editRow(const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }

  int row = index.row();
  size_t stabled_row_index = static_cast<size_t>(row);
  core::StreamValue::Stream stream = streams_[stabled_row_index];
  QString qsid;
  common::ConvertFromBytes(stream.sid, &qsid);
  auto diag = createDialog<StreamEntryDialog>(qsid, this);  // +
  for (size_t i = 0; i < stream.entries.size(); ++i) {
    core::StreamValue::Entry ent = stream.entries[i];
    QString ftext;
    QString stext;
    if (common::ConvertFromBytes(ent.name, &ftext) && common::ConvertFromBytes(ent.value, &stext)) {
      diag->insertEntry(ftext, stext);
    }
  }
  int result = diag->exec();
  if (result == QDialog::Accepted) {
    core::StreamValue::Stream st;
    if (diag->getStream(&st)) {
      updateStream(index, st);
    }
  }
}

void StreamTypeWidget::addRow(const QModelIndex& index) {
  UNUSED(index);

  auto diag = createDialog<StreamEntryDialog>(DEFAILT_ID, this);  // +
  int result = diag->exec();
  core::StreamValue::Stream st;
  if (result == QDialog::Accepted) {
    core::StreamValue::Stream st;
    if (diag->getStream(&st)) {
      insertStream(st);
    }
  }
}

void StreamTypeWidget::removeRow(const QModelIndex& index) {
  model_->removeRow(index.row());
  streams_.erase(streams_.begin() + index.row());
  emit dataChangedSignal();
}

}  // namespace gui
}  // namespace fastonosql
