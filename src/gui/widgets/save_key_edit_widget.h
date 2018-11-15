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

#include <QWidget>

#include <common/optional.h>
#include <common/value.h>

#include <fastonosql/core/db_key.h>

class QPushButton;

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace gui {

class KeyEditWidget;

class SaveKeyEditWidget : public QWidget {
  Q_OBJECT

 public:
  typedef QWidget base_class;

  explicit SaveKeyEditWidget(const std::vector<common::Value::Type>& availible_types, QWidget* parent = Q_NULLPTR);
  ~SaveKeyEditWidget() override;

  void initialize(const core::NDbKValue& key);
  void setEnableKeyEdit(bool enable);

  void startSaveKey();
  void finishSaveKey();

 Q_SIGNALS:
  void keyReadyToSave(const core::NDbKValue& dbv);

 private Q_SLOTS:
  void keySave();
  void syncControls();

 protected:
  void changeEvent(QEvent* ev) override;

 private:
  void retranslateUi();

  KeyEditWidget* editor_;
  QPushButton* save_changes_button_;
  common::Optional<core::NDbKValue> init_key_;
  common::qt::gui::GlassWidget* glass_widget_;
};

}  // namespace gui
}  // namespace fastonosql
