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

#include "gui/views/keys_table_view.h"

#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QStyledItemDelegate>

#include "gui/models/keys_table_model.h"

namespace {
class NumericDelegate : public QStyledItemDelegate {
 public:
  explicit NumericDelegate(QObject* parent = Q_NULLPTR) : QStyledItemDelegate(parent) {}

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    UNUSED(option);
    UNUSED(index);

    QSpinBox* editor = new QSpinBox(parent);
    editor->setRange(-1, INT32_MAX);
    editor->setValue(-1);
    return editor;
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
  }

  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
  }

  void updateEditorGeometry(QWidget* editor,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override {
    UNUSED(index);

    editor->setGeometry(option.rect);
  }
};
}  // namespace

namespace fastonosql {
namespace gui {

KeysTableView::KeysTableView(QWidget* parent) : FastoTableView(parent) {
  source_model_ = new KeysTableModel(this);
  proxy_model_ = new QSortFilterProxyModel(this);
  proxy_model_->setSourceModel(source_model_);
  proxy_model_->setDynamicSortFilter(true);
  VERIFY(connect(source_model_, &KeysTableModel::changedTTL, this, &KeysTableView::changedTTL, Qt::DirectConnection));

  setSortingEnabled(true);
  sortByColumn(1, Qt::AscendingOrder);
  setModel(proxy_model_);
  setAlternatingRowColors(true);
  setItemDelegateForColumn(KeysTableModel::kTTL, new NumericDelegate(this));

  // setSelectionBehavior(QAbstractItemView::SelectRows);
  // setSelectionMode(QAbstractItemView::SingleSelection);
  setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(this, &KeysTableView::customContextMenuRequested, this, &KeysTableView::showContextMenu));
}

void KeysTableView::insertKey(const core::NDbKValue& key) {
  source_model_->insertKey(key);
}

void KeysTableView::updateKey(const core::NKey& key) {
  source_model_->updateKey(key);
}

void KeysTableView::clearItems() {
  source_model_->clear();
}

void KeysTableView::showContextMenu(const QPoint& point) {
  UNUSED(point);

  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }
}

QModelIndex KeysTableView::selectedIndex() const {
  QModelIndexList indexses = selectionModel()->selectedRows();

  if (indexses.count() != 1) {
    return QModelIndex();
  }

  return proxy_model_->mapToSource(indexses[0]);
}

}  // namespace gui
}  // namespace fastonosql
