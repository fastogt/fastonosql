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

#include "gui/dialogs/cluster_dialog.h"

#include <vector>
#include <string>

#include <QDialogButtonBox>
#include <QEvent>
#include <QMenu>
#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QTreeWidget>
#include <QToolBar>
#include <QAction>
#include <QCheckBox>
#include <QSpinBox>

#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/dialogs/connection_dialog.h"
#include "gui/dialogs/discovery_dialog.h"
#include "gui/dialogs/connection_listwidget_items.h"

#include "common/qt/convert_string.h"

#include "gui/gui_factory.h"

#include "translations/global.h"

namespace {
  const QString defaultNameConnection = "New Cluster Connection";
  const QString invalidDbType = QObject::tr("Invalid database type!");
}

namespace fastonosql {

ClusterDialog::ClusterDialog(QWidget* parent, IClusterSettingsBase* connection)
  : QDialog(parent), cluster_connection_(connection) {
  setWindowIcon(GuiFactory::instance().serverIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  connectionName_ = new QLineEdit;
  QString conName = defaultNameConnection;
  if (cluster_connection_) {
    conName = common::convertFromString<QString>(cluster_connection_->name());
  }
  connectionName_->setText(conName);
  typeConnection_ = new QComboBox;

  for (int i = 0; i < SIZEOFMASS(connnectionType); ++i) {
    connectionTypes ct = static_cast<connectionTypes>(i);
    std::string str = common::convertToString(ct);
    typeConnection_->addItem(GuiFactory::instance().icon(ct),
                             common::convertFromString<QString>(str), i);
  }

  if (cluster_connection_) {
    typeConnection_->setCurrentIndex(cluster_connection_->type());
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(typeConnection_, static_cast<qind>(&QComboBox::currentIndexChanged),
                 this, &ClusterDialog::typeConnectionChange));

  QHBoxLayout* loggingLayout = new QHBoxLayout;
  logging_ = new QCheckBox;
  loggingMsec_ = new QSpinBox;
  loggingMsec_->setRange(0, INT32_MAX);
  loggingMsec_->setSingleStep(1000);

  if (cluster_connection_) {
      logging_->setChecked(cluster_connection_->loggingEnabled());
      loggingMsec_->setValue(cluster_connection_->loggingMsTimeInterval());
  } else {
      logging_->setChecked(false);
  }
  VERIFY(connect(logging_, &QCheckBox::stateChanged, this, &ClusterDialog::loggingStateChange));

  loggingLayout->addLayout(loggingLayout);
  loggingLayout->addWidget(loggingMsec_);

  listWidget_ = new QTreeWidget;
  listWidget_->setIndentation(5);

  QStringList colums;
  colums << translations::trName << translations::trAddress;
  listWidget_->setHeaderLabels(colums);
  listWidget_->setIndentation(15);
  listWidget_->setSelectionMode(QAbstractItemView::SingleSelection);  // single item can be draged or droped
  listWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);

  listWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
  VERIFY(connect(listWidget_, &QTreeWidget::customContextMenuRequested,
                 this, &ClusterDialog::showContextMenu));

  setDefault_ = new QAction(this);
  VERIFY(connect(setDefault_, &QAction::triggered, this, &ClusterDialog::setStartNode));

  if (cluster_connection_) {
    IClusterSettingsBase::cluster_connection_type clusters = cluster_connection_->nodes();
    for (IClusterSettingsBase::cluster_connection_type::const_iterator it = clusters.begin();
        it != clusters.end(); ++it) {
      IConnectionSettingsBaseSPtr serv = (*it);
      addConnection(serv);
    }
  }

  VERIFY(connect(listWidget_, &QTreeWidget::itemSelectionChanged,
                 this, &ClusterDialog::itemSelectionChanged));

  QHBoxLayout* toolBarLayout = new QHBoxLayout;
  savebar_ = new QToolBar;
  savebar_->setStyleSheet("QToolBar { border: 0px; }");
  toolBarLayout->addWidget(savebar_);

  QAction* addB = new QAction(GuiFactory::instance().loadIcon(),
                              translations::trAddConnection, savebar_);
  typedef void(QAction::*trig)(bool);
  VERIFY(connect(addB, static_cast<trig>(&QAction::triggered), this, &ClusterDialog::add));
  savebar_->addAction(addB);

  QAction* rmB = new QAction(GuiFactory::instance().removeIcon(),
                             translations::trRemoveConnection, savebar_);
  VERIFY(connect(rmB, static_cast<trig>(&QAction::triggered), this, &ClusterDialog::remove));
  savebar_->addAction(rmB);

  QAction* editB = new QAction(GuiFactory::instance().editIcon(),
                               translations::trEditConnection, savebar_);
  VERIFY(connect(editB, static_cast<trig>(&QAction::triggered), this, &ClusterDialog::edit));
  savebar_->addAction(editB);

  QSpacerItem* hSpacer = new QSpacerItem(300, 0, QSizePolicy::Expanding);
  toolBarLayout->addSpacerItem(hSpacer);

  QVBoxLayout* inputLayout = new QVBoxLayout;
  inputLayout->addWidget(connectionName_);
  inputLayout->addWidget(typeConnection_);
  inputLayout->addWidget(logging_);
  inputLayout->addLayout(toolBarLayout);
  inputLayout->addWidget(listWidget_);

  testButton_ = new QPushButton("&Test");
  testButton_->setIcon(GuiFactory::instance().messageBoxInformationIcon());
  VERIFY(connect(testButton_, &QPushButton::clicked, this, &ClusterDialog::testConnection));
  testButton_->setEnabled(false);

  discoveryButton_ = new QPushButton("&Discovery");
  discoveryButton_->setIcon(GuiFactory::instance().discoveryIcon());
  VERIFY(connect(discoveryButton_, &QPushButton::clicked, this, &ClusterDialog::discoveryCluster));
  discoveryButton_->setEnabled(false);

  QHBoxLayout* bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(testButton_, 0, Qt::AlignLeft);
  bottomLayout->addWidget(discoveryButton_, 0, Qt::AlignLeft);
  buttonBox_ = new QDialogButtonBox(this);
  buttonBox_->setOrientation(Qt::Horizontal);
  buttonBox_->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  VERIFY(connect(buttonBox_, &QDialogButtonBox::accepted, this, &ClusterDialog::accept));
  VERIFY(connect(buttonBox_, &QDialogButtonBox::rejected, this, &ClusterDialog::reject));
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

IClusterSettingsBaseSPtr ClusterDialog::connection() const {
  return cluster_connection_;
}

void ClusterDialog::accept() {
  if (validateAndApply()) {
    QDialog::accept();
  }
}

void ClusterDialog::typeConnectionChange(int index) {
  QVariant var = typeConnection_->itemData(index);
  connectionTypes currentType = (connectionTypes)qvariant_cast<unsigned char>(var);
  bool isValidType = currentType == REDIS;
  connectionName_->setEnabled(isValidType);
  buttonBox_->button(QDialogButtonBox::Save)->setEnabled(isValidType);
  savebar_->setEnabled(isValidType);
  listWidget_->selectionModel()->clear();
  listWidget_->setEnabled(isValidType);
  logging_->setEnabled(isValidType);
  itemSelectionChanged();
}

void ClusterDialog::loggingStateChange(int value) {
  loggingMsec_->setEnabled(value);
}

void ClusterDialog::testConnection() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());

