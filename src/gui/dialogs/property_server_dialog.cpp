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

#include "gui/dialogs/property_server_dialog.h"

#include <QHBoxLayout>
#include <QTableView>

#include <common/qt/gui/glass_widget.h>

#include "proxy/events/events_info.h"
#include "proxy/server/iserver.h"

#include "gui/gui_factory.h"

#include "gui/models/property_table_model.h"

#include "translations/global.h"

namespace fastonosql {
namespace gui {

PropertyServerDialog::PropertyServerDialog(const QString& title,
                                           const QIcon& icon,
                                           proxy::IServerSPtr server,
                                           QWidget* parent)
    : base_class(title, parent), glass_widget_(nullptr), properties_table_(nullptr), server_(server) {
  CHECK(server_);

  setWindowIcon(icon);

  VERIFY(connect(server.get(), &proxy::IServer::LoadServerPropertyStarted, this,
                 &PropertyServerDialog::startLoadServerProperty));
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerPropertyFinished, this,
                 &PropertyServerDialog::finishLoadServerProperty));
  VERIFY(connect(server.get(), &proxy::IServer::ChangeServerPropertyStarted, this,
                 &PropertyServerDialog::startServerChangeProperty));
  VERIFY(connect(server.get(), &proxy::IServer::ChangeServerPropertyFinished, this,
                 &PropertyServerDialog::finishServerChangeProperty));

  PropertyTableModel* mod = new PropertyTableModel(this);
  properties_table_ = new QTableView;
  VERIFY(connect(mod, &PropertyTableModel::propertyChanged, this, &PropertyServerDialog::changeProperty));
  properties_table_->setModel(mod);

  glass_widget_ = new common::qt::gui::GlassWidget(GuiFactory::GetInstance().pathToLoadingGif(),
                                                   translations::trLoad + "...", 0.5, QColor(111, 111, 100), this);

  QHBoxLayout* main_layout = new QHBoxLayout;
  main_layout->addWidget(properties_table_);
  setLayout(main_layout);
  setMinimumSize(QSize(min_width, min_height));
}

void PropertyServerDialog::startLoadServerProperty(const proxy::events_info::ServerPropertyInfoRequest& req) {
  UNUSED(req);

  glass_widget_->start();
}

void PropertyServerDialog::finishLoadServerProperty(const proxy::events_info::ServerPropertyInfoResponse& res) {
  glass_widget_->stop();
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  if (core::IsRedisCompatible(server_->GetType())) {
    core::ServerPropertiesInfo inf = res.info;
    PropertyTableModel* model = qobject_cast<PropertyTableModel*>(properties_table_->model());
    for (size_t i = 0; i < inf.properties.size(); ++i) {
      model->insertProperty(inf.properties[i]);
    }
  }
}

void PropertyServerDialog::startServerChangeProperty(const proxy::events_info::ChangeServerPropertyInfoRequest& req) {
  UNUSED(req);
}

void PropertyServerDialog::finishServerChangeProperty(const proxy::events_info::ChangeServerPropertyInfoResponse& res) {
  common::Error err = res.errorInfo();
  if (err) {
    return;
  }

  if (core::IsRedisCompatible(server_->GetType())) {
    core::property_t pr = res.new_item;
    if (res.is_change) {
      PropertyTableModel* model = qobject_cast<PropertyTableModel*>(properties_table_->model());
      model->changeProperty(pr);
    }
  }
}

void PropertyServerDialog::changeProperty(const core::property_t& prop) {
  proxy::events_info::ChangeServerPropertyInfoRequest req(this, prop);
  server_->ChangeProperty(req);
}

void PropertyServerDialog::showEvent(QShowEvent* e) {
  proxy::events_info::ServerPropertyInfoRequest req(this);
  server_->LoadServerProperty(req);
  base_class::showEvent(e);
}

}  // namespace gui
}  // namespace fastonosql
