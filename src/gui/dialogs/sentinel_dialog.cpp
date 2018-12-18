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

#include "gui/dialogs/sentinel_dialog.h"

#include <string>
#include <vector>

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QToolBar>

#include <common/qt/convert2string.h>  // for ConvertToString

#include "proxy/sentinel_connection_settings_factory.h"

#include "gui/connection_listwidget_items.h"
#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/dialogs/connection_dialog.h"  // for ConnectionDialog
#include "gui/dialogs/discovery_sentinel_dialog.h"
#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trAddConnection, trAddress, etc

namespace {
const QString trCreateSentinel = QObject::tr("Create sentinel");
const QString trEditSentinel = QObject::tr("Edit sentinel");
const char* kDefaultSentinelNameConnection = "New Sentinel Connection";
const char* kDefaultNameConnectionFolder = "/";
}  // namespace

namespace fastonosql {
namespace gui {

SentinelDialog::SentinelDialog(proxy::ISentinelSettingsBase* connection, QWidget* parent)
    : base_class(connection ? trEditSentinel : trCreateSentinel, parent),
      sentinel_connection_(connection),
      connection_name_(nullptr),
      folder_label_(nullptr),
      connection_folder_(nullptr),
      type_connection_(nullptr),
      logging_(nullptr),
      logging_msec_(nullptr),
      list_widget_(nullptr),
      test_button_(nullptr),
      discovery_button_(nullptr),
      button_box_(nullptr) {
  setWindowIcon(GuiFactory::GetInstance().sentinelIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  connection_name_ = new QLineEdit;
  connection_folder_ = new QLineEdit;
  QRegExp rxf("^/[A-z0-9]+/$");
  connection_folder_->setValidator(new QRegExpValidator(rxf, this));

  folder_label_ = new QLabel;
  QHBoxLayout* folder_layout = new QHBoxLayout;
  folder_layout->addWidget(folder_label_);
  folder_layout->addWidget(connection_folder_);
  QString con_folder = kDefaultNameConnectionFolder;
  QString con_name = kDefaultSentinelNameConnection;

  if (sentinel_connection_) {
    proxy::connection_path_t path = sentinel_connection_->GetPath();
    common::ConvertFromString(path.GetName(), &con_name);
    common::ConvertFromString(path.GetDirectory(), &con_folder);
  }
  connection_name_->setText(con_name);
  connection_folder_->setText(con_folder);

  type_connection_ = new QComboBox;

  const auto updateCombobox = [this](core::ConnectionType type) {
    type_connection_->addItem(GuiFactory::GetInstance().icon(type), core::ConnectionTypeToString(type), type);
  };
#if defined(BUILD_WITH_REDIS)
  updateCombobox(core::REDIS);
#endif

  if (sentinel_connection_) {
    type_connection_->setCurrentIndex(sentinel_connection_->GetType());
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(type_connection_, static_cast<qind>(&QComboBox::currentIndexChanged), this,
                 &SentinelDialog::typeConnectionChange));

  QHBoxLayout* logging_layout = new QHBoxLayout;
  logging_ = new QCheckBox;
  logging_msec_ = new QSpinBox;
  logging_msec_->setRange(0, INT32_MAX);
  logging_msec_->setSingleStep(1000);

  if (sentinel_connection_) {
    logging_->setChecked(sentinel_connection_->IsHistoryEnabled());
    logging_msec_->setValue(sentinel_connection_->GetLoggingMsTimeInterval());
  } else {
    logging_->setChecked(false);
  }
  VERIFY(connect(logging_, &QCheckBox::stateChanged, this, &SentinelDialog::loggingStateChange));

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

  if (sentinel_connection_) {
    auto sentinels = sentinel_connection_->GetSentinels();
    for (const auto& sentinel : sentinels) {
      addSentinel(sentinel);
    }
  }

  VERIFY(connect(list_widget_, &QTreeWidget::itemSelectionChanged, this, &SentinelDialog::itemSelectionChanged));

  QSpacerItem* spacer = new QSpacerItem(300, 0, QSizePolicy::Expanding);
  QHBoxLayout* tool_bar_layout = new QHBoxLayout;
  tool_bar_layout->addWidget(createToolBar());
  tool_bar_layout->addSpacerItem(spacer);

  QVBoxLayout* input_layout = new QVBoxLayout;
  input_layout->addWidget(connection_name_);
  input_layout->addLayout(folder_layout);
  input_layout->addWidget(type_connection_);
  input_layout->addLayout(logging_layout);
  input_layout->addLayout(tool_bar_layout);
  input_layout->addWidget(list_widget_);

  test_button_ = new QPushButton("&Test");
  test_button_->setIcon(GuiFactory::GetInstance().messageBoxInformationIcon());
  VERIFY(connect(test_button_, &QPushButton::clicked, this, &SentinelDialog::testConnection));
  test_button_->setEnabled(false);

  discovery_button_ = new QPushButton("&Discovery");
  discovery_button_->setIcon(GuiFactory::GetInstance().discoveryIcon());
  VERIFY(connect(discovery_button_, &QPushButton::clicked, this, &SentinelDialog::discoverySentinel));

  QHBoxLayout* bottom_layout = new QHBoxLayout;
  bottom_layout->addWidget(test_button_, 0, Qt::AlignLeft);
  bottom_layout->addWidget(discovery_button_, 0, Qt::AlignLeft);
  button_box_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  button_box_->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box_, &QDialogButtonBox::accepted, this, &SentinelDialog::accept));
  VERIFY(connect(button_box_, &QDialogButtonBox::rejected, this, &SentinelDialog::reject));
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

proxy::ISentinelSettingsBaseSPtr SentinelDialog::connection() const {
  CHECK(sentinel_connection_);
  return sentinel_connection_;
}

void SentinelDialog::accept() {
  if (validateAndApply()) {
    base_class::accept();
  }
}

void SentinelDialog::typeConnectionChange(int index) {
  const QVariant var = type_connection_->itemData(index);
  const core::ConnectionType current_type = static_cast<core::ConnectionType>(qvariant_cast<unsigned char>(var));
  UNUSED(current_type);

  itemSelectionChanged();
}

void SentinelDialog::loggingStateChange(int value) {
  logging_msec_->setEnabled(value);
}

void SentinelDialog::testConnection() {
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!current_item) {
    return;
  }

