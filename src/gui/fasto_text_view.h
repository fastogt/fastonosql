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

class QAbstractItemModel;  // lines 25-25
class QEvent;
class QPushButton;   // lines 24-24
class QRadioButton;  // lines 23-23
namespace fastonosql {
namespace gui {
class FastoEditorOutput;
}
}  // namespace fastonosql

namespace fastonosql {
namespace gui {

class FastoTextView : public QWidget {
  Q_OBJECT
 public:
  FastoTextView(const QString& delimiter, QWidget* parent = 0);

  void setModel(QAbstractItemModel* model);

 private Q_SLOTS:
  void viewChange(bool checked);
  void textChange();
  void saveChanges();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  FastoEditorOutput* editor_;
  QRadioButton* jsonRadioButton_;
  QRadioButton* csvRadioButton_;
  QRadioButton* rawRadioButton_;
  QRadioButton* hexRadioButton_;
  QRadioButton* msgPackRadioButton_;
  QRadioButton* gzipRadioButton_;
  QPushButton* saveChangeButton_;
};

}  // namespace gui
}  // namespace fastonosql
