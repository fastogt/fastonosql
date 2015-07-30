#include "core/connection_settings.h"

#include <sstream>

#include "common/qt/convert_string.h"
#include "common/utils.h"
#include "common/net/net.h"
#include "common/logger.h"

#include "core/settings_manager.h"


#ifdef BUILD_WITH_REDIS
#include "core/redis/redis_settings.h"
#include "core/redis/redis_cluster_settings.h"
#endif
#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/memcached_settings.h"
#endif
#ifdef BUILD_WITH_SSDB
#include "core/ssdb/ssdb_settings.h"
#endif
#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/leveldb_settings.h"
#endif

#define LOGGING_REDIS_FILE_EXTENSION ".red"
#define LOGGING_MEMCACHED_FILE_EXTENSION ".mem"
#define LOGGING_SSDB_FILE_EXTENSION ".ssdb"

namespace
{
    const char magicNumber = 0x1E;
}

namespace fastonosql
{
    IConnectionSettings::IConnectionSettings(const std::string& connectionName, connectionTypes type)
        : connectionName_(connectionName), logging_enabled_(false), type_(type), msinterval_(60000)
    {

    }

    void IConnectionSettings::setConnectionName(const std::string& name)
    {
        connectionName_ = name;
    }

    IConnectionSettings::~IConnectionSettings()
    {

    }

    std::string IConnectionSettings::connectionName() const
    {
        return connectionName_;
    }

    connectionTypes IConnectionSettings::connectionType() const
    {
        return type_;
    }

    bool IConnectionSettings::loggingEnabled() const
    {
        return logging_enabled_;
    }

    void IConnectionSettings::setLoggingEnabled(bool isLogging)
    {
        logging_enabled_ = isLogging;
    }

    uint32_t IConnectionSettings::loggingMsTimeInterval() const
    {
        return msinterval_;
    }

    void IConnectionSettings::setLoggingMsTimeInterval(uint32_t mstime)
    {
        msinterval_ = mstime;
    }

    std::string IConnectionSettings::toString() const
    {
        char buff[1024] = {0};
        common::SNPrintf(buff, sizeof(buff), "%d,%s,%d", type_, connectionName_, logging_enabled_);
        return buff;
    }

    IConnectionSettingsBase::IConnectionSettingsBase(const std::string &connectionName, connectionTypes type)
        : IConnectionSettings(connectionName, type), hash_(), sshInfo_()
    {
        setConnectionNameAndUpdateHash(connectionName);
    }

    IConnectionSettingsBase::~IConnectionSettingsBase()
    {

    }

    void IConnectionSettingsBase::setConnectionNameAndUpdateHash(const std::string& name)
    {
        using namespace common::utils;
        setConnectionName(name);
        common::buffer_type bcon = common::convertFromString<common::buffer_type>(connectionName_);
        uint64_t v = hash::crc64(0, bcon);
        hash_ = common::convertToString(v);
    }

    std::string IConnectionSettingsBase::hash() const
    {
        return hash_;
    }

    std::string IConnectionSettingsBase::loggingPath() const
    {
        std::string logDir = common::convertToString(SettingsManager::instance().loggingDirectory());
        std::string ext;
        if(type_ == REDIS){
            ext = LOGGING_REDIS_FILE_EXTENSION;
        }
        else if(type_ == MEMCACHED){
            ext = LOGGING_MEMCACHED_FILE_EXTENSION;
        }
        else if(type_ == SSDB){
            ext = LOGGING_SSDB_FILE_EXTENSION;
        }
        else {
            NOTREACHED();
        }
        return logDir + hash() + ext;
    }

    std::string IConnectionSettingsBase::fullAddress() const
    {
        const common::net::hostAndPort h = host();
        return common::convertToString(h);
    }

    IConnectionSettingsBase* IConnectionSettingsBase::createFromType(connectionTypes type, const std::string& conName)
    {
#ifdef BUILD_WITH_REDIS
        if(type == REDIS){
            return new RedisConnectionSettings(conName);
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type == MEMCACHED){
            return new MemcachedConnectionSettings(conName);
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type == SSDB){
            return new SsdbConnectionSettings(conName);
        }
#endif
#ifdef BUILD_WITH_LEVELDB
        if(type == SSDB){
            return new LeveldbConnectionSettings(conName);
        }
#endif
        return NULL;
    }

