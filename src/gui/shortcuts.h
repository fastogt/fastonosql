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

#pragma once

#include <common/qt/gui/shortcuts.h>  // for FastoQKeySequence
class QKeyEvent;

namespace fastonosql {
namespace gui {

extern const common::qt::gui::FastoQKeySequence openKey;
extern const common::qt::gui::FastoQKeySequence saveKey;
extern const common::qt::gui::FastoQKeySequence saveAsKey;
extern const common::qt::gui::FastoQKeySequence quitKey;
extern const common::qt::gui::FastoQKeySequence closeKey;
extern const common::qt::gui::FastoQKeySequence newTabKey;
extern const common::qt::gui::FastoQKeySequence nextTabKey;
extern const common::qt::gui::FastoQKeySequence prevTabKey;
extern const common::qt::gui::FastoQKeySequence refreshKey;
extern const common::qt::gui::FastoQKeySequence fullScreenKey;
extern const common::qt::gui::FastoQKeySequence executeKey;

bool isOpenShortcut(QKeyEvent* keyEvent);
bool isSaveShortcut(QKeyEvent* keyEvent);
bool isSaveAsShortcut(QKeyEvent* keyEvent);
bool isQuitShortcut(QKeyEvent* keyEvent);
bool isCloseShortcut(QKeyEvent* keyEvent);
bool isNewTabShortcut(QKeyEvent* keyEvent);
bool isNextTabShortcut(QKeyEvent* keyEvent);
bool isPreviousTabShortcut(QKeyEvent* keyEvent);
bool isRefreshShortcut(QKeyEvent* keyEvent);
bool isFullScreenShortcut(QKeyEvent* keyEvent);
bool isExecuteScriptShortcut(QKeyEvent* keyEvent);

}  // namespace gui
}  // namespace fastonosql
