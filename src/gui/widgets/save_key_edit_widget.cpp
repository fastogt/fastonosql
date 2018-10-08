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

#include "gui/widgets/save_key_edit_widget.h"

#include <QEvent>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include <common/qt/gui/glass_widget.h>

#include "gui/gui_factory.h"
#include "gui/widgets/key_edit_widget.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

SaveKeyEditWidget::SaveKeyEditWidget(const std::vector<common::Value::Type>& availible_types, QWidget* parent)
    : base_class(parent), init_key_() {
  QVBoxLayout* main_layout = new QVBoxLayout;

  editor_ = new KeyEditWidget(availible_types);

  save_changes_button_ = new QPushButton;
  save_changes_button_->setIcon(GuiFactory::GetInstance().saveIcon());
  glass_widget_ = new common::qt::gui::GlassWidget(gui::GuiFactory::GetInstance().pathToLoadingGif(), QString(), 0.5,
                                                   QColor(111, 111, 100), this);

  VERIFY(connect(save_changes_button_, &QPushButton::clicked, this, &SaveKeyEditWidget::keySave));
  VERIFY(connect(editor_, &KeyEditWidget::keyChanged, this, &SaveKeyEditWidget::syncControls));

  main_layout->addWidget(editor_);
  QHBoxLayout* save_layout = new QHBoxLayout;
  save_layout->addWidget(new QSplitter(Qt::Horizontal));
  save_layout->addWidget(save_changes_button_);
  main_layout->addLayout(save_layout);
  main_layout->setContentsMargins(0, 0, 0, 0);

  setLayout(main_layout);
  syncControls();
  retranslateUi();
}

SaveKeyEditWidget::~SaveKeyEditWidget() {}

void SaveKeyEditWidget::initialize(const core::NDbKValue& key) {
  editor_->initialize(key);
  init_key_ = key;
  syncControls();
}

void SaveKeyEditWidget::setEnableKeyEdit(bool enable) {
  editor_->setEnableKeyEdit(enable);
}

void SaveKeyEditWidget::keySave() {
  if (!init_key_) {
    return;
  }

  core::NDbKValue dbv;
  if (!editor_->getKey(&dbv)) {
    return;
  }

  if (init_key_->Equals(dbv)) {
    return;
  }

  emit keyReadyToSave(dbv);
}

void SaveKeyEditWidget::startSaveKey() {
  glass_widget_->start();
}

void SaveKeyEditWidget::finishSaveKey() {
  glass_widget_->stop();
}

void SaveKeyEditWidget::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  base_class::changeEvent(e);
}

void SaveKeyEditWidget::retranslateUi() {
  save_changes_button_->setText(translations::trSaveChanges);
}

void SaveKeyEditWidget::syncControls() {
  if (!init_key_) {
    save_changes_button_->setEnabled(false);
    return;
  }

  core::NDbKValue dbv;
  if (!editor_->getKey(&dbv)) {
    save_changes_button_->setEnabled(false);
    return;
  }

  bool eq = init_key_->Equals(dbv);
  save_changes_button_->setEnabled(!eq);
}

}  // namespace gui
}  // namespace fastonosql
