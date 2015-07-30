#pragma once

#include "common/patterns/singleton_pattern.h"

#include "core/iserver.h"
#include "core/connection_settings.h"

namespace fastonosql
{
    class ServersManager
            : public QObject, public common::patterns::LazySingleton<ServersManager>
    {
        friend class common::patterns::LazySingleton<ServersManager>;
        Q_OBJECT

    public:
        typedef std::vector<IServerSPtr> ServersContainer;

        IServerSPtr createServer(IConnectionSettingsBaseSPtr settings);
        IClusterSPtr createCluster(IClusterSettingsBaseSPtr settings);

        void setSyncServers(bool isSync);        
        void clear();

    public Q_SLOTS:
        void closeServer(IServerSPtr server);
        void closeCluster(IClusterSPtr cluster);

    private:
        template<class Server, class Driver>
        IServer* make_server(IServerSPtr pser, IConnectionSettingsBaseSPtr settings);

        ServersManager();
        ~ServersManager();

        void refreshSyncServers();
        IServerSPtr findServerBySetting(const IConnectionSettingsBaseSPtr& settings) const;
        std::vector<QObject*> findAllListeners(const IDriverSPtr& drv) const;

        ServersContainer servers_;
        bool syncServers_;
    };
}
