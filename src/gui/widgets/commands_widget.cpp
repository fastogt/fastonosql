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

#include "gui/widgets/commands_widget.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QScrollBar>
#include <QTextEdit>
#include <QTime>

#include <common/qt/convert2string.h>  // for ConvertFromString

#include "translations/global.h"  // for trClearAll

namespace fastonosql {
namespace gui {

CommandsWidget::CommandsWidget(QWidget* parent) : QWidget(parent), log_text_edit_(new QTextEdit) {
  log_text_edit_->setReadOnly(true);
  log_text_edit_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(log_text_edit_, &QTextEdit::customContextMenuRequested, this, &CommandsWidget::showContextMenu));

  QHBoxLayout* hlayout = new QHBoxLayout;
  hlayout->setContentsMargins(0, 0, 0, 0);
  hlayout->addWidget(log_text_edit_);
  setLayout(hlayout);
  retranslateUi();
}

void CommandsWidget::addCommand(core::FastoObjectCommandIPtr command) {
  QTime time = QTime::currentTime();
  log_text_edit_->setTextColor(command->GetCommandLoggingType() == core::C_INNER ? QColor(Qt::gray)
                                                                                 : QColor(Qt::black));
  QString mess;
  common::ConvertFromString(command->GetInputCommand(), &mess);
  std::string stype = common::ConvertToString(command->GetConnectionType());
  QString qstype;
  common::ConvertFromString(stype, &qstype);
  log_text_edit_->append(time.toString("[%1] hh:mm:ss.zzz: %2").arg(qstype.toUpper(), mess));
  QScrollBar* sb = log_text_edit_->verticalScrollBar();
  sb->setValue(sb->maximum());
}

void CommandsWidget::showContextMenu(const QPoint& pt) {
  QMenu* menu = log_text_edit_->createStandardContextMenu();
  QAction* clear = new QAction(translations::trClearAll, this);
  VERIFY(connect(clear, &QAction::triggered, log_text_edit_, &QTextEdit::clear));
  menu->addAction(clear);
  clear->setEnabled(!log_text_edit_->toPlainText().isEmpty());

  menu->exec(log_text_edit_->mapToGlobal(pt));
  delete menu;
}

void CommandsWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QWidget::changeEvent(ev);
}

void CommandsWidget::retranslateUi() {}

}  // namespace gui
}  // namespace fastonosql
