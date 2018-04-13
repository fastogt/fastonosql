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

#include "gui/editor/fasto_editor_output.h"

#include <QHBoxLayout>

#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerxml.h>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>  // for item

#include "gui/fasto_common_item.h"  // for FastoCommonItem, toRaw, etc

#include "translations/global.h"

#include "core/value.h"

namespace fastonosql {
namespace gui {

FastoEditorOutput::FastoEditorOutput(QWidget* parent) : QWidget(parent), model_(nullptr), view_method_(JSON_VIEW) {
  text_json_editor_ = new FastoEditor;
  json_lexer_ = new QsciLexerJSON;
  xml_lexer_ = new QsciLexerXML;
  VERIFY(connect(text_json_editor_, &FastoEditor::textChanged, this, &FastoEditorOutput::textChanged));
  VERIFY(connect(text_json_editor_, &FastoEditor::readOnlyChanged, this, &FastoEditorOutput::readOnlyChanged));

  QVBoxLayout* mainL = new QVBoxLayout;
  mainL->addWidget(text_json_editor_);
  mainL->setContentsMargins(0, 0, 0, 0);
  setLayout(mainL);
  SyncEditors();
}

FastoEditorOutput::~FastoEditorOutput() {
  delete json_lexer_;
  delete xml_lexer_;
}

void FastoEditorOutput::SyncEditors() {
  if (view_method_ == JSON_VIEW) {
    text_json_editor_->setLexer(json_lexer_);
  } else if (view_method_ == XML_VIEW) {
    text_json_editor_->setLexer(xml_lexer_);
  } else {
    text_json_editor_->setLexer(NULL);
  }
}

void FastoEditorOutput::setModel(QAbstractItemModel* model) {
  if (model == model_) {
    return;
  }

  if (model_) {
    VERIFY(disconnect(model_, &QAbstractItemModel::destroyed, this, &FastoEditorOutput::modelDestroyed));
    VERIFY(disconnect(model_, &QAbstractItemModel::dataChanged, this, &FastoEditorOutput::dataChanged));
    VERIFY(disconnect(model_, &QAbstractItemModel::headerDataChanged, this, &FastoEditorOutput::headerDataChanged));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsInserted, this, &FastoEditorOutput::rowsInserted));
    VERIFY(
        disconnect(model_, &QAbstractItemModel::rowsAboutToBeRemoved, this, &FastoEditorOutput::rowsAboutToBeRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::rowsRemoved, this, &FastoEditorOutput::rowsRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsAboutToBeRemoved, this,
                      &FastoEditorOutput::columnsAboutToBeRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsRemoved, this, &FastoEditorOutput::columnsRemoved));
    VERIFY(disconnect(model_, &QAbstractItemModel::columnsInserted, this, &FastoEditorOutput::columnsInserted));
    VERIFY(disconnect(model_, &QAbstractItemModel::modelReset, this, &FastoEditorOutput::reset));
    VERIFY(disconnect(model_, &QAbstractItemModel::layoutChanged, this, &FastoEditorOutput::layoutChanged));
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
    VERIFY(connect(model_, &QAbstractItemModel::destroyed, this, &FastoEditorOutput::modelDestroyed));
    VERIFY(connect(model_, &QAbstractItemModel::dataChanged, this, &FastoEditorOutput::dataChanged));
    VERIFY(connect(model_, &QAbstractItemModel::headerDataChanged, this, &FastoEditorOutput::headerDataChanged));
    VERIFY(connect(model_, &QAbstractItemModel::rowsInserted, this, &FastoEditorOutput::rowsInserted));
    VERIFY(connect(model_, &QAbstractItemModel::rowsAboutToBeRemoved, this, &FastoEditorOutput::rowsAboutToBeRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::rowsRemoved, this, &FastoEditorOutput::rowsRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsAboutToBeRemoved, this,
                   &FastoEditorOutput::columnsAboutToBeRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsRemoved, this, &FastoEditorOutput::columnsRemoved));
    VERIFY(connect(model_, &QAbstractItemModel::columnsInserted, this, &FastoEditorOutput::columnsInserted));
    VERIFY(connect(model_, &QAbstractItemModel::modelReset, this, &FastoEditorOutput::reset));
    VERIFY(connect(model_, &QAbstractItemModel::layoutChanged, this, &FastoEditorOutput::layoutChanged));
  }

  reset();
}

