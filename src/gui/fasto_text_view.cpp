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

#include "gui/fasto_text_view.h"

#include <QComboBox>
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>

#include "core/db_key.h"

#include "gui/editor/fasto_editor_output.h"  // for FastoEditorOutput, CSV, etc
#include "gui/gui_factory.h"                 // for GuiFactory

#include "translations/global.h"  // for trCsv, trGzip, trHex, etc

Q_DECLARE_METATYPE(fastonosql::core::NValue)

namespace fastonosql {
namespace gui {

FastoTextView::FastoTextView(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* mainL = new QVBoxLayout;

  editor_ = new FastoEditorOutput;
  views_label_ = new QLabel;
  views_combo_box_ = new QComboBox;
  views_combo_box_->addItem(translations::trJson, JSON_VIEW);
  views_combo_box_->addItem(translations::trCsv, CSV_VIEW);
  views_combo_box_->addItem(translations::trRawText, RAW_VIEW);
  views_combo_box_->addItem(translations::trHex, HEX_VIEW);
  views_combo_box_->addItem(translations::trUnicode, UNICODE_VIEW);
  views_combo_box_->addItem(translations::trMsgPack, MSGPACK_VIEW);
  views_combo_box_->addItem(translations::trGzip, GZIP_VIEW);
  views_combo_box_->addItem(translations::trLZ4, LZ4_VIEW);
  views_combo_box_->addItem(translations::trBZip2, BZIP2_VIEW);
  views_combo_box_->addItem(translations::trSnappy, SNAPPY_VIEW);
  views_combo_box_->addItem(translations::trXml, XML_VIEW);

  save_change_button_ = new QPushButton;
  save_change_button_->setIcon(GuiFactory::GetInstance().GetSaveIcon());
  save_change_button_->setEnabled(false);

  typedef void (QComboBox::*ind)(int);
  VERIFY(
      connect(views_combo_box_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &FastoTextView::viewChange));

  VERIFY(connect(save_change_button_, &QPushButton::clicked, this, &FastoTextView::saveChanges));
  VERIFY(connect(editor_, &FastoEditorOutput::textChanged, this, &FastoTextView::textChange));
  VERIFY(connect(editor_, &FastoEditorOutput::readOnlyChanged, this, &FastoTextView::textChange));

  mainL->addWidget(editor_);
  mainL->setContentsMargins(0, 0, 0, 0);
  QHBoxLayout* hlayout = new QHBoxLayout;
  hlayout->addWidget(save_change_button_);
  QSplitter* spliter_save_and_view = new QSplitter(Qt::Horizontal);
  hlayout->addWidget(spliter_save_and_view);
  hlayout->addWidget(views_label_);
  hlayout->addWidget(views_combo_box_);

  mainL->addLayout(hlayout);
  setLayout(mainL);
  views_combo_box_->setCurrentIndex(RAW_VIEW);
  retranslateUi();
}

void FastoTextView::setModel(QAbstractItemModel* model) {
  editor_->setModel(model);
}

void FastoTextView::saveChanges() {
  QModelIndex index = editor_->selectedItem(1);  // eValue
  QString qsimplif = editor_->text().simplified();
  std::string simplif = common::ConvertToString(qsimplif);
  common::StringValue* string = common::Value::CreateStringValue(simplif);
  core::NValue nv_string(string);
  QVariant var = QVariant::fromValue(nv_string);
  editor_->setData(index, var, Qt::EditRole);
}

void FastoTextView::textChange() {
  if (editor_->childCount() != 1) {
    save_change_button_->setEnabled(false);
    return;
  }

  QModelIndex index = editor_->selectedItem(1);  // eValue
  bool isEnabled = !editor_->isReadOnly() && index.isValid() && (index.flags() & Qt::ItemIsEditable) &&
                   index.data() != editor_->text().simplified();

  save_change_button_->setEnabled(isEnabled);
}

void FastoTextView::viewChange(int index) {
  QVariant var = views_combo_box_->itemData(index);
  unsigned char view = qvariant_cast<unsigned char>(var);
  editor_->viewChange(view);
}

void FastoTextView::changeEvent(QEvent* ev) {
  if (ev->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QWidget::changeEvent(ev);
}

void FastoTextView::retranslateUi() {
  save_change_button_->setText(translations::trSaveChanges);
  views_label_->setText(translations::trViews + ":");
}

}  // namespace gui
}  // namespace fastonosql
