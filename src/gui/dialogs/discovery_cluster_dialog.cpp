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

#include "gui/dialogs/discovery_cluster_dialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QThread>
#include <QVBoxLayout>

#include <common/qt/gui/glass_widget.h>  // for GlassWidget

#include "proxy/connection_settings/iconnection_settings_remote.h"
#include "proxy/connection_settings_factory.h"
#include "proxy/servers_manager.h"  // for ServersManager

#include "gui/connection_listwidget_items.h"
#include "gui/gui_factory.h"  // for GuiFactory
#include "gui/workers/discovery_cluster_connection.h"

#include "translations/global.h"

namespace {
const QSize kStateIconSize = QSize(64, 64);
const QString trTooltip = QObject::tr("Select items which you want add to cluster.");
}  // namespace

namespace fastonosql {
namespace gui {

DiscoveryClusterDiagnosticDialog::DiscoveryClusterDiagnosticDialog(const QString& title,
                                                                   const QIcon& icon,
                                                                   proxy::IConnectionSettingsBaseSPtr connection,
                                                                   proxy::IClusterSettingsBaseSPtr cluster,
                                                                   QWidget* parent)
    : QDialog(parent),
      glass_widget_(nullptr),
      execute_time_label_(nullptr),
      status_label_(nullptr),
      list_widget_(nullptr),
      icon_label_(nullptr),
      cluster_(cluster) {
  setWindowTitle(title);
  setWindowIcon(icon);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  execute_time_label_ = new QLabel;
  execute_time_label_->setText(translations::trConnectionStatusTemplate_1S.arg("execute..."));

  status_label_ = new QLabel(translations::trTimeTemplate_1S.arg("calculate..."));
  icon_label_ = new QLabel;
  const QIcon fail_icon = GuiFactory::GetInstance().failIcon();
  const QPixmap pm = fail_icon.pixmap(kStateIconSize);
  icon_label_->setPixmap(pm);

  list_widget_ = new QTreeWidget;
  list_widget_->setIndentation(5);
  QStringList colums;
  colums << translations::trName << translations::trAddress << translations::trType << translations::trState;
  list_widget_->setHeaderLabels(colums);
  list_widget_->setContextMenuPolicy(Qt::ActionsContextMenu);
  list_widget_->setIndentation(15);
  list_widget_->setSelectionMode(QAbstractItemView::MultiSelection);
  list_widget_->setSelectionBehavior(QAbstractItemView::SelectRows);
  list_widget_->setEnabled(false);
  list_widget_->setToolTip(trTooltip);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &DiscoveryClusterDiagnosticDialog::accept));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(execute_time_label_);
  main_layout->addWidget(status_label_);
  main_layout->addWidget(icon_label_, 1, Qt::AlignCenter);
  main_layout->addWidget(list_widget_);
  main_layout->addWidget(button_box);
  setLayout(main_layout);
  setFixedSize(QSize(fix_width, fix_height));

  glass_widget_ =
      new common::qt::gui::GlassWidget(GuiFactory::GetInstance().pathToLoadingGif(),
                                       translations::trTryToConnect + "...", 0.5, QColor(111, 111, 100), this);
  testConnection(connection);
}

std::vector<ConnectionListWidgetItemDiscovered*> DiscoveryClusterDiagnosticDialog::selectedConnections() const {
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

void DiscoveryClusterDiagnosticDialog::connectionResult(bool suc,
                                                        qint64 exec_mstime,
                                                        const QString& result_text,
                                                        std::vector<core::ServerDiscoveryClusterInfoSPtr> infos) {
  glass_widget_->stop();

  execute_time_label_->setText(translations::trTimeTemplate_1S.arg(exec_mstime));
  list_widget_->setEnabled(suc);
  list_widget_->clear();
  if (suc) {
    QIcon icon = GuiFactory::GetInstance().successIcon();
    const QPixmap pm = icon.pixmap(kStateIconSize);
    icon_label_->setPixmap(pm);

    for (core::ServerDiscoveryClusterInfoSPtr inf : infos) {
      common::net::HostAndPortAndSlot host = inf->GetHost();
      proxy::connection_path_t path(common::file_system::get_separator_string<char>() + inf->GetName());
      proxy::IConnectionSettingsRemote* remote =
          proxy::ConnectionSettingsFactory::GetInstance().CreateSettingsFromTypeConnection(inf->GetConnectionType(),
                                                                                           path, host);
      ConnectionListWidgetItemDiscovered* item = new ConnectionListWidgetItemDiscovered(inf->GetInfo(), nullptr);
      item->setConnection(proxy::IConnectionSettingsBaseSPtr(remote));
      item->setDisabled(inf->Self() || cluster_->FindSettingsByHost(host));
      list_widget_->addTopLevelItem(item);
    }
  }
  status_label_->setText(translations::trConnectionStatusTemplate_1S.arg(result_text));
}

void DiscoveryClusterDiagnosticDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  glass_widget_->start();
}

void DiscoveryClusterDiagnosticDialog::testConnection(proxy::IConnectionSettingsBaseSPtr connection) {
  QThread* th = new QThread;
  DiscoveryConnection* cheker = new DiscoveryConnection(connection);
  cheker->moveToThread(th);
  VERIFY(connect(th, &QThread::started, cheker, &DiscoveryConnection::routine));
  VERIFY(connect(cheker, &DiscoveryConnection::connectionResult, this,
                 &DiscoveryClusterDiagnosticDialog::connectionResult));
  VERIFY(connect(cheker, &DiscoveryConnection::connectionResult, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, cheker, &DiscoveryConnection::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

}  // namespace gui
}  // namespace fastonosql
