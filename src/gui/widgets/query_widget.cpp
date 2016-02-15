/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/widgets/query_widget.h"

#include <QHBoxLayout>
#include <QSplitter>

#include "shell/shell_widget.h"

#include "gui/widgets/output_widget.h"

namespace fastonosql {

QueryWidget::QueryWidget(IServerSPtr server, QWidget* parent)
  : QWidget(parent) {
  shellWidget_ = new BaseShellWidget(server);
  outputWidget_ = new OutputWidget(server);

  VERIFY(connect(shellWidget_, &BaseShellWidget::rootCreated,
                 outputWidget_, &OutputWidget::rootCreate));
  VERIFY(connect(shellWidget_, &BaseShellWidget::rootCompleated,
                 outputWidget_, &OutputWidget::rootCompleate));

  VERIFY(connect(shellWidget_, &BaseShellWidget::addedChild,
                 outputWidget_, &OutputWidget::addChild));
  VERIFY(connect(shellWidget_, &BaseShellWidget::itemUpdated,
                 outputWidget_, &OutputWidget::itemUpdate));

  QSplitter* splitter = new QSplitter;
#ifdef OS_WIN
  splitter->setStyleSheet("QSplitter::handle { background-color: gray }");
#endif
  splitter->setOrientation(Qt::Vertical);
  splitter->setHandleWidth(1);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  splitter->addWidget(shellWidget_);
  splitter->addWidget(outputWidget_);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  mainLayout->addWidget(splitter);
  setMinimumSize(QSize(min_width, min_height));

  setLayout(mainLayout);
}

QueryWidget::~QueryWidget() {

}

QueryWidget* QueryWidget::clone(const QString& text) {
  QueryWidget *result = new QueryWidget(shellWidget_->server(), parentWidget());
  result->shellWidget_->setText(text);
  return result;
}

connectionTypes QueryWidget::connectionType() const {
  IServerSPtr ser = shellWidget_->server();
  if (!ser) {
    return DBUNKNOWN;
  }

  return ser->type();
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

void QueryWidget::reload() {
}

}  // namespace fastonosql
