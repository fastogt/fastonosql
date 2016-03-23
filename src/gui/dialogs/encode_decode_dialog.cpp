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

#include "gui/dialogs/encode_decode_dialog.h"

#include <string>

#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QSplitter>
#include <QToolButton>
#include <QComboBox>
#include <QRadioButton>
#include <QEvent>
#include <QKeyEvent>

#include "gui/gui_factory.h"
#include "gui/fasto_editor.h"

#include "common/text_decoders/iedcoder.h"
#include "common/qt/convert_string.h"

#include "translations/global.h"

namespace fastonosql {

EncodeDecodeDialog::EncodeDecodeDialog(QWidget* parent)
  : QDialog(parent) {
  setWindowIcon(GuiFactory::instance().encodeDecodeIcon());

  setWindowTitle(translations::trEncodeDecode);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  QVBoxLayout* layout = new QVBoxLayout;

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton* closeButton = buttonBox->button(QDialogButtonBox::Close);
  buttonBox->addButton(closeButton, QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &EncodeDecodeDialog::reject));

  QToolButton* decode = new QToolButton;
  decode->setIcon(GuiFactory::instance().executeIcon());
  VERIFY(connect(decode, &QToolButton::clicked, this, &EncodeDecodeDialog::decode));

  decoders_ = new QComboBox;
  for (size_t i = 0; i < SIZEOFMASS(common::EDecoderTypes); ++i) {
    decoders_->addItem(common::convertFromString<QString>(common::EDecoderTypes[i]));
  }

  QHBoxLayout* toolBarLayout = new QHBoxLayout;
  toolBarLayout->addWidget(decode);
  toolBarLayout->addWidget(decoders_);

  encodeButton_ = new QRadioButton;
  decodeButton_ = new QRadioButton;
  toolBarLayout->addWidget(encodeButton_);
  toolBarLayout->addWidget(decodeButton_);

  QSplitter* splitter = new QSplitter;
  splitter->setOrientation(Qt::Horizontal);
  splitter->setHandleWidth(1);
  toolBarLayout->addWidget(splitter);

  input_ = new FastoEditor;
  input_->installEventFilter(this);
  output_ = new FastoEditor;
  output_->installEventFilter(this);

  layout->addWidget(input_);
  layout->addLayout(toolBarLayout);
  layout->addWidget(output_);
  layout->addWidget(buttonBox);

  setMinimumSize(QSize(width, height));
  setLayout(layout);

  retranslateUi();
}

bool EncodeDecodeDialog::eventFilter(QObject* object, QEvent* event) {
  if (object == output_ || object == input_) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_Escape) {
        reject();
        return true;
      }
    }
  }

  return QWidget::eventFilter(object, event);
}

void EncodeDecodeDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(e);
}

void EncodeDecodeDialog::decode() {
  const QString in = input_->text();
  if (in.isEmpty()) {
    return;
  }

  output_->clear();
  QString decoderText = decoders_->currentText();
  std::string sdec = common::convertToString(decoderText);
  common::IEDcoder* dec = common::IEDcoder::createEDCoder(sdec);

  if (!dec) {
    return;
  }

  std::string sin = common::convertToString(in);
  std::string out;
  common::Error er;
  if (encodeButton_->isChecked()) {
    er = dec->encode(sin, &out);
  } else {
    er = dec->decode(sin, &out);
  }

  if (!er) {
    output_->setText(common::convertFromString<QString>(out));
  }
  delete dec;
}

void EncodeDecodeDialog::retranslateUi() {
  encodeButton_->setText(translations::trEncode);
  decodeButton_->setText(translations::trDecode);
}

}  // namespace fastonosql
