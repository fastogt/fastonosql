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

#include "gui/shell/base_shell_widget.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QSpinBox>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/qt/gui/icon_label.h>  // for IconLabel
#include <common/qt/logger.h>          // for LOG_ERROR

#include "proxy/server/iserver_local.h"
#include "proxy/server/iserver_remote.h"
#include "proxy/settings_manager.h"  // for SettingsManager

#include "gui/gui_factory.h"  // for GuiFactory
#include "gui/shortcuts.h"    // for g_execute_key
#include "gui/utils.h"

#include "translations/global.h"  // for trError, trSaveAs, etc

#ifdef BUILD_WITH_REDIS
#include "gui/db/redis/shell_widget.h"
#endif
#include "gui/shell/base_shell.h"

namespace {

const QString trSupportedCommandsCountTemplate_1S = QObject::tr("Supported commands count: %1");
const QString trValidatedCommandsCountTemplate_1S = QObject::tr("Validated commands count: %1");
const QString trCommandsVersion = QObject::tr("Commands version:");
const QString trCantReadTemplate_2S = QObject::tr(PROJECT_NAME_TITLE " can't read from %1:\n%2.");
const QString trCantSaveTemplate_2S = QObject::tr(PROJECT_NAME_TITLE " can't save to %1:\n%2.");
const QString trAdvancedOptions = QObject::tr("Advanced options");
const QString trIntervalMsec = QObject::tr("Interval msec:");
const QString trRepeat = QObject::tr("Repeat:");
const QString trBasedOn_2S = QObject::tr("Based on <b>%1</b> version: <b>%2</b>");

}  // namespace

