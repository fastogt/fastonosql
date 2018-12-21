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

#include "gui/widgets/fasto_editor.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>

#include <Qsci/qsciscintilla.h>  // for QsciScintilla, etc

#include "gui/widgets/fasto_scintilla.h"  // for FastoScintilla

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"

namespace fastonosql {
namespace gui {

FastoEditor::FastoEditor(QWidget* parent) : base_class(parent), scin_(nullptr) {
  scin_ = new FastoScintilla;
  scin_->installEventFilter(this);

  find_panel_ = new QFrame;
  find_line_ = new QLineEdit;
  close_ = new QToolButton;
  next_ = new QPushButton;
  prev_ = new QPushButton;
  case_sensitive_ = new QCheckBox;

  close_->setIcon(GuiFactory::GetInstance().close16Icon());
  close_->setToolButtonStyle(Qt::ToolButtonIconOnly);
  close_->setIconSize(QSize(16, 16));
  find_line_->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);

  VERIFY(connect(close_, &QToolButton::clicked, find_panel_, &QFrame::hide));
  VERIFY(connect(scin_, &FastoScintilla::textChanged, this, &FastoEditor::textChanged));
  VERIFY(connect(next_, &QPushButton::clicked, this, &FastoEditor::goToNextElement));
  VERIFY(connect(prev_, &QPushButton::clicked, this, &FastoEditor::goToPrevElement));

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(close_);
  layout->addWidget(find_line_);
  layout->addWidget(next_);
  layout->addWidget(prev_);
  layout->addWidget(case_sensitive_);
  find_panel_->setLayout(layout);
  find_panel_->setFixedHeight(find_panel_height);
  find_panel_->setVisible(false);

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(scin_);
  main_layout->addWidget(find_panel_);
  main_layout->setContentsMargins(0, 0, 0, 0);
  setLayout(main_layout);
}

void FastoEditor::registerImage(int id, const QPixmap& im) {
  scin_->registerImage(id, im);
}

QString FastoEditor::text() const {
  return scin_->text();
}

QString FastoEditor::selectedText() const {
  return scin_->selectedText();
}

void FastoEditor::setShowAutoCompletion(bool show) {
  scin_->setShowAutoCompletion(show);
}

QMenu* FastoEditor::createStandardContextMenu() {
  return scin_->createStandardContextMenu();
}

void FastoEditor::append(const QString& text) {
  scin_->append(text);
}

void FastoEditor::setReadOnly(bool ro) {
  scin_->setReadOnly(ro);
  emit readOnlyChanged();
}

void FastoEditor::setText(const QString& text) {
  scin_->setText(text);
}

void FastoEditor::clear() {
  scin_->clear();
}

void FastoEditor::goToNextElement() {
  findElement(true);
}

void FastoEditor::goToPrevElement() {
  findElement(false);
}

void FastoEditor::setLexer(QsciLexer* lexer) {
  scin_->setLexer(lexer);
  scin_->setAutoCompletionCaseSensitivity(false);
}

QsciLexer* FastoEditor::lexer() const {
  return scin_->lexer();
}

bool FastoEditor::isReadOnly() const {
  return scin_->isReadOnly();
}

void FastoEditor::setCallTipsStyle(int style) {
  scin_->setCallTipsStyle(static_cast<QsciScintilla::CallTipsStyle>(style));
}

void FastoEditor::sendScintilla(unsigned int msg, unsigned long wParam, long lParam) {
  scin_->SendScintilla(msg, wParam, lParam);
}

void FastoEditor::keyPressEvent(QKeyEvent* key_event) {
  bool is_focus_scin = scin_->isActiveWindow();
  bool is_show_find = find_panel_->isVisible();
  if (key_event->key() == Qt::Key_Escape && is_focus_scin && is_show_find) {
    find_panel_->hide();
    scin_->setFocus();
    key_event->accept();
  } else if (key_event->key() == Qt::Key_Return && (key_event->modifiers() & Qt::ShiftModifier) && is_focus_scin &&
             is_show_find) {
    goToPrevElement();
  } else if (key_event->key() == Qt::Key_Return && is_focus_scin && is_show_find) {
    goToNextElement();
  }

  base_class::keyPressEvent(key_event);
}

bool FastoEditor::eventFilter(QObject* object, QEvent* event) {
  if (object == scin_) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
      if (((key_event->modifiers() & Qt::ControlModifier) && key_event->key() == Qt::Key_F)) {
        find_panel_->show();
        find_line_->setFocus();
        // find_panel_->selectAll();
        key_event->accept();
        return true;
      }
    }
  }

  return base_class::eventFilter(object, event);
}

void FastoEditor::retranslateUi() {
  next_->setText(translations::trNext);
  prev_->setText(translations::trPrevious);
  case_sensitive_->setText(translations::trMatchCase);
  base_class::retranslateUi();
}

void FastoEditor::findElement(bool forward) {
  const QString& text = find_line_->text();
  if (text.isEmpty()) {
    return;
  }

  int index = 0;
  int line = 0;
  scin_->getCursorPosition(&line, &index);

  if (!forward) {
    index -= scin_->selectedText().length();
  }

  scin_->setCursorPosition(line, 0);
  bool is_founded =
      scin_->findFirst(text, false, case_sensitive_->checkState() == Qt::Checked, false, true, forward, line, index);

  if (is_founded) {
    scin_->ensureCursorVisible();
  } else {
    QMessageBox::warning(this, translations::trSearch, tr("The specified text was not found."));
  }
}

}  // namespace gui
}  // namespace fastonosql
