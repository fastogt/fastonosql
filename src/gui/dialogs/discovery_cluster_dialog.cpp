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

#include "gui/dialogs/discovery_cluster_dialog.h"

#include <string>
#include <vector>

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QThread>
#include <QTreeWidget>

#include "common/time.h"
#include "common/convert2string.h"

#include "fasto/qt/gui/glass_widget.h"

#include "core/servers_manager.h"

#include "translations/global.h"

#include "gui/gui_factory.h"
#include "gui/dialogs/connection_listwidget_items.h"

namespace {
  const QSize stateIconSize = QSize(64, 64);
}

namespace fastonosql {
namespace gui {

DiscoveryConnection::DiscoveryConnection(core::IConnectionSettingsBaseSPtr conn, QObject* parent)
  : QObject(parent), connection_(conn), startTime_(common::time::current_mstime()) {
  qRegisterMetaType<std::vector<core::ServerDiscoveryClusterInfoSPtr> >("std::vector<core::ServerDiscoveryClusterInfoSPtr>");
}

void DiscoveryConnection::routine() {
  std::vector<core::ServerDiscoveryClusterInfoSPtr> inf;

  if (!connection_) {
    emit connectionResult(false, common::time::current_mstime() - startTime_,
                          "Invalid connection settings", inf);
    return;
  }

  common::Error er = core::ServersManager::instance().discoveryClusterConnection(connection_, &inf);

  if (er && er->isError()) {
    emit connectionResult(false, common::time::current_mstime() - startTime_,
                          common::ConvertFromString<QString>(er->description()), inf);
  } else {
    emit connectionResult(true, common::time::current_mstime() - startTime_,
                          translations::trSuccess, inf);
  }
}

DiscoveryClusterDiagnosticDialog::DiscoveryClusterDiagnosticDialog(QWidget* parent,
                                                     core::IConnectionSettingsBaseSPtr connection,
                                                     core::IClusterSettingsBaseSPtr cluster)
  : QDialog(parent), cluster_(cluster) {
  setWindowTitle(translations::trConnectionDiscovery);
  setWindowIcon(GuiFactory::instance().serverIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  QVBoxLayout* mainLayout = new QVBoxLayout;

  executeTimeLabel_ = new QLabel;
  executeTimeLabel_->setText(translations::trConnectionStatusTemplate_1S.arg("execute..."));
  mainLayout->addWidget(executeTimeLabel_);

  statusLabel_ = new QLabel(translations::trTimeTemplate_1S.arg("calculate..."));
  iconLabel_ = new QLabel;
  QIcon icon = GuiFactory::instance().failIcon();
  const QPixmap pm = icon.pixmap(stateIconSize);
  iconLabel_->setPixmap(pm);

  mainLayout->addWidget(statusLabel_);
  mainLayout->addWidget(iconLabel_, 1, Qt::AlignCenter);

  listWidget_ = new QTreeWidget;
  listWidget_->setIndentation(5);

  QStringList colums;
  colums << translations::trName << translations::trAddress << translations::trType << translations::trState;
  listWidget_->setHeaderLabels(colums);
  listWidget_->setContextMenuPolicy(Qt::ActionsContextMenu);
  listWidget_->setIndentation(15);
  listWidget_->setSelectionMode(QAbstractItemView::MultiSelection);  // single item can be draged or droped
  listWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);

  mainLayout->addWidget(listWidget_);
  listWidget_->setEnabled(false);
  listWidget_->setToolTip(tr("Select items which you want add to cluster."));

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &DiscoveryClusterDiagnosticDialog::accept));

  mainLayout->addWidget(buttonBox);
  setFixedSize(QSize(fix_width, fix_height));
  setLayout(mainLayout);

  glassWidget_ = new fasto::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(),
                                                 translations::trTryToConnect, 0.5,
                                                 QColor(111, 111, 100), this);
  testConnection(connection);
}

std::vector<ConnectionListWidgetItemDiscovered*> DiscoveryClusterDiagnosticDialog::selectedConnections() const {
  std::vector<ConnectionListWidgetItemDiscovered*> res;
  for (int i = 0; i < listWidget_->topLevelItemCount(); ++i) {
    QTreeWidgetItem *citem = listWidget_->topLevelItem(i);
    if (citem->isSelected()) {
      ConnectionListWidgetItemDiscovered* item = dynamic_cast<ConnectionListWidgetItemDiscovered*>(citem);  // +
      if (item) {
        res.push_back(item);
      }
    }
  }
  return res;
}

void DiscoveryClusterDiagnosticDialog::connectionResult(bool suc, qint64 mstimeExecute,
                                                 const QString& resultText,
                                                 std::vector<core::ServerDiscoveryClusterInfoSPtr> infos) {
  glassWidget_->stop();

  executeTimeLabel_->setText(translations::trTimeTemplate_1S.arg(mstimeExecute));
  listWidget_->setEnabled(suc);
  listWidget_->clear();
  if (suc) {
    QIcon icon = GuiFactory::instance().successIcon();
    const QPixmap pm = icon.pixmap(stateIconSize);
    iconLabel_->setPixmap(pm);

    for (size_t i = 0; i < infos.size(); ++i) {
      core::ServerDiscoveryClusterInfoSPtr inf = infos[i];
      common::net::HostAndPortAndSlot host = inf->host();
      core::IConnectionSettingsBase::connection_path_t path(common::file_system::get_separator_string<char>() + inf->name());
      core::IConnectionSettingsBaseSPtr con(core::IConnectionSettingsRemote::createFromType(inf->connectionType(), path, host));
      ConnectionListWidgetItemDiscovered* item = new ConnectionListWidgetItemDiscovered(inf->info(), nullptr);
      item->setConnection(con);
      item->setDisabled(inf->self() || cluster_->findSettingsByHost(host));
      listWidget_->addTopLevelItem(item);
    }
  }
  statusLabel_->setText(translations::trConnectionStatusTemplate_1S.arg(resultText));
}

void DiscoveryClusterDiagnosticDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  glassWidget_->start();
}

void DiscoveryClusterDiagnosticDialog::testConnection(core::IConnectionSettingsBaseSPtr connection) {
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
