#pragma once

#include "core/idriver.h"

#include "core/ssdb/ssdb_settings.h"

namespace fastonosql
{
    //TODO: AUTH, SETNX command imlementation
    static const CommandInfo ssdbCommands[] =
    {
        CommandInfo("QUIT", "-",
                    "Close the connection.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 0),
        CommandInfo("SET", "<key> <value>",
                    "Set the value of the key.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("GET", "<key>",
                    "Get the value related to the specified key.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("SETX", "<key> <value> <ttl>",
                    "Set the value of the key, with a time to live.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("DEL", "<key>",
                    "Delete specified key.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("INCR", "<key> [num]",
                    "Increment the number stored at key by num.\n"
                    "The num argument could be a negative integer.\n"
                    "The old number is first converted to an integer before increment, assuming it was stored as literal integer.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 1),
        CommandInfo("KEYS", "<key_start> <key_end> <limit>",
                    "List keys in range (key_start, key_end].",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("SCAN", "<key_start> <key_end> <limit>",
                    "List keys in range (key_start, key_end].",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("RSCAN", "<key_start> <key_end> <limit>",
                    "List keys in range (key_start, key_end].",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("MULTI_GET", "<keys>",
                    "Get the values related to the specified multiple keys",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("MULTI_SET", "<kvs>",
                    "Set multiple key-value pairs(kvs) in one method call.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("MULTI_DEL", "<keys>",
                    "Delete specified multiple keys.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("HSET", "<name> <key> <value>",
                    "Set the string value in argument as value of the key of a hashmap.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("HGET", "<name> <key>",
                    "Get the value related to the specified key of a hashmap",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("HDEL", "<name> <key>",
                    "Delete specified key of a hashmap.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("HINCR", "<name> <key> <num>",
                    "Increment the number stored at key in a hashmap by num.\n"
                    "The num argument could be a negative integer.\n"
                    "The old number is first converted to an integer before increment, assuming it was stored as literal integer.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("HSIZE", "<name>",
                    "Return the number of pairs of a hashmap.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("HCLEAR", "<name>",
                    "Delete all keys in a hashmap.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("HKEYS", "<name> <key_start> <key_end> <limit>",
                    "List keys of a hashmap in range (key_start, key_end].",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
        CommandInfo("HSCAN", "<name> <key_start> <key_end> <limit>",
                    "List key-value pairs of a hashmap with keys in range (key_start, key_end].",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
        CommandInfo("HRSCAN", "<name> <key_start> <key_end> <limit>",
                    "List key-value pairs of a hashmap with keys in range (key_start, key_end], in reverse order.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
        CommandInfo("MULTI_HGET", "<name> <keys>",
                    "Get the values related to the specified multiple keys of a hashmap.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("MULTI_HSET", "<name> <kvs>",
                    "Set multiple key-value pairs(kvs) of a hashmap in one method call.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("ZSET", "<name> <key> <score>",
                    "Set the score of the key of a zset.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
        CommandInfo("ZGET", "<name> <key>",
                    "Get the score related to the specified key of a zset",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("ZDEL", "<name> <key>",
                    "Delete specified key of a zset.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("ZINCR", "<name> <key> <num>",
                    "Increment the number stored at key in a zset by num.\n"
                    "The num argument could be a negative integer.\n"
                    "The old number is first converted to an integer before increment, assuming it was stored as literal integer.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("ZSIZE", "<name>",
                    "Return the number of pairs of a zset.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("ZCLEAR", "<name>",
                    "Delete all keys in a zset.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0),
        CommandInfo("ZRANK", "<name> <key>",
                    "Returns the rank(index) of a given key in the specified sorted set, starting at 0 for the item with the smallest score.\n"
                    "zrrank starts at 0 for the item with the largest score.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("ZRRANK", "<name> <key>",
                    "Returns the rank(index) of a given key in the specified sorted set, starting at 0 for the item with the smallest score.\n"
                    "zrrank starts at 0 for the item with the largest score.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("ZRANGE", "<name> <offset> <limit>",
                    "Returns a range of key-score pairs by index range [offset, offset + limit). Zrrange iterates in reverse order.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("ZRRANGE", "<name> <offset> <limit>",
                    "Returns a range of key-score pairs by index range [offset, offset + limit). Zrrange iterates in reverse order.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("ZKEYS", "<name> <key_start> <score_start> <score_end> <limit>",
                    "List keys in a zset.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 4, 0),
        CommandInfo("ZSCAN", "<name> <key_start> <score_start> <score_end> <limit>",
                   "List key-score pairs in a zset, where key-score in range (key_start+score_start, score_end].\n"
                   "If key_start is empty, keys with a score greater than or equal to score_start will be returned.\n"
                   "If key_start is not empty, keys with score larger than score_start, "
                   "and keys larger than key_start also with score equal to score_start will be returned.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 5, 0),
        CommandInfo("ZRSCAN", "<name> <key_start> <score_start> <score_end> <limit>",
                    "List key-score pairs of a zset, in reverse order.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 5, 0),
        CommandInfo("MULTI_ZGET", "<name> <keys>",
                    "Get the values related to the specified multiple keys of a zset.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("MULTI_ZSET", "<name> <kvs>",
                    "Set multiple key-score pairs(kvs) of a zset in one method call.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("MULTI_ZDEL", "<name> <keys>",
                    "Delete specified multiple keys of a zset.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("INFO", "[opt]",
                    "Return information about the server.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 0, 1),
        CommandInfo("QPUSH", "<name> <item>",
                    "Adds an or more than one element to the end of the queue.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("QPOP", "<name> <size>",
                    "Pop out one or more elements from the head of a queue.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 2, 0),
        CommandInfo("QSLICE", "<name> <begin> <end>",
                    "Returns a portion of elements from the queue at the specified range [begin, end]. begin and end could be negative.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 3, 0),
        CommandInfo("QCLEAR", "<name>",
                    "Clear the queue.",
                    UNDEFINED_SINCE, UNDEFINED_EXAMPLE_STR, 1, 0)
    };

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
