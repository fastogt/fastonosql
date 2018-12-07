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
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/models/clients_table_model.h"

#include <QIcon>

#include <common/qt/convert2string.h>
#include <common/qt/utils_qt.h>

#include "gui/gui_factory.h"
#include "gui/models/items/client_table_item.h"

namespace {
const QString trId = QObject::tr("ID");
const QString trAddr = QObject::tr("Addr");
const QString trFd = QObject::tr("Fd");
const QString trName = QObject::tr("Name");
const QString trAge = QObject::tr("Age");
const QString trIdle = QObject::tr("Idle");
const QString trFlags = QObject::tr("Flags");
const QString trDb = QObject::tr("DB");
const QString trSub = QObject::tr("Sub");
const QString trPsub = QObject::tr("Psub");
const QString trMulti = QObject::tr("Multi");
const QString trQbuf = QObject::tr("Qbuf");
const QString trQbufFree = QObject::tr("QbufFree");
const QString trOdl = QObject::tr("Odl");
const QString trOll = QObject::tr("Oll");
const QString trOmem = QObject::tr("Omem");
const QString trEvents = QObject::tr("Events");
const QString trCmd = QObject::tr("Cmd");
}  // namespace

namespace fastonosql {
namespace gui {

ClientsTableModel::ClientsTableModel(QObject* parent) : TableModel(parent) {}

ClientsTableModel::~ClientsTableModel() {}

QVariant ClientsTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  ClientTableItem* node = common::qt::item<common::qt::gui::TableItem*, ClientTableItem*>(index);
  if (!node) {
    return QVariant();
  }

  int col = index.column();
  QVariant result;
  if (role == Qt::DisplayRole) {
    const proxy::NDbClient client = node->client();
    if (col == kId) {
      result = client.GetId();
    } else if (col == kAddr) {
      QString result_str;
      common::ConvertFromString(common::ConvertToString(client.GetAddr()), &result_str);
      result = result_str;
    } else if (col == kFd) {
      result = client.GetFd();
    } else if (col == kName) {
      QString result_str;
      common::ConvertFromString(client.GetName(), &result_str);
      result = result_str;
    } else if (col == kAge) {
      result = client.GetAge();
    } else if (col == kIdle) {
      result = client.GetIdle();
    } else if (col == kFlags) {
      QString result_str;
      common::ConvertFromString(client.GetFlags(), &result_str);
      result = result_str;
    } else if (col == kDb) {
      result = client.GetDb();
    } else if (col == kSub) {
      result = client.GetSub();
    } else if (col == kPsub) {
      result = client.GetPSub();
    } else if (col == kMulti) {
      result = client.GetMulti();
    } else if (col == kQbuf) {
      result = client.GetQbuf();
    } else if (col == kQbufFree) {
      result = client.GetQbufFree();
    } else if (col == kOdl) {
      result = client.GetOdl();
    } else if (col == kOll) {
      result = client.GetOll();
    } else if (col == kOmem) {
      result = client.GetOmem();
    } else if (col == kEvents) {
      QString result_str;
      common::ConvertFromString(client.GetEvents(), &result_str);
      result = result_str;
    } else if (col == kCmd) {
      QString result_str;
      common::ConvertFromString(client.GetCmd(), &result_str);
      result = result_str;
    }
  }

  return result;
}

QVariant ClientsTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == kId) {
      return trId;
    } else if (section == kAddr) {
      return trAddr;
    } else if (section == kFd) {
      return trFd;
    } else if (section == kName) {
      return trName;
    } else if (section == kAge) {
      return trAge;
    } else if (section == kIdle) {
      return trIdle;
    } else if (section == kFlags) {
      return trFlags;
    } else if (section == kDb) {
      return trDb;
    } else if (section == kSub) {
      return trSub;
    } else if (section == kPsub) {
      return trPsub;
    } else if (section == kMulti) {
      return trMulti;
    } else if (section == kQbuf) {
      return trQbuf;
    } else if (section == kQbufFree) {
      return trQbufFree;
    } else if (section == kOdl) {
      return trOdl;
    } else if (section == kOll) {
      return trOll;
    } else if (section == kOmem) {
      return trOmem;
    } else if (section == kEvents) {
      return trEvents;
    } else if (section == kCmd) {
      return trCmd;
    }
  }

  return TableModel::headerData(section, orientation, role);
}

int ClientsTableModel::columnCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return kCountColumns;
}

void ClientsTableModel::clear() {
  beginResetModel();
  clearData();
  endResetModel();
}

}  // namespace gui
}  // namespace fastonosql
