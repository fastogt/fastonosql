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

#include "gui/widgets/connection_base_widget.h"

namespace fastonosql {
namespace gui {

class ConnectionRemoteWidget : public ConnectionBaseWidget {
  Q_OBJECT
 public:
  explicit ConnectionRemoteWidget(QWidget* parent = 0);

  virtual void syncControls(core::IConnectionSettingsBase* connection) override;
  virtual void retranslateUi() override;

 private:
  virtual core::IConnectionSettingsBase* createConnectionImpl(
      const core::connection_path_t& path) const = 0;
};

}  // namespace gui
}  // namespace fastonosql
