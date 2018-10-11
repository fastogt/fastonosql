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

#include "gui/editor/fasto_editor.h"

class QLabel;
class QComboBox;

namespace fastonosql {
namespace gui {

enum OutputView {
  JSON_VIEW = 0,
  RAW_VIEW,
  HEX_VIEW,
  UNICODE_VIEW,
  MSGPACK_VIEW,
  ZLIB_VIEW,
  LZ4_VIEW,
  BZIP2_VIEW,
  SNAPPY_VIEW,
  XML_VIEW
};

extern const std::vector<const char*> g_output_views_text;

class FastoEditorOutput : public QWidget {
  Q_OBJECT
 public:
  explicit FastoEditorOutput(QWidget* parent = Q_NULLPTR);
  virtual ~FastoEditorOutput();

  int viewMethod() const;
  QString text() const;
  void setText(const QString& text);
  void setRawText(const QString& text);
  bool isReadOnly() const;

 Q_SIGNALS:
  void textChanged();
  void readOnlyChanged();

 public Q_SLOTS:
  void setReadOnly(bool ro);
  void viewChange(int view_method);

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();
  void syncEditors();

  FastoEditor* text_json_editor_;
  QsciLexer* json_lexer_;
  QsciLexer* xml_lexer_;

  int view_method_;

  QLabel* views_label_;
  QComboBox* views_combo_box_;
};

}  // namespace gui
}  // namespace fastonosql
