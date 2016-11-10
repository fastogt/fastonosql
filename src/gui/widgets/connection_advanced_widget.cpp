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

#include "gui/widgets/connection_advanced_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QRegExpValidator>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>

#include <common/macros.h>

#include "translations/global.h"

namespace {
const QString trLoggingToolTip = QObject::tr("INFO command timeout in msec for history statistic.");
}

namespace fastonosql {
namespace gui {

ConnectionAdvancedWidget::ConnectionAdvancedWidget(QWidget* parent) : QWidget(parent) {
  connectionFolder_ = new QLineEdit;
  QRegExp rxf("^/[A-z0-9]+/$");
  connectionFolder_->setValidator(new QRegExpValidator(rxf, this));

  folderLabel_ = new QLabel;
  QHBoxLayout* folderLayout = new QHBoxLayout;
  folderLayout->addWidget(folderLabel_);
  folderLayout->addWidget(connectionFolder_);

  QHBoxLayout* loggingLayout = new QHBoxLayout;
  logging_ = new QCheckBox;

  loggingMsec_ = new QSpinBox;
  loggingMsec_->setRange(0, INT32_MAX);
  loggingMsec_->setSingleStep(1000);

  VERIFY(connect(logging_, &QCheckBox::stateChanged, this,
                 &ConnectionAdvancedWidget::loggingStateChange));

  loggingLayout->addWidget(logging_);
  loggingLayout->addWidget(loggingMsec_);

  QVBoxLayout* advancedLayout = new QVBoxLayout;
  advancedLayout->addLayout(folderLayout);
  advancedLayout->addLayout(loggingLayout);
  setLayout(advancedLayout);

  // sync controls
  loggingStateChange(logging_->checkState());
  retranslateUi();
}

QString ConnectionAdvancedWidget::connectionFolderText() const {
  return connectionFolder_->text();
}

void ConnectionAdvancedWidget::setConnectionFolderText(const QString& text) {
  connectionFolder_->setText(text);
}

void ConnectionAdvancedWidget::setFolderEnabled(bool val) {
  connectionFolder_->setEnabled(val);
}

bool ConnectionAdvancedWidget::isLogging() const {
  return logging_->isChecked();
}

void ConnectionAdvancedWidget::setLogging(bool logging) {
  logging_->setChecked(logging);
}

int ConnectionAdvancedWidget::loggingInterval() const {
  return loggingMsec_->value();
}

void ConnectionAdvancedWidget::setLoggingInterval(int val) {
  loggingMsec_->setValue(val);
}

void ConnectionAdvancedWidget::loggingStateChange(int value) {
  loggingMsec_->setEnabled(value);
}

void ConnectionAdvancedWidget::retranslateUi() {
  folderLabel_->setText(tr("UI Folder:"));
  logging_->setText(translations::trLoggingEnabled);
  loggingMsec_->setToolTip(trLoggingToolTip);
}

}  // namespace gui
}  // namespace fastonosql
