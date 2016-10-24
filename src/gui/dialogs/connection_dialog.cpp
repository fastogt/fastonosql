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

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

#include <common/convert2string.h>     // for ConvertFromString
#include <common/file_system.h>        // for stable_dir_path
#include <common/macros.h>             // for VERIFY, CHECK, SIZEOFMASS
#include <common/net/types.h>          // for HostAndPort
#include <common/qt/convert2string.h>  // for ConvertToString

#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trShow, trPrivateKey, etc

namespace {
const QString invalidDbType = QObject::tr("Invalid database type!");
const QString trTitle = QObject::tr("Connection Settings");
const QString trLoggingToolTip = QObject::tr("INFO command timeout in msec for history statistic.");
const QString trSelectPrivateKey = QObject::tr("Select private key file");
const QString trPrivateKeyFiles = QObject::tr("Private key files (*.*)");
const QString trSelectPublicKey = QObject::tr("Select public key file");
const QString trPublicKeyFiles = QObject::tr("Public key files (*.*)");
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

  connectionName_ = new QLineEdit;
  connectionFolder_ = new QLineEdit;
  QRegExp rxf("^/[A-z0-9]+/$");
  connectionFolder_->setValidator(new QRegExpValidator(rxf, this));

  folderLabel_ = new QLabel;
  QHBoxLayout* folderLayout = new QHBoxLayout;
  folderLayout->addWidget(folderLabel_);
  folderLayout->addWidget(connectionFolder_);

  QString conFolder = defaultNameConnectionFolder;
  QString conName = connectionName;

  if (connection_) {
    core::IConnectionSettings::connection_path_t path = connection_->Path();
    conName = common::ConvertFromString<QString>(path.Name());
    conFolder = common::ConvertFromString<QString>(path.Directory());
  }
  connectionName_->setText(conName);
  connectionFolder_->setText(conFolder);

  typeConnection_ = new QComboBox;

  if (availibleTypes.empty()) {
    for (size_t i = 0; i < SIZEOFMASS(core::compiled_types); ++i) {
      core::connectionTypes ct = core::compiled_types[i];
      std::string str = common::ConvertToString(ct);
      typeConnection_->addItem(GuiFactory::instance().icon(ct),
                               common::ConvertFromString<QString>(str), ct);
    }
  } else {
    for (size_t i = 0; i < availibleTypes.size(); ++i) {
      core::connectionTypes ct = availibleTypes[i];
      std::string str = common::ConvertToString(ct);
      typeConnection_->addItem(GuiFactory::instance().icon(ct),
                               common::ConvertFromString<QString>(str), ct);
    }
  }

  if (connection_) {
    typeConnection_->setCurrentIndex(connection_->Type());
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(typeConnection_, static_cast<qind>(&QComboBox::currentIndexChanged), this,
                 &ConnectionDialog::typeConnectionChange));

  QHBoxLayout* loggingLayout = new QHBoxLayout;
  logging_ = new QCheckBox;
  ;
  loggingMsec_ = new QSpinBox;
  loggingMsec_->setRange(0, INT32_MAX);
  loggingMsec_->setSingleStep(1000);

  if (connection_) {
    logging_->setChecked(connection_->IsLoggingEnabled());
    loggingMsec_->setValue(connection_->LoggingMsTimeInterval());
  } else {
    logging_->setChecked(false);
  }
  VERIFY(connect(logging_, &QCheckBox::stateChanged, this, &ConnectionDialog::loggingStateChange));

  loggingLayout->addWidget(logging_);
  loggingLayout->addWidget(loggingMsec_);

  commandLine_ = new QLineEdit;
  if (connection_) {
    commandLine_->setText(
        StableCommandLine(common::ConvertFromString<QString>(connection_->CommandLine())));
  }

  QVBoxLayout* inputLayout = new QVBoxLayout;
  inputLayout->addWidget(connectionName_);
  inputLayout->addLayout(folderLayout);
  inputLayout->addWidget(typeConnection_);
  inputLayout->addLayout(loggingLayout);
  inputLayout->addWidget(commandLine_);

  // ssh

  core::IConnectionSettingsRemoteSSH* remoteSettings =
      dynamic_cast<core::IConnectionSettingsRemoteSSH*>(connection_.get());  // +
  core::SSHInfo info = remoteSettings ? remoteSettings->SSHInfo() : core::SSHInfo();
  useSsh_ = new QCheckBox;
  useSsh_->setChecked(info.IsValid());

  sshHostName_ = new QLineEdit;
  common::net::HostAndPort host = info.host;
  sshHostName_->setText(common::ConvertFromString<QString>(host.host));

