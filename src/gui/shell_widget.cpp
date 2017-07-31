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

#include "gui/shell_widget.h"

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t

#include <memory>  // for __shared_ptr
#include <string>  // for string
#include <vector>  // for vector

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/error.h>           // for Error
#include <common/macros.h>          // for VERIFY, UNUSED, CHECK, etc
#include <common/value.h>           // for ErrorValue

#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/qt/gui/icon_label.h>  // for IconLabel
#include <common/qt/gui/shortcuts.h>   // for FastoQKeySequence
#include <common/qt/logger.h>          // for LOG_ERROR
#include <common/qt/utils_qt.h>        // for SaveToFileText, etc

#include "core/command_info.h"         // for UNDEFINED_SINCE, etc
#include "proxy/events/events_info.h"  // for DiscoveryInfoResponce, etc
#include "proxy/server/iserver.h"      // for IServer
#include "proxy/server/iserver_local.h"
#include "proxy/server/iserver_remote.h"
#include "proxy/settings_manager.h"  // for SettingsManager

#include "gui/gui_factory.h"  // for GuiFactory
#include "gui/shortcuts.h"    // for executeKey

#include "gui/base_shell.h"  // for BaseShell

#include "translations/global.h"  // for trError, trSaveAs, etc

namespace {
const QSize iconSize = QSize(24, 24);
const QString trSupportedCommandsCountTemplate_1S = QObject::tr("Supported commands count: %1");
const QString trCommandsVersion = QObject::tr("Command version:");
const QString trCantReadTemplate_2S = QObject::tr(PROJECT_NAME_TITLE " can't read from %1:\n%2.");
const QString trCantSaveTemplate_2S = QObject::tr(PROJECT_NAME_TITLE " can't save to %1:\n%2.");
const QString trAdvancedOptions = QObject::tr("Advanced options");
const QString trCalculating = QObject::tr("Calculate...");
const QString trIntervalMsec = QObject::tr("Interval msec:");
const QString trRepeat = QObject::tr("Repeat:");
}  // namespace

namespace fastonosql {
namespace gui {

namespace {
BaseShell* makeBaseShell(core::connectionTypes type, QWidget* parent) {
  BaseShell* shell = BaseShell::createFromType(type, proxy::SettingsManager::Instance().AutoCompletion());
  parent->setToolTip(QObject::tr("Based on <b>%1</b> version: <b>%2</b>").arg(shell->basedOn(), shell->version()));
  shell->setContextMenuPolicy(Qt::CustomContextMenu);
  return shell;
}
}  // namespace
BaseShellWidget::BaseShellWidget(proxy::IServerSPtr server, const QString& filePath, QWidget* parent)
    : QWidget(parent), server_(server), input_(nullptr), filePath_(filePath) {
  CHECK(server_);

  VERIFY(connect(server_.get(), &proxy::IServer::ConnectStarted, this, &BaseShellWidget::startConnect));
  VERIFY(connect(server_.get(), &proxy::IServer::ConnectFinished, this, &BaseShellWidget::finishConnect));
  VERIFY(connect(server_.get(), &proxy::IServer::DisconnectStarted, this, &BaseShellWidget::startDisconnect));
  VERIFY(connect(server_.get(), &proxy::IServer::DisconnectFinished, this, &BaseShellWidget::finishDisconnect));
  VERIFY(connect(server_.get(), &proxy::IServer::ProgressChanged, this, &BaseShellWidget::progressChange));

  VERIFY(connect(server_.get(), &proxy::IServer::ModeEntered, this, &BaseShellWidget::enterMode));
  VERIFY(connect(server_.get(), &proxy::IServer::ModeLeaved, this, &BaseShellWidget::leaveMode));

  VERIFY(connect(server_.get(), &proxy::IServer::LoadDiscoveryInfoStarted, this,
                 &BaseShellWidget::startLoadDiscoveryInfo));
  VERIFY(connect(server_.get(), &proxy::IServer::LoadDiscoveryInfoFinished, this,
                 &BaseShellWidget::finishLoadDiscoveryInfo));

  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteStarted, this, &BaseShellWidget::startExecute,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteFinished, this, &BaseShellWidget::finishExecute,
                 Qt::DirectConnection));

