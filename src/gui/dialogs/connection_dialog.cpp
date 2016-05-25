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

#include <vector>
#include <string>

#include <QDialogButtonBox>
#include <QEvent>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>

#include "gui/dialogs/connection_diagnostic_dialog.h"

#include "common/qt/convert_string.h"

#include "gui/gui_factory.h"

#include "translations/global.h"

namespace {

const QString invalidDbType = QObject::tr("Invalid database type!");
const QString trTitle = QObject::tr("Connection Settings");
const QString trSelectPrivateKey = QObject::tr("Select private key file");
const QString trPrivateKey = QObject::tr("Private key files (*.*)");
const char* defaultNameConnectionFolder = "/";

QString stableCommandLine(QString input) {
  return input.replace('\n', "\\n");
}

QString toRawCommandLine(QString input) {
  return input.replace("\\n", "\n");
}

}  // namespace

namespace fastonosql {
namespace gui {

ConnectionDialog::ConnectionDialog(QWidget* parent, core::IConnectionSettingsBase* connection,
                                   const std::vector<core::connectionTypes>& availibleTypes,
                                   const QString& connectionName)
  : QDialog(parent), connection_(connection) {
  setWindowIcon(GuiFactory::instance().serverIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

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
    core::IConnectionSettings::connection_path_t path = connection_->path();
    conName = common::convertFromString<QString>(path.name());
    conFolder = common::convertFromString<QString>(path.directory());
  }
  connectionName_->setText(conName);
  connectionFolder_->setText(conFolder);

  typeConnection_ = new QComboBox;

  if (availibleTypes.empty()) {
    for (size_t i = 0; i < SIZEOFMASS(core::connnectionType); ++i) {
      std::string str = core::connnectionType[i];
      core::connectionTypes ct = common::convertFromString<core::connectionTypes>(str);
      typeConnection_->addItem(GuiFactory::instance().icon(ct),
                               common::convertFromString<QString>(str), ct);
    }
  } else {
    for (size_t i = 0; i < availibleTypes.size(); ++i) {
      core::connectionTypes ct = availibleTypes[i];
      std::string str = common::convertToString(ct);
      typeConnection_->addItem(GuiFactory::instance().icon(ct),
                               common::convertFromString<QString>(str), ct);
    }
  }

  if (connection_) {
    typeConnection_->setCurrentIndex(connection_->type());
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(typeConnection_, static_cast<qind>(&QComboBox::currentIndexChanged),
                 this, &ConnectionDialog::typeConnectionChange));

  QHBoxLayout* loggingLayout = new QHBoxLayout;
  logging_ = new QCheckBox;;
  loggingMsec_ = new QSpinBox;
  loggingMsec_->setRange(0, INT32_MAX);
  loggingMsec_->setSingleStep(1000);

  if (connection_) {
    logging_->setChecked(connection_->loggingEnabled());
    loggingMsec_->setValue(connection_->loggingMsTimeInterval());
  } else {
    logging_->setChecked(false);
  }
  VERIFY(connect(logging_, &QCheckBox::stateChanged, this, &ConnectionDialog::loggingStateChange));

  loggingLayout->addWidget(logging_);
  loggingLayout->addWidget(loggingMsec_);

  commandLine_ = new QLineEdit;
  commandLine_->setMinimumWidth(240);
  if (connection_) {
    commandLine_->setText(stableCommandLine(common::convertFromString<QString>(connection_->commandLine())));
  }

  QVBoxLayout* inputLayout = new QVBoxLayout;
  inputLayout->addWidget(connectionName_);
  inputLayout->addLayout(folderLayout);
  inputLayout->addWidget(typeConnection_);
  inputLayout->addLayout(loggingLayout);
  inputLayout->addWidget(commandLine_);

  // ssh

  core::IConnectionSettingsRemoteSSH* remoteSettings = dynamic_cast<core::IConnectionSettingsRemoteSSH*>(connection_.get());  // +
  core::SSHInfo info;
  if (remoteSettings) {
    info = remoteSettings->sshInfo();
  }
  useSsh_ = new QCheckBox;
  useSsh_->setChecked(info.isValid());

  sshHostName_ = new QLineEdit;
  common::net::hostAndPort host = info.host;
  sshHostName_->setText(common::convertFromString<QString>(host.host));

  userName_ = new QLineEdit;
  userName_->setText(common::convertFromString<QString>(info.user_name));

  sshPort_ = new QLineEdit;
  sshPort_->setFixedWidth(80);
  QRegExp rx("\\d+");  // (0-65554)
  sshPort_->setValidator(new QRegExpValidator(rx, this));
  sshPort_->setText(QString::number(host.port));

  passwordLabel_ = new QLabel;
  sshPrivateKeyLabel_ = new QLabel;
  sshPassphraseLabel_ = new QLabel;
  sshAddressLabel_ = new QLabel;
  sshUserNameLabel_ = new QLabel;
  sshAuthMethodLabel_ = new QLabel;

  security_ = new QComboBox;
  security_->addItems(QStringList() << translations::trPassword << translations::trPrivateKey);
  if (info.authMethod() == core::SSHInfo::PUBLICKEY) {
    security_->setCurrentText(translations::trPrivateKey);
  } else {
    security_->setCurrentText(translations::trPassword);
  }

  typedef void (QComboBox::*ind)(const QString&);
  VERIFY(connect(security_, static_cast<ind>(&QComboBox::currentIndexChanged),
                 this, &ConnectionDialog::securityChange));

  passwordBox_ = new QLineEdit;
  passwordBox_->setText(common::convertFromString<QString>(info.password));
  passwordBox_->setEchoMode(QLineEdit::Password);
  passwordEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passwordEchoModeButton_, &QPushButton::clicked,
                 this, &ConnectionDialog::togglePasswordEchoMode));

