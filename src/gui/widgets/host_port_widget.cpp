/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include "gui/widgets/host_port_widget.h"

#include <limits>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QSpinBox>

#include <common/qt/convert2string.h>

namespace fastonosql {
namespace gui {

HostPortWidget::HostPortWidget(QWidget* parent) : base_class(parent) {
  host_edit_box_ = new QLineEdit;
  port_ = new QSpinBox;
  port_->setRange(0, std::numeric_limits<common::net::HostAndPort::port_t>::max());
  port_->setFixedWidth(80);

  QHBoxLayout* host_and_password_layout = new QHBoxLayout;
  host_and_password_layout->addWidget(host_edit_box_);
  host_and_password_layout->addWidget(new QLabel(":"));
  host_and_password_layout->addWidget(port_);
  setLayout(host_and_password_layout);
}

common::net::HostAndPort HostPortWidget::host() const {
  common::net::HostAndPort::port_t port = static_cast<common::net::HostAndPort::port_t>(port_->value());
  return common::net::HostAndPort(common::ConvertToString(host_edit_box_->text()), port);
}

void HostPortWidget::setHost(const common::net::HostAndPort& host) {
  QString qhost;
  common::ConvertFromString(host.GetHost(), &qhost);
  host_edit_box_->setText(qhost);
  port_->setValue(host.GetPort());
}

bool HostPortWidget::isValidHost() const {
  common::net::HostAndPort hs = host();
  return hs.IsValid();
}

}  // namespace gui
}  // namespace fastonosql