void FastoEditorOutput::setReadOnly(bool ro) {
  text_json_editor_->setReadOnly(ro);
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

bool FastoEditorOutput::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!model_) {
    return false;
  }

  return model_->setData(index, value, role);
}

int FastoEditorOutput::viewMethod() const {
  return view_method_;
}

QString FastoEditorOutput::text() const {
  QString val = text_json_editor_->text();
  if (view_method_ == HEX_VIEW) {
    std::string val_str = common::ConvertToString(val);
    std::string raw = core::detail::string_from_hex(val_str);
    QString qraw;
    common::ConvertFromString(raw, &qraw);
    return qraw;
  }
  return val;
}

bool FastoEditorOutput::isReadOnly() const {
  return text_json_editor_->isReadOnly();
}

int FastoEditorOutput::childCount() const {
  if (!model_) {
    return 0;
  }

  int rc = model_->rowCount();
  return rc;
}

void FastoEditorOutput::layoutChanged() {
  SyncEditors();

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

  QString methodText;
  if (view_method_ == JSON_VIEW) {
    methodText = translations::trJson;
  } else if (view_method_ == CSV_VIEW) {
    methodText = translations::trCsv;
  } else if (view_method_ == RAW_VIEW) {
    methodText = translations::trRawText;
  } else if (view_method_ == HEX_VIEW) {
    methodText = translations::trHex;
  } else if (view_method_ == UNICODE_VIEW) {
    methodText = translations::trUnicode;
  } else if (view_method_ == MSGPACK_VIEW) {
    methodText = translations::trMsgPack;
  } else if (view_method_ == GZIP_VIEW) {
    methodText = translations::trGzip;
  } else if (view_method_ == LZ4_VIEW) {
    methodText = translations::trLZ4;
  } else if (view_method_ == BZIP2_VIEW) {
    methodText = translations::trBZip2;
  } else if (view_method_ == SNAPPY_VIEW) {
    methodText = translations::trSnappy;
  } else if (view_method_ == XML_VIEW) {
    methodText = translations::trXml;
  } else {
    NOTREACHED();
  }

  QString result;
  for (size_t i = 0; i < root->childrenCount(); ++i) {
    FastoCommonItem* child = dynamic_cast<FastoCommonItem*>(root->child(i));  // +
    if (!child) {
      DNOTREACHED();
      continue;
    }

    if (view_method_ == JSON_VIEW) {
      QString json = toJson(child);
      result += common::EscapedText(json);
    } else if (view_method_ == CSV_VIEW) {
      QString csv = toCsv(child);
      result += common::EscapedText(csv);
    } else if (view_method_ == RAW_VIEW) {
      QString raw = toRaw(child);
      result += common::EscapedText(raw);
    } else if (view_method_ == HEX_VIEW) {
      QString raw = toRaw(child);
      std::string str_raw = common::ConvertToString(raw);
      std::string hexed = core::detail::hex_string(str_raw);
      QString qhexed;
      common::ConvertFromString(hexed, &qhexed);
      result += qhexed;
    } else if (view_method_ == UNICODE_VIEW) {
      QString raw = toRaw(child);
      std::string str_raw = common::ConvertToString(raw);
      std::string unicoded = core::detail::unicode_string(str_raw);
      QString qunicoded;
      common::ConvertFromString(unicoded, &qunicoded);
      result += qunicoded;
    } else if (view_method_ == MSGPACK_VIEW) {
      QString msgp = fromHexMsgPack(child);
      result += common::EscapedText(msgp);
    } else if (view_method_ == GZIP_VIEW) {
      QString gzip = fromGzip(child);
      result += common::EscapedText(gzip);
    } else if (view_method_ == LZ4_VIEW) {
      QString lz4 = fromLZ4(child);
      result += common::EscapedText(lz4);
    } else if (view_method_ == BZIP2_VIEW) {
      QString bzip2 = fromBZip2(child);
      result += common::EscapedText(bzip2);
    } else if (view_method_ == SNAPPY_VIEW) {
      QString snap = fromSnappy(child);
      result += common::EscapedText(snap);
    } else if (view_method_ == XML_VIEW) {
      QString raw = toRaw(child);
      result += common::EscapedText(raw);
    }
  }

  if (result.isEmpty()) {
    result = translations::trCannotConvertPattern1ArgsS.arg(methodText);
    text_json_editor_->setReadOnly(true);
  }
  text_json_editor_->setText(result);
}

}  // namespace gui
}  // namespace fastonosql