namespace fastonosql {
namespace gui {

const QSize BaseShellWidget::top_bar_icon_size = QSize(24, 24);

BaseShellWidget* BaseShellWidget::createWidget(proxy::IServerSPtr server, const QString& filePath, QWidget* parent) {
#if defined(BUILD_WITH_REDIS) && defined(PRO_VERSION)
  core::connectionTypes ct = server->GetType();
  if (ct == core::REDIS) {
    BaseShellWidget* widget = new redis::ShellWidget(server, filePath, parent);
    widget->init();
    return widget;
  }
#endif

  BaseShellWidget* widget = new BaseShellWidget(server, filePath, parent);
  widget->init();
  return widget;
}

BaseShellWidget::BaseShellWidget(proxy::IServerSPtr server, const QString& filePath, QWidget* parent)
    : QWidget(parent),
      server_(server),
      execute_action_(nullptr),
      stop_action_(nullptr),
      connect_action_(nullptr),
      disconnect_action_(nullptr),
      load_action_(nullptr),
      save_action_(nullptr),
      save_as_action_(nullptr),
      validate_action_(nullptr),
      supported_commands_count_(nullptr),
      validated_commands_count_(nullptr),
      commands_version_api_(nullptr),
      input_(nullptr),
      work_progressbar_(nullptr),
      connection_mode_(nullptr),
      server_name_(nullptr),
      db_name_(nullptr),
      advanced_options_(nullptr),
      advanced_options_widget_(nullptr),
      repeat_count_(nullptr),
      interval_msec_(nullptr),
      history_call_(nullptr),
      file_path_(filePath) {}

QToolBar* BaseShellWidget::createToolBar() {
  QToolBar* savebar = new QToolBar;
  load_action_ = new QAction;
  load_action_->setIcon(gui::GuiFactory::GetInstance().loadIcon());
  typedef void (BaseShellWidget::*lf)();
  VERIFY(connect(load_action_, &QAction::triggered, this, static_cast<lf>(&BaseShellWidget::loadFromFile)));
  savebar->addAction(load_action_);

  save_action_ = new QAction;
  save_action_->setIcon(gui::GuiFactory::GetInstance().saveIcon());
  VERIFY(connect(save_action_, &QAction::triggered, this, &BaseShellWidget::saveToFile));
  savebar->addAction(save_action_);

  save_as_action_ = new QAction;
  save_as_action_->setIcon(gui::GuiFactory::GetInstance().saveAsIcon());
  VERIFY(connect(save_as_action_, &QAction::triggered, this, &BaseShellWidget::saveToFileAs));
  savebar->addAction(save_as_action_);

  connect_action_ = new QAction;
  connect_action_->setIcon(gui::GuiFactory::GetInstance().connectIcon());
  VERIFY(connect(connect_action_, &QAction::triggered, this, &BaseShellWidget::connectToServer));
  savebar->addAction(connect_action_);

  disconnect_action_ = new QAction;
  disconnect_action_->setIcon(gui::GuiFactory::GetInstance().disConnectIcon());
  VERIFY(connect(disconnect_action_, &QAction::triggered, this, &BaseShellWidget::disconnectFromServer));
  savebar->addAction(disconnect_action_);

  execute_action_ = new QAction;
  execute_action_->setIcon(gui::GuiFactory::GetInstance().executeIcon());
  execute_action_->setShortcut(gui::g_execute_key);
  VERIFY(connect(execute_action_, &QAction::triggered, this, &BaseShellWidget::execute));
  savebar->addAction(execute_action_);

  stop_action_ = new QAction;
  stop_action_->setIcon(gui::GuiFactory::GetInstance().stopIcon());
  VERIFY(connect(stop_action_, &QAction::triggered, this, &BaseShellWidget::stop));
  savebar->addAction(stop_action_);
  return savebar;
}

void BaseShellWidget::init() {
  CHECK(server_);

  VERIFY(connect(server_.get(), &proxy::IServer::ConnectStarted, this, &BaseShellWidget::startConnect));
  VERIFY(connect(server_.get(), &proxy::IServer::ConnectFinished, this, &BaseShellWidget::finishConnect));
  VERIFY(connect(server_.get(), &proxy::IServer::DisconnectStarted, this, &BaseShellWidget::startDisconnect));
  VERIFY(connect(server_.get(), &proxy::IServer::DisconnectFinished, this, &BaseShellWidget::finishDisconnect));
  VERIFY(connect(server_.get(), &proxy::IServer::ProgressChanged, this, &BaseShellWidget::progressChange));

  VERIFY(connect(server_.get(), &proxy::IServer::ModeEntered, this, &BaseShellWidget::enterMode));
  VERIFY(connect(server_.get(), &proxy::IServer::ModeLeaved, this, &BaseShellWidget::leaveMode));

  // infos
  VERIFY(connect(server_.get(), &proxy::IServer::LoadServerInfoStarted, this, &BaseShellWidget::startLoadServerInfo));
  VERIFY(connect(server_.get(), &proxy::IServer::LoadServerInfoFinished, this, &BaseShellWidget::finishLoadServerInfo));
  VERIFY(connect(server_.get(), &proxy::IServer::LoadDiscoveryInfoStarted, this,
                 &BaseShellWidget::startLoadDiscoveryInfo));
  VERIFY(connect(server_.get(), &proxy::IServer::LoadDiscoveryInfoFinished, this,
                 &BaseShellWidget::finishLoadDiscoveryInfo));

  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteStarted, this, &BaseShellWidget::startExecute,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteFinished, this, &BaseShellWidget::finishExecute,
                 Qt::DirectConnection));

  VERIFY(connect(server_.get(), &proxy::IServer::DatabaseChanged, this, &BaseShellWidget::updateDefaultDatabase));
  VERIFY(connect(server_.get(), &proxy::IServer::Disconnected, this, &BaseShellWidget::serverDisconnect));

  QVBoxLayout* mainlayout = new QVBoxLayout;
  QHBoxLayout* hlayout = new QHBoxLayout;

  QToolBar* savebar = createToolBar();

  core::ConnectionMode mode = core::InteractiveMode;
  std::string mode_str = common::ConvertToString(mode);
  QString qmode_str;
  common::ConvertFromString(mode_str, &qmode_str);
  connection_mode_ =
      new common::qt::gui::IconLabel(gui::GuiFactory::GetInstance().modeIcon(mode), top_bar_icon_size, qmode_str);

