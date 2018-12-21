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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/shortcuts.h"

#include <QKeyEvent>

namespace fastonosql {
namespace gui {

/*bool isDuplicateTabShortcut(QKeyEvent* key_event) {
  bool ctrlShiftT = (key_event->modifiers() &
Qt::ControlModifier) &&
      (key_event->modifiers() & Qt::ShiftModifier) &&
      (key_event->key() == Qt::Key_T);

  return ctrlShiftT;
}*/

// global variables
const common::qt::gui::FastoQKeySequence g_open_key = common::qt::gui::FastoQKeySequence(QKeySequence::Open);
const common::qt::gui::FastoQKeySequence g_new_key = common::qt::gui::FastoQKeySequence(QKeySequence::New);
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

#if defined(OS_MAC)
const common::qt::gui::FastoQKeySequence g_full_screen_key =
    common::qt::gui::FastoQKeySequence(QKeySequence::FullScreen);
const common::qt::gui::FastoQKeySequence g_quit_key = common::qt::gui::FastoQKeySequence(QKeySequence::Quit);
#else
const common::qt::gui::FastoQKeySequence g_full_screen_key =
    common::qt::gui::FastoQKeySequence(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_F11);
const common::qt::gui::FastoQKeySequence g_quit_key =
    common::qt::gui::FastoQKeySequence(Qt::ControlModifier, Qt::Key_Q);
#endif

bool IsOpenShortcut(QKeyEvent* key_event) {
  return g_open_key == key_event;
}

bool IsSaveShortcut(QKeyEvent* key_event) {
  return g_save_key == key_event;
}

bool IsSaveAsShortcut(QKeyEvent* key_event) {
  return g_save_as_key == key_event;
}

bool IsQuitShortcut(QKeyEvent* key_event) {
  return g_quit_key == key_event;
}

bool IsCloseShortcut(QKeyEvent* key_event) {
  return g_close_key == key_event;
}

bool IsNewTabShortcut(QKeyEvent* key_event) {
  return g_new_tab_key == key_event;
}

bool IsNextTabShortcut(QKeyEvent* key_event) {
  return g_next_tab_key == key_event;
}

bool IsPreviousTabShortcut(QKeyEvent* key_event) {
  return g_prev_tab_key == key_event;
}

bool IsRefreshShortcut(QKeyEvent* key_event) {
  return g_refresh_key == key_event;
}

bool IsFullScreenShortcut(QKeyEvent* key_event) {
  return g_full_screen_key == key_event;
}

bool IsExecuteScriptShortcut(QKeyEvent* key_event) {
  return g_execute_key == key_event;
}

bool IsAcceptShortcut(QKeyEvent* key_event) {
  return IsExecuteScriptShortcut(key_event);
}

/*bool isToggleCommentsShortcut(QKeyEvent* key_event) {
  return ((key_event->modifiers() & Qt::ControlModifier) &&
(key_event->key() ==
Qt::Key_Slash)) ||
         ((key_event->modifiers() & Qt::ControlModifier) &&
(key_event->modifiers() &
Qt::ShiftModifier) && (key_event->key() == Qt::Key_C));
}*/

}  // namespace gui
}  // namespace fastonosql