  userName_ = new QLineEdit;
  userName_->setText(common::ConvertFromString<QString>(info.user_name));

  sshPort_ = new QLineEdit;
  sshPort_->setFixedWidth(80);
  QRegExp rx("\\d+");  // (0-65554)
  sshPort_->setValidator(new QRegExpValidator(rx, this));
  sshPort_->setText(QString::number(host.port));

  passwordLabel_ = new QLabel;
  sshPrivateKeyLabel_ = new QLabel;
  sshPublicKeyLabel_ = new QLabel;
  sshPassphraseLabel_ = new QLabel;
  sshAddressLabel_ = new QLabel;
  sshUserNameLabel_ = new QLabel;
  sshAuthMethodLabel_ = new QLabel;

  security_ = new QComboBox;
  security_->addItems(QStringList() << translations::trPassword
                                    << translations::trPublicPrivateKey);
  if (info.AuthMethod() == core::SSHInfo::PUBLICKEY) {
    security_->setCurrentText(translations::trPublicPrivateKey);
  } else {
    security_->setCurrentText(translations::trPassword);
  }

  typedef void (QComboBox::*ind)(const QString&);
  VERIFY(connect(security_, static_cast<ind>(&QComboBox::currentIndexChanged), this,
                 &ConnectionDialog::securityChange));

  passwordBox_ = new QLineEdit;
  passwordBox_->setText(common::ConvertFromString<QString>(info.password));
  passwordBox_->setEchoMode(QLineEdit::Password);
  passwordEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passwordEchoModeButton_, &QPushButton::clicked, this,
                 &ConnectionDialog::togglePasswordEchoMode));

  privateKeyBox_ = new QLineEdit;
  privateKeyBox_->setText(common::ConvertFromString<QString>(info.private_key));

  publicKeyBox_ = new QLineEdit;
  publicKeyBox_->setText(common::ConvertFromString<QString>(info.public_key));

  passphraseBox_ = new QLineEdit;
  passphraseBox_->setText(common::ConvertFromString<QString>(info.passphrase));
  passphraseBox_->setEchoMode(QLineEdit::Password);
  passphraseEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passphraseEchoModeButton_, &QPushButton::clicked, this,
                 &ConnectionDialog::togglePassphraseEchoMode));

  useSshWidget_ = new QWidget;

  QHBoxLayout* hostAndPasswordLayout = new QHBoxLayout;
  hostAndPasswordLayout->addWidget(sshHostName_);
  hostAndPasswordLayout->addWidget(new QLabel(":"));
  hostAndPasswordLayout->addWidget(sshPort_);

  QGridLayout* sshWidgetLayout = new QGridLayout;
  sshWidgetLayout->setAlignment(Qt::AlignTop);
  sshWidgetLayout->setColumnStretch(1, 1);
  sshWidgetLayout->addWidget(sshAddressLabel_, 1, 0);
  sshWidgetLayout->addLayout(hostAndPasswordLayout, 1, 1, 1, 2);
  sshWidgetLayout->addWidget(sshUserNameLabel_, 2, 0);
  sshWidgetLayout->addWidget(userName_, 2, 1, 1, 2);
  sshWidgetLayout->addWidget(sshAuthMethodLabel_, 4, 0);
  sshWidgetLayout->addWidget(security_, 4, 1, 1, 2);
  sshWidgetLayout->addWidget(passwordLabel_, 5, 0);
  sshWidgetLayout->addWidget(passwordBox_, 5, 1);
  sshWidgetLayout->addWidget(passwordEchoModeButton_, 5, 2);

  sshWidgetLayout->addWidget(sshPrivateKeyLabel_, 7, 0);
  sshWidgetLayout->addWidget(privateKeyBox_, 7, 1);
  selectPrivateFileButton_ = new QPushButton("...");
  selectPrivateFileButton_->setFixedSize(20, 20);
  sshWidgetLayout->addWidget(selectPrivateFileButton_, 7, 2);

  sshWidgetLayout->addWidget(sshPublicKeyLabel_, 8, 0);
  sshWidgetLayout->addWidget(publicKeyBox_, 8, 1);
  selectPublicFileButton_ = new QPushButton("...");
  selectPublicFileButton_->setFixedSize(20, 20);
  sshWidgetLayout->addWidget(selectPublicFileButton_, 8, 2);

  sshWidgetLayout->addWidget(sshPassphraseLabel_, 9, 0);
  sshWidgetLayout->addWidget(passphraseBox_, 9, 1);
  sshWidgetLayout->addWidget(passphraseEchoModeButton_, 9, 2);
  useSshWidget_->setLayout(sshWidgetLayout);

  inputLayout->addWidget(useSsh_);

  VERIFY(connect(selectPrivateFileButton_, &QPushButton::clicked, this,
                 &ConnectionDialog::setPrivateFile));
  VERIFY(connect(selectPublicFileButton_, &QPushButton::clicked, this,
                 &ConnectionDialog::setPublicFile));
  VERIFY(
      connect(useSsh_, &QCheckBox::stateChanged, this, &ConnectionDialog::sshSupportStateChange));

  testButton_ = new QPushButton("&Test");
  testButton_->setIcon(GuiFactory::instance().messageBoxInformationIcon());
  VERIFY(connect(testButton_, &QPushButton::clicked, this, &ConnectionDialog::TestConnection));

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(testButton_, 1, Qt::AlignLeft);
  buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  buttonBox_->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox_, &QDialogButtonBox::accepted, this, &ConnectionDialog::accept));
  VERIFY(connect(buttonBox_, &QDialogButtonBox::rejected, this, &ConnectionDialog::reject));
  bottomLayout->addWidget(buttonBox_);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addLayout(inputLayout);
  mainLayout->addWidget(useSshWidget_);
  mainLayout->addLayout(bottomLayout);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(mainLayout);

  // update controls
  sshSupportStateChange(useSsh_->checkState());
  securityChange(security_->currentText());
  typeConnectionChange(typeConnection_->currentIndex());
  loggingStateChange(logging_->checkState());
  retranslateUi();
}

