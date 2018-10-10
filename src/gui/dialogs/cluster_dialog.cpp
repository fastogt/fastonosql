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

#include "gui/dialogs/cluster_dialog.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QToolBar>

#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/types.h>

#include <fastonosql/core/connection_types.h>  // for ConnectionType, etc
#include "proxy/cluster_connection_settings_factory.h"

#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/dialogs/connection_dialog.h"  // for ConnectionDialog
#include "gui/dialogs/connection_listwidget_items.h"
#include "gui/dialogs/discovery_cluster_dialog.h"
#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trAddConnection, trAddress, etc

namespace {
const QString defaultNameConnection = QObject::tr("New Cluster Connection");
const char* defaultNameConnectionFolder = "/";
}  // namespace

namespace fastonosql {
namespace gui {

ClusterDialog::ClusterDialog(QWidget* parent, proxy::IClusterSettingsBase* connection)
    : QDialog(parent), cluster_connection_(connection) {
  setWindowIcon(GuiFactory::GetInstance().serverIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  connection_name_ = new QLineEdit;
  connection_folder_ = new QLineEdit;
  QRegExp rxf("^/[A-z0-9]+/$");
  connection_folder_->setValidator(new QRegExpValidator(rxf, this));

  folder_label_ = new QLabel;
  QHBoxLayout* folderLayout = new QHBoxLayout;
  folderLayout->addWidget(folder_label_);
  folderLayout->addWidget(connection_folder_);
  QString conFolder = defaultNameConnectionFolder;
  QString conName = defaultNameConnection;

  if (cluster_connection_) {
    proxy::connection_path_t path = cluster_connection_->GetPath();
    QString qstr;
    if (common::ConvertFromString(path.GetName(), &qstr)) {
      conName = qstr;
    }
    if (common::ConvertFromString(path.GetDirectory(), &qstr)) {
      conFolder = qstr;
    }
  }
  connection_name_->setText(conName);
  connection_folder_->setText(conFolder);

  type_connection_ = new QComboBox;

  for (size_t i = 0; i < core::g_compiled_types.size(); ++i) {
    core::ConnectionType ct = core::g_compiled_types[i];
    std::string str = common::ConvertToString(ct);
    QString qstr;
    if (common::ConvertFromString(str, &qstr)) {
      type_connection_->addItem(GuiFactory::GetInstance().icon(ct), qstr, ct);
    }
  }

  if (cluster_connection_) {
    type_connection_->setCurrentIndex(cluster_connection_->GetType());
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(type_connection_, static_cast<qind>(&QComboBox::currentIndexChanged), this,
                 &ClusterDialog::typeConnectionChange));

  QHBoxLayout* loggingLayout = new QHBoxLayout;
  logging_ = new QCheckBox;
  logging_msec_ = new QSpinBox;
  logging_msec_->setRange(0, INT32_MAX);
  logging_msec_->setSingleStep(1000);

  if (cluster_connection_) {
    logging_->setChecked(cluster_connection_->IsHistoryEnabled());
    logging_msec_->setValue(cluster_connection_->GetLoggingMsTimeInterval());
  } else {
    logging_->setChecked(false);
  }
  VERIFY(connect(logging_, &QCheckBox::stateChanged, this, &ClusterDialog::loggingStateChange));

  loggingLayout->addWidget(logging_);
  loggingLayout->addWidget(logging_msec_);

  list_widget_ = new QTreeWidget;
  list_widget_->setIndentation(5);

  QStringList colums;
  colums << translations::trName << translations::trAddress;
  list_widget_->setHeaderLabels(colums);
  list_widget_->setIndentation(15);
  list_widget_->setSelectionMode(QAbstractItemView::SingleSelection);  // single item
                                                                       // can be draged
                                                                       // or
                                                                       // droped
  list_widget_->setSelectionBehavior(QAbstractItemView::SelectRows);

  list_widget_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(list_widget_, &QTreeWidget::customContextMenuRequested, this, &ClusterDialog::showContextMenu));

  if (cluster_connection_) {
    auto nodes = cluster_connection_->GetNodes();
    for (const auto& node : nodes) {
      addConnection(node);
    }
  }

  VERIFY(connect(list_widget_, &QTreeWidget::itemSelectionChanged, this, &ClusterDialog::itemSelectionChanged));

  QHBoxLayout* toolBarLayout = new QHBoxLayout;
  savebar_ = new QToolBar;
  toolBarLayout->addWidget(savebar_);

  QAction* addB = new QAction(GuiFactory::GetInstance().addIcon(), translations::trAddConnection, savebar_);
  typedef void (QAction::*trig)(bool);
  VERIFY(connect(addB, static_cast<trig>(&QAction::triggered), this, &ClusterDialog::add));
  savebar_->addAction(addB);

  QAction* rmB = new QAction(GuiFactory::GetInstance().removeIcon(), translations::trRemoveConnection, savebar_);
  VERIFY(connect(rmB, static_cast<trig>(&QAction::triggered), this, &ClusterDialog::remove));
  savebar_->addAction(rmB);

  QAction* editB = new QAction(GuiFactory::GetInstance().editIcon(), translations::trEditConnection, savebar_);
  VERIFY(connect(editB, static_cast<trig>(&QAction::triggered), this, &ClusterDialog::edit));
  savebar_->addAction(editB);

  QSpacerItem* hSpacer = new QSpacerItem(300, 0, QSizePolicy::Expanding);
  toolBarLayout->addSpacerItem(hSpacer);

  QVBoxLayout* inputLayout = new QVBoxLayout;
  inputLayout->addWidget(connection_name_);
  inputLayout->addLayout(folderLayout);
  inputLayout->addWidget(type_connection_);
  inputLayout->addLayout(loggingLayout);
  inputLayout->addLayout(toolBarLayout);
  inputLayout->addWidget(list_widget_);

  test_button_ = new QPushButton("&Test");
  test_button_->setIcon(GuiFactory::GetInstance().messageBoxInformationIcon());
  VERIFY(connect(test_button_, &QPushButton::clicked, this, &ClusterDialog::testConnection));
  test_button_->setEnabled(false);

  discovery_button_ = new QPushButton("&Discovery");
  discovery_button_->setIcon(GuiFactory::GetInstance().discoveryIcon());
  VERIFY(connect(discovery_button_, &QPushButton::clicked, this, &ClusterDialog::discoveryCluster));
  discovery_button_->setEnabled(false);

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(test_button_, 0, Qt::AlignLeft);
  bottomLayout->addWidget(discovery_button_, 0, Qt::AlignLeft);
  button_box_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  button_box_->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box_, &QDialogButtonBox::accepted, this, &ClusterDialog::accept));
  VERIFY(connect(button_box_, &QDialogButtonBox::rejected, this, &ClusterDialog::reject));
  bottomLayout->addWidget(button_box_);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addLayout(inputLayout);
  mainLayout->addLayout(bottomLayout);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(mainLayout);