  VERIFY(
      connect(server_.get(), &proxy::IServer::CurrentDataBaseChanged, this, &BaseShellWidget::updateDefaultDatabase));
  VERIFY(connect(server_.get(), &proxy::IServer::Disconnected, this, &BaseShellWidget::serverDisconnect));

  QVBoxLayout* mainlayout = new QVBoxLayout;
  QHBoxLayout* hlayout = new QHBoxLayout;

  QToolBar* savebar = new QToolBar;

  loadAction_ = new QAction(gui::GuiFactory::Instance().loadIcon(), translations::trLoad, savebar);
  typedef void (BaseShellWidget::*lf)();
  VERIFY(connect(loadAction_, &QAction::triggered, this, static_cast<lf>(&BaseShellWidget::loadFromFile)));
  savebar->addAction(loadAction_);

  saveAction_ = new QAction(gui::GuiFactory::Instance().saveIcon(), translations::trSave, savebar);
  VERIFY(connect(saveAction_, &QAction::triggered, this, &BaseShellWidget::saveToFile));
  savebar->addAction(saveAction_);

  saveAsAction_ = new QAction(gui::GuiFactory::Instance().saveAsIcon(), translations::trSaveAs, savebar);
  VERIFY(connect(saveAsAction_, &QAction::triggered, this, &BaseShellWidget::saveToFileAs));
  savebar->addAction(saveAsAction_);

  connectAction_ = new QAction(gui::GuiFactory::Instance().connectIcon(), translations::trConnect, savebar);
  VERIFY(connect(connectAction_, &QAction::triggered, this, &BaseShellWidget::connectToServer));
  savebar->addAction(connectAction_);

  disConnectAction_ = new QAction(gui::GuiFactory::Instance().disConnectIcon(), translations::trDisconnect, savebar);
  VERIFY(connect(disConnectAction_, &QAction::triggered, this, &BaseShellWidget::disconnectFromServer));
  savebar->addAction(disConnectAction_);

  executeAction_ = new QAction(gui::GuiFactory::Instance().executeIcon(), translations::trExecute, savebar);
  executeAction_->setShortcut(gui::executeKey);
  VERIFY(connect(executeAction_, &QAction::triggered, this, &BaseShellWidget::execute));
  savebar->addAction(executeAction_);

  stopAction_ = new QAction(gui::GuiFactory::Instance().stopIcon(), translations::trStop, savebar);
  VERIFY(connect(stopAction_, &QAction::triggered, this, &BaseShellWidget::stop));
  savebar->addAction(stopAction_);

  core::ConnectionMode mode = core::InteractiveMode;
  std::string mode_str = common::ConvertToString(mode);
  QString qmode_str;
  common::ConvertFromString(mode_str, &qmode_str);
  connectionMode_ = new common::qt::gui::IconLabel(gui::GuiFactory::Instance().modeIcon(mode), qmode_str, iconSize);

  hlayout->addWidget(savebar);
  hlayout->addWidget(new QSplitter(Qt::Horizontal));

  hlayout->addWidget(connectionMode_);
  workProgressBar_ = new QProgressBar;
  workProgressBar_->setTextVisible(true);
  hlayout->addWidget(workProgressBar_);
  QToolBar* helpbar = new QToolBar;
  validateAction_ = new QAction(gui::GuiFactory::Instance().failIcon(), translations::trValidate, helpbar);
  VERIFY(connect(validateAction_, &QAction::triggered, this, &BaseShellWidget::validateClick));
  helpbar->addAction(validateAction_);

  QAction* helpAction = new QAction(gui::GuiFactory::Instance().helpIcon(), translations::trHelp, helpbar);
  VERIFY(connect(helpAction, &QAction::triggered, this, &BaseShellWidget::helpClick));
  helpbar->addAction(helpAction);
  hlayout->addWidget(helpbar);
  mainlayout->addLayout(hlayout);

  advancedOptions_ = new QCheckBox;
  advancedOptions_->setText(trAdvancedOptions);
  VERIFY(connect(advancedOptions_, &QCheckBox::stateChanged, this, &BaseShellWidget::advancedOptionsChange));

  input_ = makeBaseShell(server->Type(), this);
  VERIFY(connect(input_, &BaseShell::textChanged, this, &BaseShellWidget::inputTextChanged));

  advancedOptionsWidget_ = new QWidget;
  advancedOptionsWidget_->setVisible(false);
  QVBoxLayout* advOptLayout = new QVBoxLayout;

