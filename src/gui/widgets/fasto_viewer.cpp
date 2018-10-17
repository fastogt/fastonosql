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

#include "gui/widgets/fasto_viewer.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>

#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerxml.h>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>  // for item

#include "gui/text_converter.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

namespace {

bool convertFromViewImpl(OutputView view_method, const std::string& val, std::string* out) {
  if (!out || val.empty()) {
    return false;
  }

  if (view_method == JSON_VIEW) {
    return string_from_json(val, out);
  } else if (view_method == RAW_VIEW) {
    *out = val;
    return true;
  } else if (view_method == TO_HEX_VIEW) {
    return string_from_hex(val, out);
  } else if (view_method == FROM_HEX_VIEW) {
    return string_to_hex(val, out);
  } else if (view_method == TO_UNICODE_VIEW) {
    return string_from_unicode(val, out);
  } else if (view_method == FROM_UNICODE_VIEW) {
    return string_to_unicode(val, out);
  } else if (view_method == MSGPACK_VIEW) {
    return string_to_msgpack(val, out);
  } else if (view_method == ZLIB_VIEW) {
    return string_to_zlib(val, out);
  } else if (view_method == LZ4_VIEW) {
    return string_to_lz4(val, out);
  } else if (view_method == BZIP2_VIEW) {
    return string_to_bzip2(val, out);
  } else if (view_method == SNAPPY_VIEW) {
    return string_to_snappy(val, out);
  } else if (view_method == XML_VIEW) {
    *out = val;
    return true;
  }

  NOTREACHED() << "Please handle all types!";
  return false;
}

bool convertFromViewImpl(OutputView view_method, const QString& val, std::string* out) {
  return convertFromViewImpl(view_method, common::ConvertToString(val), out);
}

bool convertToViewImpl(OutputView view_method, const std::string& text, std::string* out) {
  if (!out || text.empty()) {
    return false;
  }

  if (view_method == JSON_VIEW) {  // raw
    return string_to_json(text, out);
  } else if (view_method == RAW_VIEW) {  // raw
    *out = text;
    return true;
  } else if (view_method == TO_HEX_VIEW) {
    return string_to_hex(text, out);
  } else if (view_method == FROM_HEX_VIEW) {
    return string_from_hex(text, out);
  } else if (view_method == TO_UNICODE_VIEW) {
    return string_to_unicode(text, out);
  } else if (view_method == FROM_UNICODE_VIEW) {
    return string_from_unicode(text, out);
  } else if (view_method == MSGPACK_VIEW) {
    return string_from_msgpack(text, out);
  } else if (view_method == ZLIB_VIEW) {
    return string_from_zlib(text, out);
  } else if (view_method == LZ4_VIEW) {
    return string_from_lz4(text, out);
  } else if (view_method == BZIP2_VIEW) {
    return string_from_bzip2(text, out);
  } else if (view_method == SNAPPY_VIEW) {
    return string_from_snappy(text, out);
  } else if (view_method == XML_VIEW) {  // raw
    *out = text;
    return true;
  }

  NOTREACHED() << "Please handle all types!";
  return false;
}

}  // namespace

const std::vector<const char*> g_output_views_text = {"Raw",        "Json",         "To Hex",         "From Hex",
                                                      "To Unicode", "From Unicode", "MsgPack (Beta)", "Gzip",
                                                      "LZ4",        "BZip2",        "Snappy",         "Xml"};