  hlayout->addWidget(savebar);
  hlayout->addWidget(new QSplitter(Qt::Horizontal));

  hlayout->addWidget(connection_mode_);
  work_progressbar_ = new QProgressBar;
  work_progressbar_->setTextVisible(true);
  hlayout->addWidget(work_progressbar_);
  QToolBar* helpbar = new QToolBar;
  validate_action_ = new QAction(gui::GuiFactory::GetInstance().failIcon(), translations::trValidate, helpbar);
  VERIFY(connect(validate_action_, &QAction::triggered, this, &BaseShellWidget::validateClick));
  helpbar->addAction(validate_action_);

  QAction* helpAction = new QAction(gui::GuiFactory::GetInstance().helpIcon(), translations::trHelp, helpbar);
  VERIFY(connect(helpAction, &QAction::triggered, this, &BaseShellWidget::helpClick));
  helpbar->addAction(helpAction);
  hlayout->addWidget(helpbar);
  mainlayout->addLayout(hlayout);

  advanced_options_ = new QCheckBox;
  VERIFY(connect(advanced_options_, &QCheckBox::stateChanged, this, &BaseShellWidget::advancedOptionsChange));

  core::connectionTypes ct = server_->GetType();
  input_ = BaseShell::createFromType(ct, proxy::SettingsManager::GetInstance()->GetAutoCompletion());
  input_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(input_, &BaseShell::textChanged, this, &BaseShellWidget::inputTextChanged));

  advanced_options_widget_ = new QWidget;
  advanced_options_widget_->setVisible(false);
  QVBoxLayout* advOptLayout = new QVBoxLayout;

  QHBoxLayout* repeatLayout = new QHBoxLayout;
  QLabel* repeatLabel = new QLabel(trRepeat);
  repeat_count_ = new QSpinBox;
  repeat_count_->setRange(0, INT32_MAX);
  repeat_count_->setSingleStep(1);
  repeatLayout->addWidget(repeatLabel);
  repeatLayout->addWidget(repeat_count_);

  QHBoxLayout* intervalLayout = new QHBoxLayout;
  QLabel* intervalLabel = new QLabel(trIntervalMsec);
  interval_msec_ = new QSpinBox;
  interval_msec_->setRange(0, INT32_MAX);
  interval_msec_->setSingleStep(1000);
  intervalLayout->addWidget(intervalLabel);
  intervalLayout->addWidget(interval_msec_);

  history_call_ = new QCheckBox;
  history_call_->setChecked(true);
  advOptLayout->addLayout(repeatLayout);
  advOptLayout->addLayout(intervalLayout);
  advOptLayout->addWidget(history_call_);
  advanced_options_widget_->setLayout(advOptLayout);

  QHBoxLayout* top_layout = createTopLayout(ct);
  QSplitter* spliter_info_and_options = new QSplitter(Qt::Horizontal);
  spliter_info_and_options->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  top_layout->addWidget(spliter_info_and_options);
  top_layout->addWidget(advanced_options_);
  mainlayout->addLayout(top_layout);

  QHBoxLayout* inputLayout = new QHBoxLayout;
  inputLayout->addWidget(input_);
  inputLayout->addWidget(advanced_options_widget_);
  mainlayout->addLayout(inputLayout);

  QHBoxLayout* apilayout = new QHBoxLayout;
  supported_commands_count_ = new QLabel;
  apilayout->addWidget(supported_commands_count_);
  validated_commands_count_ = new QLabel;
  apilayout->addWidget(validated_commands_count_);
  apilayout->addWidget(new QSplitter(Qt::Horizontal));

  commands_version_api_ = new QComboBox;
  typedef void (QComboBox::*curc)(int);
  VERIFY(connect(commands_version_api_, static_cast<curc>(&QComboBox::currentIndexChanged), this,
                 &BaseShellWidget::changeVersionApi));

  std::vector<uint32_t> versions = input_->supportedVersions();
  for (size_t i = 0; i < versions.size(); ++i) {
    uint32_t cur = versions[i];
    std::string curVers = core::ConvertVersionNumberToReadableString(cur);
    QString qcurVers;
    common::ConvertFromString(curVers, &qcurVers);
    commands_version_api_->addItem(gui::GuiFactory::GetInstance().unknownIcon(), qcurVers, cur);
    commands_version_api_->setCurrentIndex(i);
  }
  QLabel* version = new QLabel(trCommandsVersion);
  apilayout->addWidget(version);
  apilayout->addWidget(commands_version_api_);
  mainlayout->addLayout(apilayout);

  setLayout(mainlayout);

  // sync controls
  syncConnectionActions();
  updateServerInfo(server_->GetCurrentServerInfo());
  updateDefaultDatabase(server_->GetCurrentDatabaseInfo());
  updateCommands(std::vector<const core::CommandInfo*>());

  retranslateUi();
}

