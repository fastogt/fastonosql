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

#include <memory>                       // for __shared_ptr

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QIcon>
#include <QThread>

#include "common/macros.h"              // for VERIFY

#include "fasto/qt/gui/glass_widget.h"  // for GlassWidget
#include "gui/dialogs/test_connection.h"  // for TestConnection
#include "gui/gui_factory.h"            // for GuiFactory

#include "translations/global.h"

namespace {
  const QSize stateIconSize = QSize(64, 64);
}

namespace fastonosql {
namespace gui {

ConnectionDiagnosticDialog::ConnectionDiagnosticDialog(QWidget* parent,
                                                       core::IConnectionSettingsBaseSPtr connection)
  : QDialog(parent) {
  setWindowTitle(translations::trConnectionDiagnostic);
  setWindowIcon(GuiFactory::instance().icon(connection->type()));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  QVBoxLayout* mainLayout = new QVBoxLayout;

  executeTimeLabel_ = new QLabel;
  executeTimeLabel_->setText(translations::trConnectionStatusTemplate_1S.arg("execute..."));
  mainLayout->addWidget(executeTimeLabel_);

  statusLabel_ = new QLabel(translations::trTimeTemplate_1S.arg("calculate..."));
  statusLabel_->setWordWrap(true);
  iconLabel_ = new QLabel;
  QIcon icon = GuiFactory::instance().failIcon();
  const QPixmap pm = icon.pixmap(stateIconSize);
  iconLabel_->setPixmap(pm);

  mainLayout->addWidget(statusLabel_);
  mainLayout->addWidget(iconLabel_, 1, Qt::AlignCenter);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted,
                 this, &ConnectionDiagnosticDialog::accept));

  mainLayout->addWidget(buttonBox);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(mainLayout);

  glassWidget_ = new fasto::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(),
                                                 translations::trTryToConnect, 0.5,
                                                 QColor(111, 111, 100), this);
  testConnection(connection);
}

void ConnectionDiagnosticDialog::connectionResult(bool suc,
                                                  qint64 mstimeExecute, const QString& resultText) {
  glassWidget_->stop();

  executeTimeLabel_->setText(translations::trTimeTemplate_1S.arg(mstimeExecute));
  if (suc) {
    QIcon icon = GuiFactory::instance().successIcon();
    QPixmap pm = icon.pixmap(stateIconSize);
    iconLabel_->setPixmap(pm);
  }
  statusLabel_->setText(translations::trConnectionStatusTemplate_1S.arg(resultText));
}

void ConnectionDiagnosticDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  glassWidget_->start();
}

void ConnectionDiagnosticDialog::testConnection(core::IConnectionSettingsBaseSPtr connection) {
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

}  // namespace gui
}  // namespace fastonosql
