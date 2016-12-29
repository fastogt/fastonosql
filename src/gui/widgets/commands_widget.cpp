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

#include "gui/widgets/commands_widget.h"

#include <string>  // for string

#include <QAction>
#include <QEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QScrollBar>
#include <QTextEdit>
#include <QTime>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for VERIFY
#include <common/value.h>           // for Value, etc

#include "core/global.h"

#include "translations/global.h"  // for trClearAll

namespace fastonosql {
namespace gui {
CommandsWidget::CommandsWidget(QWidget* parent) : QWidget(parent), logTextEdit_(new QTextEdit) {
  logTextEdit_->setReadOnly(true);
  logTextEdit_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(logTextEdit_, &QTextEdit::customContextMenuRequested, this,
                 &CommandsWidget::showContextMenu));

  QHBoxLayout* hlayout = new QHBoxLayout;
  hlayout->setContentsMargins(0, 0, 0, 0);
  hlayout->addWidget(logTextEdit_);
  clear_ = new QAction(this);
  VERIFY(connect(clear_, &QAction::triggered, logTextEdit_, &QTextEdit::clear));
  setLayout(hlayout);
  retranslateUi();
}

void CommandsWidget::addCommand(core::FastoObjectCommandIPtr command) {
  QTime time = QTime::currentTime();
  logTextEdit_->setTextColor(command->CommandLoggingType() == core::C_INNER ? QColor(Qt::gray)
                                                                            : QColor(Qt::black));
  QString mess = common::ConvertFromString<QString>(command->InputCommand());
  std::string stype = common::ConvertToString(command->ConnectionType());
  QString qstype = common::ConvertFromString<QString>(stype);
  logTextEdit_->append(time.toString("[%1] hh:mm:ss.zzz: %2").arg(qstype.toUpper(), mess));
  QScrollBar* sb = logTextEdit_->verticalScrollBar();
  sb->setValue(sb->maximum());
}

void CommandsWidget::showContextMenu(const QPoint& pt) {
  QMenu* menu = logTextEdit_->createStandardContextMenu();
  menu->addAction(clear_);
  clear_->setEnabled(!logTextEdit_->toPlainText().isEmpty());

  menu->exec(logTextEdit_->mapToGlobal(pt));
  delete menu;
}

void CommandsWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QWidget::changeEvent(ev);
}

void CommandsWidget::retranslateUi() {
  clear_->setText(translations::trClearAll);
}
}  // namespace gui
}  // namespace fastonosql
