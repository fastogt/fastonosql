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

#include "gui/dialogs/connection_select_type_dialog.h"

#include <string>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <common/qt/convert2string.h>

#include "gui/gui_factory.h"

namespace {
const QString trDatabase = QObject::tr("Database");
}  // namespace

namespace fastonosql {
namespace gui {

ConnectionSelectTypeDialog::ConnectionSelectTypeDialog(const QString& title, QWidget* parent)
    : base_class(title, parent), type_connection_label_(nullptr), type_connection_(nullptr), button_box_(nullptr) {
  type_connection_label_ = new QLabel;
  type_connection_ = new QComboBox;
  const auto updateCombobox = [this](core::ConnectionType type) {
    type_connection_->addItem(GuiFactory::GetInstance().icon(type), core::ConnectionTypeToString(type), type);
  };
#if defined(BUILD_WITH_REDIS)
  updateCombobox(core::REDIS);
#endif
#if defined(BUILD_WITH_MEMCACHED)
  updateCombobox(core::MEMCACHED);
#endif
#if defined(BUILD_WITH_SSDB)
  updateCombobox(core::SSDB);
#endif
#if defined(BUILD_WITH_LEVELDB)
  updateCombobox(core::LEVELDB);
#endif
#if defined(BUILD_WITH_ROCKSDB)
  updateCombobox(core::ROCKSDB);
#endif
#if defined(BUILD_WITH_UNQLITE)
  updateCombobox(core::UNQLITE);
#endif
#if defined(BUILD_WITH_LMDB)
  updateCombobox(core::LMDB);
#endif
#if defined(BUILD_WITH_FORESTDB)
  updateCombobox(core::FORESTDB);
#endif
#if defined(BUILD_WITH_PIKA)
  updateCombobox(core::PIKA);
#endif
#if defined(BUILD_WITH_DYNOMITE)
  updateCombobox(core::DYNOMITE);
#endif
#if defined(BUILD_WITH_KEYDB)
  updateCombobox(core::KEYDB);
#endif

  QHBoxLayout* type_layout = new QHBoxLayout;
  type_layout->addWidget(type_connection_label_);
  type_layout->addWidget(type_connection_);

  QHBoxLayout* bottom_layout = new QHBoxLayout;
  button_box_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  button_box_->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box_, &QDialogButtonBox::accepted, this, &ConnectionSelectTypeDialog::accept));
  VERIFY(connect(button_box_, &QDialogButtonBox::rejected, this, &ConnectionSelectTypeDialog::reject));
  bottom_layout->addWidget(button_box_);

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addLayout(type_layout);
  main_layout->addLayout(bottom_layout);
  main_layout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(main_layout);
}

core::ConnectionType ConnectionSelectTypeDialog::connectionType() const {
  const QVariant var = type_connection_->currentData();
  return static_cast<core::ConnectionType>(qvariant_cast<uint8_t>(var));
}

void ConnectionSelectTypeDialog::retranslateUi() {
  type_connection_label_->setText(trDatabase + ":");
  base_class::retranslateUi();
}

}  // namespace gui
}  // namespace fastonosql