QHBoxLayout* BaseShellWidget::createTopLayout(core::connectionTypes ct) {
  QHBoxLayout* top_layout = new QHBoxLayout;
  server_name_ = new common::qt::gui::IconLabel(gui::GuiFactory::GetInstance().icon(ct), top_bar_icon_size,
                                                translations::trCalculate + "...");
  server_name_->setElideMode(Qt::ElideRight);
  top_layout->addWidget(server_name_);
  db_name_ = new common::qt::gui::IconLabel(gui::GuiFactory::GetInstance().databaseIcon(), top_bar_icon_size,
                                            translations::trCalculate + "...");
  top_layout->addWidget(db_name_);
  QSplitter* padding = new QSplitter(Qt::Horizontal);
  padding->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  top_layout->addWidget(padding);
  return top_layout;
}

void BaseShellWidget::advancedOptionsChange(int state) {
  advanced_options_widget_->setVisible(state);
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

void BaseShellWidget::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void BaseShellWidget::retranslateUi() {
  load_action_->setText(translations::trLoad);
  save_action_->setText(translations::trSave);
  save_as_action_->setText(translations::trSaveAs);
  connect_action_->setText(translations::trConnect);
  disconnect_action_->setText(translations::trDisconnect);
  execute_action_->setText(translations::trExecute);
  stop_action_->setText(translations::trStop);

  history_call_->setText(translations::trHistory);
  setToolTip(trBasedOn_2S.arg(input_->basedOn(), input_->version()));
  advanced_options_->setText(trAdvancedOptions);
  supported_commands_count_->setText(trSupportedCommandsCountTemplate_1S.arg(input_->commandsCount()));
  validated_commands_count_->setText(trValidatedCommandsCountTemplate_1S.arg(input_->validateCommandsCount()));
}

common::Error BaseShellWidget::validate(const QString& text) {
  core::translator_t tran = server_->GetTranslator();
  std::vector<core::command_buffer_t> cmds;
  core::command_buffer_t text_cmd = common::ConvertToString(text);
  common::Error err = core::ParseCommands(text_cmd, &cmds);
  if (err) {
    return err;
  }

  for (auto cmd : cmds) {
    err = tran->TestCommandLine(cmd);
    if (err) {
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

  int repeat = repeat_count_->value();
  int interval = interval_msec_->value();
  bool history = history_call_->isChecked();
  executeArgs(selected, repeat, interval, history);
}

void BaseShellWidget::executeArgs(const QString& text, int repeat, int interval, bool history) {
  core::command_buffer_t text_cmd = common::ConvertToString(text);
  proxy::events_info::ExecuteInfoRequest req(this, text_cmd, repeat, interval, history);
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
  loadFromFile(file_path_);
}

bool BaseShellWidget::loadFromFile(const QString& path) {
  QString filepath = QFileDialog::getOpenFileName(this, path, QString(), translations::trfilterForScripts);
  if (!filepath.isEmpty()) {
    QString out;
    common::qt::QtFileError err = common::qt::LoadFromFileText(filepath, &out);
    if (err) {
      QString qdesc;
      common::ConvertFromString(err->GetDescription(), &qdesc);
      QMessageBox::critical(this, translations::trError, trCantReadTemplate_2S.arg(filepath, qdesc));
      return false;
    }

    setText(out);
    file_path_ = filepath;
    return true;
  }

  return false;
}

void BaseShellWidget::saveToFileAs() {
  QString filepath = ShowSaveFileDialog(this, translations::trSaveAs, file_path_, translations::trfilterForScripts);
  if (filepath.isEmpty()) {
    return;
  }

  common::qt::QtFileError err = common::qt::SaveToFileText(filepath, text());
  if (err) {
    QString qdesc;
    common::ConvertFromString(err->GetDescription(), &qdesc);
    QMessageBox::critical(this, translations::trError, trCantSaveTemplate_2S.arg(filepath, qdesc));
    return;
  }

  file_path_ = filepath;
}

void BaseShellWidget::changeVersionApi(int index) {
  if (index == -1) {
    return;
  }

  QVariant var = commands_version_api_->itemData(index);
  uint32_t version = qvariant_cast<uint32_t>(var);
  input_->setFilteredVersion(version);
}

void BaseShellWidget::saveToFile() {
  if (file_path_.isEmpty()) {
    saveToFileAs();
  } else {
    common::qt::QtFileError err = common::qt::SaveToFileText(file_path_, text());
    if (err) {
      QString qdesc;
      common::ConvertFromString(err->GetDescription(), &qdesc);
      QMessageBox::critical(this, translations::trError, trCantSaveTemplate_2S.arg(file_path_, qdesc));
    }
  }
}

void BaseShellWidget::validateClick() {
  QString text = input_->text();
  common::Error err = validate(text);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
  }
}

void BaseShellWidget::helpClick() {
  executeArgs(DB_HELP_COMMAND, 0, 0, false);
}

void BaseShellWidget::inputTextChanged() {
  QString text = input_->text();
  common::Error err = validate(text);
  if (err) {
    validate_action_->setIcon(gui::GuiFactory::GetInstance().failIcon());
  } else {
    validate_action_->setIcon(gui::GuiFactory::GetInstance().successIcon());
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
  work_progressbar_->setValue(res.progress);
}

void BaseShellWidget::enterMode(const proxy::events_info::EnterModeInfo& res) {
  core::ConnectionMode mode = res.mode;
  connection_mode_->setIcon(gui::GuiFactory::GetInstance().modeIcon(mode), top_bar_icon_size);
  std::string modeText = common::ConvertToString(mode);
  QString qmodeText;
  common::ConvertFromString(modeText, &qmodeText);
  connection_mode_->setText(qmodeText);
}

void BaseShellWidget::leaveMode(const proxy::events_info::LeaveModeInfo& res) {
  UNUSED(res);
}

void BaseShellWidget::startLoadServerInfo(const proxy::events_info::ServerInfoRequest& res) {
  OnStartedLoadServerInfo(res);
}

void BaseShellWidget::finishLoadServerInfo(const proxy::events_info::ServerInfoResponce& res) {
  OnFinishedLoadServerInfo(res);
}

void BaseShellWidget::startLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoRequest& res) {
  OnStartedLoadDiscoveryInfo(res);
}

void BaseShellWidget::finishLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoResponce& res) {
  OnFinishedLoadDiscoveryInfo(res);
}

void BaseShellWidget::OnStartedLoadServerInfo(const proxy::events_info::ServerInfoRequest& res) {
  UNUSED(res);
}

void BaseShellWidget::OnFinishedLoadServerInfo(const proxy::events_info::ServerInfoResponce& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  updateServerInfo(res.info());
}

void BaseShellWidget::OnStartedLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoRequest& res) {
  UNUSED(res);
}

