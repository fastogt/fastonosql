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

#include "gui/dialogs/pub_sub_dialog.h"

#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>
#include <common/qt/logger.h>

#include "proxy/server/iserver.h"

#include "gui/channels_table_model.h"  // for KeysTableModel, etc
#include "gui/fasto_table_view.h"      // for FastoTableView
#include "gui/gui_factory.h"           // for GuiFactory

#include "translations/global.h"  // for trKeyCountOnThePage, etc

namespace {
const QString trPublishToChannel_1S = QObject::tr("Publish to channel %1");
const QString trEnterWhatYoWantToSend = QObject::tr("Enter what you want to send:");
const QString trSubscribeInNewConsole = QObject::tr("Subscribe in new console");
}  // namespace

namespace fastonosql {
namespace gui {

PubSubDialog::PubSubDialog(const QString& title, proxy::IServerSPtr server, QWidget* parent)
    : QDialog(parent),
      searchBox_(nullptr),
      searchButton_(nullptr),
      channelsTable_(nullptr),
      channelsModel_(nullptr),
      proxy_model_(nullptr),
      server_(server) {
  CHECK(server_);
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  VERIFY(
      connect(server.get(), &proxy::IServer::LoadServerChannelsStarted, this, &PubSubDialog::startLoadServerChannels));
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerChannelsFinished, this,
                 &PubSubDialog::finishLoadServerChannels));

  // main layout
  QVBoxLayout* mainlayout = new QVBoxLayout;
  QHBoxLayout* searchLayout = new QHBoxLayout;
  searchBox_ = new QLineEdit;
  searchBox_->setText(ALL_PUBSUB_CHANNELS);
  VERIFY(connect(searchBox_, &QLineEdit::textChanged, this, &PubSubDialog::searchLineChanged));
  searchLayout->addWidget(searchBox_);

  searchButton_ = new QPushButton;
  VERIFY(connect(searchButton_, &QPushButton::clicked, this, &PubSubDialog::searchClicked));
  searchLayout->addWidget(searchButton_);
  mainlayout->addLayout(searchLayout);

  channelsModel_ = new ChannelsTableModel(this);
  proxy_model_ = new QSortFilterProxyModel(this);
  proxy_model_->setSourceModel(channelsModel_);
  proxy_model_->setDynamicSortFilter(true);

  VERIFY(
      connect(server_.get(), &proxy::IServer::ExecuteStarted, this, &PubSubDialog::startExecute, Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteFinished, this, &PubSubDialog::finishExecute,
                 Qt::DirectConnection));

  channelsTable_ = new FastoTableView;
  channelsTable_->setSortingEnabled(true);
  channelsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
  channelsTable_->setSelectionMode(QAbstractItemView::SingleSelection);
  channelsTable_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(channelsTable_, &FastoTableView::customContextMenuRequested, this, &PubSubDialog::showContextMenu));
  channelsTable_->sortByColumn(0, Qt::AscendingOrder);
  channelsTable_->setModel(proxy_model_);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &PubSubDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &PubSubDialog::reject));
  mainlayout->addWidget(channelsTable_);
  mainlayout->addWidget(buttonBox);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainlayout);
  retranslateUi();
}

void PubSubDialog::startExecute(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void PubSubDialog::finishExecute(const proxy::events_info::ExecuteInfoResponce& res) {
  UNUSED(res);
}

void PubSubDialog::startLoadServerChannels(const proxy::events_info::LoadServerChannelsRequest& req) {
  UNUSED(req);

  channelsModel_->clear();
}

void PubSubDialog::finishLoadServerChannels(const proxy::events_info::LoadServerChannelsResponce& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  proxy::events_info::LoadServerChannelsResponce::channels_container_t channels = res.channels;

  for (core::NDbPSChannel channel : channels) {
    channelsModel_->insertItem(new ChannelTableItem(channel));
  }
}

void PubSubDialog::searchClicked() {
  QString pattern = searchBox_->text();
  if (pattern.isEmpty()) {
    return;
  }

  proxy::events_info::LoadServerChannelsRequest req(this, common::ConvertToString(pattern));
  server_->LoadChannels(req);
}

void PubSubDialog::showContextMenu(const QPoint& point) {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  QPoint menuPoint = channelsTable_->calculateMenuPoint(point);
  QMenu* menu = new QMenu(channelsTable_);

  QAction* publishAction = new QAction(translations::trPublish, this);
  VERIFY(connect(publishAction, &QAction::triggered, this, &PubSubDialog::publish));

  QAction* subscribeAction = new QAction(trSubscribeInNewConsole, this);
  VERIFY(connect(subscribeAction, &QAction::triggered, this, &PubSubDialog::subscribeInNewConsole));

  menu->addAction(publishAction);
  menu->addAction(subscribeAction);
  menu->exec(menuPoint);
  delete menu;
}

void PubSubDialog::publish() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ChannelTableItem* node = common::qt::item<common::qt::gui::TableItem*, ChannelTableItem*>(sel);
  if (!node) {
    DNOTREACHED();
    return;
  }

  bool ok;
  QString publish_text = QInputDialog::getText(this, trPublishToChannel_1S.arg(node->name()), trEnterWhatYoWantToSend,
                                               QLineEdit::Normal, QString(), &ok);
  if (ok && !publish_text.isEmpty()) {
    core::translator_t trans = server_->GetTranslator();
    core::command_buffer_t cmd_str;
    common::Error err = trans->PublishCommand(node->channel(), common::ConvertToString(publish_text), &cmd_str);
    if (err) {
      LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
      return;
    }

    proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
    server_->Execute(req);
  }
}

void PubSubDialog::subscribeInNewConsole() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ChannelTableItem* node = common::qt::item<common::qt::gui::TableItem*, ChannelTableItem*>(sel);
  if (!node) {
    DNOTREACHED();
    return;
  }

  core::translator_t trans = server_->GetTranslator();
  core::command_buffer_t cmd_str;
  common::Error err = trans->SubscribeCommand(node->channel(), &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  QString text;
  if (common::ConvertFromString(cmd_str, &text)) {
    emit consoleOpenedAndExecute(server_, text);
  }
}

QModelIndex PubSubDialog::selectedIndex() const {
  QModelIndexList indexses = channelsTable_->selectionModel()->selectedRows();

  if (indexses.count() != 1) {
    return QModelIndex();
  }

  return proxy_model_->mapToSource(indexses[0]);
}

void PubSubDialog::searchLineChanged(const QString& text) {
  UNUSED(text);
}

void PubSubDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void PubSubDialog::retranslateUi() {
  searchButton_->setText(translations::trSearch);
}

}  // namespace gui
}  // namespace fastonosql
