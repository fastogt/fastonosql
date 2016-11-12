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

#include "gui/widgets/connection_base_widget.h"

#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QEvent>

#include <common/macros.h>
#include <common/convert2string.h>
#include <common/qt/convert2string.h>

#include "gui/gui_factory.h"

#include "translations/global.h"

namespace {
const char* defaultNameConnectionFolder = "/";
const QString trLoggingToolTip = QObject::tr("INFO command timeout in msec for history statistic.");
QString StableCommandLine(QString input) {
  return input.replace('\n', "\\n");
}

QString toRawCommandLine(QString input) {
  return input.replace("\\n", "\n");
}
}

namespace fastonosql {
namespace gui {

ConnectionBaseWidget::ConnectionBaseWidget(QWidget* parent) : QWidget(parent) {
  connectionName_ = new QLineEdit;
  commandLine_ = new QLineEdit;

  QVBoxLayout* basicLayout = new QVBoxLayout;
  QHBoxLayout* connectionNameLayout = new QHBoxLayout;
  connectionNameLabel_ = new QLabel;
  connectionNameLayout->addWidget(connectionNameLabel_);
  connectionNameLayout->addWidget(connectionName_);
  basicLayout->addLayout(connectionNameLayout);

  QHBoxLayout* commandLineLayout = new QHBoxLayout;
  commandLineLabel_ = new QLabel;
  commandLineLayout->addWidget(commandLineLabel_);
  commandLineLayout->addWidget(commandLine_);
  basicLayout->addLayout(commandLineLayout);

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

  VERIFY(
      connect(logging_, &QCheckBox::stateChanged, this, &ConnectionBaseWidget::loggingStateChange));
  loggingMsec_->setEnabled(false);

  loggingLayout->addWidget(logging_);
  loggingLayout->addWidget(loggingMsec_);

  basicLayout->addLayout(folderLayout);
  basicLayout->addLayout(loggingLayout);
  setLayout(basicLayout);
}

void ConnectionBaseWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

QString ConnectionBaseWidget::connectionName() const {
  return connectionName_->text();
}

core::IConnectionSettingsBase* ConnectionBaseWidget::createConnection() const {
  std::string conName = common::ConvertToString(connectionName());
  std::string conFolder = common::ConvertToString(connectionFolderText());
  if (conFolder.empty()) {
    conFolder = defaultNameConnectionFolder;
  }

  core::connection_path_t path(common::file_system::stable_dir_path(conFolder) + conName);
  core::IConnectionSettingsBase* conn = createConnectionImpl(path);
  conn->SetCommandLine(common::ConvertToString(toRawCommandLine(commandLine())));
  if (isLogging()) {
    conn->SetLoggingMsTimeInterval(loggingInterval());
  }

  return conn;
}

void ConnectionBaseWidget::setConnectionName(const QString& name) {
  connectionName_->setText(name);
}

QString ConnectionBaseWidget::commandLine() const {
  return commandLine_->text();
}

void ConnectionBaseWidget::setCommandLine(const QString& line) {
  commandLine_->setText(line);
}

void ConnectionBaseWidget::syncControls(core::IConnectionSettingsBase* connection) {
  if (connection) {
    core::connection_path_t path = connection->Path();
    setConnectionName(common::ConvertFromString<QString>(path.Name()));
    setConnectionFolderText(common::ConvertFromString<QString>(path.Directory()));
    setCommandLine(
        StableCommandLine(common::ConvertFromString<QString>(connection->CommandLine())));
    setLogging(connection->IsLoggingEnabled());
    setLoggingInterval(connection->LoggingMsTimeInterval());

    commandLine_->setToolTip(core::CommandLineHelpText(connection->Type()));
  }
}

void ConnectionBaseWidget::retranslateUi() {
  connectionNameLabel_->setText(tr("Name:"));
  commandLineLabel_->setText(tr("Connection line:"));

  folderLabel_->setText(tr("UI Folder:"));
  logging_->setText(translations::trLoggingEnabled);
  loggingMsec_->setToolTip(trLoggingToolTip);
}

bool ConnectionBaseWidget::validated() const {
  return true;
}

QString ConnectionBaseWidget::connectionFolderText() const {
  return connectionFolder_->text();
}

void ConnectionBaseWidget::setConnectionFolderText(const QString& text) {
  connectionFolder_->setText(text);
}

void ConnectionBaseWidget::setFolderEnabled(bool val) {
  connectionFolder_->setEnabled(val);
}

bool ConnectionBaseWidget::isLogging() const {
  return logging_->isChecked();
}

void ConnectionBaseWidget::setLogging(bool logging) {
  logging_->setChecked(logging);
}

int ConnectionBaseWidget::loggingInterval() const {
  return loggingMsec_->value();
}

void ConnectionBaseWidget::setLoggingInterval(int val) {
  loggingMsec_->setValue(val);
}

void ConnectionBaseWidget::loggingStateChange(int value) {
  loggingMsec_->setEnabled(value);
}

}  // namespace gui
}  // namespace fastonosql