void BaseShellWidget::OnFinishedLoadDiscoveryInfo(const proxy::events_info::DiscoveryInfoResponce& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  updateDefaultDatabase(res.dbinfo);
  updateCommands(res.commands);
}

void BaseShellWidget::startExecute(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);

  repeat_count_->setEnabled(false);
  interval_msec_->setEnabled(false);
  history_call_->setEnabled(false);
  execute_action_->setEnabled(false);
  stop_action_->setEnabled(true);
}
void BaseShellWidget::finishExecute(const proxy::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);

  repeat_count_->setEnabled(true);
  interval_msec_->setEnabled(true);
  history_call_->setEnabled(true);
  execute_action_->setEnabled(true);
  stop_action_->setEnabled(false);
}

void BaseShellWidget::serverConnect() {
  OnServerConnected();
}

void BaseShellWidget::serverDisconnect() {
  OnServerDisconnected();
}

void BaseShellWidget::OnServerConnected() {
  syncConnectionActions();
}

void BaseShellWidget::OnServerDisconnected() {
  syncConnectionActions();
  updateServerInfo(core::IServerInfoSPtr());
  updateDefaultDatabase(core::IDataBaseInfoSPtr());
  updateCommands(std::vector<const core::CommandInfo*>());
}

void BaseShellWidget::updateServerInfo(core::IServerInfoSPtr inf) {
  if (!inf) {
    updateServerLabel(translations::trCalculate + "...");
    for (int i = 0; i < commands_version_api_->count(); ++i) {
      commands_version_api_->setItemIcon(i, gui::GuiFactory::GetInstance().unknownIcon());
    }
    return;
  }

  std::string server_label;
  if (server_->IsCanRemote()) {
    proxy::IServerRemote* rserver = dynamic_cast<proxy::IServerRemote*>(server_.get());  // +
    server_label = common::ConvertToString(rserver->GetHost());
  } else {
    proxy::IServerLocal* lserver = dynamic_cast<proxy::IServerLocal*>(server_.get());  // +
    server_label = lserver->GetPath();
  }
  QString qserver_label;
  common::ConvertFromString(server_label, &qserver_label);
  updateServerLabel(qserver_label);

  uint32_t serv_vers = inf->GetVersion();
  if (serv_vers == UNDEFINED_SINCE) {
    return;
  }

  bool updatedComboIndex = false;
  for (int i = 0; i < commands_version_api_->count(); ++i) {
    QVariant var = commands_version_api_->itemData(i);
    uint32_t version = qvariant_cast<uint32_t>(var);
    if (version == UNDEFINED_SINCE) {
      commands_version_api_->setItemIcon(i, gui::GuiFactory::GetInstance().unknownIcon());
      continue;
    }

    if (version >= serv_vers) {
      if (!updatedComboIndex) {
        updatedComboIndex = true;
        commands_version_api_->setCurrentIndex(i);
        commands_version_api_->setItemIcon(i, gui::GuiFactory::GetInstance().successIcon());
      } else {
        commands_version_api_->setItemIcon(i, gui::GuiFactory::GetInstance().failIcon());
      }
    } else {
      commands_version_api_->setItemIcon(i, gui::GuiFactory::GetInstance().successIcon());
    }
  }
}

void BaseShellWidget::updateDefaultDatabase(core::IDataBaseInfoSPtr dbs) {
  if (!dbs) {
    updateDBLabel(translations::trCalculate + "...");
    return;
  }

  std::string name = dbs->GetName();
  QString qname;
  common::ConvertFromString(name, &qname);
  updateDBLabel(qname);
}

void BaseShellWidget::updateCommands(const std::vector<const core::CommandInfo*>& commands) {
  validated_commands_count_->setText(trValidatedCommandsCountTemplate_1S.arg(commands.size()));
}

void BaseShellWidget::updateServerLabel(const QString& text) {
  server_name_->setText(text);
  server_name_->setToolTip(text);
}

void BaseShellWidget::updateDBLabel(const QString& text) {
  db_name_->setText(text);
  db_name_->setToolTip(text);
}

void BaseShellWidget::syncConnectionActions() {
  bool is_connected = server_->IsConnected();

  connect_action_->setVisible(!is_connected);
  disconnect_action_->setVisible(is_connected);
  execute_action_->setEnabled(true);
  stop_action_->setEnabled(false);
}

}  // namespace gui
}  // namespace fastonosql
