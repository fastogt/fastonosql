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

#pragma once

#include <QDateTime>
#include <QDialog>

class QLabel;
class QDialogButtonBox;

namespace fastonosql {
namespace gui {

class TrialTimeDialog : public QDialog {
  Q_OBJECT
 public:
  enum { fix_width = 1280 / 2, fix_height = 710 / 2 };  // image size

  explicit TrialTimeDialog(const QString& title,
                           const QDateTime& end_date,
                           uint32_t wait_time_sec,
                           QWidget* parent = Q_NULLPTR);

 public Q_SLOTS:
  virtual void accept() override;

 protected:
  virtual void changeEvent(QEvent* ev) override;
  virtual void timerEvent(QTimerEvent* event) override;

 private:
  void setWaitButtonText(const QString& text);

  void retranslateUi();

  QLabel* message_box_;
  QDialogButtonBox* buttons_box_;
  const QDateTime end_date_;
  uint32_t wait_time_sec_;
  int change_sec_time_id_;
};

}  // namespace gui
}  // namespace fastonosql
