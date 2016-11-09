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

#pragma once

#include <QWidget>

#include "core/connection_types.h"

class QLabel;
class QLineEdit;
class QComboBox;

namespace fastonosql {
namespace gui {

class ConnectionBasicWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ConnectionBasicWidget(const std::vector<core::connectionTypes>& availibleTypes,
                                 QWidget* parent = 0);

  QString connectionName() const;
  void setConnectionName(const QString& name);

  core::connectionTypes connectionType() const;
  void setConnectionType(core::connectionTypes type);

  QString commandLine() const;
  void setCommandLine(const QString& line);

 Q_SIGNALS:
  void typeConnectionChanged(core::connectionTypes type);

 private Q_SLOTS:
  void typeConnectionChange(int index);

 private:
  void retranslateUi();

  QLabel* connectionNameLabel_;
  QLineEdit* connectionName_;

  QLabel* typeConnectionLabel_;
  QComboBox* typeConnection_;

  QLabel* commandLineLabel_;
  QLineEdit* commandLine_;
};

}  // namespace gui
}  // namespace fastonosql
