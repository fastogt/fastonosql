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

#include <vector>

#include "core/connection_types.h"

#include "gui/fasto_editor.h"

namespace fastonosql {
namespace shell {

class BaseQsciLexer;
class BaseShell
  : public gui::FastoEditorShell {
  Q_OBJECT
 public:
  std::vector<uint32_t> supportedVersions() const;
  size_t commandsCount() const;
  QString version() const;
  QString basedOn() const;
  void setFilteredVersion(uint32_t version);

  static BaseShell* createFromType(core::connectionTypes type, bool showAutoCompl);

 protected:
  BaseShell(core::connectionTypes type, bool showAutoCompl, QWidget* parent = 0);
  BaseQsciLexer* lexer() const;
};

}  // namespace shell
}  // namespace fastonosql
