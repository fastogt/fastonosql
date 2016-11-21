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

#include <QWidget>

#include "gui/widgets/connection_base_widget.h"

#include "core/connection_settings/iconnection_settings_local.h"

namespace {
const QString trDBPath = QObject::tr("Database path:");
const QString trCaption = QObject::tr("Select Database path");
const QString trFilter = QObject::tr("Database files (*.*)");
const QString trCreateDBIfMissing = QObject::tr("Create database");
const QString trReadOnlyDB = QObject::tr("Read only database");
}

namespace fastonosql {
namespace gui {

class PathWidget;

class ConnectionLocalWidget : public ConnectionBaseWidget {
  Q_OBJECT
 public:
  explicit ConnectionLocalWidget(bool isFolderSelectOnly,
                                 const QString& pathTitle,
                                 const QString& caption,
                                 const QString& filter,
                                 QWidget* parent = 0);

  virtual void syncControls(core::IConnectionSettingsBase* connection) override;
  virtual void retranslateUi() override;
  virtual bool validated() const override;

 protected:
  virtual core::IConnectionSettingsBase* createConnectionImpl(
      const core::connection_path_t& path) const override final;

  virtual core::IConnectionSettingsLocal* createConnectionLocalImpl(
      const core::connection_path_t& path) const = 0;

 private:
  PathWidget* pathWidget_;
};

}  // namespace gui
}  // namespace fastonosql
