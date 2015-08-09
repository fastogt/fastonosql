#include "gui/explorer/explorer_tree_model.h"

#include "translations/global.h"

#include "gui/gui_factory.h"

#include "common/qt/utils_qt.h"
#include "common/qt/convert_string.h"

#include "core/icluster.h"

namespace fastonosql
{
    IExplorerTreeItem::IExplorerTreeItem(TreeItem* parent)
        : TreeItem(parent)
    {

    }

    IExplorerTreeItem::~IExplorerTreeItem()
    {

    }

    ExplorerServerItem::ExplorerServerItem(IServerSPtr server, TreeItem* parent)
        : IExplorerTreeItem(parent), server_(server)
    {

    }

    ExplorerServerItem::~ExplorerServerItem()
    {

    }


    QString ExplorerServerItem::name() const
    {
        return server_->name();
    }

    IServerSPtr ExplorerServerItem::server() const
    {
        return server_;
    }

    ExplorerServerItem::eType ExplorerServerItem::type() const
    {
        return eServer;
    }

    void ExplorerServerItem::loadDatabases()
    {
         return server_->loadDatabases();
    }

    ExplorerClusterItem::ExplorerClusterItem(IClusterSPtr cluster, TreeItem* parent)
        : IExplorerTreeItem(parent), cluster_(cluster)
    {
        ICluster::nodes_type nodes = cluster_->nodes();
        for(int i = 0; i < nodes.size(); ++i){
            ExplorerServerItem* ser = new ExplorerServerItem(nodes[i], this);
            addChildren(ser);
        }
    }

    ExplorerClusterItem::~ExplorerClusterItem()
    {

    }

    QString ExplorerClusterItem::name() const
    {
        return cluster_->name();
    }

    IServerSPtr ExplorerClusterItem::server() const
    {
        return cluster_->root();
    }

    ExplorerClusterItem::eType ExplorerClusterItem::type() const
    {
        return eCluster;
    }

    IClusterSPtr ExplorerClusterItem::cluster() const
    {
        return cluster_;
    }

    ExplorerDatabaseItem::ExplorerDatabaseItem(IDatabaseSPtr db, ExplorerServerItem* parent)
        : IExplorerTreeItem(parent), db_(db)
    {
        DCHECK(db);
    }

    ExplorerDatabaseItem::~ExplorerDatabaseItem()
    {

    }

    ExplorerServerItem *ExplorerDatabaseItem::parent() const
    {
        return dynamic_cast<ExplorerServerItem*>(IExplorerTreeItem::parent());
    }

    QString ExplorerDatabaseItem::name() const
    {
        return common::convertFromString<QString>(db_->name());
    }

    ExplorerDatabaseItem::eType ExplorerDatabaseItem::type() const
    {
        return eDatabase;
    }

    bool ExplorerDatabaseItem::isDefault() const
    {
        return info()->isDefault();
    }

    size_t ExplorerDatabaseItem::size() const
    {
        return info()->size();
    }

    IServerSPtr ExplorerDatabaseItem::server() const
    {
        ExplorerServerItem* serv = dynamic_cast<ExplorerServerItem*>(parent_);
        if(!serv){
            return IServerSPtr();
        }

        return serv->server();
    }

    IDatabaseSPtr ExplorerDatabaseItem::db() const
    {
        return db_;
    }

    void ExplorerDatabaseItem::loadContent(const std::string& pattern, uint32_t countKeys)
    {
        IDatabaseSPtr dbs = db();
        if(dbs){
            dbs->loadContent(pattern, countKeys);
        }
    }

    void ExplorerDatabaseItem::setDefault()
    {
        IDatabaseSPtr dbs = db();
        if(dbs){
            dbs->setDefault();
        }
    }

    DataBaseInfoSPtr ExplorerDatabaseItem::info() const
    {
        return db_->info();
    }

    void ExplorerDatabaseItem::removeKey(const NKey& key)
    {
        IDatabaseSPtr dbs = db();
        if(dbs){
            dbs->removeKey(key);
        }
    }

