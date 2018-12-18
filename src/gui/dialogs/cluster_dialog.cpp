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

#include <string>
#include <vector>

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
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

#include "gui/connection_listwidget_items.h"
#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/dialogs/connection_dialog.h"  // for ConnectionDialog
#include "gui/dialogs/discovery_cluster_dialog.h"
#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trAddConnection, trAddress, etc

namespace {
const QString trCreateCluster = QObject::tr("Create cluster");
const QString trEditCluster = QObject::tr("Edit cluster");
const QString trDefaultNameConnection = QObject::tr("New Cluster Connection");
const char* kDefaultNameConnectionFolder = "/";
}  // namespace

namespace fastonosql {
namespace gui {

ClusterDialog::ClusterDialog(proxy::IClusterSettingsBase* connection, QWidget* parent)
    : base_class(connection ? trEditCluster : trCreateCluster, parent),
      connection_name_(nullptr),
      folder_label_(nullptr),
      connection_folder_(nullptr),
      type_connection_(nullptr),
      logging_(nullptr),
      logging_msec_(nullptr),
      list_widget_(nullptr),
      test_button_(nullptr),
      discovery_button_(nullptr),
      button_box_(nullptr),
      cluster_connection_(connection) {
  setWindowIcon(GuiFactory::GetInstance().clusterIcon());

  connection_name_ = new QLineEdit;
  connection_folder_ = new QLineEdit;
  QRegExp rxf("^/[A-z0-9]+/$");
  connection_folder_->setValidator(new QRegExpValidator(rxf, this));

  folder_label_ = new QLabel;
  QHBoxLayout* folder_layout = new QHBoxLayout;
  folder_layout->addWidget(folder_label_);
  folder_layout->addWidget(connection_folder_);
  QString connection_folder = kDefaultNameConnectionFolder;
  QString connection_name = trDefaultNameConnection;

  if (cluster_connection_) {
    proxy::connection_path_t path = cluster_connection_->GetPath();
    QString qstr;
    if (common::ConvertFromString(path.GetName(), &qstr)) {
      connection_name = qstr;
    }
    if (common::ConvertFromString(path.GetDirectory(), &qstr)) {
      connection_folder = qstr;
    }
  }
  connection_name_->setText(connection_name);
  connection_folder_->setText(connection_folder);

  type_connection_ = new QComboBox;

  const auto updateCombobox = [this](core::ConnectionType type) {
    type_connection_->addItem(GuiFactory::GetInstance().icon(type), core::ConnectionTypeToString(type), type);
  };
#if defined(BUILD_WITH_REDIS)
  updateCombobox(core::REDIS);
#endif

  if (cluster_connection_) {
    type_connection_->setCurrentIndex(cluster_connection_->GetType());
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(type_connection_, static_cast<qind>(&QComboBox::currentIndexChanged), this,
                 &ClusterDialog::typeConnectionChange));

  QHBoxLayout* logging_layout = new QHBoxLayout;
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

  logging_layout->addWidget(logging_);
  logging_layout->addWidget(logging_msec_);

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

  QHBoxLayout* tool_bar_layout = new QHBoxLayout;
  QSpacerItem* hspacer = new QSpacerItem(300, 0, QSizePolicy::Expanding);
  tool_bar_layout->addWidget(createToolBar());
  tool_bar_layout->addSpacerItem(hspacer);

  QVBoxLayout* input_layout = new QVBoxLayout;
  input_layout->addWidget(connection_name_);
  input_layout->addLayout(folder_layout);
  input_layout->addWidget(type_connection_);
  input_layout->addLayout(logging_layout);
  input_layout->addLayout(tool_bar_layout);
  input_layout->addWidget(list_widget_);

  test_button_ = new QPushButton("&Test");
  test_button_->setIcon(GuiFactory::GetInstance().messageBoxInformationIcon());
  VERIFY(connect(test_button_, &QPushButton::clicked, this, &ClusterDialog::testConnection));
  test_button_->setEnabled(false);

  discovery_button_ = new QPushButton("&Discovery");
  discovery_button_->setIcon(GuiFactory::GetInstance().discoveryIcon());
  VERIFY(connect(discovery_button_, &QPushButton::clicked, this, &ClusterDialog::discoveryCluster));
  discovery_button_->setEnabled(false);

  QHBoxLayout* bottom_layout = new QHBoxLayout;
  bottom_layout->addWidget(test_button_, 0, Qt::AlignLeft);
  bottom_layout->addWidget(discovery_button_, 0, Qt::AlignLeft);
  button_box_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  button_box_->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box_, &QDialogButtonBox::accepted, this, &ClusterDialog::accept));
  VERIFY(connect(button_box_, &QDialogButtonBox::rejected, this, &ClusterDialog::reject));
  bottom_layout->addWidget(button_box_);

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addLayout(input_layout);
  main_layout->addLayout(bottom_layout);
  main_layout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(main_layout);

  // update controls
  typeConnectionChange(type_connection_->currentIndex());
  loggingStateChange(logging_->checkState());
}

proxy::IClusterSettingsBaseSPtr ClusterDialog::connection() const {
  CHECK(cluster_connection_);
  return cluster_connection_;
}

void ClusterDialog::accept() {
  if (validateAndApply()) {
    base_class::accept();
  }
}

