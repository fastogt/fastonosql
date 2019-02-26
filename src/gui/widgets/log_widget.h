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

#pragma once

#include <common/log_levels.h>

#include "gui/widgets/base_widget.h"

class QTextEdit;

namespace fastonosql {
namespace gui {

class LogWidget : public BaseWidget {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

 public Q_SLOTS:
  void addLogMessage(const QString& message, common::logging::LOG_LEVEL level);

 private Q_SLOTS:
  void showContextMenu(const QPoint& pt);

 protected:
  explicit LogWidget(QWidget* parent = Q_NULLPTR);

 private:
  QTextEdit* const log_text_edit_;
};

}  // namespace gui
}  // namespace fastonosql