    void ExplorerDatabaseItem::loadValue(const NKey& key)
    {
        IDatabaseSPtr dbs = db();
        if(dbs){
            dbs->loadValue(key);
        }
    }

    void ExplorerDatabaseItem::createKey(const NKey& key, FastoObjectIPtr value)
    {
        IDatabaseSPtr dbs = db();
        if(dbs){
            dbs->createKey(key, value);
        }
    }

    ExplorerKeyItem::ExplorerKeyItem(const NKey& key, ExplorerDatabaseItem* parent)
        : IExplorerTreeItem(parent), key_(key)
    {

    }

    ExplorerKeyItem::~ExplorerKeyItem()
    {

    }

    ExplorerDatabaseItem* ExplorerKeyItem::parent() const
    {
        return dynamic_cast<ExplorerDatabaseItem*>(parent_);
    }

    NKey ExplorerKeyItem::key() const
    {
        return key_;
    }

    QString ExplorerKeyItem::name() const
    {
        return common::convertFromString<QString>(key_.key_);
    }

    IServerSPtr ExplorerKeyItem::server() const
    {
        ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(parent_);
        if(!db){
            return IServerSPtr();
        }

        return db->server();
    }

    IExplorerTreeItem::eType ExplorerKeyItem::type() const
    {
        return eKey;
    }

    void ExplorerKeyItem::removeFromDb()
    {
        ExplorerDatabaseItem* par = parent();
        if(par){
            par->removeKey(key_);
        }
    }

    void ExplorerKeyItem::loadValueFromDb()
    {
        ExplorerDatabaseItem* par = parent();
        if(par){
            par->loadValue(key_);
        }
    }

    ExplorerTreeModel::ExplorerTreeModel(QObject *parent)
        : TreeModel(parent)
    {
    }

    ExplorerTreeModel::~ExplorerTreeModel()
    {

    }

