/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QDialog>

#include "core/types.h"

class QLineEdit;
class QComboBox;
class QListWidget;
class QTableWidget;
class QGroupBox;

namespace fastonosql {

class CreateDbKeyDialog
  : public QDialog {
  Q_OBJECT
 public:
  enum {
      min_height = 200,
      min_width = 320
  };

  explicit CreateDbKeyDialog(const QString& title, connectionTypes type, QWidget* parent = 0);
  NDbKValue key() const;

 public Q_SLOTS:
  virtual void accept();

 private Q_SLOTS:
  void typeChanged(int index);
  void addItem();
  void removeItem();

 protected:
  virtual void changeEvent(QEvent* ev);

 private:
  bool validateAndApply();
  void retranslateUi();

  common::Value *getItem() const;
  const connectionTypes type_;
  QGroupBox* generalBox_;
  QLineEdit* keyEdit_;
  QComboBox* typesCombo_;
  QLineEdit* valueEdit_;
  QListWidget* valueListEdit_;
  QTableWidget* valueTableEdit_;
  NValue value_;
};

}  // namespace fastonosql