  auto diag = createDialog<ConnectionDiagnosticDialog>(translations::trConnectionDiagnostic, current_item->connection(),
                                                       this);  // +
  diag->exec();
}

void SentinelDialog::discoverySentinel() {
  SentinelConnectionWidgetItem* sent_item =
      dynamic_cast<SentinelConnectionWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!sent_item) {
    return;
  }

  if (!validateAndApply()) {
    return;
  }

  auto diag = createDialog<DiscoverySentinelDiagnosticDialog>(
      translations::trConnectionDiscovery, GuiFactory::GetInstance().serverIcon(), sent_item->connection(), this);  // +
  int result = diag->exec();
  if (result != QDialog::Accepted) {
    return;
  }

  std::vector<ConnectionListWidgetItemDiscovered*> conns = diag->selectedConnections();
  for (size_t i = 0; i < conns.size(); ++i) {
    ConnectionListWidgetItemDiscovered* it = conns[i];

    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(sent_item);
    item->setConnection(it->connection());
    sent_item->addChild(item);
  }
}

void SentinelDialog::addConnectionSettings() {
#if defined(BUILD_WITH_REDIS)
  auto dlg = createDialog<ConnectionDialog>(core::REDIS, translations::trNewConnection, this);  // +
  dlg->setFolderEnabled(false);
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    proxy::IConnectionSettingsBaseSPtr sent_connection = dlg->connection();
    proxy::SentinelSettings sent;
    sent.sentinel = sent_connection;
    addSentinel(sent);
  }
#endif
}

