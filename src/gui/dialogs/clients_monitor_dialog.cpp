/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/clients_monitor_dialog.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>

#include <common/convert2string.h>
#include <common/string_util.h>

#include "proxy/server/iserver.h"

#include "gui/models/clients_table_model.h"
#include "gui/models/items/client_table_item.h"
#include "gui/views/fasto_table_view.h"

#include "translations/global.h"

#define CLIENT_COMMAND "CLIENT"
#define KILL_ARG "KILL"
#define CLIENT_KILL_COMMAND CLIENT_COMMAND SPACE_STR KILL_ARG
#define ID_ARG "ID"

namespace fastonosql {
namespace gui {

ClientsMonitorDialog::ClientsMonitorDialog(const QString& title,
                                           const QIcon& icon,
                                           proxy::IServerSPtr server,
                                           QWidget* parent)
    : base_class(title, parent),
      update_button_(nullptr),
      clients_table_(nullptr),
      clients_model_(nullptr),
      proxy_model_(nullptr),
      server_(server) {
  CHECK(server_);
  setWindowIcon(icon);

  VERIFY(connect(server.get(), &proxy::IServer::LoadServerClientsStarted, this,
                 &ClientsMonitorDialog::startLoadServerClients));
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerClientsFinished, this,
                 &ClientsMonitorDialog::finishLoadServerClients));
  VERIFY(connect(server.get(), &proxy::IServer::ExecuteStarted, this, &ClientsMonitorDialog::startExecuteCommand));
  VERIFY(connect(server.get(), &proxy::IServer::ExecuteFinished, this, &ClientsMonitorDialog::finishExecuteCommand));

  clients_model_ = new ClientsTableModel(this);
  proxy_model_ = new QSortFilterProxyModel(this);
  proxy_model_->setSourceModel(clients_model_);
  proxy_model_->setDynamicSortFilter(true);

  QHBoxLayout* search_layout = new QHBoxLayout;
  update_button_ = new QPushButton;
  VERIFY(connect(update_button_, &QPushButton::clicked, this, &ClientsMonitorDialog::updateClicked));
  search_layout->addWidget(new QSplitter(Qt::Horizontal));
  search_layout->addWidget(update_button_);

  clients_table_ = new FastoTableView;
  clients_table_->setSortingEnabled(true);
  clients_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  clients_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  clients_table_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(clients_table_, &FastoTableView::customContextMenuRequested, this,
                 &ClientsMonitorDialog::showContextMenu));
  clients_table_->sortByColumn(0, Qt::AscendingOrder);
  clients_table_->setModel(proxy_model_);

  clients_table_->setColumnHidden(ClientsTableModel::kSub, true);
  clients_table_->setColumnHidden(ClientsTableModel::kPsub, true);
  clients_table_->setColumnHidden(ClientsTableModel::kMulti, true);
  clients_table_->setColumnHidden(ClientsTableModel::kQbuf, true);
  clients_table_->setColumnHidden(ClientsTableModel::kQbufFree, true);
  clients_table_->setColumnHidden(ClientsTableModel::kOdl, true);
  clients_table_->setColumnHidden(ClientsTableModel::kOll, true);
  clients_table_->setColumnHidden(ClientsTableModel::kOmem, true);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &ClientsMonitorDialog::accept));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &ClientsMonitorDialog::reject));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addLayout(search_layout);
  main_layout->addWidget(clients_table_);
  main_layout->addWidget(button_box);
  setLayout(main_layout);
  setMinimumSize(QSize(min_width, min_height));
}

void ClientsMonitorDialog::startLoadServerClients(const proxy::events_info::LoadServerClientsRequest& req) {
  UNUSED(req);

  clients_model_->clear();
}

void ClientsMonitorDialog::finishLoadServerClients(const proxy::events_info::LoadServerClientsResponse& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  for (auto client : res.clients) {
    clients_model_->insertItem(new ClientTableItem(client));
  }
}

void ClientsMonitorDialog::startExecuteCommand(const proxy::events_info::ExecuteInfoRequest& req) {
  UNUSED(req);
}

void ClientsMonitorDialog::finishExecuteCommand(const proxy::events_info::ExecuteInfoResponse& res) {
  // some commands can be executed
  /*common::Error err = res.errorInfo();
  if (err) {
    return;
  }*/

  for (core::FastoObjectCommandIPtr command : res.executed_commands) {
    size_t off;
    core::commands_args_t argv;
    const core::CommandHolder* cmd = nullptr;
    const core::translator_t trans = server_->GetTranslator();
    const core::command_buffer_t command_str = command->GetInputCommand();
    common::Error err = trans->FindCommand(command_str, &cmd, &argv, &off);
    if (err) {
      continue;
    }

    if (argv.size() < 4) {
      continue;
    }

    if (!(common::FullEqualsASCII(argv[0], GEN_CMD_STRING(CLIENT_COMMAND), false) &&
          common::FullEqualsASCII(argv[1], GEN_CMD_STRING(KILL_ARG), false))) {
      continue;
    }

    const auto childs = command->GetChildrens();
    CHECK_EQ(childs.size(), 1);
    const auto child = childs[0];
    const auto value = child->GetValue();
    int64_t res;
    if (value->GetAsInteger64(&res) && res != 0) {
      if (res == 1) {
        const auto field = argv[2];
        if (field == GEN_CMD_STRING(ID_ARG)) {
          int iden;
          if (common::ConvertFromBytes(argv[3], &iden)) {
            ClientTableItem* ch = static_cast<ClientTableItem*>(clients_model_->findChildById(iden));
            if (ch) {
              clients_model_->removeItem(ch);
            }
          }
        }
      } else {
        updateClicked();
      }
    }
  }
}

void ClientsMonitorDialog::showEvent(QShowEvent* e) {
  base_class::showEvent(e);
  updateClicked();
}

void ClientsMonitorDialog::showContextMenu(const QPoint& point) {
  const QModelIndex selected = selectedIndex();
  if (!selected.isValid()) {
    return;
  }

  QPoint menu_point = clients_table_->calculateMenuPoint(point);
  QMenu* menu = new QMenu(clients_table_);

  QAction* kill_action = new QAction(translations::trKill, this);
  VERIFY(connect(kill_action, &QAction::triggered, this, &ClientsMonitorDialog::killClient));

  menu->addAction(kill_action);
  menu->exec(menu_point);
  delete menu;
}

void ClientsMonitorDialog::updateClicked() {
  proxy::events_info::LoadServerClientsRequest req(this);
  server_->LoadClients(req);
}

void ClientsMonitorDialog::killClient() {
  QModelIndex sel = selectedIndex();
  if (!sel.isValid()) {
    return;
  }

  ClientTableItem* node = common::qt::item<common::qt::gui::TableItem*, ClientTableItem*>(sel);
  if (!node) {
    DNOTREACHED();
    return;
  }

  const auto client = node->client();

  core::command_buffer_writer_t wr;
  wr << CLIENT_KILL_COMMAND SPACE_STR ID_ARG SPACE_STR << common::ConvertToCharBytes(client.GetId());
  proxy::events_info::ExecuteInfoRequest req(this, wr.str());
  server_->Execute(req);
}

void ClientsMonitorDialog::retranslateUi() {
  update_button_->setText(translations::trRefresh);
}

QModelIndex ClientsMonitorDialog::selectedIndex() const {
  const QModelIndexList indexses = clients_table_->selectionModel()->selectedRows();

  if (indexses.count() != 1) {
    return QModelIndex();
  }

  return proxy_model_->mapToSource(indexses[0]);
}

}  // namespace gui
}  // namespace fastonosql
