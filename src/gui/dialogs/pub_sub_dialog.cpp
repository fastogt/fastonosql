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

#include "gui/dialogs/pub_sub_dialog.h"

#include <QVBoxLayout>
#include <QEvent>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QSortFilterProxyModel>

#include <common/qt/convert2string.h>

#include "proxy/server/iserver.h"

#include "gui/fasto_table_view.h"      // for FastoTableView
#include "gui/gui_factory.h"           // for GuiFactory
#include "gui/channels_table_model.h"  // for KeysTableModel, etc

#include "translations/global.h"  // for trKeyCountOnThePage, etc

namespace fastonosql {
namespace gui {

PubSubDialog::PubSubDialog(const QString& title, proxy::IServerSPtr server, QWidget* parent)
    : QDialog(parent), server_(server) {
  CHECK(server_);
  setWindowTitle(title);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  VERIFY(connect(server.get(), &proxy::IServer::LoadServerChannelsStarted, this,
                 &PubSubDialog::startLoadServerChannels));
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerChannelsFinished, this,
                 &PubSubDialog::finishLoadServerChannels));

  // main layout
  QVBoxLayout* mainlayout = new QVBoxLayout;
  QHBoxLayout* searchLayout = new QHBoxLayout;
  searchBox_ = new QLineEdit;
  searchBox_->setText("*");
  VERIFY(connect(searchBox_, &QLineEdit::textChanged, this, &PubSubDialog::searchLineChanged));
  searchLayout->addWidget(searchBox_);

  searchButton_ = new QPushButton;
  VERIFY(connect(searchButton_, &QPushButton::clicked, this, &PubSubDialog::searchClicked));
  searchLayout->addWidget(searchButton_);
  mainlayout->addLayout(searchLayout);

  channelsModel_ = new ChannelsTableModel(this);
  QSortFilterProxyModel* proxy_model = new QSortFilterProxyModel(this);
  proxy_model->setSourceModel(channelsModel_);
  proxy_model->setDynamicSortFilter(true);

  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteStarted, this, &PubSubDialog::startExecute,
                 Qt::DirectConnection));
  VERIFY(connect(server_.get(), &proxy::IServer::ExecuteFinished, this,
                 &PubSubDialog::finishExecute, Qt::DirectConnection));

  channelsTable_ = new FastoTableView;
  channelsTable_->setSortingEnabled(true);
  channelsTable_->sortByColumn(0, Qt::AscendingOrder);
  channelsTable_->setModel(proxy_model);

  QDialogButtonBox* buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
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

void PubSubDialog::startLoadServerChannels(
    const proxy::events_info::LoadServerChannelsRequest& req) {
  UNUSED(req);

  channelsModel_->clear();
}

void PubSubDialog::finishLoadServerChannels(
    const proxy::events_info::LoadServerChannelsResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
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
