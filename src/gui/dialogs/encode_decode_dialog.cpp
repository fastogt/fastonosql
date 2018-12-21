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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/encode_decode_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSplitter>
#include <QToolButton>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/text_decoders/iedcoder_factory.h>

#include "gui/gui_factory.h"           // for GuiFactory
#include "gui/widgets/fasto_editor.h"  // for FastoEditor

#include "translations/global.h"  // for trDecode, trEncode, etc"

namespace fastonosql {
namespace gui {

EncodeDecodeDialog::EncodeDecodeDialog(const QString& title, const QIcon& icon, QWidget* parent)
    : base_class(title, parent),
      input_(nullptr),
      output_(nullptr),
      decoders_(nullptr),
      encode_button_(nullptr),
      decode_button_(nullptr) {
  setWindowIcon(icon);

  QToolButton* decode = new QToolButton;
  decode->setIcon(GuiFactory::GetInstance().executeIcon());
  VERIFY(connect(decode, &QToolButton::clicked, this, &EncodeDecodeDialog::decodeOrEncode));

  decoders_ = new QComboBox;
  for (size_t i = 0; i < common::ENCODER_DECODER_NUM_TYPES; ++i) {
    const char* estr = common::edecoder_types[i];
    common::EDType etype;
    if (common::ConvertFromString(estr, &etype)) {
      decoders_->addItem(estr, etype);
    }
  }

  QHBoxLayout* tool_bar_layout = new QHBoxLayout;
  tool_bar_layout->addWidget(decode);
  tool_bar_layout->addWidget(decoders_);

  encode_button_ = new QRadioButton;
  decode_button_ = new QRadioButton;
  tool_bar_layout->addWidget(encode_button_);
  tool_bar_layout->addWidget(decode_button_);
  tool_bar_layout->addWidget(new QSplitter(Qt::Horizontal));

  input_ = createWidget<FastoEditor>();
  output_ = createWidget<FastoEditor>();

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton* close_button = button_box->button(QDialogButtonBox::Close);
  button_box->addButton(close_button,
                        QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &EncodeDecodeDialog::reject));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(input_);
  main_layout->addLayout(tool_bar_layout);
  main_layout->addWidget(output_);
  main_layout->addWidget(button_box);
  setLayout(main_layout);
  setMinimumSize(QSize(min_width, min_height));
}

void EncodeDecodeDialog::decodeOrEncode() {
  const QString input = input_->text();
  if (input.isEmpty()) {
    return;
  }

  output_->clear();
  QVariant var = decoders_->currentData();
  common::EDType current_type = static_cast<common::EDType>(qvariant_cast<unsigned char>(var));
  common::IEDcoder* dec = common::CreateEDCoder(current_type);
  if (!dec) {
    return;
  }

  common::char_buffer_t input_str = common::ConvertToCharBytes(input);
  common::char_buffer_t out;
  common::Error err = encode_button_->isChecked() ? dec->Encode(input_str, &out) : dec->Decode(input_str, &out);
  if (err) {
    delete dec;
    return;
  }

  QString qout;
  if (common::ConvertFromBytes(out, &qout)) {
    output_->setText(qout);
  }
  delete dec;
}

void EncodeDecodeDialog::retranslateUi() {
  encode_button_->setText(translations::trEncode);
  decode_button_->setText(translations::trDecode);
  base_class::retranslateUi();
}

}  // namespace gui
}  // namespace fastonosql
