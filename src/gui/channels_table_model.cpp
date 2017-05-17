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

#include "gui/channels_table_model.h"

#include <stddef.h>  // for size_t
#include <vector>    // for vector

#include <QIcon>

#include <common/qt/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for CHECK, UNUSED
#include <common/qt/utils_qt.h>     // for item

#include <common/qt/gui/base/table_item.h>   // for TableItem
#include <common/qt/gui/base/table_model.h>  // for TableModel

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trKey, trTTL, trType

namespace fastonosql {
namespace gui {

ChannelTableItem::ChannelTableItem(const core::NDbPSChannel& chan) : channel_(chan) {}

core::NDbPSChannel ChannelTableItem::channel() const {
  return channel_;
}

QString ChannelTableItem::name() const {
  QString qname;
  common::ConvertFromString(channel_.Name(), &qname);
  return qname;
}

uint32_t ChannelTableItem::numberOfSubscribers() const {
  return channel_.NumberOfSubscribers();
}

ChannelsTableModel::ChannelsTableModel(QObject* parent) : TableModel(parent) {}

ChannelsTableModel::~ChannelsTableModel() {}

QVariant ChannelsTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  ChannelTableItem* node = common::qt::item<common::qt::gui::TableItem*, ChannelTableItem*>(index);
  if (!node) {
    return QVariant();
  }

  int col = index.column();
  if (role == Qt::DecorationRole && col == ChannelTableItem::kName) {
    return GuiFactory::Instance().channelIcon();
  }

  QVariant result;
  if (role == Qt::DisplayRole) {
    if (col == ChannelTableItem::kName) {
      result = node->name();
    } else if (col == ChannelTableItem::kNOS) {
      result = node->numberOfSubscribers();
    }
  }

  return result;
}

QVariant ChannelsTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == ChannelTableItem::kName) {
      return translations::trName;
    } else if (section == ChannelTableItem::kNOS) {
      return translations::trNumberOfSubscribers;
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int ChannelsTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return ChannelTableItem::kCountColumns;
}

void ChannelsTableModel::clear() {
  beginResetModel();
  for (size_t i = 0; i < data_.size(); ++i) {
    delete data_[i];
  }
  data_.clear();
  endResetModel();
}

}  // namespace gui
}  // namespace fastonosql
