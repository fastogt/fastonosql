/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include "gui/shortcuts.h"

#include <QKeyEvent>

#include "fasto/qt/gui/shortcuts.h"  // for FastoQKeySequence

namespace fastonosql {
namespace gui {

/*bool isDuplicateTabShortcut(QKeyEvent* keyEvent) {
  bool ctrlShiftT = (keyEvent->modifiers() &
Qt::ControlModifier) &&
      (keyEvent->modifiers() & Qt::ShiftModifier) &&
      (keyEvent->key() == Qt::Key_T);

  return ctrlShiftT;
}*/

// global variables
const fasto::qt::gui::FastoQKeySequence openKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::Open);
const fasto::qt::gui::FastoQKeySequence saveKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::Save);
const fasto::qt::gui::FastoQKeySequence saveAsKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::SaveAs);

const fasto::qt::gui::FastoQKeySequence closeKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::Close);
const fasto::qt::gui::FastoQKeySequence newTabKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::AddTab);
const fasto::qt::gui::FastoQKeySequence nextTabKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::NextChild);
const fasto::qt::gui::FastoQKeySequence prevTabKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::PreviousChild);
const fasto::qt::gui::FastoQKeySequence refreshKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::Refresh);
const fasto::qt::gui::FastoQKeySequence executeKey =
    fasto::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Return);

#ifdef OS_MAC
const fasto::qt::gui::FastoQKeySequence fullScreenKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::FullScreen);
const fasto::qt::gui::FastoQKeySequence quitKey =
    fasto::qt::gui::FastoQKeySequence(QKeySequence::Quit);
#else
const fasto::qt::gui::FastoQKeySequence fullScreenKey =
    fasto::qt::gui::FastoQKeySequence(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_F11);
const fasto::qt::gui::FastoQKeySequence quitKey =
    fasto::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Q);
#endif

bool isOpenShortcut(QKeyEvent* keyEvent) {
  return openKey == keyEvent;
}

bool isSaveShortcut(QKeyEvent* keyEvent) {
  return saveKey == keyEvent;
}

bool isSaveAsShortcut(QKeyEvent* keyEvent) {
  return saveAsKey == keyEvent;
}

bool isQuitShortcut(QKeyEvent* keyEvent) {
  return quitKey == keyEvent;
}

bool isCloseShortcut(QKeyEvent* keyEvent) {
  return closeKey == keyEvent;
}

bool isSetFocusOnQueryLineShortcut(QKeyEvent* keyEvent) {
  return keyEvent->key() == Qt::Key_F6;
}

bool isNewTabShortcut(QKeyEvent* keyEvent) {
  return newTabKey == keyEvent;
}

bool isNextTabShortcut(QKeyEvent* keyEvent) {
  return nextTabKey == keyEvent;
}

bool isPreviousTabShortcut(QKeyEvent* keyEvent) {
  return prevTabKey == keyEvent;
}

bool isRefreshShortcut(QKeyEvent* keyEvent) {
  return refreshKey == keyEvent;
}

bool isFullScreenShortcut(QKeyEvent* keyEvent) {
  return fullScreenKey == keyEvent;
}

bool isExecuteScriptShortcut(QKeyEvent* keyEvent) {
  return executeKey == keyEvent;
}

/*bool isToggleCommentsShortcut(QKeyEvent* keyEvent) {
  return ((keyEvent->modifiers() & Qt::ControlModifier) &&
(keyEvent->key() ==
Qt::Key_Slash)) ||
         ((keyEvent->modifiers() & Qt::ControlModifier) &&
(keyEvent->modifiers() &
Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_C));
}*/

}  // namespace gui
}  // namespace fastonosql
