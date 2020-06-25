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

#include "gui/widgets/fasto_viewer.h"

#include <vector>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>

#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexerxml.h>

#include <common/convert2string.h>
#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include <fastonosql/core/types.h>

#include "gui/python_converter.h"
#include "gui/text_converter.h"
#include "translations/global.h"

namespace {
const QString trNoteInHexedView = QObject::tr("Note: value is hexed (contains unreadable symbols).");
}

namespace fastonosql {
namespace gui {

namespace {

bool convertFromViewImplRoutine(OutputView view_method, const convert_in_t& val, convert_in_t* out) {
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
  } else if (view_method == TO_BASE64_VIEW) {
    return string_from_base64(val, out);
  } else if (view_method == FROM_BASE64_VIEW) {
    return string_to_base64(val, out);
  } else if (view_method == TO_UNICODE_VIEW) {
    return string_from_unicode(val, out);
  } else if (view_method == FROM_UNICODE_VIEW) {
    return string_to_unicode(val, out);
  } else if (view_method == TO_PICKLE_VIEW) {
    return string_from_pickle(val, out);
  } else if (view_method == FROM_PICKLE_VIEW) {
    return string_to_pickle(val, out);
  } else if (view_method == ZLIB_VIEW) {
    return string_to_zlib(val, out);
  } else if (view_method == GZIP_VIEW) {
    return string_to_gzip(val, out);
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

bool convertFromViewImpl(OutputView view_method, const convert_out_t& val, convert_in_t* out) {
  return convertFromViewImplRoutine(view_method, val, out);
}

bool convertToViewImpl(OutputView view_method, const convert_in_t& text, convert_out_t* out) {
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
  } else if (view_method == TO_BASE64_VIEW) {
    return string_to_base64(text, out);
  } else if (view_method == FROM_BASE64_VIEW) {
    return string_from_base64(text, out);
  } else if (view_method == TO_UNICODE_VIEW) {
    return string_to_unicode(text, out);
  } else if (view_method == FROM_UNICODE_VIEW) {
    return string_from_unicode(text, out);
  } else if (view_method == TO_PICKLE_VIEW) {
    return string_to_pickle(text, out);
  } else if (view_method == FROM_PICKLE_VIEW) {
    return string_from_pickle(text, out);
  } else if (view_method == ZLIB_VIEW) {
    return string_from_zlib(text, out);
  } else if (view_method == GZIP_VIEW) {
    return string_from_gzip(text, out);
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

const std::vector<const char*> g_output_views_text = {
    "Raw",       "Json",        "To Hex", "From Hex", "To Base64", "From Base64", "To Unicode", "From Unicode",
    "To Pickle", "From Pickle", "Zlib",   "GZip",     "LZ4",       "BZip2",       "Snappy",     "Xml"};

FastoViewer::FastoViewer(QWidget* parent)
    : base_class(parent),
      view_method_(RAW_VIEW),
      error_box_(nullptr),
      note_box_(nullptr),
      last_valid_text_(),
      is_binary_(false) {
  text_json_editor_ = createWidget<FastoEditor>();
  json_lexer_ = new QsciLexerJSON(this);
  xml_lexer_ = new QsciLexerXML(this);
  VERIFY(connect(text_json_editor_, &FastoEditor::textChanged, this, &FastoViewer::textChange));
  VERIFY(connect(text_json_editor_, &FastoEditor::readOnlyChanged, this, &FastoViewer::readOnlyChanged));

  error_box_ = new QLabel;
  error_box_->setVisible(false);

  note_box_ = new QLabel(trNoteInHexedView);
  note_box_->setVisible(false);

  views_label_ = new QLabel;
  views_combo_box_ = new QComboBox;
  for (unsigned i = 0; i < g_output_views_text.size(); ++i) {
    views_combo_box_->addItem(g_output_views_text[i], i);
  }

  typedef void (QComboBox::*ind)(int);
  VERIFY(connect(views_combo_box_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &FastoViewer::viewChange));

  QHBoxLayout* hlayout = new QHBoxLayout;
  QSplitter* spliter_save_and_view = new QSplitter(Qt::Horizontal);
  hlayout->addWidget(spliter_save_and_view);
  hlayout->addWidget(views_label_);
  hlayout->addWidget(views_combo_box_);

  QHBoxLayout* ehlayout = new QHBoxLayout;
  ehlayout->addWidget(note_box_);
  ehlayout->addWidget(error_box_);
  ehlayout->addLayout(hlayout);

  QVBoxLayout* main = new QVBoxLayout;
  main->addWidget(text_json_editor_);
  main->setContentsMargins(0, 0, 0, 0);
  main->addLayout(ehlayout);
  setLayout(main);
  syncEditors();
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
  view_method_ = static_cast<OutputView>(view_method);
  syncEditors();
  setText(last_valid_text_);
  emit viewChanged(view_method);
}

void FastoViewer::textChange() {
  view_output_text_t str_text;
  if (convertFromView(&str_text)) {
    clearError();
    last_valid_text_ = str_text;
  }
  emit textChanged();
}

void FastoViewer::clear() {
  text_json_editor_->clear();
  clearError();
  last_valid_text_.clear();
  is_binary_ = false;
}

OutputView FastoViewer::viewMethod() const {
  return view_method_;
}

FastoViewer::view_input_text_t FastoViewer::text() const {
  return last_valid_text_;
}

bool FastoViewer::setText(const view_input_text_t& text) {
  view_output_text_t result_str;
  if (!convertToView(text, &result_str)) {
    QString method_text = g_output_views_text[view_method_];
    setError(translations::trCannotConvertPattern_1S.arg(method_text));
    note_box_->setVisible(false);
    return false;
  }

  last_valid_text_ = text;
  setViewText(result_str);
  return true;
}

void FastoViewer::setViewText(const view_input_text_t& text) {
  clearError();
  QString qtext;
  is_binary_ = core::detail::is_binary_data(text);
  if (is_binary_) {
    convert_in_t hexed;
    bool is_ok = common::utils::xhex::encode(text, is_lower_hex, &hexed);
    DCHECK(is_ok) << "Can't hexed: " << text;
    common::ConvertFromBytes(hexed, &qtext);
    note_box_->setVisible(true);
  } else {
    common::ConvertFromBytes(text, &qtext);
    note_box_->setVisible(false);
  }
  text_json_editor_->setText(qtext);
}

bool FastoViewer::isReadOnly() const {
  return text_json_editor_->isReadOnly();
}

void FastoViewer::retranslateUi() {
  views_label_->setText(translations::trViews + ":");
  base_class::retranslateUi();
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

bool FastoViewer::convertToView(const view_input_text_t& text, view_output_text_t* out) const {
  return convertToViewImpl(view_method_, text, out);
}

bool FastoViewer::convertFromView(view_output_text_t* out) const {
  QString cur_text = text_json_editor_->text();
  convert_out_t cout;
  if (is_binary_) {
    convert_in_t cin = common::ConvertToCharBytes(cur_text);
    common::utils::xhex::decode(cin, &cout);
  } else {
    cout = common::ConvertToCharBytes(cur_text);
  }

  return convertFromViewImpl(view_method_, cout, out);
}

}  // namespace gui
}  // namespace fastonosql
