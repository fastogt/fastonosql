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

#include "gui/dialogs/trial_time_dialog.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>

#include <common/macros.h>

#include "gui/widgets/how_to_use_widget.h"

#define INDIVIDUAL_BUILDS_URL "https://github.com/fastogt/fastonosql/wiki/Individual-builds"
#define OK_TEXT "OK"

namespace {
const QString trPleaseWait_1D = QObject::tr("Please wait (%1) seconds...");
const QString trTrialWarning_1S =
    QObject::tr("<h3>This is trial version, and after (%1) you can't start " PROJECT_NAME_TITLE ".</h3>");
}  // namespace

namespace fastonosql {
namespace gui {

TrialTimeDialog::TrialTimeDialog(const QString& title,
                                 const QDateTime& end_date,
                                 uint32_t wait_time_sec,
                                 QWidget* parent)
    : QDialog(parent), end_date_(end_date), wait_time_sec_(wait_time_sec), change_sec_time_id_(0) {
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout;

  message_box_ = new QLabel;
  layout->addWidget(message_box_);

  const QSize fix_size(fix_width, fix_height);
  HowToUseWidget* hw = new HowToUseWidget(fix_size);
  layout->addWidget(hw);

  buttons_box_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttons_box_->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttons_box_, &QDialogButtonBox::accepted, this, &TrialTimeDialog::accept));
  VERIFY(connect(buttons_box_, &QDialogButtonBox::rejected, this, &TrialTimeDialog::reject));
  layout->addWidget(buttons_box_);

  change_sec_time_id_ = startTimer(1000);

  setLayout(layout);
  setFixedSize(fix_size);
  retranslateUi();
}

void TrialTimeDialog::accept() {
  if (wait_time_sec_ != 0) {
    QDesktopServices::openUrl(QUrl(INDIVIDUAL_BUILDS_URL));
    return;
  }

  QDialog::accept();
}

void TrialTimeDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void TrialTimeDialog::timerEvent(QTimerEvent* event) {
  if (event->timerId() == change_sec_time_id_) {
    if (wait_time_sec_ == 0) {
      killTimer(change_sec_time_id_);
      change_sec_time_id_ = 0;
      setWaitButtonText(OK_TEXT);
      return;
    }

    setWaitButtonText(trPleaseWait_1D.arg(wait_time_sec_));
    wait_time_sec_--;
  }
}

void TrialTimeDialog::setWaitButtonText(const QString& text) {
  buttons_box_->button(QDialogButtonBox::Ok)->setText(text);
}

void TrialTimeDialog::retranslateUi() {
  message_box_->setText(trTrialWarning_1S.arg(end_date_.toString()));
  setWaitButtonText(trPleaseWait_1D.arg(wait_time_sec_));
}

}  // namespace gui
}  // namespace fastonosql
