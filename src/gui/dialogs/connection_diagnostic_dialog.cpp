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

#include "gui/dialogs/connection_diagnostic_dialog.h"

#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QThread>
#include <QVBoxLayout>

#include <common/qt/gui/glass_widget.h>  // for GlassWidget

#include "gui/dialogs/test_connection.h"  // for TestConnection
#include "gui/gui_factory.h"              // for GuiFactory

#include "translations/global.h"

namespace {
const QSize stateIconSize = QSize(64, 64);
}

namespace fastonosql {
namespace gui {

ConnectionDiagnosticDialog::ConnectionDiagnosticDialog(QWidget* parent, proxy::IConnectionSettingsBaseSPtr connection)
    : QDialog(parent) {
  setWindowTitle(translations::trConnectionDiagnostic);
  setWindowIcon(GuiFactory::GetInstance().GetIcon(connection->GetType()));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  QVBoxLayout* mainLayout = new QVBoxLayout;

  execute_time_label_ = new QLabel;
  execute_time_label_->setText(translations::trConnectionStatusTemplate_1S.arg("execute..."));
  mainLayout->addWidget(execute_time_label_);

  status_label_ = new QLabel(translations::trTimeTemplate_1S.arg("calculate..."));
  status_label_->setWordWrap(true);
  icon_label_ = new QLabel;
  QIcon icon = GuiFactory::GetInstance().GetFailIcon();
  const QPixmap pm = icon.pixmap(stateIconSize);
  icon_label_->setPixmap(pm);

  mainLayout->addWidget(status_label_);
  mainLayout->addWidget(icon_label_, 1, Qt::AlignCenter);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectionDiagnosticDialog::accept));

  mainLayout->addWidget(buttonBox);
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(mainLayout);

  glass_widget_ = new common::qt::gui::GlassWidget(GuiFactory::GetInstance().GetPathToLoadingGif(),
                                                   translations::trTryToConnect, 0.5, QColor(111, 111, 100), this);
  startTestConnection(connection);
}

void ConnectionDiagnosticDialog::connectionResult(bool suc, qint64 mstimeExecute, const QString& resultText) {
  glass_widget_->stop();

  execute_time_label_->setText(translations::trTimeTemplate_1S.arg(mstimeExecute));
  if (suc) {
    QIcon icon = GuiFactory::GetInstance().GetSuccessIcon();
    QPixmap pm = icon.pixmap(stateIconSize);
    icon_label_->setPixmap(pm);
  }
  status_label_->setText(translations::trConnectionStatusTemplate_1S.arg(resultText));
}

void ConnectionDiagnosticDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  glass_widget_->start();
}

void ConnectionDiagnosticDialog::startTestConnection(proxy::IConnectionSettingsBaseSPtr connection) {
  QThread* th = new QThread;
  TestConnection* cheker = new TestConnection(connection);
  cheker->moveToThread(th);
  VERIFY(connect(th, &QThread::started, cheker, &TestConnection::routine));
  VERIFY(connect(cheker, &TestConnection::connectionResult, this, &ConnectionDiagnosticDialog::connectionResult));
  VERIFY(connect(cheker, &TestConnection::connectionResult, th, &QThread::quit));
  VERIFY(connect(th, &QThread::finished, cheker, &TestConnection::deleteLater));
  VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
  th->start();
}

}  // namespace gui
}  // namespace fastonosql
