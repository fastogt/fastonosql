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

#pragma once

#include "gui/widgets/fasto_editor.h"

namespace fastonosql {
namespace gui {

class FastoEditorShell : public FastoEditor {
  Q_OBJECT

 protected Q_SLOTS:
  void showContextMenu(const QPoint& point);

 protected:
  explicit FastoEditorShell(bool show_auto_complete, QWidget* parent = Q_NULLPTR);
};

}  // namespace gui
}  // namespace fastonosql
