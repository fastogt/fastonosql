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

#include <QWidget>

class QLineEdit;
class QLabel;
class QCheckBox;

namespace fastonosql {
namespace gui {

class IPathWidget : public QWidget {
  Q_OBJECT
 public:
  IPathWidget(const QString& pathTitle, const QString& filter, const QString& caption, QWidget* parent = Q_NULLPTR);

  QString path() const;
  void setPath(const QString& path);

  bool isValidPath() const;

  virtual int mode() const = 0;

 private Q_SLOTS:
  void selectPathDialog();

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private:
  void selectPathDialogRoutine(const QString& caption, const QString& filter, int mode);
  void retranslateUi();

  QLabel* pathLabel_;
  QLineEdit* pathEdit_;

  QString pathTitle_;
  QString filter_;
  QString caption_;
};

class FilePathWidget : public IPathWidget {
 public:
  FilePathWidget(const QString& pathTitle, const QString& filter, const QString& caption, QWidget* parent = Q_NULLPTR);

  virtual int mode() const override;
};

class DirectoryPathWidget : public IPathWidget {
 public:
  DirectoryPathWidget(const QString& pathTitle, const QString& caption, QWidget* parent = Q_NULLPTR);

  virtual int mode() const override;
};

}  // namespace gui
}  // namespace fastonosql