  privateKeyBox_ = new QLineEdit;
  privateKeyBox_->setText(common::convertFromString<QString>(info.private_key));

  passphraseBox_ = new QLineEdit;
  passphraseBox_->setText(common::convertFromString<QString>(info.passphrase));
  passphraseBox_->setEchoMode(QLineEdit::Password);
  passphraseEchoModeButton_ = new QPushButton(translations::trShow);
  VERIFY(connect(passphraseEchoModeButton_, &QPushButton::clicked,
                 this, &ConnectionDialog::togglePassphraseEchoMode));

  useSshWidget_ = new QWidget;

  QHBoxLayout* hostAndPasswordLayout = new QHBoxLayout;
  hostAndPasswordLayout->addWidget(sshHostName_);
  hostAndPasswordLayout->addWidget(new QLabel(":"));
  hostAndPasswordLayout->addWidget(sshPort_);

  QGridLayout* sshWidgetLayout = new QGridLayout;
  sshWidgetLayout->setAlignment(Qt::AlignTop);
  sshWidgetLayout->setColumnStretch(1, 1);
  sshWidgetLayout->addWidget(sshAddressLabel_ , 1, 0);
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
  sshWidgetLayout->addWidget(sshPassphraseLabel_, 8, 0);
  sshWidgetLayout->addWidget(passphraseBox_, 8, 1);
  sshWidgetLayout->addWidget(passphraseEchoModeButton_, 8, 2);
  useSshWidget_->setLayout(sshWidgetLayout);

  inputLayout->addWidget(useSsh_);

  VERIFY(connect(selectPrivateFileButton_, &QPushButton::clicked,
                 this, &ConnectionDialog::setPrivateFile));
  VERIFY(connect(useSsh_, &QCheckBox::stateChanged,
                 this, &ConnectionDialog::sshSupportStateChange));

  testButton_ = new QPushButton("&Test");
  testButton_->setIcon(GuiFactory::instance().messageBoxInformationIcon());
  VERIFY(connect(testButton_, &QPushButton::clicked,
                 this, &ConnectionDialog::testConnection));

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(testButton_, 1, Qt::AlignLeft);
  buttonBox_ = new QDialogButtonBox(this);
  buttonBox_->setOrientation(Qt::Horizontal);
  buttonBox_->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  VERIFY(connect(buttonBox_, &QDialogButtonBox::accepted,
                 this, &ConnectionDialog::accept));
  VERIFY(connect(buttonBox_, &QDialogButtonBox::rejected,
                 this, &ConnectionDialog::reject));
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
  core::connectionTypes currentType = (core::connectionTypes)qvariant_cast<unsigned char>(var);
  bool isSSHType = isCanSSHConnection(currentType);

  const char* helpText = core::commandLineHelpText(currentType);
  CHECK(helpText);
  QString trHelp = tr(helpText);
  commandLine_->setToolTip(trHelp);

