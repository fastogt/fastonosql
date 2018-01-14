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

#include "gui/widgets/host_port_widget.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QSpinBox>

#include <common/qt/convert2string.h>

namespace fastonosql {
namespace gui {

HostPortWidget::HostPortWidget(QWidget* parent) : QWidget(parent) {
  QHBoxLayout* hostAndPasswordLayout = new QHBoxLayout;
  host_edit_box_ = new QLineEdit;
  port_ = new QSpinBox;
  port_->setRange(0, UINT16_MAX);
  port_->setFixedWidth(80);
  hostAndPasswordLayout->addWidget(host_edit_box_);
  hostAndPasswordLayout->addWidget(new QLabel(":"));
  hostAndPasswordLayout->addWidget(port_);
  setLayout(hostAndPasswordLayout);

  retranslateUi();
}

common::net::HostAndPort HostPortWidget::host() const {
  return common::net::HostAndPort(common::ConvertToString(host_edit_box_->text()), port_->value());
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

void HostPortWidget::retranslateUi() {}

void HostPortWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

}  // namespace gui
}  // namespace fastonosql
