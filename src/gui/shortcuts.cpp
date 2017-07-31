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

#include "gui/shortcuts.h"

#include <QKeyEvent>

#include <common/qt/gui/shortcuts.h>  // for FastoQKeySequence

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
const common::qt::gui::FastoQKeySequence openKey = common::qt::gui::FastoQKeySequence(QKeySequence::Open);
const common::qt::gui::FastoQKeySequence saveKey = common::qt::gui::FastoQKeySequence(QKeySequence::Save);
const common::qt::gui::FastoQKeySequence saveAsKey = common::qt::gui::FastoQKeySequence(QKeySequence::SaveAs);

const common::qt::gui::FastoQKeySequence closeKey = common::qt::gui::FastoQKeySequence(QKeySequence::Close);
const common::qt::gui::FastoQKeySequence newTabKey = common::qt::gui::FastoQKeySequence(QKeySequence::AddTab);
const common::qt::gui::FastoQKeySequence nextTabKey = common::qt::gui::FastoQKeySequence(QKeySequence::NextChild);
const common::qt::gui::FastoQKeySequence prevTabKey = common::qt::gui::FastoQKeySequence(QKeySequence::PreviousChild);
const common::qt::gui::FastoQKeySequence refreshKey = common::qt::gui::FastoQKeySequence(QKeySequence::Refresh);
const common::qt::gui::FastoQKeySequence executeKey =
    common::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Return);

#ifdef OS_MAC
const common::qt::gui::FastoQKeySequence fullScreenKey = common::qt::gui::FastoQKeySequence(QKeySequence::FullScreen);
const common::qt::gui::FastoQKeySequence quitKey = common::qt::gui::FastoQKeySequence(QKeySequence::Quit);
#else
const common::qt::gui::FastoQKeySequence fullScreenKey =
    common::qt::gui::FastoQKeySequence(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_F11);
const common::qt::gui::FastoQKeySequence quitKey = common::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Q);
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
