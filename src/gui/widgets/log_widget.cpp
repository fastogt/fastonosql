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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/widgets/log_widget.h"

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QScrollBar>
#include <QTextEdit>
#include <QTime>

#include <common/macros.h>  // for VERIFY

#include "translations/global.h"  // for trClearAll

namespace fastonosql {
namespace gui {

LogWidget::LogWidget(QWidget* parent) : base_class(parent), log_text_edit_(new QTextEdit) {
  log_text_edit_->setReadOnly(true);
  log_text_edit_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(log_text_edit_, &QTextEdit::customContextMenuRequested, this, &LogWidget::showContextMenu));

  QHBoxLayout* main_layout = new QHBoxLayout;
  main_layout->setContentsMargins(0, 0, 0, 0);
  main_layout->addWidget(log_text_edit_);
  setLayout(main_layout);
}

void LogWidget::addLogMessage(const QString& message, common::logging::LOG_LEVEL level) {
  QTime time = QTime::currentTime();
  log_text_edit_->setTextColor(level == common::logging::LOG_LEVEL_CRIT ? QColor(Qt::red) : QColor(Qt::black));
  log_text_edit_->append(time.toString("hh:mm:ss.zzz: %1").arg(message));
  QScrollBar* sb = log_text_edit_->verticalScrollBar();
  sb->setValue(sb->maximum());
}

void LogWidget::showContextMenu(const QPoint& pt) {
  QMenu* menu = log_text_edit_->createStandardContextMenu();
  QAction* clear = new QAction(translations::trClearAll, menu);
  VERIFY(connect(clear, &QAction::triggered, log_text_edit_, &QTextEdit::clear));
  menu->addAction(clear);
  clear->setEnabled(!log_text_edit_->toPlainText().isEmpty());

  menu->exec(log_text_edit_->mapToGlobal(pt));
  delete menu;
}

}  // namespace gui
}  // namespace fastonosql
