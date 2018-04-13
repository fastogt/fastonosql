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

#pragma once

#include <QWidget>

#include "core/global.h"

class QAction;  // lines 24-24
class QPoint;
class QTextEdit;  // lines 23-23

namespace fastonosql {
namespace gui {

class CommandsWidget : public QWidget {
  Q_OBJECT
 public:
  explicit CommandsWidget(QWidget* parent = Q_NULLPTR);

 public Q_SLOTS:
  void addCommand(core::FastoObjectCommandIPtr command);

 private Q_SLOTS:
  void showContextMenu(const QPoint& pt);

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();
  QTextEdit* const log_text_edit_;
};

}  // namespace gui
}  // namespace fastonosql
