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

#include "gui/editor/fasto_editor_shell.h"

#include <QMenu>

#include <common/macros.h>

namespace fastonosql {
namespace gui {

FastoEditorShell::FastoEditorShell(bool show_auto_complete, QWidget* parent) : FastoEditor(parent) {
  setShowAutoCompletion(show_auto_complete);
  VERIFY(connect(this, &FastoEditorShell::customContextMenuRequested, this, &FastoEditorShell::showContextMenu));
}

void FastoEditorShell::showContextMenu(const QPoint& point) {
  QMenu* menu = createStandardContextMenu();
  menu->exec(mapToGlobal(point));
  delete menu;
}

}  // namespace gui
}  // namespace fastonosql
