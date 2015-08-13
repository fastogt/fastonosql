#pragma once

#include "core/idriver.h"

#include "core/memcached/memcached_settings.h"

namespace fastonosql
{
    //TODO: cas command implementation
    static const CommandInfo memcachedCommands[] =
    {
        CommandInfo("QUIT", "-", "Close the connection."),
        CommandInfo("VERBOSITY", "<level>", "Change the verbosity ouptut of Memcached server."),
        CommandInfo("VERSION", "-", "Return the Memcached server version."),
        CommandInfo("STATS", "[<args>]", "These command can return various stats that we will explain."),
        CommandInfo("FLUSH_ALL", "[<time>]", "Flush the server key/value pair (invalidating them) after a optional [<time>] period.\n"
                                             "It always return OK"),
        CommandInfo("DELETE", "<key> [<time>]", "Delete key/value pair in Memcached"),
        CommandInfo("INCR", "<key> <value>", "Increment value associated with key in Memcached, item must exist, increment command will not create it.\n"
                                             "The limit of increment is the 64 bit mark."),
        CommandInfo("DECR", "<key> <value>", "Decrement value associated with key in Memcached, item must exist, decrement command will not create it."),
        CommandInfo("PREPEND", "<key> <flags> <exptime> <bytes>", "Add value to an existing key before existing data.\n"
                                                                  "Prepend does not take <flags> or <exptime> parameters but you must provide them!"),
        CommandInfo("APPEND", "<key> <flags> <exptime> <value>", "Add value to an existing key after existing data.\n"
                                                                 "Append does not take <flags> or <exptime> parameters but you must provide them!"),
        CommandInfo("REPLACE", "<key> <flags> <exptime> <value>", "Store key/value pair in Memcached, but only if the server already hold data for this key."),
        CommandInfo("ADD", "<key> <flags> <exptime> <value>", "Store key/value pair in Memcached, but only if the server doesn't already hold data for this key."),
        CommandInfo("SET", "<key> <flags> <exptime> <value>", "Set the string value of a key."),
        CommandInfo("GET", "<key>", "Get the value of a key.")
    };

    common::ErrorValueSPtr testConnection(MemcachedConnectionSettings* settings);

    class MemcachedDriver
            : public IDriver
    {
        Q_OBJECT
    public:        

        MemcachedDriver(IConnectionSettingsBaseSPtr settings);
        virtual ~MemcachedDriver();

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
        virtual common::ErrorValueSPtr serverDiscoveryInfo(ServerDiscoveryInfo** dinfo);

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