    IConnectionSettingsBase* IConnectionSettingsBase::fromString(const std::string &val)
    {
        IConnectionSettingsBase *result = NULL;
        if(!val.empty()){
            size_t len = val.size();

            uint8_t commaCount = 0;
            std::string elText;

            for(size_t i = 0; i < len; ++i ){
                char ch = val[i];
                if(ch == ','){
                    if(commaCount == 0){
                        int crT = elText[0] - 48;
                        result = createFromType((connectionTypes)crT);
                        if(!result){
                            return NULL;
                        }
                    }
                    else if(commaCount == 1){
                        result->setConnectionNameAndUpdateHash(elText);
                    }
                    else if(commaCount == 2){
                        result->setLoggingEnabled(common::convertFromString<uint8_t>(elText));
                    }
                    else if(commaCount == 3){
                        result->initFromCommandLine(elText);
                        result->setSshInfo(SSHInfo(val.substr(i+1)));
                        break;
                    }
                    commaCount++;
                    elText.clear();
                }
                else{
                   elText += ch;
                }
            }
        }
        return result;
    }

    std::string IConnectionSettingsBase::toString() const
    {
        DCHECK(type_ != DBUNKNOWN);

        std::stringstream str;
        str << IConnectionSettings::toString() << ','
            << toCommandLine() << ',' << sshInfo_.toString();
        std::string res = str.str();
        return res;
    }

    SSHInfo IConnectionSettingsBase::sshInfo() const
    {
        return sshInfo_;
    }

    void IConnectionSettingsBase::setSshInfo(const SSHInfo& info)
    {
        sshInfo_ = info;
    }

    const char* useHelpText(connectionTypes type)
    {
        if(type == DBUNKNOWN){
            return NULL;
        }
        else if(type == REDIS){
            return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
                               "<b>-h &lt;hostname&gt;</b>      Server hostname (default: 127.0.0.1).<br/>"
                               "<b>-p &lt;port&gt;</b>          Server port (default: 6379).<br/>"
                               "<b>-s &lt;socket&gt;</b>        Server socket (overrides hostname and port).<br/>"
                               "<b>-a &lt;password&gt;</b>      Password to use when connecting to the server.<br/>"
                               "<b>-r &lt;repeat&gt;</b>        Execute specified command N times.<br/>"
                               "<b>-i &lt;interval&gt;</b>      When <b>-r</b> is used, waits &lt;interval&gt; seconds per command.<br/>"
                               "                   It is possible to specify sub-second times like <b>-i</b> 0.1.<br/>"
                               "<b>-n &lt;db&gt;</b>            Database number.<br/>"
                               "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>"
                               "<b>-c</b>                 Enable cluster mode (follow -ASK and -MOVED redirections).<br/>"
                               "<b>--latency</b>          Enter a special mode continuously sampling latency.<br/>"
                               "<b>--latency-history</b>  Like <b>--latency</b> but tracking latency changes over time.<br/>"
                               "                   Default time interval is 15 sec. Change it using <b>-i</b>.<br/>"
                               "<b>--slave</b>            Simulate a slave showing commands received from the master.<br/>"
                               "<b>--rdb &lt;filename&gt;</b>   Transfer an RDB dump from remote server to local file.<br/>"
                               /*"<b>--pipe</b>             Transfer raw Redis protocol from stdin to server.<br/>"
                               "<b>--pipe-timeout &lt;n&gt;</b> In <b>--pipe mode</b>, abort with error if after sending all data.<br/>"
                               "                   no reply is received within &lt;n&gt; seconds.<br/>"
                               "                   Default timeout: %d. Use 0 to wait forever.<br/>"*/
                               "<b>--bigkeys</b>          Sample Redis keys looking for big keys.<br/>"
                               "<b>--scan</b>             List all keys using the SCAN command.<br/>"
                               "<b>--pattern &lt;pat&gt;</b>    Useful with <b>--scan</b> to specify a SCAN pattern.<br/>"
                               "<b>--intrinsic-latency &lt;sec&gt;</b> Run a test to measure intrinsic system latency.<br/>"
                               "                   The test will run for the specified amount of seconds.<br/>"
                               "<b>--eval &lt;file&gt;</b>      Send an EVAL command using the Lua script at <b>&lt;file&gt;</b>.";
        }
        else if(type == MEMCACHED){
            return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
                               "<b>-h &lt;hostname&gt;</b>      Server hostname (default: 127.0.0.1).<br/>"
                               "<b>-p &lt;port&gt;</b>          Server port (default: 11211).<br/>"
                               "<b>-u &lt;username&gt;</b>      Username to use when connecting to the server.<br/>"
                               "<b>-a &lt;password&gt;</b>      Password to use when connecting to the server.<br/>"
                               "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>";
        }
        else if(type == SSDB){
            return "<b>Usage: [OPTIONS] [cmd [arg [arg ...]]]</b><br/>"
                               "<b>-h &lt;hostname&gt;</b>      Server hostname (default: 127.0.0.1).<br/>"
                               "<b>-p &lt;port&gt;</b>          Server port (default: 8888).<br/>"
                               "<b>-u &lt;username&gt;</b>      Username to use when connecting to the server.<br/>"
                               "<b>-a &lt;password&gt;</b>      Password to use when connecting to the server.<br/>"
                               "<b>-d &lt;delimiter&gt;</b>     Multi-bulk delimiter in for raw formatting (default: \\n).<br/>";
        }

        NOTREACHED();
        return NULL;
    }

