/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QDialog>

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
  void startServerProperty(const EventsInfo::ServerPropertyInfoRequest& req);
  void finishServerProperty(const EventsInfo::ServerPropertyInfoResponce& res);

  void startServerChangeProperty(const EventsInfo::ChangeServerPropertyInfoRequest& req);
  void finishServerChangeProperty(const EventsInfo::ChangeServerPropertyInfoResponce& res);

  void changedProperty(const PropertyType& prop);
 protected:
  virtual void changeEvent(QEvent* e);
  virtual void showEvent(QShowEvent *e);

 private:
  void retranslateUi();

  fasto::qt::gui::GlassWidget *glassWidget_;
  QTableView *propertyes_table_;
  const IServerSPtr server_;
};

}