void SentinelDialog::remove() {
  QTreeWidgetItem* current_list_item = list_widget_->currentItem();
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(current_list_item);  // +

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

void SentinelDialog::edit() {
  QTreeWidgetItem* current_list_item = list_widget_->currentItem();
  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(current_list_item);  // +

  // Do nothing if no item selected
  if (!current_item) {
    return;
  }

#if defined(BUILD_WITH_REDIS)
  const proxy::IConnectionSettingsBaseSPtr connection = current_item->connection();
  auto dlg = createDialog<ConnectionDialog>(connection->Clone(), this);  // +
  dlg->setFolderEnabled(false);
  int result = dlg->exec();
  if (result == QDialog::Accepted) {
    proxy::IConnectionSettingsBaseSPtr new_connection = dlg->connection();
    current_item->setConnection(new_connection);
  }
#endif
}

void SentinelDialog::itemSelectionChanged() {
  QTreeWidgetItem* current_list_item = list_widget_->currentItem();

  ConnectionListWidgetItem* current_item = dynamic_cast<ConnectionListWidgetItem*>(current_list_item);  // +
  bool is_valid_connection = current_item != nullptr;
  test_button_->setEnabled(is_valid_connection);

  SentinelConnectionWidgetItem* sent = dynamic_cast<SentinelConnectionWidgetItem*>(current_list_item);  // +
  bool is_valid_sent_connection = sent != nullptr;
  discovery_button_->setEnabled(is_valid_sent_connection);
}

void SentinelDialog::retranslateUi() {
  logging_->setText(translations::trLoggingEnabled);
  folder_label_->setText(translations::trFolder);
  add_action_->setToolTip(translations::trAddConnection);
  remove_action_->setToolTip(translations::trRemoveConnection);
  edit_action_->setToolTip(translations::trEditConnection);

  base_class::retranslateUi();
}

QToolBar* SentinelDialog::createToolBar() {
  QToolBar* savebar = new QToolBar;

  add_action_ = new QAction;
  add_action_->setIcon(GuiFactory::GetInstance().addIcon());
  VERIFY(connect(add_action_, &QAction::triggered, this, &SentinelDialog::addConnectionSettings));
  savebar->addAction(add_action_);

  remove_action_ = new QAction;
  remove_action_->setIcon(GuiFactory::GetInstance().removeIcon());
  VERIFY(connect(remove_action_, &QAction::triggered, this, &SentinelDialog::remove));
  savebar->addAction(remove_action_);

  edit_action_ = new QAction;
  edit_action_->setIcon(GuiFactory::GetInstance().editIcon());
  VERIFY(connect(edit_action_, &QAction::triggered, this, &SentinelDialog::edit));
  savebar->addAction(edit_action_);

  return savebar;
}

bool SentinelDialog::validateAndApply() {
  const QVariant var = type_connection_->currentData();
  const core::ConnectionType current_type = static_cast<core::ConnectionType>(qvariant_cast<unsigned char>(var));
  std::string connection_name = common::ConvertToString(connection_name_->text());
  if (connection_name.empty()) {
    connection_name = kDefaultSentinelNameConnection;
  }
  std::string connection_folder = common::ConvertToString(connection_folder_->text());
  if (connection_folder.empty()) {
    connection_folder = kDefaultNameConnectionFolder;
  }

  const proxy::connection_path_t path(common::file_system::stable_dir_path(connection_folder) + connection_name);
  proxy::ISentinelSettingsBase* new_connection =
      proxy::SentinelConnectionSettingsFactory::GetInstance().CreateFromTypeSentinel(current_type, path);
  if (logging_->isChecked()) {
    new_connection->SetLoggingMsTimeInterval(logging_msec_->value());
  }

  for (int i = 0; i < list_widget_->topLevelItemCount(); ++i) {
    SentinelConnectionWidgetItem* item =
        dynamic_cast<SentinelConnectionWidgetItem*>(list_widget_->topLevelItem(i));  // +
    if (!item) {
      continue;
    }

    proxy::SentinelSettings sent;
    sent.sentinel = item->connection();
    for (int i = 0; i < item->childCount(); ++i) {
      ConnectionListWidgetItem* child = dynamic_cast<ConnectionListWidgetItem*>(item->child(i));
      if (child) {
        sent.sentinel_nodes.push_back(child->connection());
      }
    }
    new_connection->AddSentinel(sent);
  }

  sentinel_connection_.reset(new_connection);
  return true;
}

void SentinelDialog::addSentinel(proxy::SentinelSettings sent) {
  SentinelConnectionWidgetItem* sent_item = new SentinelConnectionWidgetItem(core::ServerCommonInfo(), nullptr);
  sent_item->setConnection(sent.sentinel);
  for (const auto& node : sent.sentinel_nodes) {
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(sent_item);
    item->setConnection(node);
    sent_item->addChild(item);
  }
  list_widget_->addTopLevelItem(sent_item);
}

}  // namespace gui
}  // namespace fastonosql
