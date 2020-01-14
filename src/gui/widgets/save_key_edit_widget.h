/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#pragma once

#include <vector>

#include <common/optional.h>
#include <common/value.h>

#include <fastonosql/core/db_key.h>

#include "gui/widgets/base_widget.h"

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

class SaveKeyEditWidget : public BaseWidget {
  Q_OBJECT

 public:
  typedef BaseWidget base_class;
  template <typename T, typename... Args>
  friend T* createWidget(Args&&... args);

  void initialize(const std::vector<common::Value::Type>& availible_types, const core::NDbKValue& key);
  void setEnableKeyEdit(bool enable);

  void startSaveKey();
  void finishSaveKey();

 Q_SIGNALS:
  void keyReadyToSave(const core::NDbKValue& dbv);

 private Q_SLOTS:
  void keySave();
  void syncControls();

 protected:
  explicit SaveKeyEditWidget(QWidget* parent = Q_NULLPTR);
  void retranslateUi() override;

  KeyEditWidget* editor_;
  QPushButton* save_changes_button_;
  common::Optional<core::NDbKValue> init_key_;
  common::qt::gui::GlassWidget* glass_widget_;
};

}  // namespace gui
}  // namespace fastonosql