  // update controls
  typeConnectionChange(type_connection_->currentIndex());
  loggingStateChange(logging_->checkState());
  retranslateUi();
}

proxy::IClusterSettingsBaseSPtr ClusterDialog::connection() const {
  return cluster_connection_;
}

void ClusterDialog::accept() {
  if (validateAndApply()) {
    QDialog::accept();
  }
}

void ClusterDialog::typeConnectionChange(int index) {
  QVariant var = type_connection_->itemData(index);
  core::ConnectionType currentType = static_cast<core::ConnectionType>(qvariant_cast<unsigned char>(var));
  bool isValidType = currentType == core::REDIS;
  connection_name_->setEnabled(isValidType);
  button_box_->button(QDialogButtonBox::Save)->setEnabled(isValidType);
  savebar_->setEnabled(isValidType);
  list_widget_->selectionModel()->clear();
  list_widget_->setEnabled(isValidType);
  logging_->setEnabled(isValidType);
  itemSelectionChanged();
}

void ClusterDialog::loggingStateChange(int value) {
  logging_msec_->setEnabled(value);
}

void ClusterDialog::testConnection() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  ConnectionDiagnosticDialog diag(this, currentItem->connection());
  diag.exec();
}

void ClusterDialog::discoveryCluster() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  if (!validateAndApply()) {
    return;
  }

  DiscoveryClusterDiagnosticDialog diag(this, currentItem->connection(), cluster_connection_);
  int result = diag.exec();
  if (result == QDialog::Accepted) {
    std::vector<ConnectionListWidgetItemDiscovered*> conns = diag.selectedConnections();
    for (size_t i = 0; i < conns.size(); ++i) {
      proxy::IConnectionSettingsBaseSPtr it = conns[i]->connection();
      addConnection(it);
    }
  }
}

