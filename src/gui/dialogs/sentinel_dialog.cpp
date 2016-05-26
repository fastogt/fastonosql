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

#include "gui/dialogs/sentinel_dialog.h"

#include <vector>
#include <string>

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
#include <QTreeWidget>
#include <QToolBar>

#include "common/qt/convert_string.h"

#include "translations/global.h"

#include "gui/gui_factory.h"
#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/dialogs/connection_dialog.h"
#include "gui/dialogs/discovery_sentinel_dialog.h"
#include "gui/dialogs/connection_listwidget_items.h"

namespace {
  const QString defaultNameConnection = "New Sentinel Connection";
  const char* defaultNameConnectionFolder = "/";
  const QString invalidDbType = QObject::tr("Invalid database type!");
}

namespace fastonosql {
namespace gui {

SentinelDialog::SentinelDialog(QWidget* parent, core::ISentinelSettingsBase* connection)
  : QDialog(parent), sentinel_connection_(connection) {
  setWindowIcon(GuiFactory::instance().serverIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  connectionName_ = new QLineEdit;
  connectionFolder_ = new QLineEdit;
  QRegExp rxf("^/[A-z0-9]+/$");
  connectionFolder_->setValidator(new QRegExpValidator(rxf, this));

  folderLabel_ = new QLabel;
  QHBoxLayout* folderLayout = new QHBoxLayout;
  folderLayout->addWidget(folderLabel_);
  folderLayout->addWidget(connectionFolder_);
  QString conFolder = defaultNameConnectionFolder;
  QString conName = defaultNameConnection;

  if (sentinel_connection_) {
    core::IConnectionSettings::connection_path_t path = sentinel_connection_->path();
    conName = common::convertFromString<QString>(path.name());
    conFolder = common::convertFromString<QString>(path.directory());
  }
  connectionName_->setText(conName);
  connectionFolder_->setText(conFolder);

  typeConnection_ = new QComboBox;

  for (size_t i = 0; i < SIZEOFMASS(core::connnectionType); ++i) {
    std::string str = core::connnectionType[i];
    core::connectionTypes ct = common::convertFromString<core::connectionTypes>(str);
    typeConnection_->addItem(GuiFactory::instance().icon(ct),
                             common::convertFromString<QString>(str), ct);
  }

  if (sentinel_connection_) {
    typeConnection_->setCurrentIndex(sentinel_connection_->type());
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(typeConnection_, static_cast<qind>(&QComboBox::currentIndexChanged),
                 this, &SentinelDialog::typeConnectionChange));

  QHBoxLayout* loggingLayout = new QHBoxLayout;
  logging_ = new QCheckBox;
  loggingMsec_ = new QSpinBox;
  loggingMsec_->setRange(0, INT32_MAX);
  loggingMsec_->setSingleStep(1000);

  if (sentinel_connection_) {
    logging_->setChecked(sentinel_connection_->loggingEnabled());
    loggingMsec_->setValue(sentinel_connection_->loggingMsTimeInterval());
  } else {
    logging_->setChecked(false);
  }
  VERIFY(connect(logging_, &QCheckBox::stateChanged, this, &SentinelDialog::loggingStateChange));

  loggingLayout->addWidget(logging_);
  loggingLayout->addWidget(loggingMsec_);

  listWidget_ = new QTreeWidget;
  listWidget_->setIndentation(5);

  QStringList colums;
  colums << translations::trName << translations::trAddress;
  listWidget_->setHeaderLabels(colums);
  listWidget_->setIndentation(15);
  listWidget_->setSelectionMode(QAbstractItemView::SingleSelection);  // single item can be draged or droped
  listWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);

  if (sentinel_connection_) {
    auto sent = sentinel_connection_->sentinels();
    for (auto it = sent.begin(); it != sent.end(); ++it) {
      core::SentinelSettings sent = (*it);
      addSentinel(sent);
    }
  }

  VERIFY(connect(listWidget_, &QTreeWidget::itemSelectionChanged,
                 this, &SentinelDialog::itemSelectionChanged));

  QHBoxLayout* toolBarLayout = new QHBoxLayout;
  savebar_ = new QToolBar;
  savebar_->setStyleSheet("QToolBar { border: 0px; }");
  toolBarLayout->addWidget(savebar_);

  QAction* addB = new QAction(GuiFactory::instance().loadIcon(),
                              translations::trAddConnection, savebar_);
  typedef void(QAction::*trig)(bool);
  VERIFY(connect(addB, static_cast<trig>(&QAction::triggered), this, &SentinelDialog::addConnectionSettings));
  savebar_->addAction(addB);

  QAction* rmB = new QAction(GuiFactory::instance().removeIcon(),
                             translations::trRemoveConnection, savebar_);
  VERIFY(connect(rmB, static_cast<trig>(&QAction::triggered), this, &SentinelDialog::remove));
  savebar_->addAction(rmB);

  QAction* editB = new QAction(GuiFactory::instance().editIcon(),
                               translations::trEditConnection, savebar_);
  VERIFY(connect(editB, static_cast<trig>(&QAction::triggered), this, &SentinelDialog::edit));
  savebar_->addAction(editB);

  QSpacerItem* hSpacer = new QSpacerItem(300, 0, QSizePolicy::Expanding);
  toolBarLayout->addSpacerItem(hSpacer);

  QVBoxLayout* inputLayout = new QVBoxLayout;
  inputLayout->addWidget(connectionName_);
  inputLayout->addLayout(folderLayout);
  inputLayout->addWidget(typeConnection_);
  inputLayout->addLayout(loggingLayout);
  inputLayout->addLayout(toolBarLayout);
  inputLayout->addWidget(listWidget_);

  testButton_ = new QPushButton("&Test");
  testButton_->setIcon(GuiFactory::instance().messageBoxInformationIcon());
  VERIFY(connect(testButton_, &QPushButton::clicked, this, &SentinelDialog::testConnection));
  testButton_->setEnabled(false);

  discoveryButton_ = new QPushButton("&Discovery");
  discoveryButton_->setIcon(GuiFactory::instance().discoveryIcon());
  VERIFY(connect(discoveryButton_, &QPushButton::clicked, this, &SentinelDialog::discoverySentinel));

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(testButton_, 0, Qt::AlignLeft);
  bottomLayout->addWidget(discoveryButton_, 0, Qt::AlignLeft);
  buttonBox_ = new QDialogButtonBox(this);
  buttonBox_->setOrientation(Qt::Horizontal);
  buttonBox_->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  VERIFY(connect(buttonBox_, &QDialogButtonBox::accepted, this, &SentinelDialog::accept));
  VERIFY(connect(buttonBox_, &QDialogButtonBox::rejected, this, &SentinelDialog::reject));
  bottomLayout->addWidget(buttonBox_);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addLayout(inputLayout);
  mainLayout->addLayout(bottomLayout);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(mainLayout);

  // update controls
  typeConnectionChange(typeConnection_->currentIndex());
  loggingStateChange(logging_->checkState());
  retranslateUi();
}

core::ISentinelSettingsBaseSPtr SentinelDialog::connection() const {
  return sentinel_connection_;
}

void SentinelDialog::accept() {
  if (validateAndApply()) {
    QDialog::accept();
  }
}

void SentinelDialog::typeConnectionChange(int index) {
  QVariant var = typeConnection_->itemData(index);
  core::connectionTypes currentType = (core::connectionTypes)qvariant_cast<unsigned char>(var);
  bool isValidType = currentType == core::REDIS;
  connectionName_->setEnabled(isValidType);
  buttonBox_->button(QDialogButtonBox::Save)->setEnabled(isValidType);
  savebar_->setEnabled(isValidType);
  listWidget_->selectionModel()->clear();
  listWidget_->setEnabled(isValidType);
  logging_->setEnabled(isValidType);
  itemSelectionChanged();
}

void SentinelDialog::loggingStateChange(int value) {
  loggingMsec_->setEnabled(value);
}

void SentinelDialog::testConnection() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  ConnectionDiagnosticDialog diag(this, currentItem->connection());
  diag.exec();
}

void SentinelDialog::discoverySentinel() {
  SentinelConnectionWidgetItem* sentItem = dynamic_cast<SentinelConnectionWidgetItem*>(listWidget_->currentItem());  // +

  // Do nothing if no item selected
  if (!sentItem) {
    return;
  }

  if (!validateAndApply()) {
    return;
  }

  DiscoverySentinelDiagnosticDialog diag(this, sentItem->connection());
  int result = diag.exec();
  if (result == QDialog::Accepted) {
    std::vector<ConnectionListWidgetItemDiscovered*> conns = diag.selectedConnections();
    for (size_t i = 0; i < conns.size(); ++i) {
      ConnectionListWidgetItemDiscovered* it = conns[i];

      ConnectionListWidgetItem* item = new ConnectionListWidgetItem(sentItem);
      item->setConnection(it->connection());
      sentItem->addChild(item);
    }
  }
}

void SentinelDialog::addConnectionSettings() {
#ifdef BUILD_WITH_REDIS
  static const std::vector<core::connectionTypes> avail = { core::REDIS };
  ConnectionDialog dlg(this, nullptr, avail);
  dlg.setFolderEnabled(false);
  int result = dlg.exec();
  core::SentinelSettings sent;
  sent.sentinel = dlg.connection();
  if (result == QDialog::Accepted && sent.sentinel) {
    addSentinel(sent);
  }
#endif
}

void SentinelDialog::remove() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  // Ask user
  int answer = QMessageBox::question(this, translations::trConnections, translations::trDeleteConnectionTemplate_1S.arg(currentItem->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes)
    return;

  delete currentItem;
}

void SentinelDialog::edit() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());  // +

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

