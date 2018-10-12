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

class QLabel;
class QComboBox;

namespace fastonosql {
namespace gui {

enum OutputView {
  RAW_VIEW = 0,  // raw
  JSON_VIEW,     // raw

  TO_HEX_VIEW,
  FROM_HEX_VIEW,

  TO_UNICODE_VIEW,
  FROM_UNICODE_VIEW,

  MSGPACK_VIEW,  // from
  ZLIB_VIEW,     // from
  LZ4_VIEW,      // from
  BZIP2_VIEW,    // from
  SNAPPY_VIEW,   // from
  XML_VIEW       // raw
};

extern const std::vector<const char*> g_output_views_text;

class FastoViewer : public QWidget {
  Q_OBJECT
 public:
  explicit FastoViewer(QWidget* parent = Q_NULLPTR);
  virtual ~FastoViewer();

  int viewMethod() const;
  std::string text() const;

  bool setText(const std::string& text);

  void setViewText(const QString& text);
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
  virtual void changeEvent(QEvent* ev) override;

 private:
  bool isError() const;

  std::string convertToView(const std::string& text);
  bool convertFromView(std::string* out) const;

  void retranslateUi();
  void syncEditors();

  FastoEditor* text_json_editor_;
  QsciLexer* json_lexer_;
  QsciLexer* xml_lexer_;

  int view_method_;

  QLabel* views_label_;
  QComboBox* views_combo_box_;
  QLabel* error_box_;

  mutable std::string last_valid_text_;
};

}  // namespace gui
}  // namespace fastonosql
