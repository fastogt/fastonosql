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
const common::qt::gui::FastoQKeySequence g_open_key = common::qt::gui::FastoQKeySequence(QKeySequence::Open);
const common::qt::gui::FastoQKeySequence g_save_key = common::qt::gui::FastoQKeySequence(QKeySequence::Save);
const common::qt::gui::FastoQKeySequence g_save_as_key = common::qt::gui::FastoQKeySequence(QKeySequence::SaveAs);

const common::qt::gui::FastoQKeySequence g_close_key = common::qt::gui::FastoQKeySequence(QKeySequence::Close);
const common::qt::gui::FastoQKeySequence g_new_tab_key = common::qt::gui::FastoQKeySequence(QKeySequence::AddTab);
const common::qt::gui::FastoQKeySequence g_next_tab_key = common::qt::gui::FastoQKeySequence(QKeySequence::NextChild);
const common::qt::gui::FastoQKeySequence g_prev_tab_key =
    common::qt::gui::FastoQKeySequence(QKeySequence::PreviousChild);
const common::qt::gui::FastoQKeySequence g_refresh_key = common::qt::gui::FastoQKeySequence(QKeySequence::Refresh);
const common::qt::gui::FastoQKeySequence g_execute_key =
    common::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Return);

#ifdef OS_MAC
const common::qt::gui::FastoQKeySequence g_full_screen_key =
    common::qt::gui::FastoQKeySequence(QKeySequence::FullScreen);
const common::qt::gui::FastoQKeySequence g_quit_key = common::qt::gui::FastoQKeySequence(QKeySequence::Quit);
#else
const common::qt::gui::FastoQKeySequence g_full_screen_key =
    common::qt::gui::FastoQKeySequence(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_F11);
const common::qt::gui::FastoQKeySequence g_quit_key =
    common::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Q);
#endif

bool IsOpenShortcut(QKeyEvent* keyEvent) {
  return g_open_key == keyEvent;
}

bool IsSaveShortcut(QKeyEvent* keyEvent) {
  return g_save_key == keyEvent;
}

bool IsSaveAsShortcut(QKeyEvent* keyEvent) {
  return g_save_as_key == keyEvent;
}

bool IsQuitShortcut(QKeyEvent* keyEvent) {
  return g_quit_key == keyEvent;
}

bool IsCloseShortcut(QKeyEvent* keyEvent) {
  return g_close_key == keyEvent;
}

bool isSetFocusOnQueryLineShortcut(QKeyEvent* keyEvent) {
  return keyEvent->key() == Qt::Key_F6;
}

bool IsNewTabShortcut(QKeyEvent* keyEvent) {
  return g_new_tab_key == keyEvent;
}

bool IsNextTabShortcut(QKeyEvent* keyEvent) {
  return g_next_tab_key == keyEvent;
}

bool IsPreviousTabShortcut(QKeyEvent* keyEvent) {
  return g_prev_tab_key == keyEvent;
}

bool IsRefreshShortcut(QKeyEvent* keyEvent) {
  return g_refresh_key == keyEvent;
}

bool IsFullScreenShortcut(QKeyEvent* keyEvent) {
  return g_full_screen_key == keyEvent;
}

bool IsExecuteScriptShortcut(QKeyEvent* keyEvent) {
  return g_execute_key == keyEvent;
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
