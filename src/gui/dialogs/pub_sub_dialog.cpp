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

#include "gui/gui_factory.h"                  // for GuiFactory
#include "gui/models/channels_table_model.h"  // for KeysTableModel, etc
#include "gui/models/items/channel_table_item.h"
#include "gui/views/fasto_table_view.h"  // for FastoTableView

#include "translations/global.h"  // for trKeyCountOnThePage, etc

namespace {
const QString trPublishToChannel_1S = QObject::tr("Publish to channel %1");
const QString trEnterWhatYoWantToSend = QObject::tr("Enter what you want to send:");
const QString trSubscribeInNewConsole = QObject::tr("Subscribe in new console");
}  // namespace

namespace fastonosql {
namespace gui {

PubSubDialog::PubSubDialog(const QString& title, const QIcon& icon, proxy::IServerSPtr server, QWidget* parent)
    : base_class(title, parent),
      search_box_(nullptr),
      search_button_(nullptr),
      channels_table_(nullptr),
      channels_model_(nullptr),
      proxy_model_(nullptr),
      server_(server) {
  CHECK(server_);
  setWindowIcon(icon);

  VERIFY(
      connect(server.get(), &proxy::IServer::LoadServerChannelsStarted, this, &PubSubDialog::startLoadServerChannels));
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerChannelsFinished, this,
                 &PubSubDialog::finishLoadServerChannels));

  QHBoxLayout* search_layout = new QHBoxLayout;
  search_box_ = new QLineEdit;
  search_box_->setText(ALL_PUBSUB_CHANNELS);
  VERIFY(connect(search_box_, &QLineEdit::textChanged, this, &PubSubDialog::searchLineChanged));
  search_layout->addWidget(search_box_);

  search_button_ = new QPushButton;
  VERIFY(connect(search_button_, &QPushButton::clicked, this, &PubSubDialog::searchClicked));
  search_layout->addWidget(search_button_);

  channels_model_ = new ChannelsTableModel(this);
  proxy_model_ = new QSortFilterProxyModel(this);
  proxy_model_->setSourceModel(channels_model_);
  proxy_model_->setDynamicSortFilter(true);

  channels_table_ = new FastoTableView;
  channels_table_->setSortingEnabled(true);
  channels_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  channels_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  channels_table_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(channels_table_, &FastoTableView::customContextMenuRequested, this, &PubSubDialog::showContextMenu));
  channels_table_->sortByColumn(0, Qt::AscendingOrder);
  channels_table_->setModel(proxy_model_);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &PubSubDialog::accept));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &PubSubDialog::reject));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addLayout(search_layout);
  main_layout->addWidget(channels_table_);
  main_layout->addWidget(button_box);
  setLayout(main_layout);
  setMinimumSize(QSize(min_width, min_height));
}

void PubSubDialog::startLoadServerChannels(const proxy::events_info::LoadServerChannelsRequest& req) {
  UNUSED(req);

  channels_model_->clear();
}

void PubSubDialog::finishLoadServerChannels(const proxy::events_info::LoadServerChannelsResponce& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  proxy::events_info::LoadServerChannelsResponce::channels_container_t channels = res.channels;
  for (auto channel : channels) {
    channels_model_->insertItem(new ChannelTableItem(channel));
  }
}

void PubSubDialog::searchClicked() {
  const QString pattern = search_box_->text();
  if (pattern.isEmpty()) {
    return;
  }

  proxy::events_info::LoadServerChannelsRequest req(this, common::ConvertToString(pattern));
  server_->LoadChannels(req);
}

void PubSubDialog::showContextMenu(const QPoint& point) {
  const QModelIndex selected = selectedIndex();
  if (!selected.isValid()) {
    return;
  }

  QPoint menu_point = channels_table_->calculateMenuPoint(point);
  QMenu* menu = new QMenu(channels_table_);

  QAction* publish_action = new QAction(translations::trPublish, this);
  VERIFY(connect(publish_action, &QAction::triggered, this, &PubSubDialog::publish));

  QAction* subscribe_action = new QAction(trSubscribeInNewConsole, this);
  VERIFY(connect(subscribe_action, &QAction::triggered, this, &PubSubDialog::subscribeInNewConsole));

  menu->addAction(publish_action);
  menu->addAction(subscribe_action);
  menu->exec(menu_point);
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
  const auto channel = node->channel();
  const auto name = channel.GetName();
  QString qname;
  common::ConvertFromBytes(name.GetHumanReadable(), &qname);
  QString publish_text = QInputDialog::getText(this, trPublishToChannel_1S.arg(qname), trEnterWhatYoWantToSend,
                                               QLineEdit::Normal, QString(), &ok, Qt::WindowCloseButtonHint);
  if (ok && !publish_text.isEmpty()) {
    const core::translator_t trans = server_->GetTranslator();
    core::command_buffer_t cmd_str;
    common::Error err = trans->PublishCommand(name, common::ConvertToString(publish_text), &cmd_str);
    if (err) {
      LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
      return;
    }

    proxy::events_info::ExecuteInfoRequest req(this, cmd_str);
    server_->Execute(req);
  }
}

void PubSubDialog::subscribeInNewConsole() {
  const QModelIndex selected = selectedIndex();
  if (!selected.isValid()) {
    return;
  }

  ChannelTableItem* node = common::qt::item<common::qt::gui::TableItem*, ChannelTableItem*>(selected);
  if (!node) {
    DNOTREACHED();
    return;
  }

  const core::translator_t trans = server_->GetTranslator();
  const auto channel = node->channel();
  core::command_buffer_t cmd_str;
  common::Error err = trans->SubscribeCommand(channel.GetName(), &cmd_str);
  if (err) {
    LOG_ERROR(err, common::logging::LOG_LEVEL_ERR, true);
    return;
  }

  QString text;
  if (common::ConvertFromBytes(cmd_str, &text)) {
    emit consoleOpenedAndExecute(server_, text);
  }
}

QModelIndex PubSubDialog::selectedIndex() const {
  const QModelIndexList indexses = channels_table_->selectionModel()->selectedRows();

  if (indexses.count() != 1) {
    return QModelIndex();
  }

  return proxy_model_->mapToSource(indexses[0]);
}

void PubSubDialog::searchLineChanged(const QString& text) {
  UNUSED(text);
}

void PubSubDialog::retranslateUi() {
  search_button_->setText(translations::trSearch);
  base_class::retranslateUi();
}

}  // namespace gui
}  // namespace fastonosql
