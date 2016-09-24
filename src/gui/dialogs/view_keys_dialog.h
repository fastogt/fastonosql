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

#pragma once

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint32_t

#include <vector>  // for vector

#include <QDialog>

#include "core/core_fwd.h"     // for IDatabaseSPtr
#include "core/types.h"

class QEvent;
class QLabel;     // lines 30-30
class QLineEdit;  // lines 28-28
class QPushButton;
class QSpinBox;  // lines 29-29
class QWidget;

namespace fastonosql {
namespace core {
namespace events_info {
struct ExecuteInfoRequest;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct ExecuteInfoResponce;
}
}
}

namespace fastonosql {
namespace core {
namespace events_info {
struct LoadDatabaseContentRequest;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct LoadDatabaseContentResponce;
}
}
}
namespace fastonosql {
namespace gui {
class FastoTableView;
}
}  // lines 35-35
namespace fastonosql {
namespace gui {
class KeysTableModel;
}
}  // lines 36-36

namespace fastonosql {
namespace gui {

class ViewKeysDialog : public QDialog {
  Q_OBJECT
 public:
  enum {
    min_width = 320,
    min_height = 200,
    min_key_on_page = 1,
    max_key_on_page = 100,
    defaults_key = 10,
    step_keys_on_page = defaults_key
  };

  explicit ViewKeysDialog(const QString& title, core::IDatabaseSPtr db, QWidget* parent = 0);

 private Q_SLOTS:
  void startLoadDatabaseContent(const core::events_info::LoadDatabaseContentRequest& req);
  void finishLoadDatabaseContent(const core::events_info::LoadDatabaseContentResponce& res);

  void startExecute(const core::events_info::ExecuteInfoRequest& req);
  void finishExecute(const core::events_info::ExecuteInfoResponce& res);
  void keyTTLChange(core::IDataBaseInfoSPtr db, core::NKey key, core::ttl_t ttl);

  void changeTTL(const core::NDbKValue& value, core::ttl_t ttl);

  void searchLineChanged(const QString& text);
  void leftPageClicked();
  void rightPageClicked();

 protected:
  virtual void changeEvent(QEvent* ev);

 private:
  void search(bool forward);
  void retranslateUi();
  void updateControls();
  size_t keysCount() const;

  std::vector<uint32_t> cursorStack_;
  uint32_t curPos_;
  QLineEdit* searchBox_;
  QLabel* keyCountLabel_;
  QSpinBox* countSpinEdit_;

  QPushButton* searchButton_;
  QPushButton* leftButtonList_;
  QSpinBox* currentKey_;
  QSpinBox* countKey_;
  QPushButton* rightButtonList_;
  FastoTableView* keysTable_;
  KeysTableModel* keysModel_;
  core::IDatabaseSPtr db_;
};

}  // namespace gui
}  // namespace fastonosql
