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

#include "gui/gui_factory.h"              // for GuiFactory
#include "gui/workers/test_connection.h"  // for TestConnection

#include "translations/global.h"

namespace {
const QSize kStateIconSize = QSize(64, 64);
}

namespace fastonosql {
namespace gui {

ConnectionDiagnosticDialog::ConnectionDiagnosticDialog(const QString& title,
                                                       proxy::IConnectionSettingsBaseSPtr connection,
                                                       QWidget* parent)
    : base_class(title, parent),
      glass_widget_(nullptr),
      execute_time_label_(nullptr),
      status_label_(nullptr),
      icon_label_(nullptr) {
  setWindowIcon(GuiFactory::GetInstance().icon(connection->GetType()));

  execute_time_label_ = new QLabel;
  execute_time_label_->setText(translations::trConnectionStatusTemplate_1S.arg("execute..."));

  status_label_ = new QLabel(translations::trTimeTemplate_1S.arg("calculate..."));
  status_label_->setWordWrap(true);
  icon_label_ = new QLabel;
  QIcon icon = GuiFactory::GetInstance().failIcon();
  const QPixmap pm = icon.pixmap(kStateIconSize);
  icon_label_->setPixmap(pm);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &ConnectionDiagnosticDialog::accept));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(execute_time_label_);
  main_layout->addWidget(status_label_);
  main_layout->addWidget(icon_label_, 1, Qt::AlignCenter);
  main_layout->addWidget(button_box);
  main_layout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(main_layout);

  glass_widget_ =
      new common::qt::gui::GlassWidget(GuiFactory::GetInstance().pathToLoadingGif(),
                                       translations::trTryToConnect + "...", 0.5, QColor(111, 111, 100), this);
  startTestConnection(connection);
}

void ConnectionDiagnosticDialog::connectionResult(bool suc, qint64 exec_mstime, const QString& result_text) {
  glass_widget_->stop();

  execute_time_label_->setText(translations::trTimeTemplate_1S.arg(exec_mstime));
  if (suc) {
    QIcon icon = GuiFactory::GetInstance().successIcon();
    QPixmap pm = icon.pixmap(kStateIconSize);
    icon_label_->setPixmap(pm);
  }
  status_label_->setText(translations::trConnectionStatusTemplate_1S.arg(result_text));
}

void ConnectionDiagnosticDialog::showEvent(QShowEvent* e) {
  base_class::showEvent(e);
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
