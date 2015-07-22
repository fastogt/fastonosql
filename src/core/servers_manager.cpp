#include "core/servers_manager.h"

#include "core/settings_manager.h"

#include "core/redis/redis_cluster.h"
#include "core/redis/redis_server.h"
#include "core/redis/redis_driver.h"

#include "core/memcached/memcached_server.h"
#include "core/memcached/memcached_driver.h"

#include "core/ssdb/ssdb_server.h"
#include "core/ssdb/ssdb_driver.h"

namespace fastonosql
{
    ServersManager::ServersManager()
        : syncServers_(SettingsManager::instance().syncTabs())
    {
        qRegisterMetaType<ServerInfoSnapShoot>("ServerInfoSnapShoot");
    }

    ServersManager::~ServersManager()
    {

    }


    IServerSPtr ServersManager::createServer(IConnectionSettingsBaseSPtr settings)
    {
        DCHECK(settings);

        IServerSPtr result;
        connectionTypes conT = settings->connectionType();
        IServerSPtr ser = findServerBySetting(settings);
        if(conT == REDIS){
            RedisServer *newRed = NULL;
            if(!ser){
                IDriverSPtr dr(new RedisDriver(settings));
                dr->start();
                newRed = new RedisServer(dr, true);
            }
            else{
                newRed = new RedisServer(ser->driver(), false);
            }
            result.reset(newRed);
            servers_.push_back(result);
        }
        else if(conT == MEMCACHED){
            MemcachedServer *newMem = NULL;
            if(!ser){
                IDriverSPtr dr(new MemcachedDriver(settings));
                dr->start();
                newMem = new MemcachedServer(dr, true);
            }
            else{
                newMem = new MemcachedServer(ser->driver(), false);
            }
            result.reset(newMem);
            servers_.push_back(result);
        }
        else if(conT == SSDB){
            SsdbServer *newSsdb = NULL;
            if(!ser){
                IDriverSPtr dr(new SsdbDriver(settings));
                dr->start();
                newSsdb = new SsdbServer(dr, true);
            }
            else{
                newSsdb = new SsdbServer(ser->driver(), false);
            }
            result.reset(newSsdb);
            servers_.push_back(result);
        }

        DCHECK(result);
        if(ser && syncServers_){
            result->syncWithServer(ser.get());
        }

        return result;
    }

    IClusterSPtr ServersManager::createCluster(IClusterSettingsBaseSPtr settings)
    {
        DCHECK(settings);

        IClusterSPtr cl;
        connectionTypes conT = settings->connectionType();
        if(conT == REDIS){
            IConnectionSettingsBaseSPtr root = settings->root();
            if(!root){
                return IClusterSPtr();
            }

            cl.reset(new RedisCluster(settings->connectionName()));
            IClusterSettingsBase::cluster_connection_type nodes = settings->nodes();
            for(int i = 0; i < nodes.size(); ++i){
                IConnectionSettingsBaseSPtr nd = nodes[i];
                if(nd){
                    IServerSPtr serv = createServer(nd);
                    cl->addServer(serv);
                }
            }
            IDriverSPtr drv = cl->root()->driver();
            DCHECK(drv->settings() == root);
        }
        else{
            NOTREACHED();
        }
        return cl;
    }

    void ServersManager::setSyncServers(bool isSync)
    {
        syncServers_ = isSync;
        refreshSyncServers();
    }

    void ServersManager::clear()
    {
        for(size_t i = 0; i < servers_.size(); ++i){
            IServerSPtr ser = servers_[i];
            ser->driver()->stop();
        }
        servers_.clear();
    }

    void ServersManager::closeServer(IServerSPtr server)
    {
        for(size_t i = 0; i < servers_.size(); ++i){
            IServerSPtr ser = servers_[i];
            if(ser == server){
                if(ser->isSuperServer()){
                    IDriverSPtr drv = ser->driver();
                    for(size_t j = 0; j < servers_.size(); ++j){
                        IServerSPtr servj = servers_[j];
                        if(servj->driver() == drv){
                            servj->isSuperServer_ = true;
                            break;
                        }
                    }
                }

                servers_.erase(servers_.begin()+i);
                refreshSyncServers();
                break;
            }
        }
    }

    void ServersManager::closeCluster(IClusterSPtr cluster)
    {
        ICluster::nodes_type nodes = cluster->nodes();
        for(int i = 0; i < nodes.size(); ++i){
            closeServer(nodes[i]);
        }
    }

    void ServersManager::refreshSyncServers()
    {
        for(size_t i = 0; i < servers_.size(); ++i){
            IServerSPtr servi = servers_[i];
            if(servi->isSuperServer()){
                for(size_t j = 0; j < servers_.size(); ++j){
                    IServerSPtr servj = servers_[j];
                    if(servj != servi && servj->driver() == servi->driver()){
                        if(syncServers_){
                            servj->syncWithServer(servi.get());
                        }
                        else{
                            servj->unSyncFromServer(servi.get());
                        }
                    }
                }
            }
        }
    }

    IServerSPtr ServersManager::findServerBySetting(const IConnectionSettingsBaseSPtr &settings) const
    {
        for(size_t i = 0; i < servers_.size(); ++i){
            IServerSPtr drp = servers_[i];
            IDriverSPtr curDr = drp->driver();
            if(curDr->settings() == settings){
                return drp;
            }
        }
        return IServerSPtr();
    }

    std::vector<QObject *> ServersManager::findAllListeners(const IDriverSPtr &drv) const
    {
        std::vector<QObject *> result;
        for(size_t j = 0; j < servers_.size(); ++j){
            IServerSPtr ser = servers_[j];
            if(ser->driver() == drv){
                result.push_back(ser.get());
            }
        }
        return result;
    }
}
