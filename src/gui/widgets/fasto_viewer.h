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

#pragma once

#include "gui/widgets/fasto_editor.h"

#include <fastonosql/core/basic_types.h>

class QLabel;
class QComboBox;

namespace fastonosql {
namespace gui {

enum OutputView : uint8_t {
  RAW_VIEW = 0,  // raw
  JSON_VIEW,     // raw

  TO_HEX_VIEW,
  FROM_HEX_VIEW,

  TO_BASE64_VIEW,
  FROM_BASE64_VIEW,

  TO_UNICODE_VIEW,
  FROM_UNICODE_VIEW,

  MSGPACK_VIEW,  // from
  ZLIB_VIEW,     // from
  GZIP_VIEW,     // from
  LZ4_VIEW,      // from
  BZIP2_VIEW,    // from
  SNAPPY_VIEW,   // from
  XML_VIEW       // raw
};

extern const std::vector<const char*> g_output_views_text;

class FastoViewer : public QWidget {
  Q_OBJECT

 public:
  enum { is_lower_hex = true };
  typedef core::readable_string_t view_input_text_t;
  typedef core::readable_string_t view_output_text_t;

  explicit FastoViewer(QWidget* parent = Q_NULLPTR);
  ~FastoViewer() override;

  OutputView viewMethod() const;
  view_input_text_t text() const;

  bool setText(const view_input_text_t& text);

  void setError(const QString& error);
  void clearError();

  bool isReadOnly() const;

 Q_SIGNALS:
  void textChanged();
  void readOnlyChanged();
  void viewChanged(int view_method);

 public Q_SLOTS:
  void setView(int view_method);
  void setViewChangeEnabled(bool enable);
  void setReadOnly(bool ro);
  void clear();

 private Q_SLOTS:
  void viewChange(int view_method);
  void textChange();

 protected:
  void changeEvent(QEvent* ev) override;

 private:
  void setViewText(const view_input_text_t& text);

  bool isError() const;

  bool convertToView(const view_input_text_t& text, view_output_text_t* out) const;
  bool convertFromView(view_output_text_t* out) const;

  void retranslateUi();
  void syncEditors();

  FastoEditor* text_json_editor_;
  QsciLexer* json_lexer_;
  QsciLexer* xml_lexer_;

  OutputView view_method_;

  QLabel* views_label_;
  QComboBox* views_combo_box_;
  QLabel* error_box_;
  QLabel* note_box_;

  view_input_text_t last_valid_text_;
  bool is_binary_;
};

}  // namespace gui
}  // namespace fastonosql
