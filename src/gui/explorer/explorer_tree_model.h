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

#include <string>

#include "fasto/qt/gui/base/tree_model.h"

#include "core/iserver.h"
#include "core/idatabase.h"

namespace fastonosql {
struct IExplorerTreeItem
  : public fasto::qt::gui::TreeItem {
  enum eColumn {
    eName = 0,
    eCountColumns
  };

  enum eType {
    eCluster,
    eServer,
    eDatabase,
    eKey
  };

  explicit IExplorerTreeItem(TreeItem* parent);
  virtual ~IExplorerTreeItem();

  virtual QString name() const = 0;
  virtual IServerSPtr server() const = 0;
  virtual eType type() const = 0;
};

struct ExplorerServerItem
  : public IExplorerTreeItem {
  ExplorerServerItem(IServerSPtr server, TreeItem* parent);
  virtual ~ExplorerServerItem();

  virtual QString name() const;
  virtual IServerSPtr server() const;
  virtual eType type() const;

  void loadDatabases();

 private:
  const IServerSPtr server_;
};

struct ExplorerClusterItem
  : public IExplorerTreeItem {
  ExplorerClusterItem(IClusterSPtr cluster, TreeItem* parent);
  virtual ~ExplorerClusterItem();

  virtual QString name() const;
  virtual IServerSPtr server() const;
  virtual eType type() const;

  IClusterSPtr cluster() const;

 private:
  const IClusterSPtr cluster_;
};

struct ExplorerDatabaseItem
  : public IExplorerTreeItem {
  ExplorerDatabaseItem(IDatabaseSPtr db, ExplorerServerItem* parent);
  virtual ~ExplorerDatabaseItem();

  ExplorerServerItem* parent() const;

  virtual QString name() const;
  virtual eType type() const;
  bool isDefault() const;
  size_t sizeDB() const;
  size_t loadedSize() const;

  virtual IServerSPtr server() const;
  IDatabaseSPtr db() const;

  void loadContent(const std::string& pattern, uint32_t countKeys);
  void setDefault();

  IDataBaseInfoSPtr info() const;

  void removeKey(const NDbKValue& key);
  void loadValue(const NDbKValue& key);
  void createKey(const NDbKValue& key);

  void removeAllKeys();
 private:
  const IDatabaseSPtr db_;
};

struct ExplorerKeyItem
  : public IExplorerTreeItem {
  ExplorerKeyItem(const NDbKValue& key, ExplorerDatabaseItem* parent);
  virtual ~ExplorerKeyItem();

  ExplorerDatabaseItem* parent() const;

  NDbKValue key() const;

  virtual QString name() const;
  virtual IServerSPtr server() const;
  virtual eType type() const;

  void removeFromDb();
  void loadValueFromDb();

 private:
  NDbKValue key_;
};

class ExplorerTreeModel
  : public fasto::qt::gui::TreeModel {
  Q_OBJECT
 public:
  explicit ExplorerTreeModel(QObject* parent = 0);
  virtual ~ExplorerTreeModel();

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const;
  virtual int columnCount(const QModelIndex &parent) const;

  void addCluster(IClusterSPtr cluster);
  void removeCluster(IClusterSPtr cluster);

  void addServer(IServerSPtr server);
  void removeServer(IServerSPtr server);

  void addDatabase(IServer* server, IDataBaseInfoSPtr db);
  void removeDatabase(IServer* server, IDataBaseInfoSPtr db);
  void setDefaultDb(IServer* server, IDataBaseInfoSPtr db);

  void addKey(IServer* server, IDataBaseInfoSPtr db, const NDbKValue &dbv);
  void removeKey(IServer* server, IDataBaseInfoSPtr db, const NDbKValue &key);
  void removeAllKeys(IServer* server, IDataBaseInfoSPtr db);

 private:
  ExplorerClusterItem* findClusterItem(IClusterSPtr cl);
  ExplorerServerItem* findServerItem(IServer* server) const;
  ExplorerDatabaseItem* findDatabaseItem(ExplorerServerItem* server, IDataBaseInfoSPtr db) const;
  ExplorerKeyItem* findKeyItem(ExplorerDatabaseItem* db, const NDbKValue &key) const;
};

}  // namespace fastonosql
