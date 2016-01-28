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

#include <QString>

#include "core/types.h"
#include "fasto/qt/gui/base/tree_item.h"

namespace fastonosql {
class FastoCommonItem
  : public fasto::qt::gui::TreeItem
{
 public:
  enum eColumn
  {
    eKey = 0,
    eValue = 1,
    eType = 2,
    eCountColumns = 3
  };
  FastoCommonItem(const QString& key, NValue value, bool isReadOnly,
                  TreeItem* parent, void* internalPointer);

  QString key() const;
  QString value() const;
  common::Value::Type type() const;

  bool isReadOnly() const;

  void setValue(NValue val);

private:
  QString key_;
  NValue value_;
  bool isReadOnly_;
};

QString toJson(FastoCommonItem* item);
QString toRaw(FastoCommonItem* item);
QString toHex(FastoCommonItem* item);
QString toCsv(FastoCommonItem* item, const QString &delemitr);

QString fromGzip(FastoCommonItem* item);
QString fromHexMsgPack(FastoCommonItem* item);
}

