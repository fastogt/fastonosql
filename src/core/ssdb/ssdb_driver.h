#pragma once

#include "core/idriver.h"

#include "core/ssdb/ssdb_settings.h"

namespace fastoredis
{
    static const QString ssdbCommandsKeywords[] = { "quit", "set", "get", "setx", "del", "incr", "keys", "scan", "rscan", "multi_get", "multi_set", "multi_del",
                                                        "hset", "hget", "hdel", "hincr", "hsize", "hclear", "hkeys", "hscan", "hrscan", "multi_hget", "multi_hset",
                                                        "zset", "zget", "zdel", "zincr", "zsize", "zclear", "zrank", "zrrank", "zrange", "zrrange",
                                                        "zkeys", "zscan", "zrscan", "multi_zget", "multi_zset", "multi_zdel", "info" };

    common::ErrorValueSPtr testConnection(SsdbConnectionSettings* settings);

    class SsdbDriver
            : public IDriver
    {
        Q_OBJECT
    public:        

        SsdbDriver(IConnectionSettingsBaseSPtr settings);
        virtual ~SsdbDriver();

        virtual bool isConnected() const;
        virtual bool isAuthenticated() const;
        virtual void interrupt();
        common::net::hostAndPort address() const;
        std::string version() const;
        virtual std::string outputDelemitr() const;

        static const char* versionApi();

    private:
        virtual void customEvent(QEvent *event);
        virtual void initImpl();
        virtual void clearImpl();

        virtual common::ErrorValueSPtr currentLoggingInfo(ServerInfo** info);
        virtual common::ErrorValueSPtr serverDiscoveryInfo(ServerDiscoveryInfo** dinfo);\

        virtual void handleConnectEvent(events::ConnectRequestEvent* ev);
        virtual void handleDisconnectEvent(events::DisconnectRequestEvent* ev);
        virtual void handleExecuteEvent(events::ExecuteRequestEvent* ev);
        virtual void handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent* ev);
        virtual void handleLoadServerInfoEvent(events::ServerInfoRequestEvent* ev);
        virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev);

// ============== commands =============//
        virtual common::ErrorValueSPtr commandDeleteImpl(CommandDeleteKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
        virtual common::ErrorValueSPtr commandLoadImpl(CommandLoadKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
        virtual common::ErrorValueSPtr commandCreateImpl(CommandCreateKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
// ============== commands =============//

// ============== database =============//
        virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev);
        virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);
// ============== database =============//
// ============== command =============//
        virtual void handleCommandRequestEvent(events::CommandRequestEvent* ev);
// ============== command =============//
        ServerInfoSPtr makeServerInfoFromString(const std::string& val);

        struct pimpl;
        pimpl* const impl_;
    };
}
