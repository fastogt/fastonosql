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

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>

#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerxml.h>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>  // for item

#include <fastonosql/core/types.h>

#include "gui/text_converter.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

const std::vector<const char*> g_output_views_text = {"Json", "Raw", "Hex",   "Unicode", "MsgPack",
                                                      "Gzip", "LZ4", "BZip2", "Snappy",  "Xml"};

FastoEditorOutput::FastoEditorOutput(QWidget* parent) : QWidget(parent), view_method_(JSON_VIEW) {
  text_json_editor_ = new FastoEditor;
  json_lexer_ = new QsciLexerJSON;
  xml_lexer_ = new QsciLexerXML;
  VERIFY(connect(text_json_editor_, &FastoEditor::textChanged, this, &FastoEditorOutput::textChanged));
  VERIFY(connect(text_json_editor_, &FastoEditor::readOnlyChanged, this, &FastoEditorOutput::readOnlyChanged));

  QVBoxLayout* mainL = new QVBoxLayout;
  mainL->addWidget(text_json_editor_);
  mainL->setContentsMargins(0, 0, 0, 0);

  views_label_ = new QLabel;
  views_combo_box_ = new QComboBox;
  for (unsigned i = 0; i < g_output_views_text.size(); ++i) {
    views_combo_box_->addItem(g_output_views_text[i], i);
  }

  typedef void (QComboBox::*ind)(int);
  VERIFY(connect(views_combo_box_, static_cast<ind>(&QComboBox::currentIndexChanged), this,
                 &FastoEditorOutput::viewChange));

  QHBoxLayout* hlayout = new QHBoxLayout;
  QSplitter* spliter_save_and_view = new QSplitter(Qt::Horizontal);
  hlayout->addWidget(spliter_save_and_view);
  hlayout->addWidget(views_label_);
  hlayout->addWidget(views_combo_box_);

  mainL->addLayout(hlayout);
  setLayout(mainL);
  views_combo_box_->setCurrentIndex(RAW_VIEW);

  setLayout(mainL);
  syncEditors();
}

FastoEditorOutput::~FastoEditorOutput() {
  delete json_lexer_;
  delete xml_lexer_;
}

void FastoEditorOutput::syncEditors() {
  if (view_method_ == JSON_VIEW) {
    text_json_editor_->setLexer(json_lexer_);
  } else if (view_method_ == XML_VIEW) {
    text_json_editor_->setLexer(xml_lexer_);
  } else {
    text_json_editor_->setLexer(NULL);
  }
}

void FastoEditorOutput::setReadOnly(bool ro) {
  text_json_editor_->setReadOnly(ro);
}

void FastoEditorOutput::viewChange(int view_method) {
  view_method_ = view_method;
  syncEditors();
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
  } else if (view_method_ == UNICODE_VIEW) {
    std::string val_str = common::ConvertToString(val);
    std::string raw = core::detail::string_from_unicode(val_str);
    QString qraw;
    common::ConvertFromString(raw, &qraw);
    return qraw;
  }
  return val;
}

void FastoEditorOutput::setText(const QString& text) {
  QString method_text = g_output_views_text[view_method_];
  std::string text_str = common::ConvertToString(text);
  std::string result_str;
  if (view_method_ == JSON_VIEW) {  // raw
    string_from_json(text_str, &result_str);
  } else if (view_method_ == RAW_VIEW) {  // raw
    result_str = text_str;
  } else if (view_method_ == HEX_VIEW) {  // done
    string_to_hex(text_str, &result_str);
  } else if (view_method_ == UNICODE_VIEW) {  // done
    string_to_unicode(text_str, &result_str);
  } else if (view_method_ == MSGPACK_VIEW) {
    string_from_msgpack(text_str, &result_str);
  } else if (view_method_ == ZLIB_VIEW) {
    string_from_zlib(text_str, &result_str);
  } else if (view_method_ == LZ4_VIEW) {
    string_from_lz4(text_str, &result_str);
  } else if (view_method_ == BZIP2_VIEW) {
    string_from_bzip2(text_str, &result_str);
  } else if (view_method_ == SNAPPY_VIEW) {
    string_from_snappy(text_str, &result_str);
  } else if (view_method_ == XML_VIEW) {  // raw
    result_str = text_str;
  }

  QString result;
  if (result_str.empty()) {
    result = translations::trCannotConvertPattern1ArgsS.arg(method_text);
    text_json_editor_->setReadOnly(true);
  } else {
    common::ConvertFromString(result_str, &result);
  }

  setRawText(result);
}

void FastoEditorOutput::setRawText(const QString& text) {
  text_json_editor_->setText(text);
}

bool FastoEditorOutput::isReadOnly() const {
  return text_json_editor_->isReadOnly();
}

void FastoEditorOutput::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void FastoEditorOutput::retranslateUi() {
  views_label_->setText(translations::trViews + ":");
}

}  // namespace gui
}  // namespace fastonosql
