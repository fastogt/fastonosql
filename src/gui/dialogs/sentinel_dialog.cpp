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

#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/dialogs/connection_dialog.h"  // for ConnectionDialog
#include "gui/dialogs/connection_listwidget_items.h"
#include "gui/dialogs/discovery_sentinel_dialog.h"
#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trAddConnection, trAddress, etc

namespace {
const QString defaultNameConnection = QObject::tr("New Sentinel Connection");
const char* defaultNameConnectionFolder = "/";
}  // namespace

namespace fastonosql {
namespace gui {

SentinelDialog::SentinelDialog(QWidget* parent, proxy::ISentinelSettingsBase* connection)
    : QDialog(parent), sentinel_connection_(connection) {
  setWindowIcon(GuiFactory::GetInstance().GetServerIcon());
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

  if (sentinel_connection_) {
    proxy::connection_path_t path = sentinel_connection_->GetPath();
    common::ConvertFromString(path.GetName(), &conName);
    common::ConvertFromString(path.GetDirectory(), &conFolder);
  }
  connection_name_->setText(conName);
  connection_folder_->setText(conFolder);

  type_connection_ = new QComboBox;

  for (size_t i = 0; i < core::g_compiled_types.size(); ++i) {
    core::connectionTypes ct = core::g_compiled_types[i];
    std::string str = common::ConvertToString(ct);
    QString qstr;
    if (common::ConvertFromString(str, &qstr)) {
      type_connection_->addItem(GuiFactory::GetInstance().GetIcon(ct), qstr, ct);
    }
  }

  if (sentinel_connection_) {
    type_connection_->setCurrentIndex(sentinel_connection_->GetType());
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(type_connection_, static_cast<qind>(&QComboBox::currentIndexChanged), this,
                 &SentinelDialog::typeConnectionChange));

  QHBoxLayout* loggingLayout = new QHBoxLayout;
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

  if (sentinel_connection_) {
    auto sentinels = sentinel_connection_->GetSentinels();
    for (const auto& sentinel : sentinels) {
      addSentinel(sentinel);
    }
  }

  VERIFY(connect(list_widget_, &QTreeWidget::itemSelectionChanged, this, &SentinelDialog::itemSelectionChanged));

  QHBoxLayout* toolBarLayout = new QHBoxLayout;
  savebar_ = new QToolBar;
  toolBarLayout->addWidget(savebar_);

  QAction* addB = new QAction(GuiFactory::GetInstance().GetAddIcon(), translations::trAddConnection, savebar_);
  typedef void (QAction::*trig)(bool);
  VERIFY(connect(addB, static_cast<trig>(&QAction::triggered), this, &SentinelDialog::addConnectionSettings));
  savebar_->addAction(addB);

  QAction* rmB = new QAction(GuiFactory::GetInstance().GetRemoveIcon(), translations::trRemoveConnection, savebar_);
  VERIFY(connect(rmB, static_cast<trig>(&QAction::triggered), this, &SentinelDialog::remove));
  savebar_->addAction(rmB);

  QAction* editB = new QAction(GuiFactory::GetInstance().GetEditIcon(), translations::trEditConnection, savebar_);
  VERIFY(connect(editB, static_cast<trig>(&QAction::triggered), this, &SentinelDialog::edit));
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
  test_button_->setIcon(GuiFactory::GetInstance().GetMessageBoxInformationIcon());
  VERIFY(connect(test_button_, &QPushButton::clicked, this, &SentinelDialog::testConnection));
  test_button_->setEnabled(false);

  discovery_button_ = new QPushButton("&Discovery");
  discovery_button_->setIcon(GuiFactory::GetInstance().GetDiscoveryIcon());
  VERIFY(connect(discovery_button_, &QPushButton::clicked, this, &SentinelDialog::discoverySentinel));

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(test_button_, 0, Qt::AlignLeft);
  bottomLayout->addWidget(discovery_button_, 0, Qt::AlignLeft);
  button_box_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  button_box_->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box_, &QDialogButtonBox::accepted, this, &SentinelDialog::accept));
  VERIFY(connect(button_box_, &QDialogButtonBox::rejected, this, &SentinelDialog::reject));
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

proxy::ISentinelSettingsBaseSPtr SentinelDialog::connection() const {
  return sentinel_connection_;
}

void SentinelDialog::accept() {
  if (validateAndApply()) {
    QDialog::accept();
  }
}

void SentinelDialog::typeConnectionChange(int index) {
  QVariant var = type_connection_->itemData(index);
  core::connectionTypes currentType = static_cast<core::connectionTypes>(qvariant_cast<unsigned char>(var));
  bool isValidType = currentType == core::REDIS;
  connection_name_->setEnabled(isValidType);
  button_box_->button(QDialogButtonBox::Save)->setEnabled(isValidType);
  savebar_->setEnabled(isValidType);
  list_widget_->selectionModel()->clear();
  list_widget_->setEnabled(isValidType);
  logging_->setEnabled(isValidType);
  itemSelectionChanged();
}

void SentinelDialog::loggingStateChange(int value) {
  logging_msec_->setEnabled(value);
}

void SentinelDialog::testConnection() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  ConnectionDiagnosticDialog diag(this, currentItem->connection());
  diag.exec();
}

