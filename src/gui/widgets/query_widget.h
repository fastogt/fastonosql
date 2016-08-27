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

#include <QWidget>

#include "core/connection_types.h"      // for connectionTypes
#include "core/core_fwd.h"              // for IServerSPtr
namespace fastonosql { namespace gui { class OutputWidget; } }  // lines 33-33
namespace fastonosql { namespace shell { class BaseShellWidget; } }  // lines 28-28

namespace fastonosql {
namespace gui {

class QueryWidget
  : public QWidget {
  Q_OBJECT
 public:
  explicit QueryWidget(core::IServerSPtr server, QWidget* parent = 0);

  QueryWidget* clone(const QString& text);
  core::connectionTypes connectionType() const;
  QString inputText() const;
  void setInputText(const QString& text);
  void execute(const QString& text);
  void reload();

 private:
  shell::BaseShellWidget* shellWidget_;
  OutputWidget* outputWidget_;
  const core::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
