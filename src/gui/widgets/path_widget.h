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

#pragma once

#include "gui/widgets/base_widget.h"

class QLineEdit;
class QLabel;

namespace fastonosql {
namespace gui {

class IPathWidget : public BaseWidget {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  QString path() const;
  void setPath(const QString& path);

  bool isValidPath() const;

  virtual int mode() const = 0;

 private Q_SLOTS:
  void selectPathDialog();

 protected:
  IPathWidget(const QString& path_title, const QString& filter, const QString& caption, QWidget* parent = Q_NULLPTR);
  void retranslateUi() override;

 private:
  void selectPathDialogRoutine(const QString& caption, const QString& filter, int mode);

  QLabel* path_label_;
  QLineEdit* path_edit_;

  QString path_title_;
  QString filter_;
  QString caption_;
};

class FilePathWidget : public IPathWidget {
 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  int mode() const override;

 protected:
  FilePathWidget(const QString& path_title, const QString& filter, const QString& caption, QWidget* parent = Q_NULLPTR);
};

class DirectoryPathWidget : public IPathWidget {
 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  int mode() const override;

 protected:
  DirectoryPathWidget(const QString& path_title, const QString& caption, QWidget* parent = Q_NULLPTR);
};

}  // namespace gui
}  // namespace fastonosql