void ClusterDialog::showContextMenu(const QPoint& point) {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  QMenu menu(this);
  bool is_primary = list_widget_->topLevelItem(0) == currentItem;
  QAction* setDefault = new QAction(translations::trSetAsStartNode, this);
  VERIFY(connect(setDefault, &QAction::triggered, this, &ClusterDialog::setStartNode));
  setDefault->setEnabled(!is_primary);
  menu.addAction(setDefault);

  QPoint menuPoint = list_widget_->mapToGlobal(point);
  menu.exec(menuPoint);
}

void ClusterDialog::setStartNode() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  ConnectionListWidgetItem* top = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->topLevelItem(0));  // +
  if (top == currentItem) {
    return;
  }

  proxy::IConnectionSettingsBaseSPtr tc = top->connection();
  proxy::IConnectionSettingsBaseSPtr cc = currentItem->connection();
  currentItem->setConnection(tc);
  top->setConnection(cc);
}

void ClusterDialog::add() {
#ifdef BUILD_WITH_REDIS
  ConnectionDialog dlg(core::REDIS, translations::trNewConnection, this);
  dlg.setFolderEnabled(false);
  int result = dlg.exec();
  proxy::IConnectionSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
    addConnection(p);
  }
#endif
}

void ClusterDialog::remove() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  // Ask user
  int answer = QMessageBox::question(this, translations::trConnections,
                                     translations::trDeleteConnectionTemplate_1S.arg(currentItem->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  delete currentItem;
}

void ClusterDialog::edit() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

#ifdef BUILD_WITH_REDIS
  proxy::IConnectionSettingsBaseSPtr oldConnection = currentItem->connection();
  ConnectionDialog dlg(oldConnection->Clone(), this);
  dlg.setFolderEnabled(false);
  int result = dlg.exec();
  proxy::IConnectionSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    currentItem->setConnection(newConnection);
  }
#endif
}

void ClusterDialog::itemSelectionChanged() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +
  bool isValidConnection = currentItem != nullptr;

  test_button_->setEnabled(isValidConnection);
  discovery_button_->setEnabled(isValidConnection);
}

void ClusterDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QDialog::changeEvent(e);
}

void ClusterDialog::retranslateUi() {
  logging_->setText(translations::trLoggingEnabled);
  folder_label_->setText(translations::trFolder);
}

bool ClusterDialog::validateAndApply() {
  QVariant var = type_connection_->currentData();
  core::ConnectionType currentType = static_cast<core::ConnectionType>(qvariant_cast<unsigned char>(var));

  std::string conName = common::ConvertToString(connection_name_->text());
  std::string conFolder = common::ConvertToString(connection_folder_->text());
  if (conFolder.empty()) {
    conFolder = defaultNameConnectionFolder;
  }
  proxy::connection_path_t path(common::file_system::stable_dir_path(conFolder) + conName);
  proxy::IClusterSettingsBase* newConnection =
      proxy::ClusterConnectionSettingsFactory::GetInstance().CreateFromType(currentType, path);
  if (newConnection) {
    cluster_connection_.reset(newConnection);
    if (logging_->isChecked()) {
      cluster_connection_->SetLoggingMsTimeInterval(logging_msec_->value());
    }
    for (int i = 0; i < list_widget_->topLevelItemCount(); ++i) {
      ConnectionListWidgetItem* item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->topLevelItem(i));  // +
      if (item) {
        proxy::IConnectionSettingsBaseSPtr con = item->connection();
        cluster_connection_->AddNode(con);
      }
    }
  }
  return true;
}

void ClusterDialog::addConnection(proxy::IConnectionSettingsBaseSPtr con) {
  ConnectionListWidgetItem* item = new ConnectionListWidgetItem(nullptr);
  item->setConnection(con);
  list_widget_->addTopLevelItem(item);
}

}  // namespace gui
}  // namespace fastonosql
