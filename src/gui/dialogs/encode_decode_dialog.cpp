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

#include "gui/dialogs/encode_decode_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QSplitter>
#include <QToolButton>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>  // for ConvertToString
#include <common/text_decoders/iedcoder_factory.h>

#include "gui/editor/fasto_editor.h"  // for FastoEditor
#include "gui/gui_factory.h"          // for GuiFactory

#include "translations/global.h"  // for trDecode, trEncode, etc"

namespace fastonosql {
namespace gui {

EncodeDecodeDialog::EncodeDecodeDialog(QWidget* parent) : QDialog(parent) {
  setWindowIcon(GuiFactory::GetInstance().encodeDecodeIcon());
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help button (?)

  QVBoxLayout* layout = new QVBoxLayout;
  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton* closeButton = buttonBox->button(QDialogButtonBox::Close);
  buttonBox->addButton(closeButton,
                       QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &EncodeDecodeDialog::reject));

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

  QHBoxLayout* toolBarLayout = new QHBoxLayout;
  toolBarLayout->addWidget(decode);
  toolBarLayout->addWidget(decoders_);

  encode_button_ = new QRadioButton;
  decode_button_ = new QRadioButton;
  toolBarLayout->addWidget(encode_button_);
  toolBarLayout->addWidget(decode_button_);
  toolBarLayout->addWidget(new QSplitter(Qt::Horizontal));

  input_ = new FastoEditor;
  output_ = new FastoEditor;

  layout->addWidget(input_);
  layout->addLayout(toolBarLayout);
  layout->addWidget(output_);
  layout->addWidget(buttonBox);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(layout);

  retranslateUi();
}

void EncodeDecodeDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(e);
}

void EncodeDecodeDialog::decodeOrEncode() {
  QString in = input_->text();
  if (in.isEmpty()) {
    return;
  }

  output_->clear();
  QVariant var = decoders_->currentData();
  common::EDType current_type = static_cast<common::EDType>(qvariant_cast<unsigned char>(var));
  common::IEDcoder* dec = common::CreateEDCoder(current_type);
  if (!dec) {
    return;
  }

  std::string sin = common::ConvertToString(in);
  std::string out;
  common::Error err = encode_button_->isChecked() ? dec->Encode(sin, &out) : dec->Decode(sin, &out);
  if (err) {
    delete dec;
    return;
  }

  QString qout;
  if (common::ConvertFromString(out, &qout)) {
    output_->setText(qout);
  }
  delete dec;
}

void EncodeDecodeDialog::retranslateUi() {
  setWindowTitle(translations::trEncodeDecode);
  encode_button_->setText(translations::trEncode);
  decode_button_->setText(translations::trDecode);
}

}  // namespace gui
}  // namespace fastonosql
