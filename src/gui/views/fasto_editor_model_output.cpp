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

#include "gui/views/fasto_editor_model_output.h"

#include <QHBoxLayout>

#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerxml.h>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>  // for item

#include "gui/models/items/fasto_common_item.h"  // for FastoCommonItem, toRaw, etc

#include "translations/global.h"

#include "gui/widgets/fasto_viewer.h"

namespace fastonosql {
namespace gui {

FastoEditorModelOutput::FastoEditorModelOutput(QWidget* parent) : QWidget(parent), editor_(nullptr), model_(nullptr) {
  editor_ = createWidget<FastoViewer>();
  VERIFY(connect(editor_, &FastoViewer::viewChanged, this, &FastoEditorModelOutput::layoutChanged));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(editor_);
  main_layout->setContentsMargins(0, 0, 0, 0);
  setLayout(main_layout);
}

void FastoEditorModelOutput::setModel(QAbstractItemModel* model) {
  if (model == model_) {
    return;
  }

  if (model_) {
    VERIFY(disconnect(model_, &QAbstractItemModel::destroyed, this, &FastoEditorModelOutput::modelDestroyed));
    VERIFY(disconnect(model_, &QAbstractItemModel::dataChanged, this, &FastoEditorModelOutput::dataChanged));
    VERIFY(
        disconnect(model_, &QAbstractItemModel::headerDataChanged, this, &FastoEditorModelOutput::headerDataChanged));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsInserted, this, &FastoEditorModelOutput::rowsInserted));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsAboutToBeRemoved, this,
                      &FastoEditorModelOutput::rowsAboutToBeRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsRemoved, this, &FastoEditorModelOutput::rowsRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsAboutToBeRemoved, this,
                      &FastoEditorModelOutput::columnsAboutToBeRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsRemoved, this, &FastoEditorModelOutput::columnsRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsInserted, this, &FastoEditorModelOutput::columnsInserted));
    VERIFY(disconnect(model_, &QAbstractItemModel::modelReset, this, &FastoEditorModelOutput::reset));
    VERIFY(disconnect(model_, &QAbstractItemModel::layoutChanged, this, &FastoEditorModelOutput::layoutChanged));
  }

  model_ = model;

  // These asserts do basic sanity checking of the model
  Q_ASSERT_X(model_->index(0, 0) == model_->index(0, 0), "QAbstractItemView::setModel",
             "A model should return the exact same index "
             "(including its internal id/pointer) when "
             "asked for it twice in a row.");
  Q_ASSERT_X(model_->index(0, 0).parent() == QModelIndex(), "QAbstractItemView::setModel",
             "The parent of a top level index should be invalid");

  if (model_) {
    VERIFY(connect(model_, &QAbstractItemModel::destroyed, this, &FastoEditorModelOutput::modelDestroyed));
    VERIFY(connect(model_, &QAbstractItemModel::dataChanged, this, &FastoEditorModelOutput::dataChanged));
    VERIFY(connect(model_, &QAbstractItemModel::headerDataChanged, this, &FastoEditorModelOutput::headerDataChanged));
    VERIFY(connect(model_, &QAbstractItemModel::rowsInserted, this, &FastoEditorModelOutput::rowsInserted));
    VERIFY(connect(model_, &QAbstractItemModel::rowsAboutToBeRemoved, this,
                   &FastoEditorModelOutput::rowsAboutToBeRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::rowsRemoved, this, &FastoEditorModelOutput::rowsRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsAboutToBeRemoved, this,
                   &FastoEditorModelOutput::columnsAboutToBeRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsRemoved, this, &FastoEditorModelOutput::columnsRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsInserted, this, &FastoEditorModelOutput::columnsInserted));
    VERIFY(connect(model_, &QAbstractItemModel::modelReset, this, &FastoEditorModelOutput::reset));
    VERIFY(connect(model_, &QAbstractItemModel::layoutChanged, this, &FastoEditorModelOutput::layoutChanged));
  }

  reset();
}

void FastoEditorModelOutput::setReadOnly(bool ro) {
  editor_->setReadOnly(ro);
}

void FastoEditorModelOutput::modelDestroyed() {}

void FastoEditorModelOutput::dataChanged(QModelIndex first, QModelIndex last) {
  UNUSED(first);
  UNUSED(last);

  layoutChanged();
}

void FastoEditorModelOutput::headerDataChanged() {}

void FastoEditorModelOutput::rowsInserted(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);

  layoutChanged();
}

void FastoEditorModelOutput::rowsAboutToBeRemoved(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorModelOutput::rowsRemoved(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorModelOutput::columnsAboutToBeRemoved(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorModelOutput::columnsRemoved(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorModelOutput::columnsInserted(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorModelOutput::reset() {
  layoutChanged();
}

QModelIndex FastoEditorModelOutput::selectedItem(int column) const {
  if (!model_) {
    return QModelIndex();
  }

  return model_->index(0, column);
}

bool FastoEditorModelOutput::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!model_) {
    return false;
  }

  return model_->setData(index, value, role);
}

int FastoEditorModelOutput::childCount() const {
  if (!model_) {
    return 0;
  }

  return model_->rowCount();
}

void FastoEditorModelOutput::layoutChanged() {
  if (!model_) {
    return;
  }

  QModelIndex index = model_->index(0, 0);
  if (!index.isValid()) {
    return;
  }

  FastoCommonItem* child = common::qt::item<common::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!child) {
    return;
  }

  common::qt::gui::TreeItem* root = child->parent();
  if (!root) {
    return;
  }

  core::readable_string_t result;
  for (size_t i = 0; i < root->childrenCount(); ++i) {
    FastoCommonItem* child = dynamic_cast<FastoCommonItem*>(root->child(i));  // +
    if (!child) {
      DNOTREACHED();
      continue;
    }

    result += toRaw(child);
    result += END_LINE_CHAR;
  }

  int vm = editor_->viewMethod();
  if (result.empty()) {
    editor_->setError(translations::trCannotConvertPattern_1S.arg(QString(g_output_views_text[vm])));
    return;
  }

  editor_->setText(result);
}

}  // namespace gui
}  // namespace fastonosql
