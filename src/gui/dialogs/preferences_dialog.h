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

#pragma  once

#include <QDialog>

class QComboBox;
class QFontComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QGroupBox;

namespace fastonosql {

class PreferencesDialog
  : public QDialog
{
  Q_OBJECT
 public:
  explicit PreferencesDialog(QWidget* parent);
  enum
  {
    min_height = 480,
    min_width = 640
  };

 public Q_SLOTS:
  virtual void accept();

 private:
  void syncWithSettings();

 protected:
  virtual void changeEvent(QEvent* );

 private:
  void retranslateUi();

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

  QGroupBox* serverSettingsBox_;
  QCheckBox* syncTabs_;
  QLabel* logDirLabel_;
  QLineEdit* logDirPath_;
  QCheckBox* autoOpenConsole_;
  QCheckBox* fastViewKeys_;
};

}
