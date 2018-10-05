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
#include <QSplitter>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>

#include <fastonosql/core/db_key.h>

#include "gui/editor/fasto_editor_output.h"  // for FastoEditorOutput, CSV, etc
#include "gui/gui_factory.h"                 // for GuiFactory

#include "translations/global.h"

Q_DECLARE_METATYPE(fastonosql::core::NValue)

namespace fastonosql {
namespace gui {

FastoTextView::FastoTextView(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* mainL = new QVBoxLayout;

  editor_ = new FastoEditorOutput;
  editor_->setReadOnly(true);

  views_label_ = new QLabel;
  views_combo_box_ = new QComboBox;
  for (unsigned i = 0; i < g_output_views_text.size(); ++i) {
    views_combo_box_->addItem(g_output_views_text[i], i);
  }

  typedef void (QComboBox::*ind)(int);
  VERIFY(
      connect(views_combo_box_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &FastoTextView::viewChange));

  mainL->addWidget(editor_);
  mainL->setContentsMargins(0, 0, 0, 0);
  QHBoxLayout* hlayout = new QHBoxLayout;
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
  views_label_->setText(translations::trViews + ":");
}

}  // namespace gui
}  // namespace fastonosql