  // Do nothing if no item selected
  if (!currentItem)
    return;

  ConnectionDiagnosticDialog diag(this, currentItem->connection());
  diag.exec();
}

void ClusterDialog::discoveryCluster() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  if (!validateAndApply()) {
    return;
  }

  DiscoveryDiagnosticDialog diag(this, currentItem->connection(), cluster_connection_);
  int result = diag.exec();
  if (result == QDialog::Accepted) {
    std::vector<IConnectionSettingsBaseSPtr> conns = diag.selectedConnections();
    for (size_t i = 0; i < conns.size(); ++i) {
      addConnection(conns[i]);
    }
  }
}

void ClusterDialog::showContextMenu(const QPoint& point) {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());

  // Do nothing if no item selected
  if (!currentItem) {
      return;
  }

  QMenu menu(this);
  bool isPrimary = listWidget_->topLevelItem(0) == currentItem;
  setDefault_->setEnabled(!isPrimary);
  menu.addAction(setDefault_);

  QPoint menuPoint = listWidget_->mapToGlobal(point);
  menu.exec(menuPoint);
}

void ClusterDialog::setStartNode() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  ConnectionListWidgetItem* top = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->topLevelItem(0));
  if (top == currentItem) {
    return;
  }

  IConnectionSettingsBaseSPtr tc = top->connection();
  IConnectionSettingsBaseSPtr cc = currentItem->connection();
  currentItem->setConnection(tc);
  top->setConnection(cc);
}

