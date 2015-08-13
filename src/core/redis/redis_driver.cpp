#include "core/redis/redis_driver.h"

#ifdef __MINGW32__
    char *strcasestr(const char* s, const char* find)
    {
        char c, sc;
        size_t len;

        if ((c = *find++) != 0) {
            c = tolower((unsigned char)c);
            len = strlen(find);
            do {
                do {
                    if ((sc = *s++) == 0)
                        return (NULL);
                } while ((char)tolower((unsigned char)sc) != c);
            } while (strncasecmp(s, find, len) != 0);
            s--;
        }
        return ((char *)s);
    }
#endif

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef OS_POSIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

extern "C" {
    #include "sds.h"
}

#include "third-party/redis/src/version.h"
#include <hiredis/hiredis.h>

#include "third-party/redis/src/help.h"

#include "common/time.h"
#include "common/utils.h"
#include "common/file_system.h"
#include "common/string_util.h"
#include "common/sprintf.h"
#include "fasto/qt/logger.h"

#include "core/command_logger.h"
#include "core/redis/redis_infos.h"

#define REDIS_CLI_KEEPALIVE_INTERVAL 15 /* seconds */
#define CLI_HELP_COMMAND 1
#define CLI_HELP_GROUP 2

#define INFO_REQUEST "INFO"
#define SYNC_REQUEST "SYNC"
#define FIND_BIG_KEYS_REQUEST "FIND_BIG_KEYS"
#define LATENCY_REQUEST "LATENCY"
#define GET_DATABASES "CONFIG GET databases"
#define GET_DATABASES_KEYS_INFO "INFO keyspace"
#define SET_DEFAULT_DATABASE "SELECT "
#define DELETE_KEY_PATTERN_1ARGS_S "DEL %s"

#define GET_KEY_PATTERN_1ARGS_S "GET %s"
#define GET_KEY_LIST_PATTERN_1ARGS_S "LRANGE %s 0 -1"
#define GET_KEY_SET_PATTERN_1ARGS_S "SMEMBERS %s"
#define GET_KEY_ZSET_PATTERN_1ARGS_S "ZRANGE %s 0 -1"
#define GET_KEY_HASH_PATTERN_1ARGS_S "HGET %s"

#define SET_KEY_PATTERN_2ARGS_SS "SET %s %s"
#define SET_KEY_LIST_PATTERN_2ARGS_SS "LPUSH %s %s"
#define SET_KEY_SET_PATTERN_2ARGS_SS "SADD %s %s"
#define SET_KEY_ZSET_PATTERN_2ARGS_SS "ZADD %s %s"
#define SET_KEY_HASH_PATTERN_2ARGS_SS "HMSET %s %s"

#define GET_KEYS_PATTERN_2ARGS_ISI "SCAN %d MATCH %s COUNT %d"

#define GET_SERVER_TYPE "CLUSTER NODES"
#define SHUTDOWN "shutdown"
#define GET_PASSWORD "CONFIG get requirepass"
#define SET_PASSWORD_1ARGS_S "CONFIG SET requirepass %s"
#define SET_MAX_CONNECTIONS_1ARGS_I "CONFIG SET maxclients %d"
#define GET_PROPERTY_SERVER "CONFIG GET *"
#define STAT_MODE_REQUEST "STAT"
#define SCAN_MODE_REQUEST "SCAN"
#define RDM_REQUEST "RDM"
#define BACKUP "SAVE"

#define LATENCY_SAMPLE_RATE 10 /* milliseconds. */
#define LATENCY_HISTORY_DEFAULT_INTERVAL 15000 /* milliseconds. */

#define RTYPE_STRING 0
#define RTYPE_LIST   1
#define RTYPE_SET    2
#define RTYPE_HASH   3
#define RTYPE_ZSET   4
#define RTYPE_NONE   5

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

namespace
{
    void anetSetError(char *err, const char *fmt, ...)
    {
        va_list ap;

        if (!err) return;
        va_start(ap, fmt);
        vsnprintf(err, ANET_ERR_LEN, fmt, ap);
        va_end(ap);
    }

    /* Set TCP keep alive option to detect dead peers. The interval option
     * is only used for Linux as we are using Linux-specific APIs to set
     * the probe send time, interval, and count. */
    int anetKeepAlive(char *err, int fd, int interval)
    {
        int val = 1;

        if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&val, sizeof(val)) == -1)
        {
            anetSetError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
            return ANET_ERR;
        }

    #ifdef __linux__
        /* Default settings are more or less garbage, with the keepalive time
         * set to 7200 by default on Linux. Modify settings to make the feature
         * actually useful. */

        /* Send first probe after interval. */
        val = interval;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
            anetSetError(err, "setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
            return ANET_ERR;
        }

        /* Send next probes after the specified interval. Note that we set the
         * delay as interval / 3, as we send three probes before detecting
         * an error (see the next setsockopt call). */
        val = interval/3;
        if (val == 0) val = 1;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
            anetSetError(err, "setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
            return ANET_ERR;
        }

        /* Consider the socket in error state after three we send three ACK
         * probes without getting a reply. */
        val = 3;
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
            anetSetError(err, "setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
            return ANET_ERR;
        }
    #else
        ((void) interval); /* Avoid unused var warning for non Linux systems. */
    #endif

        return ANET_OK;
    }

    typedef struct{
        int type;
        int argc;
        sds *argv;
        sds full;

        /* Only used for help on commands */
        struct commandHelp *org;
    } helpEntry;

    const int helpEntriesLen = sizeof(commandHelp)/sizeof(struct commandHelp) + sizeof(commandGroups)/sizeof(char*);

    const struct RedisInit
    {
        helpEntry helpEntries[helpEntriesLen];

        RedisInit()
        {
            int pos = 0;

            for(int i = 0; i < sizeof(commandGroups)/sizeof(char*); ++i){
                helpEntry tmp;

                tmp.argc = 1;
                tmp.argv = (sds*)malloc(sizeof(sds));
                tmp.argv[0] = sdscatprintf(sdsempty(), "@%s", commandGroups[i]);
                tmp.full = tmp.argv[0];
                tmp.type = CLI_HELP_GROUP;
                tmp.org = NULL;
                helpEntries[pos++] = tmp;
            }

            for(int i = 0; i < sizeof(commandHelp)/sizeof(struct commandHelp); ++i){
                helpEntry tmp;

                tmp.argv = sdssplitargs(commandHelp[i].name, &tmp.argc);
                tmp.full = sdsnew(commandHelp[i].name);
                tmp.type = CLI_HELP_COMMAND;
                tmp.org = &commandHelp[i];
                helpEntries[pos++] = tmp;
            }
        }
    } rInit;

    std::vector<std::pair<std::string, std::string > > oppositeCommands = { {"GET", "SET"}, {"HSET", "HGET"} };

    common::Value::Type convertFromStringRType(const std::string& type)
    {
        if(type.empty()){
           return common::Value::TYPE_NULL;
        }

        if(type == "string") {
            return common::Value::TYPE_STRING;
        }
        else if(type == "list") {
            return common::Value::TYPE_ARRAY;
        }
        else if(type == "set") {
            return common::Value::TYPE_SET;
        }
        else if(type == "hash") {
            return common::Value::TYPE_HASH;
        }
        else if(type == "zset") {
            return common::Value::TYPE_ZSET;
        }
        else if(type == "none") {
            return common::Value::TYPE_NULL;
        }
        else {
            return common::Value::TYPE_NULL;
        }
    }
}

namespace fastonosql
{
    namespace
    {
        class RedisCommand
                : public FastoObjectCommand
        {
        public:
            RedisCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr)
                : FastoObjectCommand(parent, cmd, delemitr)
            {

            }

            virtual std::string inputCmd() const
            {
                common::CommandValue* command = cmd();
                if(command){
                    std::pair<std::string, std::string> kv = getKeyValueFromLine(command->inputCommand());
                    return kv.first;
                }

                return std::string();
            }

            virtual std::string inputArgs() const
            {
                common::CommandValue* command = cmd();
                if(command){
                    std::pair<std::string, std::string> kv = getKeyValueFromLine(command->inputCommand());
                    return kv.second;
                }

                return std::string();
            }
        };

        RedisCommand* createCommand(FastoObject* parent, const std::string& input, common::Value::CommandType ct)
        {
            if(input.empty()){
                return NULL;
            }

            DCHECK(parent);
            std::pair<std::string, std::string> kv = getKeyValueFromLine(input);
            std::string opposite = getOppositeCommand(kv.first, oppositeCommands);
            if(!opposite.empty()){
                opposite += " " + kv.second;
            }
            common::CommandValue* cmd = common::Value::createCommand(input, opposite, ct);
            RedisCommand* fs = new RedisCommand(parent, cmd, "");
            parent->addChildren(fs);
            return fs;
        }

        RedisCommand* createCommand(FastoObjectIPtr parent, const std::string& input, common::Value::CommandType ct)
        {
            return createCommand(parent.get(), input, ct);
        }

