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

class QLineEdit;
class QLabel;

namespace fastonosql {
namespace gui {

class PathWidget : public QWidget {
  Q_OBJECT
 public:
  PathWidget(bool isFolderSelectOnly,
             const QString& pathTitle,
             const QString& filter,
             const QString& caption,
             QWidget* parent = 0);

  QString path() const;
  void setPath(const QString& path);

 private Q_SLOTS:
  void selectPathDialog();

 protected:
  virtual void changeEvent(QEvent* ev);

 private:
  void retranslateUi();

  QLabel* pathLabel_;
  QLineEdit* pathEdit_;

  QString pathTitle_;
  QString filter_;
  QString caption_;
  bool isFolderSelectOnly_;
};

}  // namespace gui
}  // namespace fastonosql