  QHBoxLayout* repeatLayout = new QHBoxLayout;
  QLabel* repeatLabel = new QLabel(trRepeat);
  repeatCount_ = new QSpinBox;
  repeatCount_->setRange(0, INT32_MAX);
  repeatCount_->setSingleStep(1);
  repeatLayout->addWidget(repeatLabel);
  repeatLayout->addWidget(repeatCount_);

  QHBoxLayout* intervalLayout = new QHBoxLayout;
  QLabel* intervalLabel = new QLabel(trIntervalMsec);
  intervalMsec_ = new QSpinBox;
  intervalMsec_->setRange(0, INT32_MAX);
  intervalMsec_->setSingleStep(1000);
  intervalLayout->addWidget(intervalLabel);
  intervalLayout->addWidget(intervalMsec_);

  historyCall_ = new QCheckBox(translations::trHistory);
  historyCall_->setChecked(true);
  advOptLayout->addLayout(repeatLayout);
  advOptLayout->addLayout(intervalLayout);
  advOptLayout->addWidget(historyCall_);
  advancedOptionsWidget_->setLayout(advOptLayout);

  QHBoxLayout* hlayout2 = new QHBoxLayout;
  core::connectionTypes ct = server_->Type();
  serverName_ = new common::qt::gui::IconLabel(gui::GuiFactory::Instance().icon(ct), trCalculating, iconSize);
  serverName_->setElideMode(Qt::ElideRight);
  hlayout2->addWidget(serverName_);
  dbName_ = new common::qt::gui::IconLabel(gui::GuiFactory::Instance().databaseIcon(), trCalculating, iconSize);
  hlayout2->addWidget(dbName_);
  hlayout2->addWidget(new QSplitter(Qt::Horizontal));
  hlayout2->addWidget(advancedOptions_);
  mainlayout->addLayout(hlayout2);

  QHBoxLayout* inputLayout = new QHBoxLayout;
  inputLayout->addWidget(input_);
  inputLayout->addWidget(advancedOptionsWidget_);
  mainlayout->addLayout(inputLayout);

  QHBoxLayout* apilayout = new QHBoxLayout;
  apilayout->addWidget(new QLabel(trSupportedCommandsCountTemplate_1S.arg(input_->commandsCount())));
  apilayout->addWidget(new QSplitter(Qt::Horizontal));

  commandsVersionApi_ = new QComboBox;
  typedef void (QComboBox::*curc)(int);
  VERIFY(connect(commandsVersionApi_, static_cast<curc>(&QComboBox::currentIndexChanged), this,
                 &BaseShellWidget::changeVersionApi));

  std::vector<uint32_t> versions = input_->supportedVersions();
  for (size_t i = 0; i < versions.size(); ++i) {
    uint32_t cur = versions[i];
    std::string curVers = core::ConvertVersionNumberToReadableString(cur);
    QString qcurVers;
    common::ConvertFromString(curVers, &qcurVers);
    commandsVersionApi_->addItem(gui::GuiFactory::Instance().unknownIcon(), qcurVers, cur);
    commandsVersionApi_->setCurrentIndex(i);
  }
  apilayout->addWidget(new QLabel(trCommandsVersion));
  apilayout->addWidget(commandsVersionApi_);
  mainlayout->addLayout(apilayout);

  setLayout(mainlayout);

  syncConnectionActions();
  updateServerInfo(server_->CurrentServerInfo());
  updateDefaultDatabase(server_->CurrentDatabaseInfo());
}

void BaseShellWidget::advancedOptionsChange(int state) {
  advancedOptionsWidget_->setVisible(state);
}

BaseShellWidget::~BaseShellWidget() {}

QString BaseShellWidget::text() const {
  return input_->text();
}

void BaseShellWidget::setText(const QString& text) {
  input_->setText(text);
}

void BaseShellWidget::executeText(const QString& text) {
  input_->setText(text);
  execute();
}

common::Error BaseShellWidget::validate(const QString& text) {
  core::translator_t tran = server_->Translator();
  std::vector<std::string> cmds;
  common::Error err = core::ParseCommands(common::ConvertToString(text), &cmds);
  if (err && err->IsError()) {
    return err;
  }

  for (auto cmd : cmds) {
    err = tran->TestCommandLine(cmd);
    if (err && err->IsError()) {
      return err;
    }
  }

  return common::Error();
}

