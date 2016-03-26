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

class QRadioButton;
class QPushButton;
class QAbstractItemModel;

namespace fastonosql {
namespace gui {

class FastoEditorOutput;
class FastoTextView
  : public QWidget {
  Q_OBJECT
 public:
  FastoTextView(const QString& delemitr, QWidget* parent = 0);

  void setModel(QAbstractItemModel* model);
  void setReadOnly(bool ro);

 private Q_SLOTS:
  void viewChanged(bool checked);
  void textChange();
  void saveChanges();

 protected:
  virtual void changeEvent(QEvent* ev);

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