    QVariant ExplorerTreeModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid()){
            return QVariant();
        }

        IExplorerTreeItem* node = common::utils_qt::item<IExplorerTreeItem*>(index);

        if (!node){
            return QVariant();
        }

        int col = index.column();

        IExplorerTreeItem::eType t = node->type();

        if(role == Qt::ToolTipRole){
            IServerSPtr serv = node->server();
            if(t == IExplorerTreeItem::eServer && serv){
                ServerDiscoveryInfoSPtr disc = serv->discoveryInfo();
                if(disc){
                    QString dname = common::convertFromString<QString>(disc->name());
                    QString dtype = common::convertFromString<QString>(common::convertToString(disc->type()));
                    QString dhost = common::convertFromString<QString>(common::convertToString(disc->host()));
                    return QString("<b>Name:</b> %1<br/>"
                                   "<b>Type:</b> %2<br/>"
                                   "<b>Host:</b> %3<br/>").arg(dname).arg(dtype).arg(dhost);
                }
            }
            else if(t == IExplorerTreeItem::eDatabase){
                ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(node);
                if(db && db->isDefault()){
                    return QString("<b>Db size:</b> %1 keys<br/>").arg(db->size());
                }
            }
        }

        if(role == Qt::DecorationRole && col == ExplorerServerItem::eName ){
            if(t == IExplorerTreeItem::eCluster){
                return GuiFactory::instance().clusterIcon();
            }
            else if(t == IExplorerTreeItem::eServer){
                return GuiFactory::instance().icon(node->server()->type());
            }
            else if(t == IExplorerTreeItem::eKey){
                return GuiFactory::instance().keyIcon();
            }
            else if(t == IExplorerTreeItem::eDatabase){
                return GuiFactory::instance().databaseIcon();
            }
            else{
                NOTREACHED();
            }
        }

        if (role == Qt::DisplayRole) {
            if (col == IExplorerTreeItem::eName) {
                if(t == IExplorerTreeItem::eKey){
                    return node->name();
                }
                else{
                    return QString("%1 (%2)").arg(node->name()).arg(node->childrenCount());
                }
            }
        }

        if(role == Qt::ForegroundRole){
            if(t == IExplorerTreeItem::eDatabase){
                ExplorerDatabaseItem* db = dynamic_cast<ExplorerDatabaseItem*>(node);
                if(db && db->isDefault()){
                    return QVariant( QColor( Qt::red ) );
                }
            }
        }

        return QVariant();
    }

    Qt::ItemFlags ExplorerTreeModel::flags(const QModelIndex& index) const
    {
        Qt::ItemFlags result = 0;
        if (index.isValid()) {
            result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        }
        return result;
    }

    QVariant ExplorerTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == ExplorerServerItem::eName) {
                return translations::trName;
            }
        }

        return TreeModel::headerData(section,orientation,role);
    }

    int ExplorerTreeModel::columnCount(const QModelIndex& parent) const
    {
        return ExplorerServerItem::eCountColumns;
    }

    void ExplorerTreeModel::addCluster(IClusterSPtr cluster)
    {
        ExplorerClusterItem* cl = findClusterItem(cluster);
        if(!cl){
            fasto::qt::gui::TreeItem *parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
            DCHECK(parent);
            if(!parent){
                return;
            }

            ExplorerClusterItem* item = new ExplorerClusterItem(cluster, parent);
            insertItem(QModelIndex(), item);
        }
    }

    void ExplorerTreeModel::removeCluster(IClusterSPtr cluster)
    {
        fasto::qt::gui::TreeItem *par = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
        DCHECK(par);
        if(!par){
            return;
        }

        ExplorerClusterItem *serverItem = findClusterItem(cluster);
        if(serverItem){
            removeItem(QModelIndex(), serverItem);
        }
    }

    void ExplorerTreeModel::addServer(IServerSPtr server)
    {
        if(!server){
            return;
        }

        ExplorerServerItem *serv = findServerItem(server.get());
        if(!serv){
            fasto::qt::gui::TreeItem *parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
            DCHECK(parent);
            if(!parent){
                return;
            }

            ExplorerServerItem *item = new ExplorerServerItem(server, parent);
            insertItem(QModelIndex(), item);
        }
    }

    void ExplorerTreeModel::removeServer(IServerSPtr server)
    {
        fasto::qt::gui::TreeItem *par = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
        DCHECK(par);
        if(!par){
            return;
        }

        ExplorerServerItem *serverItem = findServerItem(server.get());
        if(serverItem){
            removeItem(QModelIndex(), serverItem);
        }
    }

    void ExplorerTreeModel::addDatabase(IServer* server, DataBaseInfoSPtr db)
    {
        ExplorerServerItem *parent = findServerItem(server);
        DCHECK(parent);
        if(!parent){
            return;
        }

        ExplorerDatabaseItem *dbs = findDatabaseItem(parent, db);
        if(!dbs){
            QModelIndex index = createIndex(0, 0, parent);
            ExplorerDatabaseItem *item = new ExplorerDatabaseItem(server->findDatabaseByInfo(db), parent);
            insertItem(index, item);
        }
    }

    void ExplorerTreeModel::removeDatabase(IServer* server, DataBaseInfoSPtr db)
    {
        ExplorerServerItem *parent = findServerItem(server);
        DCHECK(parent);
        if(!parent){
            return;
        }

        ExplorerDatabaseItem *dbs = findDatabaseItem(parent, db);
        if(dbs){
            removeItem(createIndex(0,0,parent), dbs);
        }
    }

    void ExplorerTreeModel::setDefaultDb(IServer* server, DataBaseInfoSPtr db)
    {
        ExplorerServerItem *parent = findServerItem(server);
        DCHECK(parent);
        if(!parent){
            return;
        }

        int child_count = parent->childrenCount();
        for(int i = 0; i < child_count ; ++i){
            ExplorerDatabaseItem *item = dynamic_cast<ExplorerDatabaseItem*>(parent->child(i));
            DCHECK(item);
            if(!item){
                continue;
            }

            DataBaseInfoSPtr info = item->info();
            if(info->isDefault()){
                if(info->name() != db->name()){
                    info->setIsDefault(false);
                    updateItem(createIndex(i,0,parent), createIndex(i,0,parent));
                }
            }
            else{
                if(info->name() == db->name()){
                    info->setIsDefault(true);
                    updateItem(createIndex(i,0,parent), createIndex(i,0,parent));
                }
            }
        }
    }

    void ExplorerTreeModel::addKey(IServer* server, DataBaseInfoSPtr db, const NKey &key)
    {
        ExplorerServerItem *parent = findServerItem(server);
        DCHECK(parent);
        if(!parent){
            return;
        }

        ExplorerDatabaseItem *dbs = findDatabaseItem(parent, db);
        DCHECK(dbs);
        if(!dbs){
            return;
        }

        ExplorerKeyItem *keyit = findKeyItem(dbs, key);
        if(!keyit){
            QModelIndex parentdb = createIndex(parent->indexOf(dbs), 0, dbs);
            ExplorerKeyItem *item = new ExplorerKeyItem(key, dbs);
            insertItem(parentdb, item);
        }
    }

    void ExplorerTreeModel::removeKey(IServer* server, DataBaseInfoSPtr db, const NKey &key)
    {
        ExplorerServerItem *parent = findServerItem(server);
        DCHECK(parent);
        if(!parent){
            return;
        }

        ExplorerDatabaseItem *dbs = findDatabaseItem(parent, db);
        DCHECK(dbs);
        if(!dbs){
            return;
        }

        ExplorerKeyItem *keyit = findKeyItem(dbs, key);
        if(keyit){
            QModelIndex parentdb = createIndex(parent->indexOf(dbs), 0, dbs);
            removeItem(parentdb, keyit);
        }
    }

    ExplorerClusterItem* ExplorerTreeModel::findClusterItem(IClusterSPtr cl)
    {
        fasto::qt::gui::TreeItem *parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
        DCHECK(parent);
        if(!parent){
            return NULL;
        }

        for(int i = 0; i < parent->childrenCount() ; ++i){
            ExplorerClusterItem *item = dynamic_cast<ExplorerClusterItem*>(parent->child(i));
            if(item && item->cluster() == cl){
                return item;
            }
        }
        return NULL;
    }

    ExplorerServerItem* ExplorerTreeModel::findServerItem(IServer* server) const
    {
        fasto::qt::gui::TreeItem *parent = dynamic_cast<fasto::qt::gui::TreeItem*>(root_);
        DCHECK(parent);
        if(!parent){
            return NULL;
        }

        for(int i = 0; i < parent->childrenCount(); ++i){
            ExplorerServerItem *item = dynamic_cast<ExplorerServerItem*>(parent->child(i));
            if(item){
                if(item->server().get() == server){
                    return item;
                }
            }
            else{
                ExplorerClusterItem* citem = dynamic_cast<ExplorerClusterItem*>(parent->child(i));
                if(citem){
                    for(int j = 0; j < citem->childrenCount(); ++j){
                        ExplorerServerItem *item = dynamic_cast<ExplorerServerItem*>(citem->child(i));
                        if(item){
                            if(item->server().get() == server){
                                return item;
                            }
                        }
                    }
                }
            }
        }
        return NULL;
    }

    ExplorerDatabaseItem *ExplorerTreeModel::findDatabaseItem(ExplorerServerItem* server, DataBaseInfoSPtr db) const
    {
        if(server){
            for(int i = 0; i < server->childrenCount() ; ++i){
                ExplorerDatabaseItem *item = dynamic_cast<ExplorerDatabaseItem*>(server->child(i));
                DCHECK(item);
                if(!item){
                    continue;
                }

                IDatabaseSPtr inf = item->db();
                if(inf && inf->name() == db->name()){
                    return item;
                }
            }
        }
        return NULL;
    }

    ExplorerKeyItem *ExplorerTreeModel::findKeyItem(ExplorerDatabaseItem* db, const NKey& key) const
    {
        if(db){
            for(int i = 0; i < db->childrenCount() ; ++i){
                ExplorerKeyItem *item = dynamic_cast<ExplorerKeyItem*>(db->child(i));
                DCHECK(item);
                if(!item){
                    continue;
                }

                if(item->key() == key){
                    return item;
                }
            }
        }
        return NULL;
    }
}
