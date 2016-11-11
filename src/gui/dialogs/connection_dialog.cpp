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

#include "gui/dialogs/connection_dialog.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for INT32_MAX

#include <memory>  // for __shared_ptr
#include <string>  // for string, operator+, etc
#include <vector>  // for allocator, vector

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QEvent>

#include <common/convert2string.h>     // for ConvertFromString
#include <common/qt/convert2string.h>  // for ConvertToString

#include "core/connection_settings_factory.h"

#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/widgets/connection_advanced_widget.h"
#include "gui/widgets/connection_basic_widget.h"
#include "gui/widgets/connection_ssh_widget.h"
#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trShow, trPrivateKey, etc

namespace {
const QString invalidDbType = QObject::tr("Invalid database type!");
const QString trTitle = QObject::tr("Connection Settings");
const QString trPrivateKeyInvalidInput = QObject::tr("Invalid private key value!");
const char* defaultNameConnectionFolder = "/";

QString StableCommandLine(QString input) {
  return input.replace('\n', "\\n");
}

QString toRawCommandLine(QString input) {
  return input.replace("\\n", "\n");
}
}  // namespace

namespace fastonosql {
namespace gui {

ConnectionDialog::ConnectionDialog(QWidget* parent,
                                   core::IConnectionSettingsBase* connection,
                                   const std::vector<core::connectionTypes>& availibleTypes,
                                   const QString& connectionName)
    : QDialog(parent), connection_(connection) {
  setWindowIcon(GuiFactory::instance().serverIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  QTabWidget* tabs = new QTabWidget;
  basic_widget_ = new ConnectionBasicWidget(availibleTypes);
  tabs->addTab(basic_widget_, translations::trBasic);
  ssh_widget_ = new ConnectionSSHWidget;
  tabs->addTab(ssh_widget_, "SSH");
  advanced_widget_ = new ConnectionAdvancedWidget;
  tabs->addTab(advanced_widget_, translations::trAdvanced);

  testButton_ = new QPushButton("&Test");
  testButton_->setIcon(GuiFactory::instance().messageBoxInformationIcon());
  VERIFY(connect(testButton_, &QPushButton::clicked, this, &ConnectionDialog::testConnection));

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(testButton_, 1, Qt::AlignLeft);
  buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  buttonBox_->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox_, &QDialogButtonBox::accepted, this, &ConnectionDialog::accept));
  VERIFY(connect(buttonBox_, &QDialogButtonBox::rejected, this, &ConnectionDialog::reject));
  bottomLayout->addWidget(buttonBox_);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabs);
  mainLayout->addLayout(bottomLayout);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(mainLayout);

  // update controls
  VERIFY(connect(basic_widget_, &ConnectionBasicWidget::typeConnectionChanged, this,
                 &ConnectionDialog::typeConnectionChange));

  QString conFolder = defaultNameConnectionFolder;
  QString conName = connectionName;

  core::SSHInfo info;
  if (connection) {
    core::IConnectionSettingsRemoteSSH* remoteSettings =
        dynamic_cast<core::IConnectionSettingsRemoteSSH*>(connection);  // +
    if (remoteSettings) {
      info = remoteSettings->SSHInfo();
    }
    core::connection_path_t path = connection->Path();
    conName = common::ConvertFromString<QString>(path.Name());
    conFolder = common::ConvertFromString<QString>(path.Directory());
    advanced_widget_->setLogging(connection->IsLoggingEnabled());
    advanced_widget_->setLoggingInterval(connection->LoggingMsTimeInterval());
    QString stabled =
        StableCommandLine(common::ConvertFromString<QString>(connection->CommandLine()));
    basic_widget_->setCommandLine(stabled);
    basic_widget_->setConnectionType(connection->Type());
  } else {
    advanced_widget_->setLogging(false);
    typeConnectionChange(basic_widget_->connectionType());
  }

  basic_widget_->setConnectionName(conName);
  ssh_widget_->setInfo(info);
  advanced_widget_->setConnectionFolderText(conFolder);
  retranslateUi();
}

core::IConnectionSettingsBaseSPtr ConnectionDialog::connection() const {
  return connection_;
}

void ConnectionDialog::setFolderEnabled(bool val) {
  advanced_widget_->setFolderEnabled(val);
}

void ConnectionDialog::accept() {
  if (validateAndApply()) {
    QDialog::accept();
  }
}

void ConnectionDialog::typeConnectionChange(core::connectionTypes type) {
  bool is_ssh_type = IsCanSSHConnection(type);
  std::string commandLineText;
  if (connection_ && type == connection_->Type()) {
    commandLineText = connection_->CommandLine();
  } else {
    commandLineText = DefaultCommandLine(type);
  }

  basic_widget_->setCommandLine(
      StableCommandLine(common::ConvertFromString<QString>(commandLineText)));
  ssh_widget_->setSSHEnabled(is_ssh_type);
}

void ConnectionDialog::testConnection() {
  if (validateAndApply()) {
    ConnectionDiagnosticDialog diag(this, connection_);
    diag.exec();
  }
}

void ConnectionDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void ConnectionDialog::retranslateUi() {
  setWindowTitle(trTitle);
}

bool ConnectionDialog::validateAndApply() {
  core::connectionTypes currentType = basic_widget_->connectionType();

  bool is_ssh_type = IsCanSSHConnection(currentType);
  std::string conName = common::ConvertToString(basic_widget_->connectionName());
  std::string conFolder = common::ConvertToString(advanced_widget_->connectionFolderText());
  if (conFolder.empty()) {
    conFolder = defaultNameConnectionFolder;
  }
  core::connection_path_t path(common::file_system::stable_dir_path(conFolder) + conName);
  if (is_ssh_type) {
    core::IConnectionSettingsRemoteSSH* newConnection =
        core::ConnectionSettingsFactory::instance().CreateSSHFromType(currentType, path,
                                                                      common::net::HostAndPort());
    connection_.reset(newConnection);

    core::SSHInfo info = ssh_widget_->info();
    if (ssh_widget_->isSSHChecked()) {
      if (info.current_method == core::SSHInfo::PUBLICKEY && info.private_key.empty()) {
        QMessageBox::critical(this, translations::trError, trPrivateKeyInvalidInput);
        return false;
      }
    } else {
      info.current_method = core::SSHInfo::UNKNOWN;
    }
    newConnection->SetSSHInfo(info);
  } else {
    core::IConnectionSettingsBase* newConnection =
        core::ConnectionSettingsFactory::instance().CreateFromType(currentType, path);
    connection_.reset(newConnection);
  }
  connection_->SetCommandLine(
      common::ConvertToString(toRawCommandLine(basic_widget_->commandLine())));
  if (advanced_widget_->isLogging()) {
    connection_->SetLoggingMsTimeInterval(advanced_widget_->loggingInterval());
  }

  return true;
}

}  // namespace gui
}  // namespace fastonosql