        RedisCommand* createCommandFast(const std::string& input, common::Value::CommandType ct)
        {
            common::CommandValue* cmd = common::Value::createCommand(input, "", ct);
            RedisCommand* fs = new RedisCommand(NULL, cmd, "");
            return fs;
        }
    }

    std::vector<CommandInfo> make_from_redis_types(const char **begin, const char **end)
    {
        std::vector<CommandInfo> result;

        for(const char **cur = begin; cur != end; ++cur){
            CommandInfo curcmd(*cur, "-", "type");
            result.push_back(curcmd);
        }
        return result;
    }

    std::vector<CommandInfo> make_from_redis_commands(struct commandHelp *begin, struct commandHelp *end)
    {
        std::vector<CommandInfo> result;

        for(struct commandHelp *cur = begin; cur != end; ++cur){
            CommandInfo curcmd(cur->name, cur->params, cur->summary, cur->since);
            result.push_back(curcmd);
        }
        return result;
    }

    const std::vector<CommandInfo> redisTypesKeywords = make_from_redis_types(commandGroups, commandGroups + sizeof(commandGroups)/sizeof(char*));
    const std::vector<CommandInfo> redisCommandsKeywords = make_from_redis_commands(commandHelp, commandHelp + sizeof(commandHelp)/sizeof(struct commandHelp));

    namespace
    {
        common::ErrorValueSPtr createConnection(const redisConfig& config, const SSHInfo& sinfo, redisContext** context)
        {
            DCHECK(*context == NULL);

            using namespace common::utils;

            redisContext *lcontext = NULL;

            if (config.hostsocket == NULL) {
                const char *username = c_strornull(sinfo.userName_);
                const char *password = c_strornull(sinfo.password_);
                const char *ssh_address = c_strornull(sinfo.hostName_);
                int ssh_port = sinfo.port_;
                int curM = sinfo.currentMethod_;
                const char *publicKey = c_strornull(sinfo.publicKey_);
                const char *privateKey = c_strornull(sinfo.privateKey_);
                const char *passphrase = c_strornull(sinfo.passphrase_);

                lcontext = redisConnect(config.hostip_, config.hostport_, ssh_address, ssh_port, username, password,
                                       publicKey, privateKey, passphrase, curM);
            }
            else {
                lcontext = redisConnectUnix(config.hostsocket);
            }

            if(!lcontext){
                char buff[512] = {0};
                if (!config.hostsocket){
                    common::SNPrintf(buff, sizeof(buff), "Could not connect to Redis at %s:%d : no context", config.hostip_, config.hostport_);
                }
                else{
                    common::SNPrintf(buff, sizeof(buff), "Could not connect to Redis at %s : no context", config.hostsocket);
                }
                return common::make_error_value(buff, common::Value::E_ERROR);
            }

            if (lcontext->err) {
                char buff[512] = {0};
                if (!config.hostsocket){
                    common::SNPrintf(buff, sizeof(buff), "Could not connect to Redis at %s:%d : %s", config.hostip_, config.hostport_, lcontext->errstr);
                }
                else{
                    common::SNPrintf(buff, sizeof(buff), "Could not connect to Redis at %s : %s", config.hostsocket, lcontext->errstr);
                }
                return common::make_error_value(buff, common::Value::E_ERROR);
            }

            *context = lcontext;

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr createConnection(RedisConnectionSettings* settings, redisContext** context)
        {
            if(!settings){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            redisConfig config = settings->info();
            SSHInfo sinfo = settings->sshInfo();
            return createConnection(config, sinfo, context);
        }
    }

    common::ErrorValueSPtr testConnection(RedisConnectionSettings* settings)
    {
        redisContext* context = NULL;
        common::ErrorValueSPtr er = createConnection(settings, &context);
        if(er){
            return er;
        }

        redisFree(context);
        context = NULL;
        return er;
    }

    common::ErrorValueSPtr discoveryConnection(RedisConnectionSettings* settings, std::vector<ServerDiscoveryInfoSPtr>& infos)
    {
        redisContext* context = NULL;
        common::ErrorValueSPtr er = createConnection(settings, &context);
        if(er){
            return er;
        }

        /* Send the GET CLUSTER command. */
        redisReply *reply = (redisReply*)redisCommand(context, GET_SERVER_TYPE);
        if (reply == NULL) {
            er = common::make_error_value("I/O error", common::Value::E_ERROR);
            goto cleanup;
        }

        if(reply->type == REDIS_REPLY_STRING){
            er = makeAllDiscoveryInfo(settings->host(), std::string(reply->str, reply->len), infos);
        }
        else if(reply->type == REDIS_REPLY_ERROR){
            er = common::make_error_value(std::string(reply->str, reply->len), common::Value::E_ERROR);
        }
        else{
            NOTREACHED();
        }

        freeReplyObject(reply);

    cleanup:
        redisFree(context);
        context = NULL;
        return er;
    }

    struct RedisDriver::pimpl
    {
        pimpl()
            : context_(NULL), isAuth_(false), parent_(NULL)
        {

        }

        ~pimpl()
        {
            if(context_){
                redisFree(context_);
                context_ = NULL;
            }
        }

        redisContext *context_;
        redisConfig config_;
        bool isAuth_;
        SSHInfo sinfo_;
        RedisDriver* parent_;

        /*------------------------------------------------------------------------------
         * Latency and latency history modes
         *--------------------------------------------------------------------------- */

        common::ErrorValueSPtr latencyMode(FastoObject* out) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            FastoObjectCommand* cmd = createCommand(out, "PING", common::Value::C_INNER);
            DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid createCommand input argument", common::ErrorValue::E_ERROR);
            }

            redisReply *reply;
            common::time64_t start;
            uint64_t latency, min = 0, max = 0, tot = 0, count = 0;
            uint64_t history_interval =
                    config_.interval ? config_.interval/1000 :
                                      LATENCY_HISTORY_DEFAULT_INTERVAL;
            double avg;
            common::time64_t history_start = common::time::current_mstime();

            if (!context_){
                return common::make_error_value("Not connected", common::Value::E_ERROR);
            }

            FastoObject* child = NULL;
            const std::string command = cmd->inputCommand();

            while(!config_.shutdown_) {
                start = common::time::current_mstime();
                reply = (redisReply*)redisCommand(context_, command.c_str());
                if (reply == NULL) {
                    return common::make_error_value("I/O error", common::Value::E_ERROR);
                }

                common::time64_t curTime = common::time::current_mstime();

                latency = curTime - start;
                freeReplyObject(reply);
                count++;
                if (count == 1) {
                    min = max = tot = latency;
                    avg = (double) latency;
                } else {
                    if (latency < min) min = latency;
                    if (latency > max) max = latency;
                    tot += latency;
                    avg = (double) tot/count;
                }

                char buff[1024];
                common::SNPrintf(buff, sizeof(buff), "min: %lld, max: %lld, avg: %.2f (%lld samples)",
                                    min, max, avg, count);
                common::Value *val = common::Value::createStringValue(buff);

                if(!child){
                    child = new FastoObject(cmd, val, config_.mb_delim_);
                    cmd->addChildren(child);
                    continue;
                }

                if(config_.latency_history && curTime - history_start > history_interval){
                    child = new FastoObject(cmd, val, config_.mb_delim_);
                    cmd->addChildren(child);
                    history_start = curTime;
                    min = max = tot = count = 0;
                }
                else{
                    child->setValue(val);
                }

                common::utils::msleep(LATENCY_SAMPLE_RATE);
            }

            return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
        }

        /*------------------------------------------------------------------------------
         * Slave mode
         *--------------------------------------------------------------------------- */

        /* Sends SYNC and reads the number of bytes in the payload. Used both by
         * slaveMode() and getRDB(). */
        common::ErrorValueSPtr sendSync(unsigned long long& payload) WARN_UNUSED_RESULT
        {
            /* To start we need to send the SYNC command and return the payload.
             * The hiredis client lib does not understand this part of the protocol
             * and we don't want to mess with its buffers, so everything is performed
             * using direct low-level I/O. */
            char buf[4096], *p;

            /* Send the SYNC command. */
            ssize_t nwrite = 0;
            if (redisWriteFromBuffer(context_, "SYNC\r\n", &nwrite) == REDIS_ERR) {
                return common::make_error_value("Error writing to master", common::ErrorValue::E_ERROR);
            }

            /* Read $<payload>\r\n, making sure to read just up to "\n" */
            p = buf;
            while(1) {
                ssize_t nread = 0;
                int res = redisReadToBuffer(context_, p, 1, &nread);
                if (res == REDIS_ERR) {
                    return common::make_error_value("Error reading bulk length while SYNCing", common::ErrorValue::E_ERROR);
                }

                if(!nread){
                    continue;
                }

                if (*p == '\n' && p != buf) break;
                if (*p != '\n') p++;
            }
            *p = '\0';
            if (buf[0] == '-') {
                char buf2[4096];
                common::SNPrintf(buf2, sizeof(buf2), "SYNC with master failed: %s", buf);
                return common::make_error_value(buf2, common::ErrorValue::E_ERROR);
            }

            payload = strtoull(buf+1, NULL, 10);
            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr slaveMode(FastoObject* out) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            unsigned long long payload = 0;
            common::ErrorValueSPtr er = sendSync(payload);
            if(er){
                return er;
            }

            FastoObjectCommand* cmd = createCommand(out, SYNC_REQUEST, common::Value::C_INNER);
            DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid createCommand input argument", common::ErrorValue::E_ERROR);
            }

            char buf[1024];
            /* Discard the payload. */
            while(payload) {
                ssize_t nread = 0;
                int res = redisReadToBuffer(context_, buf, (payload > sizeof(buf)) ? sizeof(buf) : payload, &nread);
                if (res == REDIS_ERR) {
                    return common::make_error_value("Error reading RDB payload while SYNCing", common::ErrorValue::E_ERROR);
                }
                payload -= nread;
            }

            /* Now we can use hiredis to read the incoming protocol. */
            while (1){
                er = cliReadReply(cmd);
                if(er){
                    break;
                }

                if (config_.shutdown_){
                    return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
                }
            }

            return er;
        }

        /*------------------------------------------------------------------------------
         * RDB transfer mode
         *--------------------------------------------------------------------------- */

        /* This function implements --rdb, so it uses the replication protocol in order
         * to fetch the RDB file from a remote server. */
        common::ErrorValueSPtr getRDB(FastoObject* out) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            unsigned long long payload = 0;
            common::ErrorValueSPtr er = sendSync(payload);
            if(er){
                return er;
            }

            FastoObject* child = NULL;
            common::ArrayValue* val = NULL;
            FastoObjectCommand* cmd = createCommand(out, RDM_REQUEST, common::Value::C_INNER);
            DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid createCommand input argument", common::ErrorValue::E_ERROR);
            }

            int fd = INVALID_DESCRIPTOR;
            /* Write to file. */
            if (!strcmp(config_.rdb_filename,"-")) {
                val = new common::ArrayValue;
                child = new FastoObject(cmd, val, config_.mb_delim_);
                cmd->addChildren(child);
            }
            else{
                fd = open(config_.rdb_filename, O_CREAT | O_WRONLY, 0644);
                if (fd == INVALID_DESCRIPTOR) {
                    char bufeEr[2048];
                    common::SNPrintf(bufeEr, sizeof(bufeEr), "Error opening '%s': %s", config_.rdb_filename, strerror(errno));

                    return common::make_error_value(bufeEr, common::ErrorValue::E_ERROR);
                }
            }

            char buf[4096];

            while(payload) {
                ssize_t nread = 0, nwritten = 0;

                int res = redisReadToBuffer(context_, buf,(payload > sizeof(buf)) ? sizeof(buf) : payload, &nread);
                if (res == REDIS_ERR) {
                    return common::make_error_value("Error reading RDB payload while SYNCing", common::ErrorValue::E_ERROR);
                }

                if(!nread){
                    continue;
                }

                if(fd != INVALID_DESCRIPTOR){
                    nwritten = write(fd, buf, nread);
                }
                else{
                    val->appendString(buf);
                }

                if (nwritten != nread) {
                    char bufeEr[2048];
                    common::SNPrintf(bufeEr, sizeof(bufeEr), "Error writing data to file: %s", strerror(errno));

                    if(fd != INVALID_DESCRIPTOR){
                        close(fd);
                    }

                    return common::make_error_value(bufeEr, common::ErrorValue::E_ERROR);
                }

                payload -= nread;
            }

            if(fd != INVALID_DESCRIPTOR){
                close(fd);
            }

            LOG_MSG("Transfer finished with success.", common::logging::L_INFO, true);
            return common::ErrorValueSPtr();
        }

        /*------------------------------------------------------------------------------
         * Find big keys
         *--------------------------------------------------------------------------- */

        redisReply *sendScan(common::ErrorValueSPtr& er, unsigned long long *it)
        {
            redisReply *reply = (redisReply *)redisCommand(context_, "SCAN %llu", *it);

            /* Handle any error conditions */
            if(reply == NULL) {
                er.reset(new common::ErrorValue("I/O error", common::Value::E_ERROR));
                return NULL;
            } else if(reply->type == REDIS_REPLY_ERROR) {
                char buff[512];
                common::SNPrintf(buff, sizeof(buff), "SCAN error: %s", reply->str);
                er.reset(new common::ErrorValue(buff, common::Value::E_ERROR));
                return NULL;
            } else if(reply->type != REDIS_REPLY_ARRAY) {
                er.reset(new common::ErrorValue("Non ARRAY response from SCAN!", common::Value::E_ERROR));
                return NULL;
            } else if(reply->elements != 2) {
                er.reset(new common::ErrorValue("Invalid element count from SCAN!", common::Value::E_ERROR));
                return NULL;
            }

            /* Validate our types are correct */
            assert(reply->element[0]->type == REDIS_REPLY_STRING);
            assert(reply->element[1]->type == REDIS_REPLY_ARRAY);

            /* Update iterator */
            *it = atoi(reply->element[0]->str);

            return reply;
        }

        common::ErrorValueSPtr getDbSize(long long& size) WARN_UNUSED_RESULT
        {
            redisReply *reply = (redisReply *)redisCommand(context_, "DBSIZE");

            if(reply == NULL || reply->type != REDIS_REPLY_INTEGER) {
                return common::make_error_value("Couldn't determine DBSIZE!", common::Value::E_ERROR);
            }

            /* Grab the number of keys and free our reply */
            size = reply->integer;
            freeReplyObject(reply);

            return common::ErrorValueSPtr();
        }

        int toIntType(common::ErrorValueSPtr& er, char *key, char *type) {
            if(!strcmp(type, "string")) {
                return RTYPE_STRING;
            } else if(!strcmp(type, "list")) {
                return RTYPE_LIST;
            } else if(!strcmp(type, "set")) {
                return RTYPE_SET;
            } else if(!strcmp(type, "hash")) {
                return RTYPE_HASH;
            } else if(!strcmp(type, "zset")) {
                return RTYPE_ZSET;
            } else if(!strcmp(type, "none")) {
                return RTYPE_NONE;
            } else {
                char buff[4096];
                common::SNPrintf(buff, sizeof(buff), "Unknown type '%s' for key '%s'", type, key);
                er.reset(new common::ErrorValue(buff, common::Value::E_ERROR));
                return -1;
            }
        }

        common::ErrorValueSPtr getKeyTypes(redisReply *keys, int *types) WARN_UNUSED_RESULT
        {
            redisReply *reply;
            unsigned int i;

            /* Pipeline TYPE commands */
            for(i=0;i<keys->elements;i++) {
                redisAppendCommand(context_, "TYPE %s", keys->element[i]->str);
            }

            /* Retrieve types */
            for(i=0;i<keys->elements;i++) {
                if(redisGetReply(context_, (void**)&reply)!=REDIS_OK) {
                    char buff[4096];
                    common::SNPrintf(buff, sizeof(buff), "Error getting type for key '%s' (%d: %s)",
                        keys->element[i]->str, context_->err, context_->errstr);

                    return common::make_error_value(buff, common::Value::E_ERROR);
                } else if(reply->type != REDIS_REPLY_STATUS) {
                    char buff[4096];
                    common::SNPrintf(buff, sizeof(buff), "Invalid reply type (%d) for TYPE on key '%s'!",
                        reply->type, keys->element[i]->str);

                    return common::make_error_value(buff, common::Value::E_ERROR);
                }

                common::ErrorValueSPtr er;
                int res = toIntType(er, keys->element[i]->str, reply->str);
                freeReplyObject(reply);
                if(res != -1){
                    types[i] = res;
                }
                else{
                    return er;
                }
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr getKeySizes(redisReply *keys, int *types,
                                unsigned long long *sizes) WARN_UNUSED_RESULT
        {
            redisReply *reply;
            const char *sizecmds[] = {"STRLEN","LLEN","SCARD","HLEN","ZCARD"};
            unsigned int i;

            /* Pipeline size commands */
            for(i=0;i<keys->elements;i++) {
                /* Skip keys that were deleted */
                if(types[i]==RTYPE_NONE)
                    continue;

                redisAppendCommand(context_, "%s %s", sizecmds[types[i]],
                    keys->element[i]->str);
            }

            /* Retreive sizes */
            for(i=0;i<keys->elements;i++) {
                /* Skip keys that dissapeared between SCAN and TYPE */
                if(types[i] == RTYPE_NONE) {
                    sizes[i] = 0;
                    continue;
                }

                /* Retreive size */
                if(redisGetReply(context_, (void**)&reply)!=REDIS_OK) {
                    char buff[4096];
                    common::SNPrintf(buff, sizeof(buff), "Error getting size for key '%s' (%d: %s)",
                        keys->element[i]->str, context_->err, context_->errstr);
                    return common::make_error_value(buff, common::Value::E_ERROR);
                } else if(reply->type != REDIS_REPLY_INTEGER) {
                    /* Theoretically the key could have been removed and
                     * added as a different type between TYPE and SIZE */
                    char buff[4096];
                    common::SNPrintf(buff, sizeof(buff),
                        "Warning:  %s on '%s' failed (may have changed type)",
                         sizecmds[types[i]], keys->element[i]->str);
                    LOG_MSG(buff, common::logging::L_WARNING, true);
                    sizes[i] = 0;
                } else {
                    sizes[i] = reply->integer;
                }

                freeReplyObject(reply);
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr findBigKeys(FastoObject* out) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            FastoObjectCommand* cmd = createCommand(out, FIND_BIG_KEYS_REQUEST, common::Value::C_INNER);
            DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid createCommand input argument", common::ErrorValue::E_ERROR);
            }

            unsigned long long biggest[5] = {0}, counts[5] = {0}, totalsize[5] = {0};
            unsigned long long sampled = 0, totlen=0, *sizes=NULL, it=0;
            long long total_keys;
            sds maxkeys[5] = {0};
            const char *typeName[] = {"string","list","set","hash","zset"};
            const char *typeunit[] = {"bytes","items","members","fields","members"};
            redisReply *reply, *keys;
            unsigned int arrsize=0, i;
            int type, *types=NULL;
            double pct;

            /* Total keys pre scanning */
            common::ErrorValueSPtr er = getDbSize(total_keys);
            if(er){
                return er;
            }

            /* Status message */
            LOG_MSG("# Scanning the entire keyspace to find biggest keys as well as", common::logging::L_INFO, true);
            LOG_MSG("# average sizes per key type.  You can use -i 0.1 to sleep 0.1 sec", common::logging::L_INFO, true);
            LOG_MSG("# per 100 SCAN commands (not usually needed).", common::logging::L_INFO, true);

            /* New up sds strings to keep track of overall biggest per type */
            for(i=0;i<RTYPE_NONE; i++) {
                maxkeys[i] = sdsempty();
                if(!maxkeys[i]) {
                    return common::make_error_value("Failed to allocate memory for largest key names!", common::Value::E_ERROR);
                }
            }

            /* SCAN loop */
            do {
                /* Calculate approximate percentage completion */
                pct = 100 * (double)sampled/total_keys;

                /* Grab some keys and point to the keys array */
                reply = sendScan(er, &it);
                if(er){
                    return er;
                }

                keys  = reply->element[1];

                /* Reallocate our type and size array if we need to */
                if(keys->elements > arrsize) {
                    types = (int*)realloc(types, sizeof(int)*keys->elements);
                    sizes = (unsigned long long*)realloc(sizes, sizeof(unsigned long long)*keys->elements);

                    if(!types || !sizes) {
                        return common::make_error_value("Failed to allocate storage for keys!", common::Value::E_ERROR);
                    }

                    arrsize = keys->elements;
                }

                /* Retreive types and then sizes */
                er = getKeyTypes(keys, types);
                if(er){
                    return er;
                }

                er = getKeySizes(keys, types, sizes);
                if(er){
                    return er;
                }

                /* Now update our stats */
                for(i=0;i<keys->elements;i++) {
                    if((type = types[i]) == RTYPE_NONE)
                        continue;

                    totalsize[type] += sizes[i];
                    counts[type]++;
                    totlen += keys->element[i]->len;
                    sampled++;

                    if(biggest[type]<sizes[i]) {
                        char buff[4096];
                        common::SNPrintf(buff, sizeof(buff),
                           "[%05.2f%%] Biggest %-6s found so far '%s' with %llu %s",
                           pct, typeName[type], keys->element[i]->str, sizes[i],
                           typeunit[type]);

                        LOG_MSG(buff, common::logging::L_INFO, true);

                        /* Keep track of biggest key name for this type */
                        maxkeys[type] = sdscpy(maxkeys[type], keys->element[i]->str);
                        if(!maxkeys[type]) {
                            return common::make_error_value("Failed to allocate memory for key!", common::Value::E_ERROR);
                        }

                        /* Keep track of the biggest size for this type */
                        biggest[type] = sizes[i];
                    }
                }

                /* Sleep if we've been directed to do so */
                if(sampled && (sampled %100) == 0 && config_.interval) {
                    common::utils::usleep(config_.interval);
                }

                freeReplyObject(reply);
            } while(it != 0);

            if(types) free(types);
            if(sizes) free(sizes);

            /* We're done */
            char buff[4096];
            common::SNPrintf(buff, sizeof(buff), "Sampled %llu keys in the keyspace!", sampled);
            LOG_MSG(buff, common::logging::L_INFO, true);

            memset(&buff, 0, sizeof(buff));
            common::SNPrintf(buff, sizeof(buff), "Total key length in bytes is %llu (avg len %.2f)",
               totlen, totlen ? (double)totlen/sampled : 0);
            LOG_MSG(buff, common::logging::L_INFO, true);

            /* Output the biggest keys we found, for types we did find */
            for(i=0;i<RTYPE_NONE;i++) {
                if(sdslen(maxkeys[i])>0) {
                    memset(&buff, 0, sizeof(buff));
                    common::SNPrintf(buff, sizeof(buff), "Biggest %6s found '%s' has %llu %s", typeName[i], maxkeys[i],
                       biggest[i], typeunit[i]);
                    common::StringValue *val = common::Value::createStringValue(buff);
                    FastoObject* obj = new FastoObject(cmd, val, config_.mb_delim_);
                    cmd->addChildren(obj);
                }
            }

            for(i=0;i<RTYPE_NONE;i++) {
                memset(&buff, 0, sizeof(buff));
                common::SNPrintf(buff, sizeof(buff), "%llu %ss with %llu %s (%05.2f%% of keys, avg size %.2f)",
                   counts[i], typeName[i], totalsize[i], typeunit[i],
                   sampled ? 100 * (double)counts[i]/sampled : 0,
                   counts[i] ? (double)totalsize[i]/counts[i] : 0);
                common::StringValue *val = common::Value::createStringValue(buff);
                FastoObject* obj = new FastoObject(cmd, val, config_.mb_delim_);
                cmd->addChildren(obj);
            }

            /* Free sds strings containing max keys */
            for(i=0;i<RTYPE_NONE;i++) {
                sdsfree(maxkeys[i]);
            }

            /* Success! */
            return common::ErrorValueSPtr();
        }

        /*------------------------------------------------------------------------------
         * Stats mode
         *--------------------------------------------------------------------------- */

        /* Return the specified INFO field from the INFO command output "info".
         * A new buffer is allocated for the result, that needs to be free'd.
         * If the field is not found NULL is returned. */
        static char *getInfoField(char *info, const char *field) {
            char *p = strstr(info,field);
            char *n1, *n2;
            char *result;

            if (!p) return NULL;
            p += strlen(field)+1;
            n1 = strchr(p,'\r');
            n2 = strchr(p,',');
            if (n2 && n2 < n1) n1 = n2;
            result = (char*)malloc(sizeof(char)*(n1-p)+1);
            memcpy(result,p,(n1-p));
            result[n1-p] = '\0';
            return result;
        }

        /* Like the above function but automatically convert the result into
         * a long. On error (missing field) LONG_MIN is returned. */
        static long getLongInfoField(char *info, const char *field) {
            char *value = getInfoField(info,field);
            long l;

            if (!value) return LONG_MIN;
            l = strtol(value,NULL,10);
            free(value);
            return l;
        }

        /* Convert number of bytes into a human readable string of the form:
         * 100B, 2G, 100M, 4K, and so forth. */
        static void bytesToHuman(char *s, long long n) {
            double d;

            if (n < 0) {
                *s = '-';
                s++;
                n = -n;
            }
            if (n < 1024) {
                /* Bytes */
                common::SPrintf(s, "%lluB", n);
                return;
            } else if (n < (1024*1024)) {
                d = (double)n/(1024);
                common::SPrintf(s,"%.2fK",d);
            } else if (n < (1024LL*1024*1024)) {
                d = (double)n/(1024*1024);
                common::SPrintf(s,"%.2fM",d);
            } else if (n < (1024LL*1024*1024*1024)) {
                d = (double)n/(1024LL*1024*1024);
                common::SPrintf(s,"%.2fG",d);
            }
        }

        common::ErrorValueSPtr statMode(FastoObject* out) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            FastoObjectCommand* cmd = createCommand(out, INFO_REQUEST, common::Value::C_INNER);
            DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid createCommand input argument", common::ErrorValue::E_ERROR);
            }

            const std::string command = cmd->inputCommand();
            long aux, requests = 0;

            while(!config_.shutdown_) {
                char buf[64];
                int j;

                redisReply *reply = NULL;
                while(reply == NULL) {
                    reply = (redisReply*)redisCommand(context_, command.c_str());
                    if (context_->err && !(context_->err & (REDIS_ERR_IO | REDIS_ERR_EOF))) {
                        char buff[2048];
                        common::SNPrintf(buff, sizeof(buff), "ERROR: %s", context_->errstr);
                        return common::make_error_value(buff, common::ErrorValue::E_ERROR);
                    }
                }

                if (reply->type == REDIS_REPLY_ERROR) {
                    char buff[2048];
                    common::SNPrintf(buff, sizeof(buff), "ERROR: %s", reply->str);
                    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
                }

                /* Keys */
                aux = 0;
                for (j = 0; j < 20; j++) {
                    long k;

                    common::SNPrintf(buf, sizeof(buf), "db%d:keys", j);
                    k = getLongInfoField(reply->str, buf);
                    if (k == LONG_MIN) continue;
                    aux += k;
                }

                std::string result;

                common::SNPrintf(buf, sizeof(buf), "keys %ld", aux);
                result += buf;

                /* Used memory */
                aux = getLongInfoField(reply->str, "used_memory");
                bytesToHuman(buf,aux);
                result += " used_memory: ";
                result += buf;

                /* Clients */
                aux = getLongInfoField(reply->str, "connected_clients");
                common::SNPrintf(buf, sizeof(buf), " connected_clients: %ld", aux);
                result += buf;

                /* Blocked (BLPOPPING) Clients */
                aux = getLongInfoField(reply->str, "blocked_clients");
                common::SPrintf(buf," blocked_clients: %ld",aux);
                result += buf;

                /* Requets */
                aux = getLongInfoField(reply->str, "total_commands_processed");
                common::SPrintf(buf," total_commands_processed: %ld (+%ld)",aux,requests == 0 ? 0 : aux-requests);
                result += buf;
                requests = aux;

                /* Connections */
                aux = getLongInfoField(reply->str, "total_connections_received");
                common::SPrintf(buf," total_connections_received: %ld",aux);
                result += buf;

                /* Children */
                aux = getLongInfoField(reply->str, "bgsave_in_progress");
                aux |= getLongInfoField(reply->str, "aof_rewrite_in_progress") << 1;
                switch(aux) {
                case 0: break;
                case 1:
                    result += " SAVE";
                    break;
                case 2:
                    result += " AOF";
                    break;
                case 3:
                    result += " SAVE+AOF";
                    break;
                }

                common::StringValue *val = common::Value::createStringValue(result);
                FastoObject* obj = new FastoObject(cmd, val, config_.mb_delim_);
                cmd->addChildren(obj);

                freeReplyObject(reply);

                common::utils::usleep(config_.interval);
            }

            return common::make_error_value("Interrupted.", common::ErrorValue::E_INTERRUPTED);
        }

        /*------------------------------------------------------------------------------
         * Scan mode
         *--------------------------------------------------------------------------- */

        common::ErrorValueSPtr scanMode(FastoObject* out) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            FastoObjectCommand* cmd = createCommand(out, SCAN_MODE_REQUEST, common::Value::C_INNER);
            DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid createCommand input argument", common::ErrorValue::E_ERROR);
            }

            redisReply *reply;
            unsigned long long cur = 0;

            do {
                if (config_.pattern)
                    reply = (redisReply*)redisCommand(context_,"SCAN %llu MATCH %s",
                        cur,config_.pattern);
                else
                    reply = (redisReply*)redisCommand(context_,"SCAN %llu",cur);
                if (reply == NULL) {
                    return common::make_error_value("I/O error", common::ErrorValue::E_ERROR);
                } else if (reply->type == REDIS_REPLY_ERROR) {
                    char buff[2048];
                    common::SNPrintf(buff, sizeof(buff), "ERROR: %s", reply->str);
                    return common::make_error_value(buff, common::ErrorValue::E_ERROR);
                } else {
                    unsigned int j;

                    cur = strtoull(reply->element[0]->str,NULL,10);
                    for (j = 0; j < reply->element[1]->elements; j++){
                        common::StringValue *val = common::Value::createStringValue(reply->element[1]->element[j]->str);
                        FastoObject* obj = new FastoObject(cmd, val, config_.mb_delim_);
                        cmd->addChildren(obj);
                    }
                }
                freeReplyObject(reply);
            } while(cur != 0);

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr cliAuth() WARN_UNUSED_RESULT
        {
            if (config_.auth == NULL){
                isAuth_ = true;
                return common::ErrorValueSPtr();
            }

            redisReply *reply = static_cast<redisReply*>(redisCommand(context_, "AUTH %s", config_.auth));
            if (reply != NULL) {
                isAuth_ = true;
                freeReplyObject(reply);
                return common::ErrorValueSPtr();
            }

            isAuth_ = false;
            return cliPrintContextError();
        }

        common::ErrorValueSPtr cliSelect() WARN_UNUSED_RESULT
        {
            if (config_.dbnum == 0){
                return common::ErrorValueSPtr();
            }

            redisReply *reply = static_cast<redisReply*>(redisCommand(context_, "SELECT %d", config_.dbnum));
            if (reply != NULL) {
                parent_->currentDatabaseInfo_.reset(new RedisDataBaseInfo(common::convertToString(config_.dbnum), 0, true));
                freeReplyObject(reply);
                return common::ErrorValueSPtr();
            }

            return cliPrintContextError();
        }

        common::ErrorValueSPtr cliConnect(int force) WARN_UNUSED_RESULT
        {
            using namespace common::utils;

            if (context_ == NULL || force) {
                if (context_ != NULL){
                    redisFree(context_);
                    context_ = NULL;
                }

                redisContext* context = NULL;
                common::ErrorValueSPtr er = createConnection(config_, sinfo_, &context);
                if(er){
                    return er;
                }

                context_ = context;

                /* Set aggressive KEEP_ALIVE socket option in the Redis context socket
                 * in order to prevent timeouts caused by the execution of long
                 * commands. At the same time this improves the detection of real
                 * errors. */
                anetKeepAlive(NULL, context->fd, REDIS_CLI_KEEPALIVE_INTERVAL);

                /*struct timeval timeout;
                timeout.tv_sec = 10;
                timeout.tv_usec = 0;
                int res = redisSetTimeout(context, timeout);
                if(res == REDIS_ERR){
                    char buff[512] = {0};
                    common::SNPrintf(buff, sizeof(buff), "Redis connection set timeout failed error is: %s.", context->errstr);
                    LOG_MSG(buff, common::logging::L_WARNING, true);
                }*/

                /* Do AUTH and select the right DB. */
                er = cliAuth();
                if (er){
                    return er;
                }

                er = cliSelect();
                if (er){
                    return er;
                }
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr cliPrintContextError() WARN_UNUSED_RESULT
        {
            if (context_ == NULL){
                return common::make_error_value("Not connected", common::Value::E_ERROR);
            }

            char buff[512] = {0};
            common::SNPrintf(buff, sizeof(buff), "Error: %s", context_->errstr);
            return common::make_error_value(buff, common::ErrorValue::E_ERROR);
        }

        common::ErrorValueSPtr cliFormatReplyRaw(FastoObjectArray* ar, redisReply *r) WARN_UNUSED_RESULT
        {
            DCHECK(ar);
            if(!ar){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            switch (r->type) {
                case REDIS_REPLY_NIL:
                {
                    common::Value* val = common::Value::createNullValue();
                    ar->append(val);

                    break;
                }
                case REDIS_REPLY_ERROR:
                {
                    common::ErrorValue* val = common::Value::createErrorValue(std::string(r->str, r->len), common::ErrorValue::E_NONE, common::logging::L_WARNING);
                    ar->append(val);
                    break;
                }
                case REDIS_REPLY_STATUS:
                case REDIS_REPLY_STRING:
                {
                    common::StringValue *val = common::Value::createStringValue(std::string(r->str, r->len));
                    ar->append(val);
                    break;
                }
                case REDIS_REPLY_INTEGER:
                {
                    common::FundamentalValue *val = common::Value::createIntegerValue(r->integer);
                    ar->append(val);
                    break;
                }
                case REDIS_REPLY_ARRAY:
                {
                    common::ArrayValue* arv = common::Value::createArrayValue();
                    FastoObjectArray* child = new FastoObjectArray(ar, arv, config_.mb_delim_);
                    ar->addChildren(child);

                    for (size_t i = 0; i < r->elements; ++i) {
                        common::ErrorValueSPtr er = cliFormatReplyRaw(child, r->element[i]);
                        if(er){
                            return er;
                        }
                    }
                    break;
                }
            default:
                {
                    char tmp2[128] = {0};
                    common::SNPrintf(tmp2, sizeof(tmp2), "Unknown reply type: %d", r->type);
                    common::ErrorValue* val = common::Value::createErrorValue(tmp2, common::ErrorValue::E_NONE, common::logging::L_WARNING);
                    ar->append(val);
                }
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr cliFormatReplyRaw(FastoObject* out, redisReply *r) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            FastoObject* obj = NULL;
            switch (r->type) {
                case REDIS_REPLY_NIL:
                {
                    common::Value *val = common::Value::createNullValue();
                    obj = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(obj);

                    break;
                }
                case REDIS_REPLY_ERROR:
                {
                    common::ErrorValue* val = common::Value::createErrorValue(r->str, common::ErrorValue::E_NONE, common::logging::L_WARNING);
                    if(strcasestr(r->str, "NOAUTH")){ //"NOAUTH Authentication required."
                        isAuth_ = false;
                    }
                    obj = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(obj);
                    break;
                }
                case REDIS_REPLY_STATUS:
                case REDIS_REPLY_STRING:
                {
                    common::StringValue *val = common::Value::createStringValue(r->str);
                    obj = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(obj);
                    break;
                }
                case REDIS_REPLY_INTEGER:
                {
                    common::FundamentalValue *val = common::Value::createIntegerValue(r->integer);
                    obj = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(obj);
                    break;
                }
                case REDIS_REPLY_ARRAY:
                {
                    common::ArrayValue* arv = common::Value::createArrayValue();
                    FastoObjectArray* child = new FastoObjectArray(out, arv, config_.mb_delim_);
                    out->addChildren(child);

                    for (size_t i = 0; i < r->elements; ++i) {
                        common::ErrorValueSPtr er = cliFormatReplyRaw(child, r->element[i]);
                        if(er){
                            return er;
                        }
                    }
                    break;
                }
            default:
                {
                    char tmp2[128] = {0};
                    common::SNPrintf(tmp2, sizeof(tmp2), "Unknown reply type: %d", r->type);
                    common::ErrorValue* val = common::Value::createErrorValue(tmp2, common::ErrorValue::E_NONE, common::logging::L_WARNING);
                    obj = new FastoObject(out, val, config_.mb_delim_);
                    out->addChildren(obj);
                }
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr cliOutputCommandHelp(FastoObject* out, struct commandHelp *help, int group) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            char buff[1024] = {0};
            common::SNPrintf(buff, sizeof(buff), "name: %s %s\r\n  summary: %s\r\n  since: %s", help->name, help->params, help->summary, help->since);
            common::StringValue *val =common::Value::createStringValue(buff);
            FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
            out->addChildren(child);
            if (group) {
                char buff2[1024] = {0};
                common::SNPrintf(buff2, sizeof(buff2), "  group: %s", commandGroups[help->group]);
                val = common::Value::createStringValue(buff2);
                FastoObject* gchild = new FastoObject(out, val, config_.mb_delim_);
                out->addChildren(gchild);
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr cliOutputGenericHelp(FastoObject* out) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            common::StringValue *val = common::Value::createStringValue(PROJECT_NAME_TITLE " based on hiredis " REDIS_VERSION "\r\n"
                                                                        "Type: \"help @<group>\" to get a list of commands in <group>\r\n"
                                                                        "      \"help <command>\" for help on <command>\r\n"
                                                                        "      \"help <tab>\" to get a list of possible help topics\r\n"
                                                                        "      \"quit\" to exit");
            FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
            out->addChildren(child);

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr cliOutputHelp(FastoObject* out, int argc, char **argv) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            int i, j, len;
            int group = -1;
            const helpEntry *entry;
            struct commandHelp *help;

            if (argc == 0) {
                return cliOutputGenericHelp(out);
            }
            else if (argc > 0 && argv[0][0] == '@') {
                len = sizeof(commandGroups)/sizeof(char*);
                for (i = 0; i < len; i++) {
                    if (strcasecmp(argv[0]+1,commandGroups[i]) == 0) {
                        group = i;
                        break;
                    }
                }
            }

            assert(argc > 0);
            for (i = 0; i < helpEntriesLen; i++) {
                entry = &rInit.helpEntries[i];
                if (entry->type != CLI_HELP_COMMAND) continue;

                help = entry->org;
                if (group == -1) {
                    /* Compare all arguments */
                    if (argc == entry->argc) {
                        for (j = 0; j < argc; j++) {
                            if (strcasecmp(argv[j],entry->argv[j]) != 0) break;
                        }
                        if (j == argc) {
                            common::ErrorValueSPtr er = cliOutputCommandHelp(out, help,1);
                            if(er){
                                return er;
                            }
                        }
                    }
                }
                else {
                    if (group == help->group) {
                        common::ErrorValueSPtr er = cliOutputCommandHelp(out, help,0);
                        if(er){
                            return er;
                        }
                    }
                }
            }

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr cliReadReply(FastoObject* out) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            void *_reply = NULL;
            if (redisGetReply(context_, &_reply) != REDIS_OK) {
                if (config_.shutdown_){
                    return common::make_error_value("Interrupted connect.", common::ErrorValue::E_INTERRUPTED);
                }

                /* Filter cases where we should reconnect */
                if (context_->err == REDIS_ERR_IO && errno == ECONNRESET){
                    return common::make_error_value("Needed reconnect.", common::ErrorValue::E_ERROR);
                }
                if (context_->err == REDIS_ERR_EOF){
                    return common::make_error_value("Needed reconnect.", common::ErrorValue::E_ERROR);
                }

                return cliPrintContextError(); /* avoid compiler warning */
            }

            redisReply *reply = static_cast<redisReply*>(_reply);
            config_.last_cmd_type = reply->type;

            if (config_.cluster_mode && reply->type == REDIS_REPLY_ERROR &&
                (!strncmp(reply->str,"MOVED",5) || !strcmp(reply->str,"ASK")))
            {
                char *p = reply->str, *s;
                int slot;

                s = strchr(p,' ');      /* MOVED[S]3999 127.0.0.1:6381 */
                p = strchr(s+1,' ');    /* MOVED[S]3999[P]127.0.0.1:6381 */
                *p = '\0';
                slot = atoi(s+1);
                s = strchr(p+1,':');    /* MOVED 3999[P]127.0.0.1[S]6381 */
                *s = '\0';                
                config_.hostip_ = p+1;
                config_.hostport_ = atoi(s+1);
                char redir[512] = {0};
                common::SNPrintf(redir, sizeof(redir), "-> Redirected to slot [%d] located at %s:%d", slot, config_.hostip_, config_.hostport_);
                common::StringValue *val = common::Value::createStringValue(redir);
                FastoObject* child = new FastoObject(out, val, config_.mb_delim_);
                out->addChildren(child);
                config_.cluster_reissue_command = 1;

                freeReplyObject(reply);
                return common::ErrorValueSPtr();
            }

            common::ErrorValueSPtr er = cliFormatReplyRaw(out, reply);
            freeReplyObject(reply);
            return er;
        }

        common::ErrorValueSPtr cliSendCommand(FastoObject* out, int argc, char **argv) WARN_UNUSED_RESULT
        {
            DCHECK(out);
            if(!out){
                return common::make_error_value("Invalid input argument", common::ErrorValue::E_ERROR);
            }

            char *command = argv[0];

            if (!strcasecmp(command,"help") || !strcasecmp(command,"?")) {
                return cliOutputHelp(out, --argc, ++argv);
            }

            if (context_ == NULL){
                return common::make_error_value("Not connected", common::Value::E_ERROR);
            }

            if (!strcasecmp(command,"shutdown")) config_.shutdown_ = 1;
            if (!strcasecmp(command,"monitor")) config_.monitor_mode = 1;
            if (!strcasecmp(command,"subscribe") || !strcasecmp(command,"psubscribe")) config_.pubsub_mode = 1;
            if (!strcasecmp(command,"sync") || !strcasecmp(command,"psync")) config_.slave_mode = 1;

            redisAppendCommandArgv(context_, argc, (const char**)argv, NULL);
            while (config_.monitor_mode) {
                common::ErrorValueSPtr er = cliReadReply(out);
                if (er){
                    return er;
                }
            }

            if (config_.pubsub_mode) {
                while (1) {
                    common::ErrorValueSPtr er = cliReadReply(out);
                    if (er){
                        return er;
                    }
                }
            }

            if (config_.slave_mode) {
                common::ErrorValueSPtr er = slaveMode(out);
                config_.slave_mode = 0;
                return er;  /* Error = slaveMode lost connection to master */
            }

            common::ErrorValueSPtr er = cliReadReply(out);
            if (er) {
                return er;
            }
            else {
                /* Store database number when SELECT was successfully executed. */
                if (!strcasecmp(command, "select") && argc == 2) {
                    config_.dbnum = atoi(argv[1]);
                }
                else if (!strcasecmp(command, "auth") && argc == 2) {
                    er = cliSelect();
                    if(er){
                        return er;
                    }
                }
            }
            if (config_.interval){
                common::utils::usleep(config_.interval);
            }

            return common::ErrorValueSPtr();
        }

        static bool isPipeLineCommand(const char *command)
        {
            if(!command){
                return false;
            }

            bool skip = strcasecmp(command, "quit") == 0
                        || strcasecmp(command, "exit") == 0
                        || strcasecmp(command, "connect") == 0
                        || strcasecmp(command, "help") == 0
                        || strcasecmp(command, "?") == 0
                        || strcasecmp(command, "shutdown") == 0
                        || strcasecmp(command, "monitor") == 0
                        || strcasecmp(command, "subscribe") == 0
                        || strcasecmp(command, "psubscribe") == 0
                        || strcasecmp(command, "sync") == 0
                        || strcasecmp(command, "psync") == 0;

            return !skip;
        }

        common::ErrorValueSPtr executeAsPipeline(std::vector<FastoObjectCommandIPtr> cmds) WARN_UNUSED_RESULT
        {
            //DCHECK(cmd);
            if(cmds.empty()){
                return common::make_error_value("Invalid input command", common::ErrorValue::E_ERROR);
            }

            if (context_ == NULL){
                return common::make_error_value("Not connected", common::Value::E_ERROR);
            }

            //start piplene mode
            std::vector<FastoObjectCommandIPtr> valid_cmds;
            for(int i = 0; i < cmds.size(); ++i){
                FastoObjectCommandIPtr cmd = cmds[i];
                common::CommandValue* cmdc = cmd->cmd();

                const std::string command = cmdc->inputCommand();
                common::Value::CommandType type = cmdc->commandType();

                if(command.empty()){
                    continue;
                }

                LOG_COMMAND(Command(command, type));

                const char* ccommand = common::utils::c_strornull(command);

                if (ccommand) {
                    int argc = 0;
                    sds *argv = sdssplitargs(ccommand, &argc);

                    if (argv == NULL) {
                        common::StringValue *val = common::Value::createStringValue("Invalid argument(s)");
                        FastoObject* child = new FastoObject(cmd.get(), val, config_.mb_delim_);
                        cmd->addChildren(child);
                    }
                    else if (argc > 0){
                        if (isPipeLineCommand(argv[0])){
                            valid_cmds.push_back(cmd);
                            redisAppendCommandArgv(context_, argc, (const char**)argv, NULL);
                        }
                    }
                    sdsfreesplitres(argv, argc);
                }
            }

            for(int i = 0; i < valid_cmds.size(); ++i){
                FastoObjectCommandIPtr cmd = cmds[i];
                common::ErrorValueSPtr er = cliReadReply(cmd.get());
                if (er) {
                    return er;
                }
            }
            //end piplene

            return common::ErrorValueSPtr();
        }

        common::ErrorValueSPtr execute(RedisCommand* cmd) WARN_UNUSED_RESULT
        {
            //DCHECK(cmd);
            if(!cmd){
                return common::make_error_value("Invalid input command", common::ErrorValue::E_ERROR);
            }

            common::CommandValue* cmdc = cmd->cmd();

            const std::string command = cmdc->inputCommand();
            common::Value::CommandType type = cmdc->commandType();

            if(command.empty()){
                return common::make_error_value("Command empty", common::ErrorValue::E_ERROR);
            }

            LOG_COMMAND(Command(command, type));

            common::ErrorValueSPtr er;

            const char* ccommand = common::utils::c_strornull(command);

            if (ccommand) {
                int argc = 0;
                sds *argv = sdssplitargs(ccommand, &argc);

                if (argv == NULL) {
                    common::StringValue *val = common::Value::createStringValue("Invalid argument(s)");
                    FastoObject* child = new FastoObject(cmd, val, config_.mb_delim_);
                    cmd->addChildren(child);
                }
                else if (argc > 0)
                {
                    if (strcasecmp(argv[0], "quit") == 0 || strcasecmp(argv[0], "exit") == 0){
                        config_.shutdown_ = 1;
                    }
                    else if (argc == 3 && !strcasecmp(argv[0], "connect")) {
                        config_.hostip_ = argv[1];
                        config_.hostport_ = atoi(argv[2]);
                        sdsfreesplitres(argv, argc);
                        return cliConnect(1);
                    }
                    else {
                        er = cliSendCommand(cmd, argc, argv);
                    }
                }
                sdsfreesplitres(argv, argc);
            }

            return er;
        }
    };

    RedisDriver::RedisDriver(IConnectionSettingsBaseSPtr settings)
        : IDriver(settings), impl_(new pimpl)
    {
        impl_->parent_ = this;
    }

    RedisDriver::~RedisDriver()
    {
        interrupt();
        stop();
        delete impl_;
    }

    common::net::hostAndPort RedisDriver::address() const
    {
        return common::net::hostAndPort(impl_->config_.hostip_, impl_->config_.hostport_);
    }

    std::string RedisDriver::version() const
    {
        return versionApi();
    }

    std::string RedisDriver::outputDelemitr() const
    {
        return impl_->config_.mb_delim_;
    }

    const char* RedisDriver::versionApi()
    {
        return REDIS_VERSION;
    }

    bool RedisDriver::isConnected() const
    {
        return impl_->context_;
    }

    bool RedisDriver::isAuthenticated() const
    {
        if(!impl_->context_){
            return false;
        }

        return impl_->isAuth_;
    }

    void RedisDriver::customEvent(QEvent *event)
    {
        IDriver::customEvent(event);
        impl_->config_.shutdown_ = 0;
    }    

    void RedisDriver::initImpl()
    {
        currentDatabaseInfo_.reset(new RedisDataBaseInfo("0", 0, true));
    }

    void RedisDriver::clearImpl()
    {

    }

    common::ErrorValueSPtr RedisDriver::currentLoggingInfo(ServerInfo** info)
    {
        *info = NULL;
        FastoObjectIPtr root = FastoObject::createRoot(INFO_REQUEST);
        RedisCommand* cmd = createCommand(root, INFO_REQUEST, common::Value::C_INNER);
        common::ErrorValueSPtr res = impl_->execute(cmd);
        if(!res){
            FastoObject::child_container_type ch = root->childrens();
            if(ch.size()){
                *info = makeRedisServerInfo(ch[0]);
            }
            if(*info == NULL){
                res = common::make_error_value("Invalid " INFO_REQUEST " command output", common::ErrorValue::E_ERROR);
            }
        }
        return res;
    }

    common::ErrorValueSPtr RedisDriver::serverDiscoveryInfo(ServerDiscoveryInfo** dinfo)
    {
        *dinfo = NULL;
        FastoObjectIPtr root = FastoObject::createRoot(GET_SERVER_TYPE);
        RedisCommand* cmd = createCommand(root, GET_SERVER_TYPE, common::Value::C_INNER);
        common::ErrorValueSPtr er = impl_->execute(cmd);

        if(!er){
            FastoObject::child_container_type ch = cmd->childrens();
            if(ch.size()){
                FastoObject* obj = ch[0];
                if(obj){
                    common::Value::Type t = obj->type();
                    if(t == common::Value::TYPE_STRING){
                        *dinfo = makeOwnRedisDiscoveryInfo(obj);
                    }
                    else if(t == common::Value::TYPE_ERROR){
                        return common::make_error_value(obj->toString(), common::ErrorValue::E_ERROR);
                    }
                    else{
                        NOTREACHED();
                    }
                }
            }

            if(*dinfo == NULL){
                er = common::make_error_value("Invalid " GET_SERVER_TYPE " command output", common::ErrorValue::E_ERROR);
            }
        }
        return er;
    }

    void RedisDriver::handleConnectEvent(events::ConnectRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ConnectResponceEvent::value_type res(ev->value());
            RedisConnectionSettings *set = dynamic_cast<RedisConnectionSettings*>(settings_.get());
            if(set){
                impl_->config_ = set->info();
                impl_->sinfo_ = set->sshInfo();
        notifyProgress(sender, 25);
                    if(impl_->config_.shutdown_){
                        common::ErrorValueSPtr er = common::make_error_value("Interrupted connect.", common::ErrorValue::E_INTERRUPTED);
                        res.setErrorInfo(er);
                    }
                    else{
                        common::ErrorValueSPtr er = impl_->cliConnect(0);
                        if(er){
                            res.setErrorInfo(er);
                        }
                    }
        notifyProgress(sender, 75);
            }
            reply(sender, new events::ConnectResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleProcessCommandLineArgs(events::ProcessConfigArgsRequestEvent* ev)
    {
        /* Latency mode */
        if (impl_->config_.latency_mode) {
            latencyMode(ev);
        }

        /* Slave mode */
        if (impl_->config_.slave_mode) {
            slaveMode(ev);
        }

        /* Get RDB mode. */
        if (impl_->config_.getrdb_mode) {
            getRDBMode(ev);
        }

        /* Pipe mode */
        /*if (impl_->config.pipe_mode) {
            pipeMode(ev);
        }*/

        /* Find big keys */
        if (impl_->config_.bigkeys) {
            findBigKeysMode(ev);
        }

        /* Stat mode */
        if (impl_->config_.stat_mode) {
            if (impl_->config_.interval == 0){
                impl_->config_.interval = 1000000;
            }
            statMode(ev);
        }

        /* Scan mode */
        if (impl_->config_.scan_mode) {
            scanMode(ev);
        }

        interacteveMode(ev);

        QObject *sender = ev->sender();
        events::ProcessConfigArgsResponceEvent::value_type res(ev->value());
        reply(sender, new events::ProcessConfigArgsResponceEvent(this, res));
    }

    void RedisDriver::handleShutdownEvent(events::ShutDownRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ShutDownResponceEvent::value_type res(ev->value());
        notifyProgress(sender, 25);
            FastoObjectIPtr root = FastoObject::createRoot(SHUTDOWN);
            RedisCommand* cmd = createCommand(root, SHUTDOWN, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
        notifyProgress(sender, 75);
            reply(sender, new events::ShutDownResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleBackupEvent(events::BackupRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::BackupRequestEvent::value_type res(ev->value());
        notifyProgress(sender, 25);
            FastoObjectIPtr root = FastoObject::createRoot(BACKUP);
            RedisCommand* cmd = createCommand(root, BACKUP, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            else{
                bool rc = common::file_system::copy_file("/var/lib/redis/dump.rdb", res.path_);
                if(!rc){
                    res.setErrorInfo(common::make_error_value("Copy failed.", common::ErrorValue::E_ERROR));
                }
            }
        notifyProgress(sender, 75);
            reply(sender, new events::BackupResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleExportEvent(events::ExportRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ExportRequestEvent::value_type res(ev->value());
        notifyProgress(sender, 25);
            bool rc = common::file_system::copy_file(res.path_, "/var/lib/redis/dump.rdb");
            if(!rc){
                res.setErrorInfo(common::make_error_value("Copy failed.", common::ErrorValue::E_ERROR));
            }
        notifyProgress(sender, 75);
            reply(sender, new events::ExportResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleChangePasswordEvent(events::ChangePasswordRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ChangePasswordRequestEvent::value_type res(ev->value());
        notifyProgress(sender, 25);
            char patternResult[1024] = {0};
            common::SNPrintf(patternResult, sizeof(patternResult), SET_PASSWORD_1ARGS_S, res.newPassword_);
            FastoObjectIPtr root = FastoObject::createRoot(patternResult);
            RedisCommand* cmd = createCommand(root, patternResult, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }

        notifyProgress(sender, 75);
            reply(sender, new events::ChangePasswordResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleChangeMaxConnectionEvent(events::ChangeMaxConnectionRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ChangeMaxConnectionRequestEvent::value_type res(ev->value());
        notifyProgress(sender, 25);
            char patternResult[1024] = {0};
            common::SNPrintf(patternResult, sizeof(patternResult), SET_MAX_CONNECTIONS_1ARGS_I, res.maxConnection_);
            FastoObjectIPtr root = FastoObject::createRoot(patternResult);
            RedisCommand* cmd = createCommand(root, patternResult, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }

        notifyProgress(sender, 75);
            reply(sender, new events::ChangeMaxConnectionResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    common::ErrorValueSPtr RedisDriver::interacteveMode(events::ProcessConfigArgsRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
        events::EnterModeEvent::value_type res(IntaractiveMode);
        reply(sender, new events::EnterModeEvent(this, res));

        events::LeaveModeEvent::value_type res2(IntaractiveMode);
        reply(sender, new events::LeaveModeEvent(this, res2));
        notifyProgress(sender, 100);
        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr RedisDriver::latencyMode(events::ProcessConfigArgsRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
        events::EnterModeEvent::value_type resEv(LatencyMode);
        reply(sender, new events::EnterModeEvent(this, resEv));

        RootLocker lock = make_locker(sender, LATENCY_REQUEST);

        FastoObjectIPtr obj = lock.root_;
        common::ErrorValueSPtr er = impl_->latencyMode(obj.get());
        if(er){
            LOG_ERROR(er, true);
        }

        events::LeaveModeEvent::value_type resEv2(LatencyMode);
        reply(sender, new events::LeaveModeEvent(this, resEv2));
        notifyProgress(sender, 100);
        return er;
    }

    common::ErrorValueSPtr RedisDriver::slaveMode(events::ProcessConfigArgsRequestEvent* ev)
    {
        QObject* sender = ev->sender();
        notifyProgress(sender, 0);
        events::EnterModeEvent::value_type resEv(SlaveMode);
        reply(sender, new events::EnterModeEvent(this, resEv));

        RootLocker lock = make_locker(sender, SYNC_REQUEST);

        FastoObjectIPtr obj = lock.root_;
        common::ErrorValueSPtr er = impl_->slaveMode(obj.get());
        if(er){
            LOG_ERROR(er, true);
        }

        events::LeaveModeEvent::value_type resEv2(SlaveMode);
        reply(sender, new events::LeaveModeEvent(this, resEv2));
        notifyProgress(sender, 100);
        return er;
    }

    common::ErrorValueSPtr RedisDriver::getRDBMode(events::ProcessConfigArgsRequestEvent* ev)
    {
        QObject* sender = ev->sender();
        notifyProgress(sender, 0);
        events::EnterModeEvent::value_type resEv(GetRDBMode);
        reply(sender, new events::EnterModeEvent(this, resEv));

        RootLocker lock = make_locker(sender, RDM_REQUEST);

        FastoObjectIPtr obj = lock.root_;
        common::ErrorValueSPtr er = impl_->getRDB(obj.get());
        if(er){
            LOG_ERROR(er, true);
        }

        events::LeaveModeEvent::value_type resEv2(GetRDBMode);
        reply(sender, new events::LeaveModeEvent(this, resEv2));
        notifyProgress(sender, 100);
        return er;
    }

    /*void RedisDriver::pipeMode(events::ProcessConfigArgsRequestEvent* ev)
    {
        QObject* sender = ev->sender();
        notifyProgress(sender, 0);
        events::EnterModeEvent::value_type res(PipeMode);
        reply(sender, new events::EnterModeEvent(this, res));

        events::LeaveModeEvent::value_type res2(PipeMode);
        reply(sender, new events::LeaveModeEvent(this, res2));
        notifyProgress(sender, 100);
    }*/

    common::ErrorValueSPtr RedisDriver::findBigKeysMode(events::ProcessConfigArgsRequestEvent* ev)
    {
        QObject* sender = ev->sender();
        notifyProgress(sender, 0);
        events::EnterModeEvent::value_type resEv(FindBigKeysMode);
        reply(sender, new events::EnterModeEvent(this, resEv));

        RootLocker lock = make_locker(sender, FIND_BIG_KEYS_REQUEST);

        FastoObjectIPtr obj = lock.root_;
        common::ErrorValueSPtr er = impl_->findBigKeys(obj.get());
        if(er){
            LOG_ERROR(er, true);
        }

        events::LeaveModeEvent::value_type resEv2(FindBigKeysMode);
        reply(sender, new events::LeaveModeEvent(this, resEv2));
        notifyProgress(sender, 100);
        return er;
    }

    common::ErrorValueSPtr RedisDriver::statMode(events::ProcessConfigArgsRequestEvent* ev)
    {
        QObject* sender = ev->sender();
        notifyProgress(sender, 0);
        events::EnterModeEvent::value_type resEv(StatMode);
        reply(sender, new events::EnterModeEvent(this, resEv));

        RootLocker lock = make_locker(sender, STAT_MODE_REQUEST);

        FastoObjectIPtr obj = lock.root_;
        common::ErrorValueSPtr er = impl_->statMode(obj.get());
        if(er){
            LOG_ERROR(er, true);
        }

        events::LeaveModeEvent::value_type resEv2(StatMode);
        reply(sender, new events::LeaveModeEvent(this, resEv2));
        notifyProgress(sender, 100);
        return er;
    }

    common::ErrorValueSPtr RedisDriver::scanMode(events::ProcessConfigArgsRequestEvent* ev)
    {
        QObject* sender = ev->sender();
        notifyProgress(sender, 0);
        events::EnterModeEvent::value_type resEv(ScanMode);
        reply(sender, new events::EnterModeEvent(this, resEv));

        RootLocker lock = make_locker(sender, SCAN_MODE_REQUEST);

        FastoObjectIPtr obj = lock.root_;
        common::ErrorValueSPtr er = impl_->scanMode(obj.get());
        if(er){
            LOG_ERROR(er, true);
        }

        events::LeaveModeEvent::value_type resEv2(ScanMode);
        reply(sender, new events::LeaveModeEvent(this, resEv2));
        notifyProgress(sender, 100);
        return er;
    }

    void RedisDriver::handleExecuteEvent(events::ExecuteRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ExecuteRequestEvent::value_type res(ev->value());
            const char *inputLine = common::utils::c_strornull(res.text_);

            common::ErrorValueSPtr er;
            if(inputLine){
                size_t length = strlen(inputLine);
                int offset = 0;
                RootLocker lock = make_locker(sender, inputLine);
                FastoObjectIPtr outRoot = lock.root_;
                double step = 100.0f/length;
                for(size_t n = 0; n < length; ++n){
                    if(impl_->config_.shutdown_){
                        er.reset(new common::ErrorValue("Interrupted exec.", common::ErrorValue::E_INTERRUPTED));
                        res.setErrorInfo(er);
                        break;
                    }

                    if(inputLine[n] == '\n' || n == length-1){
        notifyProgress(sender, step * n);
                        char command[128] = {0};
                        if(n == length-1){
                            strcpy(command, inputLine + offset);
                        }
                        else{
                            strncpy(command, inputLine + offset, n - offset);
                        }

                        offset = n + 1;
                        std::string stabc = stableCommand(command);
                        RedisCommand* cmd = createCommand(outRoot, stabc, common::Value::C_USER);
                        er = impl_->execute(cmd);
                        if(er){
                            res.setErrorInfo(er);
                            break;
                        }
                        else{
                            std::string cmdcom = cmd->inputCmd();
                            std::transform(cmdcom.begin(), cmdcom.end(), cmdcom.begin(), ::tolower);
                            FastoObject::child_container_type rchildrens = cmd->childrens();
                            if(cmdcom == "auth"){
                                if(rchildrens.size() == 1){
                                    FastoObject* obj = dynamic_cast<FastoObject*>(rchildrens[0]);
                                    impl_->isAuth_ = obj && obj->toString() == "OK";
                                }
                                else{
                                    impl_->isAuth_ = false;
                                }
                            }
                        }
                    }
                }
            }
            else{
                er.reset(new common::ErrorValue("Empty command line.", common::ErrorValue::E_ERROR));
            }

            if(er){
                LOG_ERROR(er, true);
            }
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleCommandRequestEvent(events::CommandRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::CommandResponceEvent::value_type res(ev->value());
            std::string cmdtext;
            common::ErrorValueSPtr er = commandByType(res.cmd_, cmdtext);
            if(er){
                res.setErrorInfo(er);
                reply(sender, new events::CommandResponceEvent(this, res));
                notifyProgress(sender, 100);
                return;
            }

            RootLocker lock = make_locker(sender, cmdtext);
            FastoObjectIPtr root = lock.root_;
            RedisCommand* cmd = createCommand(root, cmdtext, common::Value::C_INNER);
        notifyProgress(sender, 50);
            er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            reply(sender, new events::CommandResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleDisconnectEvent(events::DisconnectRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::DisconnectResponceEvent::value_type res(ev->value());            
        notifyProgress(sender, 50);

        if(impl_->context_){
            redisFree(impl_->context_);
            impl_->context_ = NULL;
        }

            reply(sender, new events::DisconnectResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleLoadDatabaseInfosEvent(events::LoadDatabasesInfoRequestEvent *ev)
    {
            QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::LoadDatabasesInfoResponceEvent::value_type res(ev->value());
            FastoObjectIPtr root = FastoObject::createRoot(GET_DATABASES);
        notifyProgress(sender, 50);
            RedisCommand* cmd = createCommand(root, GET_DATABASES, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            else{
                FastoObject::child_container_type rchildrens = cmd->childrens();
                if(rchildrens.size()){
                    DCHECK(rchildrens.size() == 1);
                    FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0]);
                    if(!array){
                        goto done;
                    }
                    common::ArrayValue* ar = array->array();
                    if(!ar){
                        goto done;
                    }

                    if(ar->getSize() == 2){
                        std::string scountDb;
                        bool isok = ar->getString(1, &scountDb);
                        if(isok){
                            int countDb = common::convertFromString<int>(scountDb);
                            if(countDb > 0){
                                for(int i = 0; i < countDb; ++i){
                                    DataBaseInfoSPtr dbInf(new RedisDataBaseInfo(common::convertToString(i), 0, false));
                                    if(dbInf->name() == currentDatabaseInfo_->name()){
                                        res.databases_.push_back(currentDatabaseInfo_);
                                    }
                                    else {
                                        res.databases_.push_back(dbInf);
                                    }
                                }                                
                            }
                        }
                    }
                    else{
                        res.databases_.push_back(currentDatabaseInfo_);
                    }

                    long long sz = 0;
                    er = impl_->getDbSize(sz);
                    currentDatabaseInfo_->setSize(sz);
                }
            }
    done:
        notifyProgress(sender, 75);
            reply(sender, new events::LoadDatabasesInfoResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleLoadDatabaseContentEvent(events::LoadDatabaseContentRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::LoadDatabaseContentResponceEvent::value_type res(ev->value());
            char patternResult[1024] = {0};
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEYS_PATTERN_2ARGS_ISI, res.cursorIn_, res.pattern_, res.countKeys_);
            FastoObjectIPtr root = FastoObject::createRoot(patternResult);
        notifyProgress(sender, 50);
            RedisCommand* cmd = createCommand(root, patternResult, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            else{
                FastoObject::child_container_type rchildrens = cmd->childrens();
                if(rchildrens.size()){
                    DCHECK(rchildrens.size() == 1);
                    FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(rchildrens[0]);
                    if(!array){
                        goto done;
                    }

                    common::ArrayValue* arm = array->array();
                    if(!arm->getSize()){
                        goto done;
                    }

                    std::string cursor;
                    bool isok = arm->getString(0, &cursor);
                    if(!isok){
                       goto done;
                    }

                    res.cursorOut_ = common::convertFromString<uint32_t>(cursor);

                    rchildrens = array->childrens();
                    if(!rchildrens.size()){
                        goto done;
                    }

                    FastoObject* obj = rchildrens[0];
                    FastoObjectArray* arr = dynamic_cast<FastoObjectArray*>(obj);
                    if(!arr){
                        goto done;
                    }

                    common::ArrayValue* ar = arr->array();
                    std::vector<FastoObjectCommandIPtr> cmds;
                    cmds.reserve(ar->getSize() * 2);
                    for(int i = 0; i < ar->getSize(); ++i){
                        std::string key;
                        bool isok = ar->getString(i, &key);
                        DCHECK(isok);
                        if(isok){
                            NKey ress(key);
                            cmds.push_back(createCommandFast("TYPE " + ress.key_, common::Value::C_INNER));
                            cmds.push_back(createCommandFast("TTL " + ress.key_, common::Value::C_INNER));
                            res.keys_.push_back(ress);
                        }
                    }

                    er = impl_->executeAsPipeline(cmds);
                    if(er){
                       goto done;
                    }

                    for(int i = 0; i < res.keys_.size(); ++i){
                        FastoObjectIPtr cmdType = cmds[i*2];
                        FastoObject::child_container_type tchildrens = cmdType->childrens();
                        if(tchildrens.size()){
                            DCHECK(tchildrens.size() == 1);
                            if(tchildrens.size() == 1){
                                FastoObject* type = tchildrens[0];
                                std::string typeRedis = type->toString();
                                res.keys_[i].type_ = convertFromStringRType(typeRedis);
                            }
                        }

                        FastoObjectIPtr cmdType2 = cmds[i*2+1];
                        tchildrens = cmdType2->childrens();
                        if(tchildrens.size()){
                            DCHECK(tchildrens.size() == 1);
                            if(tchildrens.size() == 1){
                                FastoObject* fttl = tchildrens[0];
                                common::Value* vttl = fttl->value();
                                int32_t ttl = 0;
                                if(vttl->getAsInteger(&ttl)){
                                    if(ttl != -1){
                                        res.keys_[i].ttl_msec_ = ttl * 1000;
                                    }
                                }
                            }
                        }
                    }
                }
            }
    done:
        notifyProgress(sender, 75);
            reply(sender, new events::LoadDatabaseContentResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleSetDefaultDatabaseEvent(events::SetDefaultDatabaseRequestEvent* ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::SetDefaultDatabaseResponceEvent::value_type res(ev->value());
            const std::string setDefCommand = SET_DEFAULT_DATABASE + res.inf_->name();
            FastoObjectIPtr root = FastoObject::createRoot(setDefCommand);
        notifyProgress(sender, 50);
            RedisCommand* cmd = createCommand(root, setDefCommand, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            else{
                long long sz = 0;
                er = impl_->getDbSize(sz);
                currentDatabaseInfo_.reset(new RedisDataBaseInfo(res.inf_->name(), sz, true));
            }
        notifyProgress(sender, 75);
            reply(sender, new events::SetDefaultDatabaseResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleLoadServerInfoEvent(events::ServerInfoRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
            events::ServerInfoResponceEvent::value_type res(ev->value());
            FastoObjectIPtr root = FastoObject::createRoot(INFO_REQUEST);
        notifyProgress(sender, 50);
            RedisCommand* cmd = createCommand(root, INFO_REQUEST, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            else{
                FastoObject::child_container_type ch = cmd->childrens();
                if(ch.size()){
                    DCHECK(ch.size() == 1);
                    ServerInfoSPtr red(makeRedisServerInfo(ch[0]));
                    res.setInfo(red);
                }
            }
        notifyProgress(sender, 75);
            reply(sender, new events::ServerInfoResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleLoadServerPropertyEvent(events::ServerPropertyInfoRequestEvent *ev)
    {
        QObject* sender = ev->sender();
        notifyProgress(sender, 0);
        events::ServerPropertyInfoResponceEvent::value_type res(ev->value());
            FastoObjectIPtr root = FastoObject::createRoot(GET_PROPERTY_SERVER);
        notifyProgress(sender, 50);
            RedisCommand* cmd = createCommand(root, GET_PROPERTY_SERVER, common::Value::C_INNER);
            common::ErrorValueSPtr er = impl_->execute(cmd);
            if(er){
                res.setErrorInfo(er);
            }
            else{
                FastoObject::child_container_type ch = cmd->childrens();
                if(ch.size()){
                    DCHECK(ch.size() == 1);
                    FastoObjectArray* array = dynamic_cast<FastoObjectArray*>(ch[0]);
                    if(array){
                        res.info_ = makeServerProperty(array);
                    }
                }
            }
        notifyProgress(sender, 75);
            reply(sender, new events::ServerPropertyInfoResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    void RedisDriver::handleServerPropertyChangeEvent(events::ChangeServerPropertyInfoRequestEvent *ev)
    {
        QObject *sender = ev->sender();
        notifyProgress(sender, 0);
        events::ChangeServerPropertyInfoResponceEvent::value_type res(ev->value());

        notifyProgress(sender, 50);
        const std::string changeRequest = "CONFIG SET " + res.newItem_.first + " " + res.newItem_.second;
        FastoObjectIPtr root = FastoObject::createRoot(changeRequest);
        RedisCommand* cmd = createCommand(root, changeRequest, common::Value::C_INNER);
        common::ErrorValueSPtr er = impl_->execute(cmd);
        if(er){
            res.setErrorInfo(er);
        }
        else{
            res.isChange_ = true;
        }
        notifyProgress(sender, 75);
            reply(sender, new events::ChangeServerPropertyInfoResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    common::ErrorValueSPtr RedisDriver::commandDeleteImpl(CommandDeleteKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        const NKey key = command->key();
        common::SNPrintf(patternResult, sizeof(patternResult), DELETE_KEY_PATTERN_1ARGS_S, key.key_);
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr RedisDriver::commandLoadImpl(CommandLoadKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        const NKey key = command->key();
        if(key.type_ == common::Value::TYPE_ARRAY){
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_LIST_PATTERN_1ARGS_S, key.key_);
        }
        else if(key.type_ == common::Value::TYPE_SET){
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_SET_PATTERN_1ARGS_S, key.key_);
        }
        else if(key.type_ == common::Value::TYPE_ZSET){
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_ZSET_PATTERN_1ARGS_S, key.key_);
        }
        else if(key.type_ == common::Value::TYPE_HASH){
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_HASH_PATTERN_1ARGS_S, key.key_);
        }
        else{
            common::SNPrintf(patternResult, sizeof(patternResult), GET_KEY_PATTERN_1ARGS_S, key.key_);\
        }
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    common::ErrorValueSPtr RedisDriver::commandCreateImpl(CommandCreateKey* command, std::string& cmdstring) const
    {
        char patternResult[1024] = {0};
        const NKey key = command->key();
        FastoObjectIPtr val = command->value();
        if(key.type_ == common::Value::TYPE_ARRAY){
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_LIST_PATTERN_2ARGS_SS, key.key_, val->toString());
        }
        else if(key.type_ == common::Value::TYPE_SET){
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_SET_PATTERN_2ARGS_SS, key.key_, val->toString());
        }
        else if(key.type_ == common::Value::TYPE_ZSET){
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_ZSET_PATTERN_2ARGS_SS, key.key_, val->toString());
        }
        else if(key.type_ == common::Value::TYPE_HASH){
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_HASH_PATTERN_2ARGS_SS, key.key_, val->toString());
        }
        else{
            common::SNPrintf(patternResult, sizeof(patternResult), SET_KEY_PATTERN_2ARGS_SS, key.key_, val->toString());\
        }
        cmdstring = patternResult;

        return common::ErrorValueSPtr();
    }

    void RedisDriver::handleDbValueChangeEvent(events::ChangeDbValueRequestEvent* ev)
    {
        QObject* sender = ev->sender();
        notifyProgress(sender, 0);
        events::ChangeDbValueResponceEvent::value_type res(ev->value());

        notifyProgress(sender, 50);
        const std::string changeRequest = res.command_ + " " + res.newItem_.valueString();
        FastoObjectIPtr root = FastoObject::createRoot(changeRequest);
        RedisCommand* cmd = createCommand(root, changeRequest, common::Value::C_INNER);
        common::ErrorValueSPtr er = impl_->execute(cmd);
        if(er){
            res.setErrorInfo(er);
        }
        else{
            res.isChange_ = true;
        }

        notifyProgress(sender, 75);
            reply(sender, new events::ChangeDbValueResponceEvent(this, res));
        notifyProgress(sender, 100);
    }

    ServerInfoSPtr RedisDriver::makeServerInfoFromString(const std::string& val)
    {
        ServerInfoSPtr res(makeRedisServerInfo(val));
        return res;
    }

    void RedisDriver::interrupt()
    {
        impl_->config_.shutdown_ = 1;
    }
}
