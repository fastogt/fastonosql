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

#include "gui/dialogs/discovery_sentinel_dialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QThread>
#include <QVBoxLayout>

#include <common/qt/gui/glass_widget.h>  // for GlassWidget

#include "proxy/connection_settings_factory.h"
#include "proxy/servers_manager.h"  // for ServersManager

#include "gui/dialogs/connection_listwidget_items.h"
#include "gui/dialogs/discovery_sentinel_connection.h"
#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"

namespace {
const QSize stateIconSize = QSize(64, 64);
}

namespace fastonosql {
namespace gui {

DiscoverySentinelDiagnosticDialog::DiscoverySentinelDiagnosticDialog(QWidget* parent,
                                                                     proxy::IConnectionSettingsBaseSPtr connection)
    : QDialog(parent) {
  setWindowTitle(translations::trConnectionDiscovery);
  setWindowIcon(GuiFactory::GetInstance().GetServerIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  QVBoxLayout* mainLayout = new QVBoxLayout;

  execute_time_label_ = new QLabel;
  execute_time_label_->setText(translations::trConnectionStatusTemplate_1S.arg("execute..."));
  mainLayout->addWidget(execute_time_label_);

  status_label_ = new QLabel(translations::trTimeTemplate_1S.arg("calculate..."));
  icon_label_ = new QLabel;
  QIcon icon = GuiFactory::GetInstance().GetFailIcon();
  const QPixmap pm = icon.pixmap(stateIconSize);
  icon_label_->setPixmap(pm);

  mainLayout->addWidget(status_label_);
  mainLayout->addWidget(icon_label_, 1, Qt::AlignCenter);

  list_widget_ = new QTreeWidget;
  list_widget_->setIndentation(5);

  QStringList colums;
  colums << translations::trName << translations::trAddress << translations::trType << translations::trState;
  list_widget_->setHeaderLabels(colums);
  list_widget_->setContextMenuPolicy(Qt::ActionsContextMenu);
  list_widget_->setIndentation(15);
  list_widget_->setSelectionMode(QAbstractItemView::MultiSelection);  // single item
                                                                      // can be draged
                                                                      // or
                                                                      // droped
  list_widget_->setSelectionBehavior(QAbstractItemView::SelectRows);

  mainLayout->addWidget(list_widget_);
  list_widget_->setEnabled(false);
  list_widget_->setToolTip(tr("Select items which you want add to sentinel."));

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &DiscoverySentinelDiagnosticDialog::accept));

  mainLayout->addWidget(buttonBox);
  setFixedSize(QSize(fix_width, fix_height));
  setLayout(mainLayout);

  glass_widget_ = new common::qt::gui::GlassWidget(GuiFactory::GetInstance().GetPathToLoadingGif(),
                                                   translations::trTryToConnect, 0.5, QColor(111, 111, 100), this);
  testConnection(connection);
}

std::vector<ConnectionListWidgetItemDiscovered*> DiscoverySentinelDiagnosticDialog::selectedConnections() const {
  std::vector<ConnectionListWidgetItemDiscovered*> res;
  for (int i = 0; i < list_widget_->topLevelItemCount(); ++i) {
    QTreeWidgetItem* citem = list_widget_->topLevelItem(i);
    if (citem->isSelected()) {
      ConnectionListWidgetItemDiscovered* item = dynamic_cast<ConnectionListWidgetItemDiscovered*>(citem);  // +
      if (item) {
        res.push_back(item);
      }
    }
  }
  return res;
}

void DiscoverySentinelDiagnosticDialog::connectionResultReady(
    bool suc,
    qint64 mstimeExecute,
    const QString& resultText,
    std::vector<core::ServerDiscoverySentinelInfoSPtr> infos) {
  glass_widget_->stop();

  execute_time_label_->setText(translations::trTimeTemplate_1S.arg(mstimeExecute));
  list_widget_->setEnabled(suc);
  list_widget_->clear();
  if (suc) {
    QIcon icon = GuiFactory::GetInstance().GetSuccessIcon();
    QPixmap pm = icon.pixmap(stateIconSize);
    icon_label_->setPixmap(pm);

    for (size_t i = 0; i < infos.size(); ++i) {
      core::ServerDiscoverySentinelInfoSPtr inf = infos[i];
      common::net::HostAndPort host = inf->GetHost();
      proxy::connection_path_t path(common::file_system::get_separator_string<char>() + inf->GetName());
      proxy::IConnectionSettingsBaseSPtr con(
          proxy::ConnectionSettingsFactory::GetInstance().CreateFromType(inf->GetConnectionType(), path, host));

      ConnectionListWidgetItemDiscovered* item = new ConnectionListWidgetItemDiscovered(inf->GetInfo(), nullptr);
      item->setConnection(con);
      list_widget_->addTopLevelItem(item);
    }
  }
  status_label_->setText(translations::trConnectionStatusTemplate_1S.arg(resultText));
}

void DiscoverySentinelDiagnosticDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  glass_widget_->start();
}

void DiscoverySentinelDiagnosticDialog::testConnection(proxy::IConnectionSettingsBaseSPtr connection) {
  QThread* th = new QThread;
  DiscoverySentinelConnection* cheker = new DiscoverySentinelConnection(connection);
  cheker->moveToThread(th);
  VERIFY(connect(th, &QThread::started, cheker, &DiscoverySentinelConnection::routine));
  VERIFY(connect(cheker, &DiscoverySentinelConnection::connectionResult, this,
                 &DiscoverySentinelDiagnosticDialog::connectionResultReady));
  VERIFY(connect(cheker, &DiscoverySentinelConnection::connectionResult, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, cheker, &DiscoverySentinelConnection::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

}  // namespace gui
}  // namespace fastonosql
