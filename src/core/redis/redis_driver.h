#pragma once

#include "core/idriver.h"

#include "core/redis/redis_settings.h"

namespace fastonosql
{
    extern const std::vector<CommandInfo> redisTypesKeywords;
    extern const std::vector<CommandInfo> redisCommandsKeywords;

    const CommandInfo redisSentinelCommands[] =
    {
        CommandInfo("SENTINEL MASTERS", "-", "Show a list of monitored masters and their state."),
        CommandInfo("SENTINEL MASTER", "<master name>", "Show the state and info of the specified master."),
        CommandInfo("SENTINEL SLAVES", "<master name>", "Show a list of slaves for this master, and their state."),
        CommandInfo("SENTINEL SENTINELS", "<master name>", "Show a list of sentinel instances for this master, and their state."),
        CommandInfo("SENTINEL GET-MASTER-ADDR-BY-NAME", "<master name>", "Return the ip and port number of the master with that name. If a failover is in progress or terminated successfully for this master it returns the address and port of the promoted slave."),
        CommandInfo("SENTINEL RESET", "<pattern>", "This command will reset all the masters with matching name. The pattern argument is a glob-style pattern. The reset process clears any previous state in a master (including a failover in progress), and removes every slave and sentinel already discovered and associated with the master."),
        CommandInfo("SENTINEL FAILOVER", "<master name>", "Force a failover as if the master was not reachable, and without asking for agreement to other Sentinels (however a new version of the configuration will be published so that the other Sentinels will update their configurations)."),
        CommandInfo("SENTINEL CKQUORUM", "<master name>", "Check if the current Sentinel configuration is able to reach the quorum needed to failover a master, and the majority needed to authorize the failover. This command should be used in monitoring systems to check if a Sentinel deployment is ok."),
        CommandInfo("SENTINEL FLUSHCONFIG", "-", "Force Sentinel to rewrite its configuration on disk, including the current Sentinel state. Normally Sentinel rewrites the configuration every time something changes in its state (in the context of the subset of the state which is persisted on disk across restart). However sometimes it is possible that the configuration file is lost because of operation errors, disk failures, package upgrade scripts or configuration managers. In those cases a way to to force Sentinel to rewrite the configuration file is handy. This command works even if the previous configuration file is completely missing."),
        CommandInfo("SENTINEL MONITOR", "<name> <ip> <port> <quorum>", "This command tells the Sentinel to start monitoring a new master with the specified name, ip, port, and quorum. It is identical to the sentinel monitor configuration directive in sentinel.conf configuration file, with the difference that you can't use an hostname in as ip, but you need to provide an IPv4 or IPv6 address."),
        CommandInfo("SENTINEL REMOVE", "<name>", "is used in order to remove the specified master: the master will no longer be monitored, and will totally be removed from the internal state of the Sentinel, so it will no longer listed by SENTINEL masters and so forth."),
        CommandInfo("SENTINEL SET", "<name> <option> <value>", "The SET command is very similar to the CONFIG SET command of Redis, and is used in order to change configuration parameters of a specific master. Multiple option / value pairs can be specified (or none at all). All the configuration parameters that can be configured via sentinel.conf are also configurable using the SET command.")
    };

    common::ErrorValueSPtr testConnection(RedisConnectionSettings* settings);
    common::ErrorValueSPtr discoveryConnection(RedisConnectionSettings* settings, std::vector<ServerDiscoveryInfoSPtr>& infos);

    class RedisDriver
            : public IDriver
    {
        Q_OBJECT
    public:
        RedisDriver(IConnectionSettingsBaseSPtr settings);
        virtual ~RedisDriver();

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
        virtual void handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent* ev);
        virtual void handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent* ev);
        virtual void handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev);
        virtual void handleShutdownEvent(events::ShutDownRequestEvent* ev);
        virtual void handleBackupEvent(events::BackupRequestEvent* ev);
        virtual void handleExportEvent(events::ExportRequestEvent* ev);
        virtual void handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev);
        virtual void handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev);

        virtual common::ErrorValueSPtr commandDeleteImpl(CommandDeleteKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
        virtual common::ErrorValueSPtr commandLoadImpl(CommandLoadKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;
        virtual common::ErrorValueSPtr commandCreateImpl(CommandCreateKey* command, std::string& cmdstring) const WARN_UNUSED_RESULT;

        virtual void handleDbValueChangeEvent(events::ChangeDbValueRequestEvent* ev);
        virtual void handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent* ev);
        virtual void handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev);

        virtual void handleCommandRequestEvent(events::CommandRequestEvent* ev);

        ServerInfoSPtr makeServerInfoFromString(const std::string& val);

        struct pimpl;
        pimpl* const impl_;

        common::ErrorValueSPtr interacteveMode(events::ProcessConfigArgsRequestEvent* ev);
        common::ErrorValueSPtr latencyMode(events::ProcessConfigArgsRequestEvent* ev);
        common::ErrorValueSPtr slaveMode(events::ProcessConfigArgsRequestEvent* ev);
        common::ErrorValueSPtr getRDBMode(events::ProcessConfigArgsRequestEvent* ev);
        //void pipeMode(events::ProcessConfigArgsRequestEvent* ev);
        common::ErrorValueSPtr findBigKeysMode(events::ProcessConfigArgsRequestEvent* ev);
        common::ErrorValueSPtr statMode(events::ProcessConfigArgsRequestEvent* ev);
        common::ErrorValueSPtr scanMode(events::ProcessConfigArgsRequestEvent* ev);
    };
}