void SentinelDialog::discoverySentinel() {
  SentinelConnectionWidgetItem* sentItem =
      dynamic_cast<SentinelConnectionWidgetItem*>(list_widget_->currentItem());  // +

  // Do nothing if no item selected
  if (!sentItem) {
    return;
  }

  if (!validateAndApply()) {
    return;
  }

  DiscoverySentinelDiagnosticDialog diag(this, sentItem->connection());
  int result = diag.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  std::vector<ConnectionListWidgetItemDiscovered*> conns = diag.selectedConnections();
  for (size_t i = 0; i < conns.size(); ++i) {
    ConnectionListWidgetItemDiscovered* it = conns[i];

    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(sentItem);
    item->setConnection(it->connection());
    sentItem->addChild(item);
  }
}

void SentinelDialog::addConnectionSettings() {
#ifdef BUILD_WITH_REDIS
  ConnectionDialog dlg(core::REDIS, translations::trNewConnection, this);
  dlg.setFolderEnabled(false);
  int result = dlg.exec();
  proxy::SentinelSettings sent;
  sent.sentinel = dlg.connection();
  if (result == QDialog::Accepted && sent.sentinel) {
    addSentinel(sent);
  }
#endif
}

void SentinelDialog::remove() {
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

void SentinelDialog::edit() {
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

void SentinelDialog::itemSelectionChanged() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(list_widget_->currentItem());  // +
  bool isValidConnection = currentItem != nullptr;

  test_button_->setEnabled(isValidConnection);

  SentinelConnectionWidgetItem* sent = dynamic_cast<SentinelConnectionWidgetItem*>(list_widget_->currentItem());  // +
  bool isValidSentConnection = sent != nullptr;
  discovery_button_->setEnabled(isValidSentConnection);
}

void SentinelDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QDialog::changeEvent(e);
}

void SentinelDialog::retranslateUi() {
  logging_->setText(translations::trLoggingEnabled);
  folder_label_->setText(translations::trFolder);
}

bool SentinelDialog::validateAndApply() {
  QVariant var = type_connection_->currentData();
  core::connectionTypes currentType = static_cast<core::connectionTypes>(qvariant_cast<unsigned char>(var));
  std::string conName = common::ConvertToString(connection_name_->text());
  std::string conFolder = common::ConvertToString(connection_folder_->text());
  if (conFolder.empty()) {
    conFolder = defaultNameConnectionFolder;
  }

  proxy::connection_path_t path(common::file_system::stable_dir_path(conFolder) + conName);
  proxy::ISentinelSettingsBase* newConnection =
      proxy::SentinelConnectionSettingsFactory::GetInstance().CreateFromType(currentType, path);
  if (logging_->isChecked()) {
    newConnection->SetLoggingMsTimeInterval(logging_msec_->value());
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
    newConnection->AddSentinel(sent);
  }

  sentinel_connection_.reset(newConnection);
  return true;
}

void SentinelDialog::addSentinel(proxy::SentinelSettings sent) {
  SentinelConnectionWidgetItem* sent_item = new SentinelConnectionWidgetItem(core::ServerCommonInfo(), nullptr);
  sent_item->setConnection(sent.sentinel);
  auto nodes = sent.sentinel_nodes;
  for (const auto& node : nodes) {
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(sent_item);
    item->setConnection(node);
    sent_item->addChild(item);
  }
  list_widget_->addTopLevelItem(sent_item);
}

}  // namespace gui
}  // namespace fastonosql
