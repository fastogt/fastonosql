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

#include "gui/widgets/log_widget.h"

#include <QAction>
#include <QEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QScrollBar>
#include <QTextEdit>
#include <QTime>

#include <common/macros.h>  // for VERIFY

#include "translations/global.h"  // for trClearAll

namespace fastonosql {
namespace gui {
LogWidget::LogWidget(QWidget* parent) : QWidget(parent), logTextEdit_(new QTextEdit) {
  logTextEdit_->setReadOnly(true);
  logTextEdit_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(logTextEdit_, &QTextEdit::customContextMenuRequested, this, &LogWidget::showContextMenu));

  QHBoxLayout* hlayout = new QHBoxLayout;
  hlayout->setContentsMargins(0, 0, 0, 0);
  hlayout->addWidget(logTextEdit_);
  setLayout(hlayout);
  retranslateUi();
}

void LogWidget::addLogMessage(const QString& message, common::logging::LOG_LEVEL level) {
  QTime time = QTime::currentTime();
  logTextEdit_->setTextColor(level == common::logging::LOG_LEVEL_CRIT ? QColor(Qt::red) : QColor(Qt::black));
  logTextEdit_->append(time.toString("hh:mm:ss.zzz: %1").arg(message));
  QScrollBar* sb = logTextEdit_->verticalScrollBar();
  sb->setValue(sb->maximum());
}

void LogWidget::showContextMenu(const QPoint& pt) {
  QMenu* menu = logTextEdit_->createStandardContextMenu();
  QAction* clear = new QAction(translations::trClearAll, this);
  VERIFY(connect(clear, &QAction::triggered, logTextEdit_, &QTextEdit::clear));
  menu->addAction(clear);
  clear->setEnabled(!logTextEdit_->toPlainText().isEmpty());

  menu->exec(logTextEdit_->mapToGlobal(pt));
  delete menu;
}

void LogWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void LogWidget::retranslateUi() {}

}  // namespace gui
}  // namespace fastonosql