void BaseShellWidget::execute() {
  QString selected = input_->selectedText();
  if (selected.isEmpty()) {
    selected = input_->text();
  }

  int repeat = repeatCount_->value();
  int interval = intervalMsec_->value();
  bool history = historyCall_->isChecked();
  executeArgs(selected, repeat, interval, history);
}

void BaseShellWidget::executeArgs(const QString& text, int repeat, int interval, bool history) {
  proxy::events_info::ExecuteInfoRequest req(this, common::ConvertToString(text), repeat, interval, history);
  server_->Execute(req);
}

void BaseShellWidget::stop() {
  server_->StopCurrentEvent();
}

void BaseShellWidget::connectToServer() {
  proxy::events_info::ConnectInfoRequest req(this);
  server_->Connect(req);
}

void BaseShellWidget::disconnectFromServer() {
  proxy::events_info::DisConnectInfoRequest req(this);
  server_->Disconnect(req);
}

void BaseShellWidget::loadFromFile() {
  loadFromFile(filePath_);
}

bool BaseShellWidget::loadFromFile(const QString& path) {
  QString filepath = QFileDialog::getOpenFileName(this, path, QString(), translations::trfilterForScripts);
  if (!filepath.isEmpty()) {
    QString out;
    common::Error err = common::qt::LoadFromFileText(filepath, &out);
    if (err && err->IsError()) {
      QString qdesc;
      common::ConvertFromString(err->GetDescription(), &qdesc);
      QMessageBox::critical(this, translations::trError, trCantReadTemplate_2S.arg(filepath, qdesc));
      return false;
    }

    setText(out);
    filePath_ = filepath;
    return true;
  }

  return false;
}

void BaseShellWidget::saveToFileAs() {
  QString filepath =
      QFileDialog::getSaveFileName(this, translations::trSaveAs, filePath_, translations::trfilterForScripts);
  if (filepath.isEmpty()) {
    return;
  }

  common::Error err = common::qt::SaveToFileText(filepath, text());
  if (err && err->IsError()) {
    QString qdesc;
    common::ConvertFromString(err->GetDescription(), &qdesc);
    QMessageBox::critical(this, translations::trError, trCantSaveTemplate_2S.arg(filepath, qdesc));
    return;
  }

  filePath_ = filepath;
}

void BaseShellWidget::changeVersionApi(int index) {
  if (index == -1) {
    return;
  }

  QVariant var = commandsVersionApi_->itemData(index);
  uint32_t version = qvariant_cast<uint32_t>(var);
  input_->setFilteredVersion(version);
}

void BaseShellWidget::saveToFile() {
  if (filePath_.isEmpty()) {
    saveToFileAs();
  } else {
    common::Error err = common::qt::SaveToFileText(filePath_, text());
    if (err && err->IsError()) {
      QString qdesc;
      common::ConvertFromString(err->GetDescription(), &qdesc);
      QMessageBox::critical(this, translations::trError, trCantSaveTemplate_2S.arg(filePath_, qdesc));
    }
  }
}

void BaseShellWidget::validateClick() {
  QString text = input_->text();
  common::Error err = validate(text);
  if (err && err->IsError()) {
    LOG_ERROR(err, true);
  }
}

void BaseShellWidget::helpClick() {
  executeArgs("HELP", 0, 0, false);
}

void BaseShellWidget::inputTextChanged() {
  QString text = input_->text();
  common::Error err = validate(text);
  if (err && err->IsError()) {
    validateAction_->setIcon(gui::GuiFactory::Instance().failIcon());
  } else {
    validateAction_->setIcon(gui::GuiFactory::Instance().successIcon());
  }
}

void BaseShellWidget::startConnect(const proxy::events_info::ConnectInfoRequest& req) {
  UNUSED(req);

  syncConnectionActions();
}

void BaseShellWidget::finishConnect(const proxy::events_info::ConnectInfoResponce& res) {
  UNUSED(res);

  serverConnect();
}

void BaseShellWidget::startDisconnect(const proxy::events_info::DisConnectInfoRequest& req) {
  UNUSED(req);

  syncConnectionActions();
}

void BaseShellWidget::finishDisconnect(const proxy::events_info::DisConnectInfoResponce& res) {
  UNUSED(res);

  serverDisconnect();
}

void BaseShellWidget::progressChange(const proxy::events_info::ProgressInfoResponce& res) {
  workProgressBar_->setValue(res.progress);
}

