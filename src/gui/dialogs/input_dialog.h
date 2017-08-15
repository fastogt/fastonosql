/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <QDialog>

class QLineEdit;  // lines 23-23

namespace fastonosql {
namespace gui {

class InputDialog : public QDialog {
  Q_OBJECT
 public:
  enum InputType { SingleLine, DoubleLine };
  explicit InputDialog(QWidget* parent,
                       const QString& title,
                       InputType type,
                       const QString& firstLabelText,
                       const QString& secondLabelText = QString());

  QString firstText() const;
  QString secondText() const;

  void setFirstPlaceholderText(const QString& placeh);
  void setSecondPlaceholderText(const QString& placeh);

 private:
  QLineEdit* firstLine_;
  QLineEdit* secondLine_;
};

}  // namespace gui
}  // namespace fastonosql
