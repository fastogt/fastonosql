/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "gui/fasto_scintilla.h"

#include <QKeyEvent>

#include <common/macros.h>  // for VERIFY

#include <common/qt/gui/shortcuts.h>  // for isAutoCompleteShortcut, etc

namespace {
int GetNumberOfDigits(int x) {
  if (x < 0) {
    return GetNumberOfDigits(-x) + 1;
  }

  if (x >= 10000) {
    if (x >= 10000000) {
      if (x >= 100000000) {
        if (x >= 1000000000) {
          return 10;
        }
        return 9;
      }
      return 8;
    }
    if (x >= 100000) {
      if (x >= 1000000) {
        return 7;
      }
      return 6;
    }
    return 5;
  }

  if (x >= 100) {
    if (x >= 1000) {
      return 4;
    }
    return 3;
  }

  if (x >= 10) {
    return 2;
  }

  return 1;
}

const QColor caretForegroundColor = QColor(Qt::black);
const QColor selectionBackgroundColor = QColor(Qt::blue);
const QColor selectionForegroundColor = QColor(Qt::white);

const QColor matchedBraceForegroundColor = QColor(190, 190, 190);
const QColor matchedBraceBackgroundColor = QColor(30, 36, 38);

const QColor marginsBackgroundColor = QColor(Qt::green);
const QColor marginsForegroundColor = QColor(Qt::white);
}  // namespace

namespace fastonosql {
namespace gui {

FastoScintilla::FastoScintilla(QWidget* parent) : QsciScintilla(parent), lineNumberMarginWidth_(0) {
  setAutoIndent(true);
  setIndentationsUseTabs(false);
  setIndentationWidth(indentationWidth);
  setUtf8(true);
  setMarginWidth(1, 0);

  setCaretForegroundColor(caretForegroundColor);

  setMatchedBraceForegroundColor(matchedBraceForegroundColor);
  setMatchedBraceBackgroundColor(matchedBraceBackgroundColor);

  setSelectionBackgroundColor(selectionBackgroundColor);
  setSelectionForegroundColor(selectionForegroundColor);

  setContentsMargins(0, 0, 0, 0);
  setViewportMargins(3, 3, 3, 3);
  setMarginLineNumbers(0, true);

  // Margins colors
  // line numbers margin
  setMarginsBackgroundColor(marginsBackgroundColor);
  setMarginsForegroundColor(marginsForegroundColor);

  SendScintilla(QsciScintilla::SCI_SETHSCROLLBAR, 0);

  setWrapMode(QsciScintilla::WrapNone);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  VERIFY(connect(this, &FastoScintilla::linesChanged, this, &FastoScintilla::UpdateLineNumbersMarginWidth));

  setAutoCompletionThreshold(1);
  setAutoCompletionCaseSensitivity(false);
  setAutoCompletionSource(QsciScintilla::AcsNone);
}

void FastoScintilla::SetShowAutoCompletion(bool showA) {
  setAutoCompletionSource(showA ? QsciScintilla::AcsAPIs : QsciScintilla::AcsNone);
}

bool FastoScintilla::IsShowAutoCompletion() const {
  AutoCompletionSource aut = autoCompletionSource();
  return aut == QsciScintilla::AcsAPIs;
}

void FastoScintilla::UpdateLineNumbersMarginWidth() {
  int numberOfDigits = GetNumberOfDigits(lines());

  int tw = GetTextWidth(QsciScintilla::STYLE_LINENUMBER, "0");
  lineNumberMarginWidth_ = numberOfDigits * tw + rowNumberWidth;

  // If line numbers margin already displayed, update its width
  if (GetLineNumberMarginWidth()) {
    setMarginWidth(0, lineNumberMarginWidth_);
  }
}

void FastoScintilla::keyPressEvent(QKeyEvent* keyEvent) {
  if (keyEvent->key() == Qt::Key_F11) {
    keyEvent->ignore();
    ToggleLinesNumbers();
    return;
  }

  if (IsShowAutoCompletion()) {
    if (common::qt::gui::isAutoCompleteShortcut(keyEvent)) {
      ShowAutocompletion();
      return;
    } else if (common::qt::gui::isHideAutoCompleteShortcut(keyEvent)) {
      HideAutocompletion();
      return;
    }
  }

  QsciScintilla::keyPressEvent(keyEvent);
}

int FastoScintilla::GetLineNumberMarginWidth() const {
  return marginWidth(0);
}

int FastoScintilla::GetTextWidth(int style, const QString& text) {
  const QByteArray utf8 = text.toUtf8();
  const char* byteArray = utf8.constData();
  return SendScintilla(QsciScintilla::SCI_TEXTWIDTH, style, byteArray);
}

void FastoScintilla::ToggleLinesNumbers() {
  UpdateLineNumbersMarginWidth();
  if (!GetLineNumberMarginWidth()) {
    setMarginWidth(0, lineNumberMarginWidth_);
  } else {
    setMarginWidth(0, 0);
  }
}

void FastoScintilla::ShowAutocompletion() {
  if (IsShowAutoCompletion()) {
    autoCompleteFromAPIs();
  }
}

void FastoScintilla::HideAutocompletion() {
  if (IsShowAutoCompletion()) {
    cancelList();
  }
}

}  // namespace gui
}  // namespace fastonosql
