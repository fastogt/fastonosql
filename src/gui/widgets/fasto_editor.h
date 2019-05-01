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

#pragma once

#include "gui/widgets/base_widget.h"

class QMenu;
class QsciLexer;
class QFrame;
class QLineEdit;
class QToolButton;
class QPushButton;
class QCheckBox;

namespace fastonosql {
namespace gui {
class FastoScintilla;

class FastoEditor : public BaseWidget {
  Q_OBJECT

 public:
  enum { find_panel_height = 40 };
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  void registerImage(int id, const QPixmap& im);

  QString text() const;
  QString selectedText() const;

  void setLexer(QsciLexer* lexer);
  QsciLexer* lexer() const;

  bool isReadOnly() const;

 Q_SIGNALS:
  void textChanged();
  void readOnlyChanged();

 public Q_SLOTS:
  void append(const QString& text);
  void setReadOnly(bool ro);
  void setText(const QString& text);
  void clear();

 private Q_SLOTS:
  void goToNextElement();
  void goToPrevElement();

 protected:
  explicit FastoEditor(QWidget* parent = Q_NULLPTR);
  void retranslateUi() override;

  void setShowAutoCompletion(bool show);
  QMenu* createStandardContextMenu();

  void setCallTipsStyle(int style);
  void sendScintilla(unsigned int msg, unsigned long wParam = 0, long lParam = 0);

  void keyPressEvent(QKeyEvent* key_event) override;
  bool eventFilter(QObject* object, QEvent* event) override;

 private:
  void findElement(bool forward);

  FastoScintilla* scin_;
  QFrame* find_panel_;
  QLineEdit* find_line_;
  QToolButton* close_;
  QPushButton* next_;
  QPushButton* prev_;
  QCheckBox* case_sensitive_;
};

}  // namespace gui
}  // namespace fastonosql
