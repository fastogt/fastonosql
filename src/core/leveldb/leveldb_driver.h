#pragma once

#include "core/idriver.h"

#include "core/leveldb/leveldb_settings.h"

namespace fastonosql
{
    static const CommandInfo leveldbCommands[] =
    {
        CommandInfo("PUT", "<key> <value>",
                    "Set the value of a key.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("GET", "<key>",
                    "Get the value of a key.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("DEL", "<key>",
                    "Delete key.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("KEYS", "<key_start> <key_end> <limit>",
                    "Find all keys matching the given limits.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("INFO", "<args>",
                    "These command return database information.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("QUIT", "-",
                    "Close the connection.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0)
    };

    common::ErrorValueSPtr testConnection(LeveldbConnectionSettings* settings);

    class LeveldbDriver
            : public IDriver
    {
        Q_OBJECT
    public:        

        explicit LeveldbDriver(IConnectionSettingsBaseSPtr settings);
        virtual ~LeveldbDriver();

        virtual bool isConnected() const;
        virtual bool isAuthenticated() const;
        virtual void interrupt();
        common::net::hostAndPort address() const;
        virtual std::string outputDelemitr() const;

        static const char* versionApi();

    private:
        virtual void customEvent(QEvent *event);
        virtual void initImpl();
        virtual void clearImpl();

        virtual common::ErrorValueSPtr serverInfo(ServerInfo** info);
        virtual common::ErrorValueSPtr serverDiscoveryInfo(ServerInfo** sinfo, ServerDiscoveryInfo** dinfo, DataBaseInfo** dbinfo);
        virtual common::ErrorValueSPtr currentDataBaseInfo(DataBaseInfo** info);

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
        virtual common::ErrorValueSPtr commandChangeTTLImpl(CommandChangeTTL* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
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