    std::string defaultCommandLine(connectionTypes type)
    {
#ifdef BUILD_WITH_REDIS
        if(type == REDIS){
            redisConfig r;
            return common::convertToString(r);
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type == MEMCACHED){
            memcachedConfig r;
            return common::convertToString(r);
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type == SSDB){
            ssdbConfig r;
            return common::convertToString(r);
        }
#endif
#ifdef BUILD_WITH_LEVELDB
        if(type == LEVELDB){
            ssdbConfig r;
            return common::convertToString(r);
        }
#endif
        return std::string();
    }

    IClusterSettingsBase::IClusterSettingsBase(const std::string& connectionName, connectionTypes type)
        : IConnectionSettings(connectionName, type)
    {

    }

    IClusterSettingsBase::cluster_connection_type IClusterSettingsBase::nodes() const
    {
        return clusters_nodes_;
    }

    IConnectionSettingsBaseSPtr IClusterSettingsBase::root() const
    {
        if(clusters_nodes_.empty()){
            return IConnectionSettingsBaseSPtr();
        }

        return clusters_nodes_[0];
    }

    void IClusterSettingsBase::addNode(IConnectionSettingsBaseSPtr node)
    {
        if(!node){
            return;
        }

        clusters_nodes_.push_back(node);
    }

    IClusterSettingsBase* IClusterSettingsBase::createFromType(connectionTypes type, const std::string& conName)
    {
#ifdef BUILD_WITH_REDIS
        if(type == REDIS){
            return new RedisClusterSettings(conName);
        }
#endif

        return NULL;
    }

    IClusterSettingsBase* IClusterSettingsBase::fromString(const std::string& val)
    {
        IClusterSettingsBase *result = NULL;
        if(!val.empty()){
            const size_t len = val.size();

            uint8_t commaCount = 0;
            std::string elText;

            for(size_t i = 0; i < len; ++i){
                char ch = val[i];
                if(ch == ','){
                    if(commaCount == 0){
                        int crT = elText[0] - 48;
                        result = createFromType((connectionTypes)crT);
                        if(!result){
                            return NULL;
                        }
                    }
                    else if(commaCount == 1){
                        result->setConnectionName(elText);
                    }
                    else if(commaCount == 2){
                        result->setLoggingEnabled(common::convertFromString<uint8_t>(elText));
                        std::string serText;
                        for(size_t j = i + 2; j < len; ++j){
                            ch = val[j];
                            if(ch == magicNumber || j == len - 1){
                                IConnectionSettingsBaseSPtr ser(IConnectionSettingsBase::fromString(serText));
                                result->addNode(ser);
                                serText.clear();
                            }
                            else{
                                serText += ch;
                            }
                        }
                        break;
                    }
                    commaCount++;
                    elText.clear();
                }
                else{
                   elText += ch;
                }
            }
        }
        return result;
    }

    std::string IClusterSettingsBase::toString() const
    {
        DCHECK(type_ != DBUNKNOWN);

        std::stringstream str;
        str << IConnectionSettings::toString() << ',';
        for(int i = 0; i < clusters_nodes_.size(); ++i){
           IConnectionSettingsBaseSPtr serv = clusters_nodes_[i];
           if(serv){
               str << magicNumber << serv->toString();
           }
        }

        std::string res = str.str();
        return res;
    }

    IConnectionSettingsBaseSPtr IClusterSettingsBase::findSettingsByHost(const common::net::hostAndPort& host) const
    {
        for(int i = 0; i < clusters_nodes_.size(); ++i){
            IConnectionSettingsBaseSPtr cur = clusters_nodes_[i];
            if(cur->host() == host){
                return cur;
            }
        }

        return IConnectionSettingsBaseSPtr();
    }
}
