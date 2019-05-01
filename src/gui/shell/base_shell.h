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

#pragma once

#include <fastonosql/core/connection_types.h>

#include "gui/editor/fasto_editor_shell.h"

namespace fastonosql {
namespace gui {

class BaseQsciLexer;

class BaseShell : public FastoEditorShell {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  std::vector<uint32_t> supportedVersions() const;
  size_t commandsCount() const;
  size_t validateCommandsCount() const;
  QString version() const;
  QString basedOn() const;
  void setFilteredVersion(uint32_t version);

  static BaseShell* createFromType(core::ConnectionType type, bool show_auto_complete);

 protected:
  BaseShell(core::ConnectionType type, bool show_auto_complete, QWidget* parent = Q_NULLPTR);
  BaseQsciLexer* lexer() const;

  void updateFont() override;
};

}  // namespace gui
}  // namespace fastonosql
