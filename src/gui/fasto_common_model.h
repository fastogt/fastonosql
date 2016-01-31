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

#include "fasto/qt/gui/base/tree_model.h"

#include "core/types.h"

namespace fastonosql {

class FastoCommonModel
  : public fasto::qt::gui::TreeModel {
  Q_OBJECT
public:
  explicit FastoCommonModel(QObject *parent = 0);
  virtual ~FastoCommonModel();

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  virtual int columnCount(const QModelIndex& parent) const;

  void changeValue(const NDbKValue& value);

 Q_SIGNALS:
  void changedValue(CommandKeySPtr cmd);
};

}
