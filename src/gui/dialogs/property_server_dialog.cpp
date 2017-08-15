/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include "gui/dialogs/property_server_dialog.h"

#include <QHBoxLayout>
#include <QTableView>

#include <common/qt/convert2string.h>    // for ConvertFromString
#include <common/qt/gui/glass_widget.h>  // for GlassWidget

#include "proxy/events/events_info.h"
#include "proxy/server/iserver.h"  // for IServer

#include "gui/gui_factory.h"  // for GuiFactory
#include "gui/property_table_item.h"
#include "gui/property_table_model.h"  // for PropertyTableModel, etc

#include "translations/global.h"  // for trLoading

namespace {
const QString trPropertiesTemplate_1S = QObject::tr("%1 properties");
}

namespace fastonosql {
namespace gui {

PropertyServerDialog::PropertyServerDialog(proxy::IServerSPtr server, QWidget* parent)
    : QDialog(parent), server_(server) {
  CHECK(server_);

  setWindowIcon(GuiFactory::GetInstance().icon(server->Type()));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  PropertyTableModel* mod = new PropertyTableModel(this);
  propertyes_table_ = new QTableView;
  VERIFY(connect(mod, &PropertyTableModel::propertyChanged, this, &PropertyServerDialog::changedProperty));
  propertyes_table_->setModel(mod);

  QHBoxLayout* mainL = new QHBoxLayout;
  mainL->addWidget(propertyes_table_);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainL);

  glassWidget_ = new common::qt::gui::GlassWidget(GuiFactory::GetInstance().pathToLoadingGif(), translations::trLoading,
                                                  0.5, QColor(111, 111, 100), this);

  VERIFY(connect(server.get(), &proxy::IServer::LoadServerPropertyStarted, this,
                 &PropertyServerDialog::startServerProperty));
  VERIFY(connect(server.get(), &proxy::IServer::LoadServerPropertyFinished, this,
                 &PropertyServerDialog::finishServerProperty));
  VERIFY(connect(server.get(), &proxy::IServer::ChangeServerPropertyStarted, this,
                 &PropertyServerDialog::startServerChangeProperty));
  VERIFY(connect(server.get(), &proxy::IServer::ChangeServerPropertyFinished, this,
                 &PropertyServerDialog::finishServerChangeProperty));
  retranslateUi();
}

void PropertyServerDialog::startServerProperty(const proxy::events_info::ServerPropertyInfoRequest& req) {
  UNUSED(req);

  glassWidget_->start();
}

void PropertyServerDialog::finishServerProperty(const proxy::events_info::ServerPropertyInfoResponce& res) {
  glassWidget_->stop();
  common::Error er = res.errorInfo();
  if (er && er->IsError()) {
    return;
  }

  if (server_->Type() == core::REDIS) {
    core::ServerPropertiesInfo inf = res.info;
    PropertyTableModel* model = qobject_cast<PropertyTableModel*>(propertyes_table_->model());
    for (size_t i = 0; i < inf.properties.size(); ++i) {
      core::property_t it = inf.properties[i];
      model->insertItem(new PropertyTableItem(it));
    }
  }
}

void PropertyServerDialog::startServerChangeProperty(const proxy::events_info::ChangeServerPropertyInfoRequest& req) {
  UNUSED(req);
}

void PropertyServerDialog::finishServerChangeProperty(const proxy::events_info::ChangeServerPropertyInfoResponce& res) {
  common::Error er = res.errorInfo();
  if (er && er->IsError()) {
    return;
  }

  if (server_->Type() == core::REDIS) {
    core::property_t pr = res.new_item;
    if (res.is_change) {
      PropertyTableModel* model = qobject_cast<PropertyTableModel*>(propertyes_table_->model());
      model->changeProperty(pr);
    }
  }
}

void PropertyServerDialog::changedProperty(const core::property_t& prop) {
  proxy::events_info::ChangeServerPropertyInfoRequest req(this, prop);
  server_->ChangeProperty(req);
}

void PropertyServerDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void PropertyServerDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  proxy::events_info::ServerPropertyInfoRequest req(this);
  server_->ServerProperty(req);
}

void PropertyServerDialog::retranslateUi() {
  QString name;
  if (common::ConvertFromString(server_->Name(), &name)) {
    setWindowTitle(trPropertiesTemplate_1S.arg(name));
  }
}

}  // namespace gui
}  // namespace fastonosql
