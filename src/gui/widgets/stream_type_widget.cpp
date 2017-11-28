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

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/action_cell_delegate.h"
#include "gui/dialogs/stream_entry_dialog.h"
#include "gui/hash_table_model.h"
#include "gui/key_value_table_item.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

namespace {
class StreamTableModelInner : public HashTableModel {
 public:
  explicit StreamTableModelInner(QObject* parent = Q_NULLPTR) : HashTableModel(parent) {}

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (!index.isValid()) {
      return Qt::NoItemFlags;
    }

    return Qt::NoItemFlags;
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (role != Qt::DisplayRole) {
      return QVariant();
    }

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      if (section == KeyValueTableItem::kKey) {
        return "ID";
      } else if (section == KeyValueTableItem::kValue) {
        return translations::trValue;
      } else if (section == KeyValueTableItem::kAction) {
        return translations::trAction;
      }
    }

    return TableModel::headerData(section, orientation, role);
  }
};
}  // namespace

StreamTypeWidget::StreamTypeWidget(QWidget* parent) : QTableView(parent) {
  model_ = new StreamTableModelInner(this);
  setModel(model_);

  setColumnHidden(KeyValueTableItem::kValue, true);

  ActionDelegate* del = new ActionDelegate(this);
  VERIFY(connect(del, &ActionDelegate::addClicked, this, &StreamTypeWidget::addRow));
  VERIFY(connect(del, &ActionDelegate::removeClicked, this, &StreamTypeWidget::removeRow));

  setItemDelegateForColumn(KeyValueTableItem::kAction, del);
  setContextMenuPolicy(Qt::ActionsContextMenu);
  setSelectionBehavior(QAbstractItemView::SelectRows);
}

void StreamTypeWidget::insertStream(const core::StreamValue::Stream& sid) {
  streams_.push_back(sid);
  QString qsid;
  common::ConvertFromString(sid.id_, &qsid);
  model_->insertRow(qsid, QString());
}

void StreamTypeWidget::clear() {
  model_->clear();
}

core::StreamValue* StreamTypeWidget::GetStreamValue() const {
  if (streams_.empty()) {
    return nullptr;
  }

  core::StreamValue* str = new core::StreamValue;
  str->SetStreams(streams_);
  return str;
}

void StreamTypeWidget::addRow(const QModelIndex& index) {
  KeyValueTableItem* node = common::qt::item<common::qt::gui::TableItem*, KeyValueTableItem*>(index);
  UNUSED(node);

  StreamEntryDialog diag(DEFAILT_ID, this);
  int result = diag.exec();
  core::StreamValue::Stream st;
  if (result == QDialog::Accepted && diag.GetStream(&st)) {
    insertStream(st);
  }
}

void StreamTypeWidget::removeRow(const QModelIndex& index) {
  model_->removeRow(index.row());
  streams_.erase(streams_.begin() + index.row());
}

}  // namespace gui
}  // namespace fastonosql
