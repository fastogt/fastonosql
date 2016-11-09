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

class QLabel;
class QLineEdit;
class QCheckBox;
class QSpinBox;

namespace fastonosql {
namespace gui {

class ConnectionAdvancedWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ConnectionAdvancedWidget(QWidget* parent = 0);

  QString connectionFolderText() const;
  void setConnectionFolderText(const QString& text);

  void setFolderEnabled(bool val);

  bool isLogging() const;
  void setLogging(bool logging);

  int loggingInterval() const;
  void setLoggingInterval(int val);

 private Q_SLOTS:
  void loggingStateChange(int value);

 private:
  void retranslateUi();

  QLabel* folderLabel_;
  QLineEdit* connectionFolder_;

  QCheckBox* logging_;
  QSpinBox* loggingMsec_;
};

}  // namespace gui
}  // namespace fastonosql
