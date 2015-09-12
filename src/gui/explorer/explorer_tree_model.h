#pragma once

#include "fasto/qt/gui/base/tree_model.h"

#include "core/iserver.h"
#include "core/idatabase.h"

namespace fastonosql
{
    struct IExplorerTreeItem
            : public fasto::qt::gui::TreeItem
    {
        enum eColumn
        {
            eName = 0,
            eCountColumns
        };

        enum eType
        {
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
            : public IExplorerTreeItem
    {
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
            : public IExplorerTreeItem
    {
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
            : public IExplorerTreeItem
    {
        ExplorerDatabaseItem(IDatabaseSPtr db, ExplorerServerItem* parent);
        virtual ~ExplorerDatabaseItem();

        ExplorerServerItem* parent() const;

        virtual QString name() const;
        virtual eType type() const;       
        bool isDefault() const;
        size_t size() const;

        virtual IServerSPtr server() const;
        IDatabaseSPtr db() const;

        void loadContent(const std::string& pattern, uint32_t countKeys);
        void setDefault();

        DataBaseInfoSPtr info() const;

        void removeKey(const NDbValue& key);
        void loadValue(const NDbValue& key);
        void createKey(const NDbValue& key);

    private:
        const IDatabaseSPtr db_;
    };

    struct ExplorerKeyItem
            : public IExplorerTreeItem
    {
        ExplorerKeyItem(const NDbValue& key, ExplorerDatabaseItem* parent);
        virtual ~ExplorerKeyItem();

        ExplorerDatabaseItem* parent() const;

        NDbValue key() const;

        virtual QString name() const;        
        virtual IServerSPtr server() const;
        virtual eType type() const;

        void removeFromDb();
        void loadValueFromDb();

    private:
        NDbValue key_;
    };

    class ExplorerTreeModel
            : public fasto::qt::gui::TreeModel
    {
        Q_OBJECT
    public:
        explicit ExplorerTreeModel(QObject* parent = 0);
        virtual ~ExplorerTreeModel();

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        virtual int columnCount(const QModelIndex &parent) const;

        void addCluster(IClusterSPtr cluster);
        void removeCluster(IClusterSPtr cluster);

        void addServer(IServerSPtr server);
        void removeServer(IServerSPtr server);

        void addDatabase(IServer* server, DataBaseInfoSPtr db);
        void removeDatabase(IServer* server, DataBaseInfoSPtr db);
        void setDefaultDb(IServer* server, DataBaseInfoSPtr db);

        void addKey(IServer* server, DataBaseInfoSPtr db, const NDbValue &dbv);
        void removeKey(IServer* server, DataBaseInfoSPtr db, const NDbValue &key);

    private:
        ExplorerClusterItem* findClusterItem(IClusterSPtr cl);
        ExplorerServerItem* findServerItem(IServer* server) const;
        ExplorerDatabaseItem* findDatabaseItem(ExplorerServerItem* server, DataBaseInfoSPtr db) const;
        ExplorerKeyItem* findKeyItem(ExplorerDatabaseItem* db, const NDbValue &key) const;
    };
}