void ClusterDialog::add() {
  const std::vector<connectionTypes> avail = { DBUNKNOWN, REDIS };
  ConnectionDialog dlg(this, nullptr, avail);
  int result = dlg.exec();
  IConnectionSettingsBaseSPtr p = dlg.connection();
  if (result == QDialog::Accepted && p) {
    addConnection(p);
  }
}

void ClusterDialog::remove() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());

  // Do nothing if no item selected
  if (!currentItem)
    return;

  // Ask user
  int answer = QMessageBox::question(this, "Connections", QString("Really delete \"%1\" connection?").arg(currentItem->text(0)),
                                     QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

  if (answer != QMessageBox::Yes)
    return;

  delete currentItem;
}

void ClusterDialog::edit() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());

  // Do nothing if no item selected
  if (!currentItem) {
    return;
  }

  IConnectionSettingsBaseSPtr oldConnection = currentItem->connection();

  static const std::vector<connectionTypes> avail = { DBUNKNOWN, REDIS };
  ConnectionDialog dlg(this, oldConnection->clone(), avail);
  int result = dlg.exec();
  IConnectionSettingsBaseSPtr newConnection = dlg.connection();
  if (result == QDialog::Accepted && newConnection) {
    currentItem->setConnection(newConnection);
  }
}

void ClusterDialog::itemSelectionChanged() {
  ConnectionListWidgetItem* currentItem = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->currentItem());

  bool isValidConnection = currentItem != nullptr;

  testButton_->setEnabled(isValidConnection);
  discoveryButton_->setEnabled(isValidConnection);
}

void ClusterDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QDialog::changeEvent(e);
}

void ClusterDialog::retranslateUi() {
  logging_->setText(tr("Logging enabled"));
  setDefault_->setText(translations::trSetAsStartNode);
}

bool ClusterDialog::validateAndApply() {
  connectionTypes currentType = common::convertFromString<connectionTypes>(common::convertToString(typeConnection_->currentText()));
  bool isValidType = currentType != DBUNKNOWN;
  if (isValidType) {
    std::string conName = common::convertToString(connectionName_->text());
    IClusterSettingsBase* newConnection = IClusterSettingsBase::createFromType(currentType, conName);
    if (newConnection) {
      cluster_connection_.reset(newConnection);
      if (logging_->isChecked()) {
        cluster_connection_->setLoggingMsTimeInterval(loggingMsec_->value());
      }
      for (size_t i = 0; i < listWidget_->topLevelItemCount(); ++i) {
        ConnectionListWidgetItem* item = dynamic_cast<ConnectionListWidgetItem*>(listWidget_->topLevelItem(i));
        if (item) {
          IConnectionSettingsBaseSPtr con = item->connection();
          cluster_connection_->addNode(con);
        }
      }
    }
    return true;
  } else {
    QMessageBox::critical(this, translations::trError, invalidDbType);
    return false;
  }
}

void ClusterDialog::addConnection(IConnectionSettingsBaseSPtr con) {
  ConnectionListWidgetItem* item = new ConnectionListWidgetItem(con);
  listWidget_->addTopLevelItem(item);
}

}  // namespace fastonosql