FastoViewer::FastoViewer(QWidget* parent) : QWidget(parent), view_method_(RAW_VIEW), last_valid_text_() {
  text_json_editor_ = new FastoEditor;
  json_lexer_ = new QsciLexerJSON;
  xml_lexer_ = new QsciLexerXML;
  VERIFY(connect(text_json_editor_, &FastoEditor::textChanged, this, &FastoViewer::textChange));
  VERIFY(connect(text_json_editor_, &FastoEditor::readOnlyChanged, this, &FastoViewer::readOnlyChanged));

  QVBoxLayout* main = new QVBoxLayout;
  main->addWidget(text_json_editor_);
  main->setContentsMargins(0, 0, 0, 0);

  error_box_ = new QLabel;
  error_box_->setVisible(false);

  views_label_ = new QLabel;
  views_combo_box_ = new QComboBox;
  for (unsigned i = 0; i < g_output_views_text.size(); ++i) {
    views_combo_box_->addItem(g_output_views_text[i], i);
  }

  typedef void (QComboBox::*ind)(int);
  VERIFY(connect(views_combo_box_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &FastoViewer::viewChange));

  QHBoxLayout* ehlayout = new QHBoxLayout;

  QHBoxLayout* hlayout = new QHBoxLayout;
  QSplitter* spliter_save_and_view = new QSplitter(Qt::Horizontal);
  hlayout->addWidget(spliter_save_and_view);
  hlayout->addWidget(views_label_);
  hlayout->addWidget(views_combo_box_);

  ehlayout->addWidget(error_box_);
  ehlayout->addLayout(hlayout);
  main->addLayout(ehlayout);
  setLayout(main);
  syncEditors();
}

FastoViewer::~FastoViewer() {
  delete json_lexer_;
  delete xml_lexer_;
}

void FastoViewer::syncEditors() {
  if (view_method_ == JSON_VIEW) {
    text_json_editor_->setLexer(json_lexer_);
  } else if (view_method_ == XML_VIEW) {
    text_json_editor_->setLexer(xml_lexer_);
  } else {
    text_json_editor_->setLexer(nullptr);
  }
}

void FastoViewer::setView(int view_method) {
  views_combo_box_->setCurrentIndex(view_method);
}

void FastoViewer::setViewChangeEnabled(bool enable) {
  views_combo_box_->setEnabled(enable);
}

void FastoViewer::setReadOnly(bool ro) {
  text_json_editor_->setReadOnly(ro);
}

void FastoViewer::viewChange(int view_method) {
  std::string last_valid = text();
  view_method_ = static_cast<OutputView>(view_method);
  syncEditors();
  setText(last_valid);
  emit viewChanged(view_method);
}

void FastoViewer::textChange() {
  if (isError()) {
    std::string str_text;
    if (convertFromView(&str_text)) {
      clearError();
    }
  }
  emit textChanged();
}

void FastoViewer::clear() {
  text_json_editor_->clear();
  clearError();
  last_valid_text_.clear();
}

OutputView FastoViewer::viewMethod() const {
  return view_method_;
}

std::string FastoViewer::text() const {
  if (!isError()) {
    std::string str_text;
    if (convertFromView(&str_text)) {
      last_valid_text_ = str_text;
    }
  }

  return last_valid_text_;
}

bool FastoViewer::setText(const std::string& text) {
  std::string result_str;
  if (!convertToView(text, &result_str)) {
    QString method_text = g_output_views_text[view_method_];
    setError(translations::trCannotConvertPattern1ArgsS.arg(method_text));
    return false;
  }

  QString result;
  common::ConvertFromString(result_str, &result);
  setViewText(result);
  return true;
}

void FastoViewer::setViewText(const QString& text) {
  clearError();
  text_json_editor_->setText(text);
}

bool FastoViewer::isReadOnly() const {
  return text_json_editor_->isReadOnly();
}

void FastoViewer::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void FastoViewer::retranslateUi() {
  views_label_->setText(translations::trViews + ":");
}

void FastoViewer::setError(const QString& error) {
  error_box_->setText(error);
  error_box_->setVisible(true);
}

void FastoViewer::clearError() {
  error_box_->clear();
  error_box_->setVisible(false);
}

bool FastoViewer::isError() const {
  return error_box_->isVisible();
}

bool FastoViewer::convertToView(const std::string& text, std::string* out) const {
  return convertToViewImpl(view_method_, text, out);
}

bool FastoViewer::convertFromView(std::string* out) const {
  QString text = text_json_editor_->text();
  return convertFromViewImpl(view_method_, text, out);
}

}  // namespace gui
}  // namespace fastonosql
