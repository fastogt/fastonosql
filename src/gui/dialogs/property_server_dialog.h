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

#include <QDialog>

#include "core/core_fwd.h"
#include "core/events/events_info.h"

class QTableView;

namespace fasto {
namespace qt {
namespace gui {
class GlassWidget;
}
}
}

namespace fastonosql {

class PropertyServerDialog
  : public QDialog {
  Q_OBJECT
 public:
  explicit PropertyServerDialog(IServerSPtr server, QWidget* parent = 0);

 Q_SIGNALS:
  void showed();

 private Q_SLOTS:
  void startServerProperty(const events_info::ServerPropertyInfoRequest& req);
  void finishServerProperty(const events_info::ServerPropertyInfoResponce& res);

  void startServerChangeProperty(const events_info::ChangeServerPropertyInfoRequest& req);
  void finishServerChangeProperty(const events_info::ChangeServerPropertyInfoResponce& res);

  void changedProperty(const PropertyType& prop);
 protected:
  virtual void changeEvent(QEvent* e);
  virtual void showEvent(QShowEvent *e);

 private:
  void retranslateUi();

  fasto::qt::gui::GlassWidget* glassWidget_;
  QTableView* propertyes_table_;
  const IServerSPtr server_;
};

}  // namespace fastonosql
