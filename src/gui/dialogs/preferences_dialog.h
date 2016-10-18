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

#include <QDialog>

class QCheckBox;  // lines 25-25
class QComboBox;  // lines 23-23
class QEvent;
class QFontComboBox;  // lines 24-24
class QGroupBox;      // lines 28-28
class QLabel;         // lines 26-26
class QLineEdit;      // lines 27-27
class QWidget;

namespace fastonosql {
namespace gui {
class PreferencesDialog : public QDialog {
  Q_OBJECT
 public:
  explicit PreferencesDialog(QWidget* parent);
  enum { min_width = 640, min_height = 480 };

 public Q_SLOTS:
  virtual void accept();

 private:
  void syncWithSettings();

 protected:
  virtual void changeEvent(QEvent* ev);

 private:
  void retranslateUi();
#ifndef IS_PUBLIC_BUILD
  QGroupBox* profileBox_;
  QLabel* loginLabel_;
  QLineEdit* loginText_;
#endif

  QGroupBox* generalBox_;
  QCheckBox* autoCheckUpdates_;
  QCheckBox* autoComletionEnable_;
  QLabel* langLabel_;
  QComboBox* languagesComboBox_;

  QLabel* stylesLabel_;
  QComboBox* stylesComboBox_;
  QLabel* fontLabel_;
  QFontComboBox* fontComboBox_;
  QLabel* defaultViewLabel_;
  QComboBox* defaultViewComboBox_;
  QLabel* logDirLabel_;
  QLineEdit* logDirPath_;
  QCheckBox* autoOpenConsole_;
  QCheckBox* fastViewKeys_;
};
}  // namespace gui
}  // namespace fastonosql
