/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <common/net/net.h>

#include "gui/widgets/base_widget.h"

class QLineEdit;
class QSpinBox;

namespace fastonosql {
namespace gui {

class HostPortWidget : public BaseWidget {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  common::net::HostAndPort host() const;
  void setHost(const common::net::HostAndPort& host);

  bool isValidHost() const;

 protected:
  explicit HostPortWidget(QWidget* parent = Q_NULLPTR);

  QLineEdit* host_edit_box_;
  QSpinBox* port_;
};

}  // namespace gui
}  // namespace fastonosql
