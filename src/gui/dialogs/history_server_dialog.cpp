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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/history_server_dialog.h"

#include <vector>

#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>

#include <common/qt/convert2string.h>
#include <common/qt/gui/base/graph_widget.h>  // for GraphWidget, etc
#include <common/qt/gui/glass_widget.h>       // for GlassWidget

#include "proxy/server/iserver.h"  // for IServer

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trClearHistory

namespace fastonosql {
namespace gui {

ServerHistoryDialog::ServerHistoryDialog(const QString& title,
                                         const QIcon& icon,
                                         proxy::IServerSPtr server,
                                         QWidget* parent)
    : base_class(title, parent),
      settings_graph_(nullptr),
      clear_history_(nullptr),
      server_info_groups_names_(nullptr),
      server_info_fields_(nullptr),
      graph_widget_(nullptr),
      glass_widget_(nullptr),
      infos_(),
      server_(server) {
  CHECK(server_);
  setWindowIcon(icon);

  VERIFY(connect(server.get(), &proxy::IServer::LoadServerHistoryInfoStarted, this,
                 &ServerHistoryDialog::startLoadServerHistoryInfo));
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerHistoryInfoFinished, this,
                 &ServerHistoryDialog::finishLoadServerHistoryInfo));
  VERIFY(connect(server.get(), &proxy::IServer::ClearServerHistoryStarted, this,
                 &ServerHistoryDialog::startClearServerHistory));
  VERIFY(connect(server.get(), &proxy::IServer::ClearServerHistoryFinished, this,
                 &ServerHistoryDialog::finishClearServerHistory));
  VERIFY(connect(server.get(), &proxy::IServer::ServerInfoSnapShooted, this, &ServerHistoryDialog::snapShotAdd));

  graph_widget_ = new common::qt::gui::GraphWidget;
  settings_graph_ = new QWidget;

  clear_history_ = new QPushButton;
  VERIFY(connect(clear_history_, &QPushButton::clicked, this, &ServerHistoryDialog::clearHistory));
  server_info_groups_names_ = new QComboBox;
  server_info_fields_ = new QComboBox;

  typedef void (QComboBox::*curc)(int);
  VERIFY(connect(server_info_groups_names_, static_cast<curc>(&QComboBox::currentIndexChanged), this,
                 &ServerHistoryDialog::refreshInfoFields));
  VERIFY(connect(server_info_fields_, static_cast<curc>(&QComboBox::currentIndexChanged), this,
                 &ServerHistoryDialog::refreshGraph));

  const auto fields = server_->GetInfoFields();
  for (auto field : fields) {
    QString qitem;
    if (common::ConvertFromString(field.first, &qitem)) {
      server_info_groups_names_->addItem(qitem);
    }
  }
  QVBoxLayout* settings_layout = new QVBoxLayout;
  settings_layout->addWidget(clear_history_);
  settings_layout->addWidget(server_info_groups_names_);
  settings_layout->addWidget(server_info_fields_);
  settings_graph_->setLayout(settings_layout);

  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(settings_graph_);
  splitter->addWidget(graph_widget_);
  splitter->setCollapsible(0, false);
  splitter->setCollapsible(1, false);
  splitter->setHandleWidth(1);
  setMinimumSize(QSize(min_width, min_height));

  QHBoxLayout* main_layout = new QHBoxLayout;
  main_layout->addWidget(splitter);
  setLayout(main_layout);

  glass_widget_ = new common::qt::gui::GlassWidget(GuiFactory::GetInstance().pathToLoadingGif(),
                                                   translations::trLoad + "...", 0.5, QColor(111, 111, 100), this);
}

void ServerHistoryDialog::startLoadServerHistoryInfo(const proxy::events_info::ServerInfoHistoryRequest& req) {
  UNUSED(req);

  glass_widget_->start();
}

void ServerHistoryDialog::finishLoadServerHistoryInfo(const proxy::events_info::ServerInfoHistoryResponse& res) {
  glass_widget_->stop();
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  infos_ = res.infos();
  reset();
}

void ServerHistoryDialog::startClearServerHistory(const proxy::events_info::ClearServerHistoryRequest& req) {
  UNUSED(req);
}

void ServerHistoryDialog::finishClearServerHistory(const proxy::events_info::ClearServerHistoryResponse& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  requestHistoryInfo();
}

void ServerHistoryDialog::snapShotAdd(core::ServerInfoSnapShoot snapshot) {
  infos_.push_back(snapshot);
  reset();
}

void ServerHistoryDialog::clearHistory() {
  proxy::events_info::ClearServerHistoryRequest req(this);
  server_->ClearHistory(req);
}

void ServerHistoryDialog::refreshInfoFields(int index) {
  if (index < 0) {
    return;
  }

  const unsigned int stabled_index = static_cast<unsigned int>(index);
  server_info_fields_->clear();

  const auto fields = server_->GetInfoFields();
  std::vector<core::Field> field = fields[stabled_index].second;
  for (uint32_t i = 0; i < field.size(); ++i) {
    core::Field fl = field[i];
    if (fl.IsIntegral()) {
      QString qitem;
      if (common::ConvertFromString(fl.name, &qitem)) {
        server_info_fields_->addItem(qitem, i);
      }
    }
  }
}

void ServerHistoryDialog::refreshGraph(int index) {
  if (index == -1) {
    return;
  }

  const int server_index = server_info_groups_names_->currentIndex();
  QVariant var = server_info_fields_->itemData(index);
  uint32_t index_in = qvariant_cast<uint32_t>(var);
  common::qt::gui::GraphWidget::nodes_container_type nodes;
  for (auto val : infos_) {
    if (!val.IsValid()) {
      continue;
    }

    common::Value* value = val.info->GetValueByIndexes(server_index, index_in);  // allocate
    if (value) {
      qreal graphy = 0.0f;
      if (value->GetAsDouble(&graphy)) {
        nodes.push_back(std::make_pair(val.msec, graphy));
      }
      delete value;
    }
  }

  graph_widget_->setNodes(nodes);
}

void ServerHistoryDialog::showEvent(QShowEvent* e) {
  base_class::showEvent(e);
  requestHistoryInfo();
}

void ServerHistoryDialog::reset() {
  refreshGraph(server_info_fields_->currentIndex());
}

void ServerHistoryDialog::retranslateUi() {
  clear_history_->setText(translations::trClearHistory);
  base_class::retranslateUi();
}

void ServerHistoryDialog::requestHistoryInfo() {
  proxy::events_info::ServerInfoHistoryRequest req(this);
  server_->RequestHistoryInfo(req);
}

}  // namespace gui
}  // namespace fastonosql
