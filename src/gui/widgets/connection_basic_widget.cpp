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

#include "gui/widgets/connection_basic_widget.h"

#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <common/macros.h>
#include <common/convert2string.h>

#include "gui/gui_factory.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

ConnectionBasicWidget::ConnectionBasicWidget(
    const std::vector<core::connectionTypes>& availibleTypes,
    QWidget* parent)
    : QWidget(parent) {
  connectionName_ = new QLineEdit;
  typeConnection_ = new QComboBox;
  commandLine_ = new QLineEdit;

  if (availibleTypes.empty()) {
    for (size_t i = 0; i < SIZEOFMASS(core::compiled_types); ++i) {
      core::connectionTypes ct = core::compiled_types[i];
      std::string str = common::ConvertToString(ct);
      typeConnection_->addItem(GuiFactory::instance().icon(ct),
                               common::ConvertFromString<QString>(str), ct);
    }
  } else {
    for (size_t i = 0; i < availibleTypes.size(); ++i) {
      core::connectionTypes ct = availibleTypes[i];
      std::string str = common::ConvertToString(ct);
      typeConnection_->addItem(GuiFactory::instance().icon(ct),
                               common::ConvertFromString<QString>(str), ct);
    }
  }

  typedef void (QComboBox::*qind)(int);
  VERIFY(connect(typeConnection_, static_cast<qind>(&QComboBox::currentIndexChanged), this,
                 &ConnectionBasicWidget::typeConnectionChange));

  QVBoxLayout* basicLayout = new QVBoxLayout;
  QHBoxLayout* connectionNameLayout = new QHBoxLayout;
  connectionNameLabel_ = new QLabel;
  connectionNameLayout->addWidget(connectionNameLabel_);
  connectionNameLayout->addWidget(connectionName_);
  basicLayout->addLayout(connectionNameLayout);

  QHBoxLayout* typeLayout = new QHBoxLayout;
  typeConnectionLabel_ = new QLabel;
  typeLayout->addWidget(typeConnectionLabel_);
  typeLayout->addWidget(typeConnection_);
  basicLayout->addLayout(typeLayout);

  QHBoxLayout* commandLineLayout = new QHBoxLayout;
  commandLineLabel_ = new QLabel;
  commandLineLayout->addWidget(commandLineLabel_);
  commandLineLayout->addWidget(commandLine_);
  basicLayout->addLayout(commandLineLayout);
  setLayout(basicLayout);

  retranslateUi();
}

void ConnectionBasicWidget::typeConnectionChange(int index) {
  QVariant var = typeConnection_->itemData(index);
  core::connectionTypes currentType =
      static_cast<core::connectionTypes>(qvariant_cast<unsigned char>(var));

  const char* helpText = core::CommandLineHelpText(currentType);
  CHECK(helpText);
  QString trHelp = tr(helpText);
  commandLine_->setToolTip(trHelp);
  emit typeConnectionChanged(currentType);
}

QString ConnectionBasicWidget::connectionName() const {
  return connectionName_->text();
}

void ConnectionBasicWidget::setConnectionName(const QString& name) {
  connectionName_->setText(name);
}

core::connectionTypes ConnectionBasicWidget::connectionType() const {
  QVariant var = typeConnection_->currentData();
  return static_cast<core::connectionTypes>(qvariant_cast<unsigned char>(var));
}

void ConnectionBasicWidget::setConnectionType(core::connectionTypes type) {
  typeConnection_->setCurrentIndex(type);
}

QString ConnectionBasicWidget::commandLine() const {
  return commandLine_->text();
}

void ConnectionBasicWidget::setCommandLine(const QString& line) {
  commandLine_->setText(line);
}

void ConnectionBasicWidget::retranslateUi() {
  connectionNameLabel_->setText(tr("Name:"));
  typeConnectionLabel_->setText(tr("Database:"));
  commandLineLabel_->setText(tr("Connection line:"));
}

}  // namespace gui
}  // namespace fastonosql
