/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it
   and/or modify
    it under the terms of the GNU General Public License as
   published by
    the Free Software Foundation, either version 3 of the
   License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be
   useful,
    but WITHOUT ANY WARRANTY; without even the implied
   warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General
   Public License
    along with FastoNoSQL.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#include "gui/editor/fasto_editor_output.h"

#include <QHBoxLayout>

#include "common/macros.h"
#include "common/qt/convert2string.h"
#include "common/qt/utils_qt.h"  // for item

#include "gui/editor/fasto_hex_edit.h"  // for FastoHexEdit, etc
#include "gui/fasto_common_item.h"      // for FastoCommonItem, toRaw, etc

#include "translations/global.h"

namespace fastonosql {
namespace gui {

FastoEditorOutput::FastoEditorOutput(const QString& delimiter, QWidget* parent)
    : QWidget(parent), model_(nullptr), view_method_(JSON), delimiter_(delimiter) {
  editor_ = new FastoHexEdit;
  VERIFY(connect(editor_, &FastoHexEdit::textChanged, this, &FastoEditorOutput::textChanged));
  VERIFY(
      connect(editor_, &FastoHexEdit::readOnlyChanged, this, &FastoEditorOutput::readOnlyChanged));

  QVBoxLayout* mainL = new QVBoxLayout;
  mainL->addWidget(editor_);
  mainL->setContentsMargins(0, 0, 0, 0);
  setLayout(mainL);
}

void FastoEditorOutput::setModel(QAbstractItemModel* model) {
  if (model == model_) {
    return;
  }

  if (model_) {
    VERIFY(disconnect(model_, &QAbstractItemModel::destroyed, this,
                      &FastoEditorOutput::modelDestroyed));
    VERIFY(disconnect(model_, &QAbstractItemModel::dataChanged, this,
                      &FastoEditorOutput::dataChanged));
    VERIFY(disconnect(model_, &QAbstractItemModel::headerDataChanged, this,
                      &FastoEditorOutput::headerDataChanged));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsInserted, this,
                      &FastoEditorOutput::rowsInserted));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsAboutToBeRemoved, this,
                      &FastoEditorOutput::rowsAboutToBeRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsRemoved, this,
                      &FastoEditorOutput::rowsRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsAboutToBeRemoved, this,
                      &FastoEditorOutput::columnsAboutToBeRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsRemoved, this,
                      &FastoEditorOutput::columnsRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsInserted, this,
                      &FastoEditorOutput::columnsInserted));
    VERIFY(disconnect(model_, &QAbstractItemModel::modelReset, this, &FastoEditorOutput::reset));
    VERIFY(disconnect(model_, &QAbstractItemModel::layoutChanged, this,
                      &FastoEditorOutput::layoutChanged));
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
    VERIFY(
        connect(model_, &QAbstractItemModel::destroyed, this, &FastoEditorOutput::modelDestroyed));
    VERIFY(
        connect(model_, &QAbstractItemModel::dataChanged, this, &FastoEditorOutput::dataChanged));
    VERIFY(connect(model_, &QAbstractItemModel::headerDataChanged, this,
                   &FastoEditorOutput::headerDataChanged));
    VERIFY(
        connect(model_, &QAbstractItemModel::rowsInserted, this, &FastoEditorOutput::rowsInserted));
    VERIFY(connect(model_, &QAbstractItemModel::rowsAboutToBeRemoved, this,
                   &FastoEditorOutput::rowsAboutToBeRemoved));
    VERIFY(
        connect(model_, &QAbstractItemModel::rowsRemoved, this, &FastoEditorOutput::rowsRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsAboutToBeRemoved, this,
                   &FastoEditorOutput::columnsAboutToBeRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsRemoved, this,
                   &FastoEditorOutput::columnsRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsInserted, this,
                   &FastoEditorOutput::columnsInserted));
    VERIFY(connect(model_, &QAbstractItemModel::modelReset, this, &FastoEditorOutput::reset));
    VERIFY(connect(model_, &QAbstractItemModel::layoutChanged, this,
                   &FastoEditorOutput::layoutChanged));
  }

  reset();
}