void BaseShellWidget::enterMode(const proxy::events_info::EnterModeInfo& res) {
  core::ConnectionMode mode = res.mode;
  connectionMode_->setIcon(gui::GuiFactory::Instance().modeIcon(mode), iconSize);
  std::string modeText = common::ConvertToString(mode);
  QString qmodeText;
  common::ConvertFromString(modeText, &qmodeText);
  connectionMode_->setText(qmodeText);
}

void BaseShellWidget::leaveMode(const proxy::events_info::LeaveModeInfo& res) {
  UNUSED(res);
}

void BaseShellWidget::startLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoRequest& res) {
  UNUSED(res);
}

void BaseShellWidget::finishLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoResponce& res) {
  common::Error err = res.errorInfo();
  if (err && err->IsError()) {
    return;
  }

  updateServerInfo(res.sinfo);
  updateDefaultDatabase(res.dbinfo);
}

void BaseShellWidget::startExecute(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);

  repeatCount_->setEnabled(false);
  intervalMsec_->setEnabled(false);
  historyCall_->setEnabled(false);
  executeAction_->setEnabled(false);
  stopAction_->setEnabled(true);
}
void BaseShellWidget::finishExecute(const proxy::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);

  repeatCount_->setEnabled(true);
  intervalMsec_->setEnabled(true);
  historyCall_->setEnabled(true);
  executeAction_->setEnabled(true);
  stopAction_->setEnabled(false);
}

void BaseShellWidget::serverConnect() {
  syncConnectionActions();
}

void BaseShellWidget::serverDisconnect() {
  syncConnectionActions();
  updateServerInfo(core::IServerInfoSPtr());
  updateDefaultDatabase(core::IDataBaseInfoSPtr());
}

void BaseShellWidget::updateServerInfo(core::IServerInfoSPtr inf) {
  if (!inf) {
    serverName_->setText(trCalculating);
    for (int i = 0; i < commandsVersionApi_->count(); ++i) {
      commandsVersionApi_->setItemIcon(i, gui::GuiFactory::Instance().unknownIcon());
    }
    return;
  }

  std::string server_label;
  if (server_->IsCanRemote()) {
    proxy::IServerRemote* rserver = dynamic_cast<proxy::IServerRemote*>(server_.get());  // +
    server_label = common::ConvertToString(rserver->Host());
  } else {
    proxy::IServerLocal* lserver = dynamic_cast<proxy::IServerLocal*>(server_.get());  // +
    server_label = lserver->Path();
  }
  QString qserver_label;
  common::ConvertFromString(server_label, &qserver_label);
  serverName_->setText(qserver_label);

  uint32_t serv_vers = inf->Version();
  if (serv_vers == UNDEFINED_SINCE) {
    return;
  }

  bool updatedComboIndex = false;
  for (int i = 0; i < commandsVersionApi_->count(); ++i) {
    QVariant var = commandsVersionApi_->itemData(i);
    uint32_t version = qvariant_cast<uint32_t>(var);
    if (version == UNDEFINED_SINCE) {
      commandsVersionApi_->setItemIcon(i, gui::GuiFactory::Instance().unknownIcon());
      continue;
    }

    if (version >= serv_vers) {
      if (!updatedComboIndex) {
        updatedComboIndex = true;
        commandsVersionApi_->setCurrentIndex(i);
        commandsVersionApi_->setItemIcon(i, gui::GuiFactory::Instance().successIcon());
      } else {
        commandsVersionApi_->setItemIcon(i, gui::GuiFactory::Instance().failIcon());
      }
    } else {
      commandsVersionApi_->setItemIcon(i, gui::GuiFactory::Instance().successIcon());
    }
  }
}

void BaseShellWidget::updateDefaultDatabase(core::IDataBaseInfoSPtr dbs) {
  if (!dbs) {
    dbName_->setText(trCalculating);
    return;
  }

  std::string name = dbs->Name();
  QString qname;
  common::ConvertFromString(name, &qname);
  dbName_->setText(qname);
}

void BaseShellWidget::syncConnectionActions() {
  bool is_connected = server_->IsConnected();

  connectAction_->setVisible(!is_connected);
  disConnectAction_->setVisible(is_connected);
  executeAction_->setEnabled(true);
  stopAction_->setEnabled(false);
}

}  // namespace gui
}  // namespace fastonosql
