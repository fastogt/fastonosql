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

#include "gui/dialogs/history_server_dialog.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>

#include <common/qt/convert2string.h>         // for ConvertFromString
#include <common/qt/gui/base/graph_widget.h>  // for GraphWidget, etc
#include <common/qt/gui/glass_widget.h>       // for GlassWidget

#include "core/db_traits.h"
#include "proxy/server/iserver.h"  // for IServer

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trClearHistory, trLoading

namespace {
const QString trHistoryTemplate_1S = QObject::tr("%1 history");
}

namespace fastonosql {
namespace gui {

ServerHistoryDialog::ServerHistoryDialog(proxy::IServerSPtr server, QWidget* parent)
    : QDialog(parent, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint), server_(server) {
  CHECK(server_);
  setWindowIcon(GuiFactory::GetInstance().icon(server_->GetType()));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  graphWidget_ = new common::qt::gui::GraphWidget;
  settingsGraph_ = new QWidget;

  clearHistory_ = new QPushButton;
  VERIFY(connect(clearHistory_, &QPushButton::clicked, this, &ServerHistoryDialog::clearHistory));
  serverInfoGroupsNames_ = new QComboBox;
  serverInfoFields_ = new QComboBox;

  typedef void (QComboBox::*curc)(int);
  VERIFY(connect(serverInfoGroupsNames_, static_cast<curc>(&QComboBox::currentIndexChanged), this,
                 &ServerHistoryDialog::refreshInfoFields));
  VERIFY(connect(serverInfoFields_, static_cast<curc>(&QComboBox::currentIndexChanged), this,
                 &ServerHistoryDialog::refreshGraph));

  const auto fields = core::InfoFieldsFromType(server_->GetType());
  for (size_t i = 0; i < fields.size(); ++i) {
    core::info_field_t field = fields[i];
    QString qitem;
    if (common::ConvertFromString(field.first, &qitem)) {
      serverInfoGroupsNames_->addItem(qitem);
    }
  }
  QVBoxLayout* setingsLayout = new QVBoxLayout;
  setingsLayout->addWidget(clearHistory_);
  setingsLayout->addWidget(serverInfoGroupsNames_);
  setingsLayout->addWidget(serverInfoFields_);
  settingsGraph_->setLayout(setingsLayout);

  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(settingsGraph_);
  splitter->addWidget(graphWidget_);
  splitter->setCollapsible(0, false);
  splitter->setCollapsible(1, false);
  splitter->setHandleWidth(1);
  setMinimumSize(QSize(min_width, min_height));
  QHBoxLayout* mainL = new QHBoxLayout;
  mainL->addWidget(splitter);
  setLayout(mainL);

  glassWidget_ = new common::qt::gui::GlassWidget(GuiFactory::GetInstance().pathToLoadingGif(), translations::trLoading,
                                                  0.5, QColor(111, 111, 100), this);
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerHistoryInfoStarted, this,
                 &ServerHistoryDialog::startLoadServerHistoryInfo));
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerHistoryInfoFinished, this,
                 &ServerHistoryDialog::finishLoadServerHistoryInfo));
  VERIFY(connect(server.get(), &proxy::IServer::ClearServerHistoryStarted, this,
                 &ServerHistoryDialog::startClearServerHistory));
  VERIFY(connect(server.get(), &proxy::IServer::ClearServerHistoryFinished, this,
                 &ServerHistoryDialog::finishClearServerHistory));
  VERIFY(connect(server.get(), &proxy::IServer::ServerInfoSnapShoot, this, &ServerHistoryDialog::snapShotAdd));
  retranslateUi();
}

void ServerHistoryDialog::startLoadServerHistoryInfo(const proxy::events_info::ServerInfoHistoryRequest& req) {
  UNUSED(req);

  glassWidget_->start();
}

void ServerHistoryDialog::finishLoadServerHistoryInfo(const proxy::events_info::ServerInfoHistoryResponce& res) {
  glassWidget_->stop();
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

void ServerHistoryDialog::finishClearServerHistory(const proxy::events_info::ClearServerHistoryResponce& res) {
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
  if (index == -1) {
    return;
  }

  serverInfoFields_->clear();

  std::vector<core::info_field_t> fields = InfoFieldsFromType(server_->GetType());
  std::vector<core::Field> field = fields[index].second;
  for (uint32_t i = 0; i < field.size(); ++i) {
    core::Field fl = field[i];
    if (fl.IsIntegral()) {
      QString qitem;
      if (common::ConvertFromString(fl.name, &qitem)) {
        serverInfoFields_->addItem(qitem, i);
      }
    }
  }
}

void ServerHistoryDialog::refreshGraph(int index) {
  if (index == -1) {
    return;
  }

  int serverIndex = serverInfoGroupsNames_->currentIndex();
  QVariant var = serverInfoFields_->itemData(index);
  uint32_t indexIn = qvariant_cast<uint32_t>(var);
  common::qt::gui::GraphWidget::nodes_container_type nodes;
  for (auto it = infos_.begin(); it != infos_.end(); ++it) {
    auto val = *it;
    if (!val.IsValid()) {
      continue;
    }

    common::Value* value = val.info->GetValueByIndexes(serverIndex, indexIn);  // allocate
    if (value) {
      qreal graphY = 0.0f;
      if (value->GetAsDouble(&graphY)) {
        nodes.push_back(std::make_pair(val.msec, graphY));
      }
      delete value;
    }
  }

  graphWidget_->setNodes(nodes);
}

void ServerHistoryDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void ServerHistoryDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  requestHistoryInfo();
}

void ServerHistoryDialog::reset() {
  refreshGraph(serverInfoFields_->currentIndex());
}

void ServerHistoryDialog::retranslateUi() {
  QString name;
  if (common::ConvertFromString(server_->GetName(), &name)) {
    setWindowTitle(trHistoryTemplate_1S.arg(name));
  }
  clearHistory_->setText(translations::trClearHistory);
}

void ServerHistoryDialog::requestHistoryInfo() {
  proxy::events_info::ServerInfoHistoryRequest req(this);
  server_->RequestHistoryInfo(req);
}

}  // namespace gui
}  // namespace fastonosql