#ifdef BUILD_WITH_REDIS
  core::IConnectionSettingsBaseSPtr oldConnection = currentItem->connection();
  static const std::vector<core::connectionTypes> avail = { core::REDIS };
  ConnectionDialog dlg(this, oldConnection->clone(), avail);
  dlg.setFolderEnabled(false);
  int result = dlg.exec();
  core::IConnectionSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    currentItem->setConnection(newConnection);
  }
#endif
}

void SentinelDialog::itemSelectionChanged() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());  // +
  bool isValidConnection = currentItem != nullptr;

  testButton_->setEnabled(isValidConnection);

  SentinelConnectionWidgetItem* sent = dynamic_cast<SentinelConnectionWidgetItem*>(listWidget_->currentItem());  // +
  bool isValidSentConnection = sent != nullptr;
  discoveryButton_->setEnabled(isValidSentConnection);
}

void SentinelDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QDialog::changeEvent(e);
}

void SentinelDialog::retranslateUi() {
  logging_->setText(translations::trLoggingEnabled);
  folderLabel_->setText(translations::trFolder);
}

bool SentinelDialog::validateAndApply() {
  QVariant var = typeConnection_->currentData();
  core::connectionTypes currentType = (core::connectionTypes)qvariant_cast<unsigned char>(var);
  std::string conName = common::convertToString(connectionName_->text());
  std::string conFolder = common::convertToString(connectionFolder_->text());
  if (conFolder.empty()) {
    conFolder = defaultNameConnectionFolder;
  }

  core::ISentinelSettingsBase::connection_path_t path(common::file_system::stable_dir_path(conFolder) + conName);
  core::ISentinelSettingsBase* newConnection = core::ISentinelSettingsBase::createFromType(currentType, path);
  if (logging_->isChecked()) {
    newConnection->setLoggingMsTimeInterval(loggingMsec_->value());
  }

  for (size_t i = 0; i < listWidget_->topLevelItemCount(); ++i) {
    SentinelConnectionWidgetItem* item = dynamic_cast<SentinelConnectionWidgetItem*>(listWidget_->topLevelItem(i));  // +
    CHECK(item);
    core::SentinelSettings sent;
    sent.sentinel = item->connection();
    for (int i = 0; i < item->childCount(); ++i) {
      ConnectionListWidgetItem* child = dynamic_cast<ConnectionListWidgetItem*>(item->child(i));
      CHECK(child);
      sent.sentinel_nodes.push_back(child->connection());
    }
    newConnection->addSentinel(sent);
  }

  sentinel_connection_.reset(newConnection);
  return true;
}

void SentinelDialog::addSentinel(core::SentinelSettings sent) {
  SentinelConnectionWidgetItem* sent_item = new SentinelConnectionWidgetItem(core::MASTER, nullptr);
  sent_item->setConnection(sent.sentinel);
  auto nodes = sent.sentinel_nodes;
  for (auto it = nodes.begin(); it != nodes.end(); ++it) {
    core::IConnectionSettingsBaseSPtr con = *it;
    ConnectionListWidgetItem* item = new ConnectionListWidgetItem(sent_item);
    item->setConnection(con);
    sent_item->addChild(item);
  }
  listWidget_->addTopLevelItem(sent_item);
}

}  // namespace gui
}  // namespace fastonosql
