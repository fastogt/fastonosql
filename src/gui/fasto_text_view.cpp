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

#include "gui/fasto_text_view.h"

#include <QEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include <common/macros.h>  // for VERIFY
#include <common/qt/convert2string.h>

#include "core/db_key.h"

#include "gui/editor/fasto_editor_output.h"  // for FastoEditorOutput, CSV, etc
#include "gui/gui_factory.h"                 // for GuiFactory

#include "translations/global.h"  // for trCsv, trGzip, trHex, etc

Q_DECLARE_METATYPE(fastonosql::core::NValue)

namespace fastonosql {
namespace gui {

FastoTextView::FastoTextView(const QString& delimiter, QWidget* parent) : QWidget(parent) {
  QVBoxLayout* mainL = new QVBoxLayout;

  editor_ = new FastoEditorOutput(delimiter);

  jsonRadioButton_ = new QRadioButton;
  csvRadioButton_ = new QRadioButton;
  rawRadioButton_ = new QRadioButton;
  hexRadioButton_ = new QRadioButton;
  msgPackRadioButton_ = new QRadioButton;
  gzipRadioButton_ = new QRadioButton;

  saveChangeButton_ = new QPushButton;
  saveChangeButton_->setIcon(GuiFactory::instance().saveIcon());
  saveChangeButton_->setEnabled(false);

  VERIFY(connect(jsonRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChange));
  VERIFY(connect(csvRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChange));
  VERIFY(connect(rawRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChange));
  VERIFY(connect(hexRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChange));
  VERIFY(connect(msgPackRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChange));
  VERIFY(connect(gzipRadioButton_, &QRadioButton::toggled, this, &FastoTextView::viewChange));
  VERIFY(connect(saveChangeButton_, &QPushButton::clicked, this, &FastoTextView::saveChanges));
  VERIFY(connect(editor_, &FastoEditorOutput::textChanged, this, &FastoTextView::textChange));
  VERIFY(connect(editor_, &FastoEditorOutput::readOnlyChanged, this, &FastoTextView::textChange));

  QHBoxLayout* radLaout = new QHBoxLayout;
  radLaout->addWidget(jsonRadioButton_);
  radLaout->addWidget(csvRadioButton_);
  radLaout->addWidget(rawRadioButton_);
  radLaout->addWidget(hexRadioButton_);
  radLaout->addWidget(msgPackRadioButton_);
  radLaout->addWidget(gzipRadioButton_);

  mainL->addLayout(radLaout);
  mainL->addWidget(editor_);
  mainL->setContentsMargins(0, 0, 0, 0);
  QHBoxLayout* hlayout = new QHBoxLayout;
  hlayout->addWidget(saveChangeButton_, 0, Qt::AlignRight);

  mainL->addLayout(hlayout);
  setLayout(mainL);

  rawRadioButton_->setChecked(true);
  retranslateUi();
}

void FastoTextView::setModel(QAbstractItemModel* model) {
  editor_->setModel(model);
}

void FastoTextView::saveChanges() {
  QModelIndex index = editor_->selectedItem(1);  // eValue
  common::StringValue* string =
      common::Value::CreateStringValue(common::ConvertToString(editor_->text().simplified()));
  QVariant var = QVariant::fromValue(core::NValue(string));
  editor_->setData(index, var, Qt::EditRole);
}

void FastoTextView::textChange() {
  if (editor_->childCount() != 1) {
    saveChangeButton_->setEnabled(false);
    return;
  }

  QModelIndex index = editor_->selectedItem(1);  // eValue
  bool isEnabled = !editor_->isReadOnly() && index.isValid() &&
                   (index.flags() & Qt::ItemIsEditable) &&
                   index.data() != editor_->text().simplified();

  saveChangeButton_->setEnabled(isEnabled);
}

void FastoTextView::viewChange(bool checked) {
  if (!checked) {
    return;
  }

  if (jsonRadioButton_->isChecked()) {
    editor_->viewChange(JSON);
    return;
  }

  if (csvRadioButton_->isChecked()) {
    editor_->viewChange(CSV);
    return;
  }

  if (rawRadioButton_->isChecked()) {
    editor_->viewChange(RAW);
    return;
  }

  if (hexRadioButton_->isChecked()) {
    editor_->viewChange(HEX);
    return;
  }

  if (msgPackRadioButton_->isChecked()) {
    editor_->viewChange(MSGPACK);
    return;
  }

  if (gzipRadioButton_->isChecked()) {
    editor_->viewChange(GZIP);
    return;
  }
}

void FastoTextView::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void FastoTextView::retranslateUi() {
  jsonRadioButton_->setText(translations::trJson);
  csvRadioButton_->setText(translations::trCsv);
  rawRadioButton_->setText(translations::trRawText);
  hexRadioButton_->setText(translations::trHex);
  msgPackRadioButton_->setText(translations::trMsgPack);
  gzipRadioButton_->setText(translations::trGzip);
  saveChangeButton_->setText(translations::trSave);
}

}  // namespace gui
}  // namespace fastonosql