  std::string commandLineText;
  if (connection_ && currentType == connection_->type()) {
    commandLineText = connection_->commandLine();
  } else {
    commandLineText = defaultCommandLine(currentType);
  }
  commandLine_->setText(stableCommandLine(common::convertFromString<QString>(commandLineText)));

  useSsh_->setEnabled(isSSHType);
  updateSshControls(isSSHType);
}

void ConnectionDialog::loggingStateChange(int value) {
  loggingMsec_->setEnabled(value);
}

void ConnectionDialog::securityChange(const QString& ) {
  bool isKey = selectedAuthMethod() == core::SSHInfo::PUBLICKEY;
  sshPrivateKeyLabel_->setVisible(isKey);
  privateKeyBox_->setVisible(isKey);
  selectPrivateFileButton_->setVisible(isKey);
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
  passwordBox_->setEchoMode(isPassword ? QLineEdit::Normal: QLineEdit::Password);
  passwordEchoModeButton_->setText(isPassword ? translations::trHide: translations::trShow);
}

void ConnectionDialog::togglePassphraseEchoMode() {
  bool isPassword = passphraseBox_->echoMode() == QLineEdit::Password;
  passphraseBox_->setEchoMode(isPassword ? QLineEdit::Normal: QLineEdit::Password);
  passphraseEchoModeButton_->setText(isPassword ? translations::trHide: translations::trShow);
}

void ConnectionDialog::setPrivateFile() {
  QString filepath = QFileDialog::getOpenFileName(this, trSelectPrivateKey,
                                                  privateKeyBox_->text(), trPrivateKey);
  if (filepath.isNull()) {
    return;
  }

  privateKeyBox_->setText(filepath);
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
  folderLabel_->setText(translations::trFolder);
  logging_->setText(translations::trLoggingEnabled);
  useSsh_->setText(tr("Use SSH tunnel"));
  passwordLabel_->setText(tr("User Password:"));
  sshPrivateKeyLabel_->setText(tr("Private key:"));
  sshPassphraseLabel_->setText(tr("Passphrase:"));
  sshAddressLabel_->setText(tr("SSH Address:"));
  sshUserNameLabel_->setText(tr("SSH User Name:"));
  sshAuthMethodLabel_->setText(tr("SSH Auth Method:"));
}

bool ConnectionDialog::validateAndApply() {
  QVariant var = typeConnection_->currentData();
  core::connectionTypes currentType = (core::connectionTypes)qvariant_cast<unsigned char>(var);

  bool isSSHType = isCanSSHConnection(currentType);
  std::string conName = common::convertToString(connectionName_->text());
  std::string conFolder = common::convertToString(connectionFolder_->text());
  if (conFolder.empty()) {
    conFolder = defaultNameConnectionFolder;
  }
  core::IConnectionSettingsRemoteSSH::connection_path_t path(common::file_system::stable_dir_path(conFolder) + conName);
  if (isSSHType) {
    core::IConnectionSettingsRemoteSSH* newConnection = core::IConnectionSettingsRemoteSSH::createFromType(currentType, path, common::net::hostAndPort());
    connection_.reset(newConnection);

    core::SSHInfo info = newConnection->sshInfo();
    info.host = common::net::hostAndPort(common::convertToString(sshHostName_->text()),
                                           sshPort_->text().toInt());
    info.user_name = common::convertToString(userName_->text());
    info.password = common::convertToString(passwordBox_->text());
    info.public_key = std::string();
    info.private_key = common::convertToString(privateKeyBox_->text());
    info.passphrase = common::convertToString(passphraseBox_->text());
    if (useSsh_->isChecked()) {
      info.current_method = selectedAuthMethod();
    } else {
      info.current_method = core::SSHInfo::UNKNOWN;
    }
    newConnection->setSshInfo(info);
  } else {
    core::IConnectionSettingsBase* newConnection = core::IConnectionSettingsBase::createFromType(currentType, path);
    connection_.reset(newConnection);
  }
  connection_->setCommandLine(common::convertToString(toRawCommandLine(commandLine_->text())));
  if (logging_->isChecked()) {
    connection_->setLoggingMsTimeInterval(loggingMsec_->value());
  }

  return true;
}

core::SSHInfo::SupportedAuthenticationMetods ConnectionDialog::selectedAuthMethod() const {
  if (security_->currentText() == translations::trPrivateKey) {
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
