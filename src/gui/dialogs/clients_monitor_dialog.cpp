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

#include "gui/dialogs/clients_monitor_dialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>

#include "proxy/server/iserver.h"

#include "gui/models/clients_table_model.h"
#include "gui/models/items/client_table_item.h"
#include "gui/views/fasto_table_view.h"

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

  clients_model_ = new ClientsTableModel(this);
  proxy_model_ = new QSortFilterProxyModel(this);
  proxy_model_->setSourceModel(clients_model_);
  proxy_model_->setDynamicSortFilter(true);

  QHBoxLayout* search_layout = new QHBoxLayout;
  update_button_ = new QPushButton;
  VERIFY(connect(update_button_, &QPushButton::clicked, this, &ClientsMonitorDialog::updateClicked));
  search_layout->addWidget(update_button_);

  clients_table_ = new FastoTableView;
  clients_table_->setSortingEnabled(true);
  clients_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  clients_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  clients_table_->sortByColumn(0, Qt::AscendingOrder);
  clients_table_->setModel(proxy_model_);

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

void ClientsMonitorDialog::finishLoadServerClients(const proxy::events_info::LoadServerClientsResponce& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  for (core::NDbClient client : res.clients) {
    clients_model_->insertItem(new ClientTableItem(client));
  }
}

void ClientsMonitorDialog::updateClicked() {
  proxy::events_info::LoadServerClientsRequest req(this);
  server_->LoadClients(req);
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
