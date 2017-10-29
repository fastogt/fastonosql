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

#include "gui/widgets/query_widget.h"

#include <QSplitter>
#include <QVBoxLayout>

#include "proxy/server/iserver.h"  // for IServer

#include "gui/widgets/output_widget.h"  // for OutputWidget

#include "gui/shell/base_shell_widget.h"

namespace fastonosql {
namespace gui {

QueryWidget::QueryWidget(proxy::IServerSPtr server, QWidget* parent) : QWidget(parent), server_(server) {
  shellWidget_ = BaseShellWidget::createWidget(server);
  outputWidget_ = new OutputWidget(server);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  QSplitter* splitter = new QSplitter(Qt::Vertical);
  splitter->addWidget(shellWidget_);
  splitter->addWidget(outputWidget_);
  splitter->setCollapsible(0, false);
  splitter->setCollapsible(1, false);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 3);
  splitter->setHandleWidth(1);
  mainLayout->addWidget(splitter);

  setLayout(mainLayout);
}

QueryWidget* QueryWidget::clone(const QString& text) {
  QueryWidget* result = new QueryWidget(server_, parentWidget());
  result->setInputText(text);
  return result;
}

core::connectionTypes QueryWidget::connectionType() const {
  return server_->GetType();
}

QString QueryWidget::inputText() const {
  return shellWidget_->text();
}

void QueryWidget::setInputText(const QString& text) {
  shellWidget_->setText(text);
}

void QueryWidget::execute(const QString& text) {
  shellWidget_->executeText(text);
}

void QueryWidget::executeArgs(const QString& text, int repeat, int interval, bool history) {
  shellWidget_->executeArgs(text, repeat, interval, history);
}

void QueryWidget::reload() {}
}  // namespace gui
}  // namespace fastonosql