void FastoEditorOutput::setReadOnly(bool ro) {
  editor_->setReadOnly(ro);
}

void FastoEditorOutput::viewChange(int viewMethod) {
  view_method_ = viewMethod;
  layoutChanged();
}

void FastoEditorOutput::modelDestroyed() {}

void FastoEditorOutput::dataChanged(QModelIndex first, QModelIndex last) {
  UNUSED(first);
  UNUSED(last);

  layoutChanged();
}

void FastoEditorOutput::headerDataChanged() {}

void FastoEditorOutput::rowsInserted(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);

  layoutChanged();
}

void FastoEditorOutput::rowsAboutToBeRemoved(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorOutput::rowsRemoved(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorOutput::columnsAboutToBeRemoved(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorOutput::columnsRemoved(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorOutput::columnsInserted(QModelIndex index, int r, int c) {
  UNUSED(index);
  UNUSED(r);
  UNUSED(c);
}

void FastoEditorOutput::reset() {
  layoutChanged();
}

QModelIndex FastoEditorOutput::selectedItem(int column) const {
  if (!model_) {
    return QModelIndex();
  }

  return model_->index(0, column);
}

bool FastoEditorOutput::setData(const QModelIndex& index, const QVariant& value) {
  if (!model_) {
    return false;
  }

  return model_->setData(index, value);
}

int FastoEditorOutput::viewMethod() const {
  return view_method_;
}

QString FastoEditorOutput::text() const {
  return editor_->text();
}

bool FastoEditorOutput::isReadOnly() const {
  return editor_->isReadOnly();
}

void FastoEditorOutput::layoutChanged() {
  editor_->clear();
  if (!model_) {
    return;
  }

  QModelIndex index = model_->index(0, 0);
  if (!index.isValid()) {
    return;
  }

  FastoCommonItem* child =
      common::utils_qt::item<fasto::qt::gui::TreeItem*, FastoCommonItem*>(index);
  if (!child) {
    return;
  }

  fasto::qt::gui::TreeItem* root = child->parent();
  if (!root) {
    return;
  }

  QString methodText;
  if (view_method_ == JSON) {
    methodText = translations::trJson;
  } else if (view_method_ == CSV) {
    methodText = translations::trCsv;
  } else if (view_method_ == RAW) {
    methodText = translations::trRawText;
  } else if (view_method_ == HEX) {
    methodText = translations::trHex;
  } else if (view_method_ == MSGPACK) {
    methodText = translations::trMsgPack;
  } else if (view_method_ == GZIP) {
    methodText = translations::trGzip;
  } else {
    NOTREACHED();
  }

  QString result;
  for (size_t i = 0; i < root->childrenCount(); ++i) {
    FastoCommonItem* child = dynamic_cast<FastoCommonItem*>(root->child(i));  // +
    if (!child) {
      continue;
    }

    if (view_method_ == JSON) {
      QString json = toJson(child);
      result += common::EscapedText(json);
    } else if (view_method_ == CSV) {
      QString csv = toCsv(child, delimiter_);
      result += common::EscapedText(csv);
    } else if (view_method_ == RAW) {
      QString raw = toRaw(child);
      result += common::EscapedText(raw);
    } else if (view_method_ == HEX) {
      result += toRaw(child);
    } else if (view_method_ == MSGPACK) {
      QString msgp = fromHexMsgPack(child);
      result += common::EscapedText(msgp);
    } else if (view_method_ == GZIP) {
      QString gzip = fromGzip(child);
      result += common::EscapedText(gzip);
    }
  }

  if (view_method_ == HEX) {
    editor_->setMode(FastoHexEdit::HEX_MODE);
  } else {
    editor_->setMode(FastoHexEdit::TEXT_MODE);
  }

  if (result.isEmpty()) {
    result = QString(translations::trCannotConvertPattern1ArgsS).arg(methodText);
    editor_->setReadOnly(true);
  }
  editor_->setData(result.toUtf8());
}

}  // namespace gui
}  // namespace fastonosql
