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

#include "gui/dialogs/connection_diagnostic_dialog.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QThread>
#include <QLabel>

#include "common/time.h"

#include "core/servers_manager.h"

#include "gui/gui_factory.h"
#include "fasto/qt/gui/glass_widget.h"

#include "translations/global.h"

namespace {
  const QString timeTemplate = "Time execute msec: %1";
  const QString connectionStatusTemplate = "Connection state: %1";
  const QSize stateIconSize = QSize(64, 64);
}

namespace fastonosql {
TestConnection::TestConnection(IConnectionSettingsBaseSPtr conn, QObject* parent)
  : QObject(parent), connection_(conn), startTime_(common::time::current_mstime()) {
}

void TestConnection::routine() {
  if (!connection_) {
    emit connectionResult(false, common::time::current_mstime() - startTime_,
                          "Invalid connection settings");
    return;
  }

  common::Error er = ServersManager::instance().testConnection(connection_);

  if (er && er->isError()) {
    emit connectionResult(false, common::time::current_mstime() - startTime_,
                          common::convertFromString<QString>(er->description()));
  } else {
    emit connectionResult(true, common::time::current_mstime() - startTime_, "Success");
  }
}

ConnectionDiagnosticDialog::ConnectionDiagnosticDialog(QWidget* parent,
                                                       IConnectionSettingsBaseSPtr connection)
  : QDialog(parent) {
  setWindowTitle(translations::trConnectionDiagnostic);
  setWindowIcon(GuiFactory::instance().icon(connection->type()));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  QVBoxLayout* mainLayout = new QVBoxLayout;

  executeTimeLabel_ = new QLabel;
  executeTimeLabel_->setText(connectionStatusTemplate.arg("execute..."));
  mainLayout->addWidget(executeTimeLabel_);

  statusLabel_ = new QLabel(timeTemplate.arg("calculate..."));
  statusLabel_->setWordWrap(true);
  iconLabel_ = new QLabel;
  QIcon icon = GuiFactory::instance().failIcon();
  const QPixmap pm = icon.pixmap(stateIconSize);
  iconLabel_->setPixmap(pm);

  mainLayout->addWidget(statusLabel_);
  mainLayout->addWidget(iconLabel_, 1, Qt::AlignCenter);

  QDialogButtonBox* buttonBox = new QDialogButtonBox;
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Ok);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted,
                 this, &ConnectionDiagnosticDialog::accept));

  mainLayout->addWidget(buttonBox);
  setFixedSize(QSize(fix_width, fix_height));
  setLayout(mainLayout);

  glassWidget_ = new fasto::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(),
                                                 translations::trTryToConnect, 0.5,
                                                 QColor(111, 111, 100), this);
  testConnection(connection);
}

void ConnectionDiagnosticDialog::connectionResult(bool suc,
                                                  qint64 mstimeExecute, const QString& resultText) {
  glassWidget_->stop();

  executeTimeLabel_->setText(timeTemplate.arg(mstimeExecute));
  if (suc) {
    QIcon icon = GuiFactory::instance().successIcon();
    const QPixmap pm = icon.pixmap(stateIconSize);
    iconLabel_->setPixmap(pm);
  }
  statusLabel_->setText(connectionStatusTemplate.arg(resultText));
}

void ConnectionDiagnosticDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  glassWidget_->start();
}

void ConnectionDiagnosticDialog::testConnection(IConnectionSettingsBaseSPtr connection) {
  QThread* th = new QThread;
  TestConnection* cheker = new TestConnection(connection);
  cheker->moveToThread(th);
  VERIFY(connect(th, &QThread::started, cheker, &TestConnection::routine));
  VERIFY(connect(cheker, &TestConnection::connectionResult,
                 this, &ConnectionDiagnosticDialog::connectionResult));
  VERIFY(connect(cheker, &TestConnection::connectionResult, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, cheker, &TestConnection::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

}  // namespace fastonosql
