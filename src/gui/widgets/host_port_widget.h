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

#pragma once

#include <QWidget>

#include <common/net/net.h>

class QLineEdit;
class QSpinBox;

namespace fastonosql {
namespace gui {

class HostPortWidget : public QWidget {
  Q_OBJECT
 public:
  explicit HostPortWidget(QWidget* parent = 0);

  common::net::HostAndPort host() const;
  void setHost(const common::net::HostAndPort& host);

  bool isValidHost() const;

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  QLineEdit* hostName_;
  QSpinBox* port_;
};

}  // namespace gui
}  // namespace fastonosql