void ConnectionDialog::setFolderEnabled(bool val) {
  connectionFolder_->setEnabled(val);
}

core::IConnectionSettingsBaseSPtr ConnectionDialog::connection() const {
  return connection_;
}

void ConnectionDialog::accept() {
  if (validateAndApply()) {
    QDialog::accept();
  }
}

void ConnectionDialog::typeConnectionChange(int index) {
  QVariant var = typeConnection_->itemData(index);
  core::connectionTypes currentType =
      static_cast<core::connectionTypes>(qvariant_cast<unsigned char>(var));
  bool is_ssh_type = IsCanSSHConnection(currentType);

  const char* helpText = core::CommandLineHelpText(currentType);
  CHECK(helpText);
  QString trHelp = tr(helpText);
  commandLine_->setToolTip(trHelp);

  std::string commandLineText;
  if (connection_ && currentType == connection_->Type()) {
    commandLineText = connection_->CommandLine();
  } else {
    commandLineText = DefaultCommandLine(currentType);
  }
  commandLine_->setText(StableCommandLine(common::ConvertFromString<QString>(commandLineText)));

  useSsh_->setEnabled(is_ssh_type);
  updateSshControls(is_ssh_type);
}

void ConnectionDialog::loggingStateChange(int value) {
  loggingMsec_->setEnabled(value);
}

void ConnectionDialog::securityChange(const QString&) {
  bool isKey = selectedAuthMethod() == core::SSHInfo::PUBLICKEY;
  sshPrivateKeyLabel_->setVisible(isKey);
  privateKeyBox_->setVisible(isKey);
  selectPrivateFileButton_->setVisible(isKey);
  sshPublicKeyLabel_->setVisible(isKey);
  publicKeyBox_->setVisible(isKey);
  selectPublicFileButton_->setVisible(isKey);
  sshPassphraseLabel_->setVisible(isKey);
  passphraseBox_->setVisible(isKey);
  passphraseEchoModeButton_->setVisible(isKey);
  passwordBox_->setVisible(!isKey);
  passwordLabel_->setVisible(!isKey);
  passwordEchoModeButton_->setVisible(!isKey);
}

void ConnectionDialog::sshSupportStateChange(int value) {
  useSshWidget_->setVisible(value);
  updateSshControls(value);
}

void ConnectionDialog::togglePasswordEchoMode() {
  bool isPassword = passwordBox_->echoMode() == QLineEdit::Password;
  passwordBox_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  passwordEchoModeButton_->setText(isPassword ? translations::trHide : translations::trShow);
}

void ConnectionDialog::togglePassphraseEchoMode() {
  bool isPassword = passphraseBox_->echoMode() == QLineEdit::Password;
  passphraseBox_->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
  passphraseEchoModeButton_->setText(isPassword ? translations::trHide : translations::trShow);
}

void ConnectionDialog::setPrivateFile() {
  QString filepath = QFileDialog::getOpenFileName(this, trSelectPrivateKey, privateKeyBox_->text(),
                                                  trPrivateKeyFiles);
  if (filepath.isNull()) {
    return;
  }

  privateKeyBox_->setText(filepath);
}

