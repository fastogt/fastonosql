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

#include "gui/widgets/query_widget.h"

#include <QGroupBox>
#include <QSplitter>
#include <QVBoxLayout>

#include "proxy/server/iserver.h"  // for IServer

#include "gui/widgets/output_widget.h"  // for OutputWidget

#include "gui/shell/base_shell_widget.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

QueryWidget::QueryWidget(proxy::IServerSPtr server, QWidget* parent) : QWidget(parent), server_(server) {
  shell_widget_ = BaseShellWidget::createWidget(server);
  output_widget_ = new OutputWidget(server);

  QVBoxLayout* main_layout = new QVBoxLayout;
  QSplitter* splitter = new QSplitter(Qt::Vertical);

  console_gb_ = new QGroupBox;
  QVBoxLayout* console_layout = new QVBoxLayout;
  console_layout->addWidget(shell_widget_);
  console_gb_->setLayout(console_layout);
  splitter->addWidget(console_gb_);

  output_gb_ = new QGroupBox;
  QVBoxLayout* output_layout = new QVBoxLayout;
  output_layout->addWidget(output_widget_);
  output_gb_->setLayout(output_layout);
  splitter->addWidget(output_gb_);

  splitter->setCollapsible(0, false);
  splitter->setCollapsible(1, false);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 3);
  splitter->setHandleWidth(1);
  main_layout->addWidget(splitter);

  setLayout(main_layout);
  retranslateUi();
}

QueryWidget* QueryWidget::clone(const QString& text) {
  QueryWidget* result = new QueryWidget(server_, parentWidget());
  result->setInputText(text);
  return result;
}

core::ConnectionTypes QueryWidget::connectionType() const {
  return server_->GetType();
}

QString QueryWidget::inputText() const {
  return shell_widget_->text();
}

void QueryWidget::setInputText(const QString& text) {
  shell_widget_->setText(text);
}

void QueryWidget::execute(const QString& text) {
  shell_widget_->executeText(text);
}

void QueryWidget::executeArgs(const QString& text, int repeat, int interval, bool history) {
  shell_widget_->executeArgs(text, repeat, interval, history);
}

void QueryWidget::reload() {}

void QueryWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void QueryWidget::retranslateUi() {
  console_gb_->setTitle(translations::trConsole);
  output_gb_->setTitle(translations::trOutput);
}

}  // namespace gui
}  // namespace fastonosql
