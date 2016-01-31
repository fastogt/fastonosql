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
class QRadioButton;

namespace fastonosql {

class FastoEditor;
class EncodeDecodeDialog
  : public QDialog {
 Q_OBJECT
 public:
  explicit EncodeDecodeDialog(QWidget* parent);

  enum
  {
    height = 480,
    width = 640
  };

 protected:
  virtual void changeEvent(QEvent* );
  virtual bool eventFilter(QObject* object, QEvent* event);

 private Q_SLOTS:
  void decode();

 private:
  void retranslateUi();

  FastoEditor* input_;
  FastoEditor* output_;
  QComboBox* decoders_;
  QRadioButton* encodeButton_;
  QRadioButton* decodeButton_;
};

}