void ClusterDialog::typeConnectionChange(int index) {
  const QVariant var = type_connection_->itemData(index);
  const core::ConnectionType current_type = static_cast<core::ConnectionType>(qvariant_cast<unsigned char>(var));
  UNUSED(current_type);

  itemSelectionChanged();
}

void ClusterDialog::loggingStateChange(int value) {
  logging_msec_->setEnabled(value);
}

void ClusterDialog::testConnection() {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!current_item) {
    return;
  }

  auto diagnostic_dialog = createDialog<ConnectionDiagnosticDialog>(translations::trConnectionDiagnostic,
                                                                    current_item->connection(), this);  // +
  diagnostic_dialog->exec();
}

void ClusterDialog::discoveryCluster() {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!current_item) {
    return;
  }

  if (!validateAndApply()) {
    return;
  }

  auto cdiagnostic_dialog = createDialog<DiscoveryClusterDiagnosticDialog>(
      translations::trConnectionDiscovery, GuiFactory::GetInstance().serverIcon(), current_item->connection(),
      cluster_connection_, this);  // +
  int result = cdiagnostic_dialog->exec();
  if (result == QDialog::Accepted) {
    std::vector<ConnectionListWidgetItemDiscovered*> conns = cdiagnostic_dialog->selectedConnections();
    for (ConnectionListWidgetItemDiscovered* connection : conns) {
      addConnection(connection->connection());
    }
  }
}

void ClusterDialog::showContextMenu(const QPoint& point) {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!current_item) {
    return;
  }

  QMenu menu(this);
  bool is_primary = list_widget_->topLevelItem(0) == current_item;
  QAction* set_default_action = new QAction(translations::trSetAsStartNode, this);
  VERIFY(connect(set_default_action, &QAction::triggered, this, &ClusterDialog::setStartNode));
  set_default_action->setEnabled(!is_primary);
  menu.addAction(set_default_action);

  QPoint menu_point = list_widget_->mapToGlobal(point);
  menu.exec(menu_point);
}

void ClusterDialog::setStartNode() {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!current_item) {
    return;
  }

  ConnectionListWidgetItem* top = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->topLevelItem(0));  // +
  if (top == current_item) {
    return;
  }

  proxy::IConnectionSettingsBaseSPtr tc = top->connection();
  proxy::IConnectionSettingsBaseSPtr cc = current_item->connection();
  current_item->setConnection(tc);
  top->setConnection(cc);
}

void ClusterDialog::add() {
#if defined(BUILD_WITH_REDIS)
  auto dlg = createDialog<ConnectionDialog>(core::REDIS, translations::trNewConnection, this);  // +
  dlg->setFolderEnabled(false);
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    addConnection(dlg->connection());
  }
#endif
}

void ClusterDialog::remove() {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!current_item) {
    return;
  }

  // Ask user
  int answer = QMessageBox::question(this, translations::trConnections,
                                     translations::trRemoveConnectionTemplate_1S.arg(current_item->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes) {
    return;
  }

  delete current_item;
}

void ClusterDialog::edit() {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!current_item) {
    return;
  }

  proxy::IConnectionSettingsBaseSPtr old_connection = current_item->connection();
  auto dlg = createDialog<ConnectionDialog>(old_connection->Clone(), this);  // +
  dlg->setFolderEnabled(false);
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    current_item->setConnection(dlg->connection());
  }
}

void ClusterDialog::itemSelectionChanged() {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +
  bool is_valid_connection = current_item != nullptr;

  test_button_->setEnabled(is_valid_connection);
  discovery_button_->setEnabled(is_valid_connection);
}

void ClusterDialog::retranslateUi() {
  logging_->setText(translations::trLoggingEnabled);
  folder_label_->setText(translations::trFolder);

  add_action_->setToolTip(translations::trAddConnection);
  remove_action_->setToolTip(translations::trRemoveConnection);
  edit_action_->setToolTip(translations::trEditConnection);
  base_class::retranslateUi();
}

QToolBar* ClusterDialog::createToolBar() {
  QToolBar* savebar = new QToolBar;
  add_action_ = new QAction;
  add_action_->setIcon(GuiFactory::GetInstance().addIcon());
  VERIFY(connect(add_action_, &QAction::triggered, this, &ClusterDialog::add));
  savebar->addAction(add_action_);

  remove_action_ = new QAction;
  remove_action_->setIcon(GuiFactory::GetInstance().removeIcon());
  VERIFY(connect(remove_action_, &QAction::triggered, this, &ClusterDialog::remove));
  savebar->addAction(remove_action_);

  edit_action_ = new QAction;
  edit_action_->setIcon(GuiFactory::GetInstance().editIcon());
  VERIFY(connect(edit_action_, &QAction::triggered, this, &ClusterDialog::edit));
  savebar->addAction(edit_action_);
  return savebar;
}

bool ClusterDialog::validateAndApply() {
  QVariant var = type_connection_->currentData();
  core::ConnectionType currentType = static_cast<core::ConnectionType>(qvariant_cast<unsigned char>(var));

  std::string conName = common::ConvertToString(connection_name_->text());
  std::string conFolder = common::ConvertToString(connection_folder_->text());
  if (conFolder.empty()) {
    conFolder = kDefaultNameConnectionFolder;
  }
  proxy::connection_path_t path(common::file_system::stable_dir_path(conFolder) + conName);
  proxy::IClusterSettingsBase* newConnection =
      proxy::ClusterConnectionSettingsFactory::GetInstance().CreateFromTypeCluster(currentType, path);
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
