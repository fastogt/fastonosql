/*  Copyright (C) 2014-2019 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/connection_dialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>

#include <common/qt/convert2string.h>

#include "proxy/connection_settings_factory.h"

#include "gui/connection_widgets_factory.h"
#include "gui/dialogs/connection_diagnostic_dialog.h"
#include "gui/gui_factory.h"

#include "translations/global.h"

namespace {
const QString trTest = QObject::tr("&Test");
const QString trTitle = QObject::tr("Connection Settings");
const QString trPrivateKeyInvalidInput = QObject::tr("Invalid private key value!");
const char* kDefaultNameConnectionFolder = "/";
}  // namespace

namespace fastonosql {
namespace gui {

ConnectionDialog::ConnectionDialog(core::ConnectionType type, const QString& connection_name, QWidget* parent)
    : base_class(trTitle, parent), connection_widget_(nullptr), test_button_(nullptr), connection_() {
  proxy::connection_path_t path(kDefaultNameConnectionFolder + common::ConvertToString(connection_name));
  proxy::IConnectionSettingsBase* connection =
      proxy::ConnectionSettingsFactory::GetInstance().CreateSettingsFromTypeConnection(type, path);
  init(connection);
}

ConnectionDialog::ConnectionDialog(proxy::IConnectionSettingsBase* connection, QWidget* parent)
    : base_class(trTitle, parent), connection_() {
  CHECK(connection);
  init(connection);
}

proxy::IConnectionSettingsBaseSPtr ConnectionDialog::connection() const {
  CHECK(connection_);
  return connection_;
}

void ConnectionDialog::setFolderEnabled(bool val) {
  connection_widget_->setUIFolderEnabled(val);
}

void ConnectionDialog::accept() {
  if (validateAndApply()) {
    base_class::accept();
  }
}

void ConnectionDialog::testConnection() {
  if (validateAndApply()) {
    auto diag = createDialog<ConnectionDiagnosticDialog>(translations::trConnectionDiagnostic, connection_, this);  // +
    diag->exec();
  }
}

void ConnectionDialog::init(proxy::IConnectionSettingsBase* connection) {
  setWindowIcon(GuiFactory::GetInstance().icon(connection->GetType()));

  connection_.reset(connection);
  connection_widget_ = ConnectionWidgetsFactory::GetInstance().createWidget(connection);
  QLayout* connection_widget_layout = connection_widget_->layout();
  QMargins mar = connection_widget_layout->contentsMargins();
  connection_widget_layout->setContentsMargins(0, 0, 0, mar.bottom());

  QHBoxLayout* bottom_layout = new QHBoxLayout;
  test_button_ = new QPushButton;
  test_button_->setIcon(GuiFactory::GetInstance().messageBoxInformationIcon());
  VERIFY(connect(test_button_, &QPushButton::clicked, this, &ConnectionDialog::testConnection));

  bottom_layout->addWidget(test_button_, 1, Qt::AlignLeft);
  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &ConnectionDialog::accept));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &ConnectionDialog::reject));
  bottom_layout->addWidget(button_box);

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(connection_widget_);
  main_layout->addLayout(bottom_layout);
  main_layout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(main_layout);
}

void ConnectionDialog::retranslateUi() {
  test_button_->setText(trTest);
  base_class::retranslateUi();
}

bool ConnectionDialog::validateAndApply() {
  if (!connection_widget_->validated()) {
    return false;
  }

  connection_.reset(connection_widget_->createConnection());
  return true;
}

}  // namespace gui
}  // namespace fastonosql
