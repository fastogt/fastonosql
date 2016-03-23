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

#include "shell/shell_widget.h"

#include <string>
#include <vector>

#include <QProgressBar>
#include <QSplitter>
#include <QAction>
#include <QToolBar>
#include <QVBoxLayout>
#include <QApplication>
#include <QFileDialog>
#include <QComboBox>
#include <QLabel>

#include "common/qt/convert_string.h"
#include "common/sprintf.h"

#include "fasto/qt/logger.h"
#include "fasto/qt/gui/icon_label.h"
#include "fasto/qt/utils_qt.h"

#include "core/settings_manager.h"
#include "core/iserver.h"

#include "shell/base_shell.h"
#include "gui/shortcuts.h"

#include "gui/gui_factory.h"

#include "translations/global.h"

namespace {
  const QSize iconSize = QSize(24, 24);
}

namespace fastonosql {
namespace shell {

BaseShellWidget::BaseShellWidget(IServerSPtr server, const QString& filePath, QWidget* parent)
  : QWidget(parent), server_(server), filePath_(filePath), input_(nullptr) {
  CHECK(server_);

  VERIFY(connect(server_.get(), &IServer::startedConnect,
                 this, &BaseShellWidget::startConnect));
  VERIFY(connect(server_.get(), &IServer::finishedConnect,
                 this, &BaseShellWidget::finishConnect));
  VERIFY(connect(server_.get(), &IServer::startedDisconnect,
                 this, &BaseShellWidget::startDisconnect));
  VERIFY(connect(server_.get(), &IServer::finishedDisconnect,
                 this, &BaseShellWidget::finishDisconnect));
  VERIFY(connect(server_.get(), &IServer::progressChanged,
                 this, &BaseShellWidget::progressChange));

  VERIFY(connect(server_.get(), &IServer::startedSetDefaultDatabase,
                 this, &BaseShellWidget::startSetDefaultDatabase));
  VERIFY(connect(server_.get(), &IServer::finishedSetDefaultDatabase,
                 this, &BaseShellWidget::finishSetDefaultDatabase));

  VERIFY(connect(server_.get(), &IServer::enteredMode,
                 this, &BaseShellWidget::enterMode));
  VERIFY(connect(server_.get(), &IServer::leavedMode,
                 this, &BaseShellWidget::leaveMode));

  VERIFY(connect(server_.get(), &IServer::startedLoadDiscoveryInfo,
                 this, &BaseShellWidget::startLoadDiscoveryInfo));
  VERIFY(connect(server_.get(), &IServer::finishedLoadDiscoveryInfo,
                 this, &BaseShellWidget::finishLoadDiscoveryInfo));

  QVBoxLayout* mainlayout = new QVBoxLayout;
  QHBoxLayout* hlayout = new QHBoxLayout;

  QToolBar* savebar = new QToolBar;
  savebar->setStyleSheet("QToolBar { border: 0px; }");

  loadAction_ = new QAction(GuiFactory::instance().loadIcon(), translations::trLoad, savebar);
  typedef void (BaseShellWidget::*lf)();
  VERIFY(connect(loadAction_, &QAction::triggered,
                 this, static_cast<lf>(&BaseShellWidget::loadFromFile)));
  savebar->addAction(loadAction_);

  saveAction_ = new QAction(GuiFactory::instance().saveIcon(), translations::trSave, savebar);
  VERIFY(connect(saveAction_, &QAction::triggered,
                 this, &BaseShellWidget::saveToFile));
  savebar->addAction(saveAction_);

  saveAsAction_ = new QAction(GuiFactory::instance().saveAsIcon(), translations::trSaveAs, savebar);
  VERIFY(connect(saveAsAction_, &QAction::triggered, this, &BaseShellWidget::saveToFileAs));
  savebar->addAction(saveAsAction_);

  connectAction_ = new QAction(GuiFactory::instance().connectIcon(),
                               translations::trConnect, savebar);
  VERIFY(connect(connectAction_, &QAction::triggered, this, &BaseShellWidget::connectToServer));
  savebar->addAction(connectAction_);

  disConnectAction_ = new QAction(GuiFactory::instance().disConnectIcon(),
                                  translations::trDisconnect, savebar);
  VERIFY(connect(disConnectAction_, &QAction::triggered,
                 this, &BaseShellWidget::disconnectFromServer));
  savebar->addAction(disConnectAction_);

  executeAction_ = new QAction(GuiFactory::instance().executeIcon(),
                               translations::trExecute, savebar);
  executeAction_->setShortcut(executeKey);
  VERIFY(connect(executeAction_, &QAction::triggered, this, &BaseShellWidget::execute));
  savebar->addAction(executeAction_);

  QAction* stopAction = new QAction(GuiFactory::instance().stopIcon(),
                                    translations::trStop, savebar);
  VERIFY(connect(stopAction, &QAction::triggered, this, &BaseShellWidget::stop));
  savebar->addAction(stopAction);

  const ConnectionMode mode = InteractiveMode;
  connectionMode_ = new fasto::qt::gui::IconLabel(GuiFactory::instance().modeIcon(mode),
                                                  common::convertFromString<QString>(common::convertToString(mode)), iconSize);

  dbName_ = new fasto::qt::gui::IconLabel(GuiFactory::instance().databaseIcon(),
                                          "Calculate...", iconSize);

  hlayout->addWidget(savebar);

  QSplitter* splitter = new QSplitter;
  splitter->setOrientation(Qt::Horizontal);
  splitter->setHandleWidth(1);
  hlayout->addWidget(splitter);

  hlayout->addWidget(dbName_);
  hlayout->addWidget(connectionMode_);
  workProgressBar_ = new QProgressBar;
  workProgressBar_->setTextVisible(true);
  hlayout->addWidget(workProgressBar_);

  initShellByType(server->type());

  mainlayout->addLayout(hlayout);
  mainlayout->addWidget(input_);

  QHBoxLayout* apilayout = new QHBoxLayout;

  apilayout->addWidget(new QLabel(tr("Supported commands count: %1").arg(input_->commandsCount())));

  QSplitter* splitterButtom = new QSplitter;
  splitterButtom->setOrientation(Qt::Horizontal);
  splitterButtom->setHandleWidth(1);
  apilayout->addWidget(splitterButtom);

  commandsVersionApi_ = new QComboBox;
  typedef void (QComboBox::*curc)(int);
  VERIFY(connect(commandsVersionApi_, static_cast<curc>(&QComboBox::currentIndexChanged),
                 this, &BaseShellWidget::changeVersionApi));

  std::vector<uint32_t> versions = input_->supportedVersions();
  for (size_t i = 0; i < versions.size(); ++i) {
    uint32_t cur = versions[i];
    std::string curVers = convertVersionNumberToReadableString(cur);
    commandsVersionApi_->addItem(GuiFactory::instance().unknownIcon(),
                                 common::convertFromString<QString>(curVers), cur);
    commandsVersionApi_->setCurrentIndex(i);
  }
  apilayout->addWidget(new QLabel(tr("Command version:")));
  apilayout->addWidget(commandsVersionApi_);
  mainlayout->addLayout(apilayout);

  setLayout(mainlayout);

  syncConnectionActions();
  syncServerInfo(server_->serverInfo());
  updateDefaultDatabase(server_->currentDatabaseInfo());
}

void BaseShellWidget::syncServerInfo(IServerInfoSPtr inf) {
  if (!inf) {
    return;
  }

  uint32_t servVers = inf->version();
  if (servVers == UNDEFINED_SINCE) {
    return;
  }

  bool updatedComboIndex = false;
  for (int i = 0; i < commandsVersionApi_->count(); ++i) {
    QVariant var = commandsVersionApi_->itemData(i);
    uint32_t version = qvariant_cast<uint32_t>(var);
    if (version == UNDEFINED_SINCE) {
      commandsVersionApi_->setItemIcon(i, GuiFactory::instance().unknownIcon());
      continue;
    }

    if (version >= servVers) {
      if (!updatedComboIndex) {
        updatedComboIndex = true;
        commandsVersionApi_->setCurrentIndex(i);
        commandsVersionApi_->setItemIcon(i, GuiFactory::instance().successIcon());
      } else {
        commandsVersionApi_->setItemIcon(i, GuiFactory::instance().failIcon());
      }
    } else {
      commandsVersionApi_->setItemIcon(i, GuiFactory::instance().successIcon());
    }
  }
}

void BaseShellWidget::initShellByType(connectionTypes type) {
  input_ = BaseShell::createFromType(type, SettingsManager::instance().autoCompletion());
  setToolTip(tr("Based on %1 version: %2").arg(input_->basedOn()).arg(input_->version()));
  input_->setContextMenuPolicy(Qt::CustomContextMenu);
}

BaseShellWidget::~BaseShellWidget() {
}

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

void BaseShellWidget::execute() {
  QString selected = input_->selectedText();
  if (selected.isEmpty()) {
    selected = input_->text();
  }

  events_info::ExecuteInfoRequest req(this, common::convertToString(selected));
  server_->execute(req);
}

void BaseShellWidget::stop() {
  server_->stopCurrentEvent();
}

void BaseShellWidget::connectToServer() {
  events_info::ConnectInfoRequest req(this);
  server_->connect(req);
}

void BaseShellWidget::disconnectFromServer() {
  events_info::DisConnectInfoRequest req(this);
  server_->disconnect(req);
}

void BaseShellWidget::loadFromFile() {
  loadFromFile(filePath_);
}

bool BaseShellWidget::loadFromFile(const QString& path) {
  bool res = false;
  QString filepath = QFileDialog::getOpenFileName(this, path, QString(),
                                                  translations::trfilterForScripts);
  if (!filepath.isEmpty()) {
    QString out;
    if (common::utils_qt::loadFromFileText(filepath, out, this)) {
      setText(out);
      filePath_ = filepath;
      res = true;
    }
  }

  return res;
}

void BaseShellWidget::saveToFileAs() {
  QString filepath = QFileDialog::getSaveFileName(this, translations::trSaveAs, filePath_,
                                                  translations::trfilterForScripts);

  if (common::utils_qt::saveToFileText(filepath, text(), this)) {
    filePath_ = filepath;
  }
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
    common::utils_qt::saveToFileText(filePath_, text(), this);
  }
}

void BaseShellWidget::startConnect(const events_info::ConnectInfoRequest& req) {
  syncConnectionActions();
}

void BaseShellWidget::finishConnect(const events_info::ConnectInfoResponce& res) {
  syncConnectionActions();
}

void BaseShellWidget::startDisconnect(const events_info::DisConnectInfoRequest& req) {
  syncConnectionActions();
}

void BaseShellWidget::finishDisconnect(const events_info::DisConnectInfoResponce& res) {
  syncConnectionActions();
}

void BaseShellWidget::startSetDefaultDatabase(const events_info::SetDefaultDatabaseRequest& req) {
}

void BaseShellWidget::finishSetDefaultDatabase(const events_info::SetDefaultDatabaseResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  IServer* serv = qobject_cast<IServer*>(sender());
  DCHECK(serv);
  if (!serv) {
    return;
  }

  IDataBaseInfoSPtr db = res.inf;
  updateDefaultDatabase(db);
}

void BaseShellWidget::progressChange(const events_info::ProgressInfoResponce& res) {
  workProgressBar_->setValue(res.progress);
}

void BaseShellWidget::enterMode(const events_info::EnterModeInfo& res) {
  ConnectionMode mode = res.mode;
  connectionMode_->setIcon(GuiFactory::instance().modeIcon(mode), iconSize);
  std::string modeText = common::convertToString(mode);
  connectionMode_->setText(common::convertFromString<QString>(modeText));
}

void BaseShellWidget::leaveMode(const events_info::LeaveModeInfo& res) {
}

void BaseShellWidget::startLoadDiscoveryInfo(const events_info::DiscoveryInfoRequest& res) {
}

void BaseShellWidget::finishLoadDiscoveryInfo(const events_info::DiscoveryInfoResponce& res) {
  common::Error err = res.errorInfo();
  if (err && err->isError()) {
    return;
  }

  syncServerInfo(res.sinfo);
  updateDefaultDatabase(res.dbinfo);
}

void BaseShellWidget::updateDefaultDatabase(IDataBaseInfoSPtr dbs) {
  if (dbs) {
    std::string name = dbs->name();
    dbName_->setText(common::convertFromString<QString>(name));
  }
}

void BaseShellWidget::syncConnectionActions() {
  connectAction_->setVisible(!server_->isConnected());
  disConnectAction_->setVisible(server_->isConnected());
  executeAction_->setEnabled(server_->isConnected());
}

}  // namespace shell
}  // namespace fastonosql