void ConnectionDialog::setPublicFile() {
  QString filepath = QFileDialog::getOpenFileName(this, trSelectPublicKey, publicKeyBox_->text(),
                                                  trPublicKeyFiles);
  if (filepath.isNull()) {
    return;
  }

  publicKeyBox_->setText(filepath);
}

void ConnectionDialog::TestConnection() {
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
  folderLabel_->setText(translations::trFolder);
  logging_->setText(translations::trLoggingEnabled);
  useSsh_->setText(tr("Use SSH tunnel"));
  passwordLabel_->setText(tr("User Password:"));
  sshPrivateKeyLabel_->setText(tr("Private key:"));
  sshPublicKeyLabel_->setText(tr("Public key:"));
  sshPassphraseLabel_->setText(tr("Passphrase:"));
  sshAddressLabel_->setText(tr("SSH Address:"));
  sshUserNameLabel_->setText(tr("SSH User Name:"));
  sshAuthMethodLabel_->setText(tr("SSH Auth Method:"));
  loggingMsec_->setToolTip(trLoggingToolTip);
}

bool ConnectionDialog::validateAndApply() {
  QVariant var = typeConnection_->currentData();
  core::connectionTypes currentType =
      static_cast<core::connectionTypes>(qvariant_cast<unsigned char>(var));

  bool is_ssh_type = IsCanSSHConnection(currentType);
  std::string conName = common::ConvertToString(connectionName_->text());
  std::string conFolder = common::ConvertToString(connectionFolder_->text());
  if (conFolder.empty()) {
    conFolder = defaultNameConnectionFolder;
  }
  core::IConnectionSettingsRemoteSSH::connection_path_t path(
      common::file_system::stable_dir_path(conFolder) + conName);
  if (is_ssh_type) {
    core::IConnectionSettingsRemoteSSH* newConnection =
        core::IConnectionSettingsRemoteSSH::createFromType(currentType, path,
                                                           common::net::HostAndPort());
    connection_.reset(newConnection);

    core::SSHInfo info = newConnection->SSHInfo();
    info.host = common::net::HostAndPort(common::ConvertToString(sshHostName_->text()),
                                         sshPort_->text().toInt());
    info.user_name = common::ConvertToString(userName_->text());
    info.password = common::ConvertToString(passwordBox_->text());
    info.public_key = common::ConvertToString(publicKeyBox_->text());
    info.private_key = common::ConvertToString(privateKeyBox_->text());
    info.passphrase = common::ConvertToString(passphraseBox_->text());
    if (useSsh_->isChecked()) {
      info.current_method = selectedAuthMethod();
      if (info.current_method == core::SSHInfo::PUBLICKEY && info.private_key.empty()) {
        QMessageBox::critical(this, translations::trError, trPrivateKeyInvalidInput);
        privateKeyBox_->setFocus();
        return false;
      }
    } else {
      info.current_method = core::SSHInfo::UNKNOWN;
    }
    newConnection->SetSSHInfo(info);
  } else {
    core::IConnectionSettingsBase* newConnection =
        core::IConnectionSettingsBase::CreateFromType(currentType, path);
    connection_.reset(newConnection);
  }
  connection_->SetCommandLine(common::ConvertToString(toRawCommandLine(commandLine_->text())));
  if (logging_->isChecked()) {
    connection_->SetLoggingMsTimeInterval(loggingMsec_->value());
  }

  return true;
}

core::SSHInfo::SupportedAuthenticationMetods ConnectionDialog::selectedAuthMethod() const {
  if (security_->currentText() == translations::trPublicPrivateKey) {
    return core::SSHInfo::PUBLICKEY;
  }

  return core::SSHInfo::PASSWORD;
}

void ConnectionDialog::updateSshControls(bool isValidType) {
  sshHostName_->setEnabled(isValidType);
  userName_->setEnabled(isValidType);
  sshPort_->setEnabled(isValidType);
  security_->setEnabled(isValidType);
  sshPrivateKeyLabel_->setEnabled(isValidType);
  privateKeyBox_->setEnabled(isValidType);
  publicKeyBox_->setEnabled(isValidType);
  selectPrivateFileButton_->setEnabled(isValidType);
  sshAddressLabel_->setEnabled(isValidType);
  sshUserNameLabel_->setEnabled(isValidType);
  sshAuthMethodLabel_->setEnabled(isValidType);
  sshPassphraseLabel_->setEnabled(isValidType);
  passphraseBox_->setEnabled(isValidType);
  passwordBox_->setEnabled(isValidType);
  passwordLabel_->setEnabled(isValidType);
}
}  // namespace gui
}  // namespace fastonosql
