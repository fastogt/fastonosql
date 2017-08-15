/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "gui/editor/fasto_editor.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>

#include <Qsci/qsciscintilla.h>  // for QsciScintilla, etc

#include <common/qt/convert2string.h>  // for EscapedText

#include "gui/fasto_scintilla.h"  // for FastoScintilla

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"

namespace fastonosql {
namespace gui {

FastoEditor::FastoEditor(QWidget* parent) : QWidget(parent), scin_(nullptr) {
  scin_ = new FastoScintilla;

  findPanel_ = new QFrame;
  findLine_ = new QLineEdit;
  close_ = new QToolButton;
  next_ = new QPushButton;
  prev_ = new QPushButton;
  caseSensitive_ = new QCheckBox;

  close_->setIcon(GuiFactory::GetInstance().close16Icon());
  close_->setToolButtonStyle(Qt::ToolButtonIconOnly);
  close_->setIconSize(QSize(16, 16));
  findLine_->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(close_);
  layout->addWidget(findLine_);
  layout->addWidget(next_);
  layout->addWidget(prev_);
  layout->addWidget(caseSensitive_);

  findPanel_->setFixedHeight(HeightFindPanel);
  findPanel_->setLayout(layout);

  scin_->installEventFilter(this);

  QVBoxLayout* mainL = new QVBoxLayout;
  mainL->addWidget(scin_);
  mainL->addWidget(findPanel_);
  mainL->setContentsMargins(0, 0, 0, 0);
  setLayout(mainL);

  findPanel_->hide();

  VERIFY(connect(close_, &QToolButton::clicked, findPanel_, &QFrame::hide));
  VERIFY(connect(scin_, &FastoScintilla::textChanged, this, &FastoEditor::textChanged));
  VERIFY(connect(next_, &QPushButton::clicked, this, &FastoEditor::goToNextElement));
  VERIFY(connect(prev_, &QPushButton::clicked, this, &FastoEditor::goToPrevElement));
  retranslateUi();
}

FastoEditor::~FastoEditor() {}

void FastoEditor::registerImage(int id, const QPixmap& im) {
  scin_->registerImage(id, im);
}

QString FastoEditor::text() const {
  return scin_->text();
}

QString FastoEditor::selectedText() const {
  return scin_->selectedText();
}

void FastoEditor::setShowAutoCompletion(bool showA) {
  scin_->setShowAutoCompletion(showA);
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

void FastoEditor::keyPressEvent(QKeyEvent* keyEvent) {
  bool isFocusScin = scin_->isActiveWindow();
  bool isShowFind = findPanel_->isVisible();
  if (keyEvent->key() == Qt::Key_Escape && isFocusScin && isShowFind) {
    findPanel_->hide();
    scin_->setFocus();
    keyEvent->accept();
  } else if (keyEvent->key() == Qt::Key_Return && (keyEvent->modifiers() & Qt::ShiftModifier) && isFocusScin &&
             isShowFind) {
    goToPrevElement();
  } else if (keyEvent->key() == Qt::Key_Return && isFocusScin && isShowFind) {
    goToNextElement();
  }

  QWidget::keyPressEvent(keyEvent);
}

bool FastoEditor::eventFilter(QObject* object, QEvent* event) {
  if (object == scin_) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if (((keyEvent->modifiers() & Qt::ControlModifier) && keyEvent->key() == Qt::Key_F)) {
        findPanel_->show();
        findLine_->setFocus();
        // findPanel_->selectAll();
        keyEvent->accept();
        return true;
      }
    }
  }

  return QWidget::eventFilter(object, event);
}

void FastoEditor::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void FastoEditor::retranslateUi() {
  next_->setText(translations::trNext);
  prev_->setText(translations::trPrevious);
  caseSensitive_->setText(translations::trMatchCase);
}

void FastoEditor::findElement(bool forward) {
  const QString& text = findLine_->text();
  if (!text.isEmpty()) {
    bool re = false;
    bool wo = false;
    bool looped = true;
    int index = 0;
    int line = 0;
    scin_->getCursorPosition(&line, &index);

    if (!forward) {
      index -= scin_->selectedText().length();
    }

    scin_->setCursorPosition(line, 0);
    bool isFounded =
        scin_->findFirst(text, re, caseSensitive_->checkState() == Qt::Checked, wo, looped, forward, line, index);

    if (isFounded) {
      scin_->ensureCursorVisible();
    } else {
      QMessageBox::warning(this, translations::trSearch, tr("The specified text was not found."));
    }
  }
}

}  // namespace gui
}  // namespace fastonosql
