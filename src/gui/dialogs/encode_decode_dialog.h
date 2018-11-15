/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include "gui/dialogs/base_dialog.h"

class QComboBox;     // lines 23-23
class QRadioButton;  // lines 24-24

namespace fastonosql {
namespace gui {

class FastoEditor;

class EncodeDecodeDialog : public BaseDialog {
  Q_OBJECT

 public:
  typedef BaseDialog base_class;
  template <typename T, typename... Args>
  friend T* createDialog(Args&&... args);

  enum { min_width = 640, min_height = 480 };

 protected:
  explicit EncodeDecodeDialog(const QString& title, const QIcon& icon, QWidget* parent = Q_NULLPTR);

  void retranslateUi() override;

 private Q_SLOTS:
  void decodeOrEncode();

 private:
  FastoEditor* input_;
  FastoEditor* output_;
  QComboBox* decoders_;
  QRadioButton* encode_button_;
  QRadioButton* decode_button_;
};

}  // namespace gui
}  // namespace fastonosql
