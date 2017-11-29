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

class QComboBox;  // lines 23-23
class QEvent;
class QRadioButton;  // lines 24-24
class QWidget;

namespace fastonosql {
namespace gui {

class FastoEditor;

class EncodeDecodeDialog : public QDialog {
  Q_OBJECT
 public:
  explicit EncodeDecodeDialog(QWidget* parent);

  enum { min_width = 640, min_height = 480 };

 protected:
  virtual void changeEvent(QEvent* ev) override;

 private Q_SLOTS:
  void decodeOrEncode();

 private:
  void retranslateUi();

  FastoEditor* input_;
  FastoEditor* output_;
  QComboBox* decoders_;
  QRadioButton* encodeButton_;
  QRadioButton* decodeButton_;
};

}  // namespace gui
}  // namespace fastonosql
