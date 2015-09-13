#include "core/memcached/memcached_infos.h"

#include <ostream>
#include <sstream>

namespace
{
    using namespace fastonosql;

    const std::vector<Field> memcachedCommonFields =
    {
        Field(MEMCACHED_PID_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_UPTIME_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_TIME_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_VERSION_LABEL, common::Value::TYPE_STRING),
        Field(MEMCACHED_POINTER_SIZE_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_RUSAGE_USER_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_RUSAGE_SYSTEM_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_CURR_ITEMS_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_TOTAL_ITEMS_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_BYTES_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_CURR_CONNECTIONS_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_TOTAL_CONNECTIONS_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_CONNECTION_STRUCTURES_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_CMD_GET_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_CMD_SET_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_GET_HITS_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_GET_MISSES_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_EVICTIONS_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_BYTES_READ_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_BYTES_WRITTEN_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_LIMIT_MAXBYTES_LABEL, common::Value::TYPE_UINTEGER),
        Field(MEMCACHED_THREADS_LABEL, common::Value::TYPE_UINTEGER)
    };
}

namespace fastonosql
{
    template<>
    std::vector<common::Value::Type> DBTraits<MEMCACHED>::supportedTypes()
    {
        return  {
                    common::Value::TYPE_BOOLEAN,
                    common::Value::TYPE_INTEGER,
                    common::Value::TYPE_UINTEGER,
                    common::Value::TYPE_DOUBLE,
                    common::Value::TYPE_STRING,
                    common::Value::TYPE_ARRAY
                };
    }

    template<>
    std::vector<std::string> DBTraits<MEMCACHED>::infoHeaders()
    {
        return  { MEMCACHED_COMMON_LABEL };
    }

    template<>
    std::vector<std::vector<Field> > DBTraits<MEMCACHED>::infoFields()
    {
        return  { memcachedCommonFields };
    }

    MemcachedServerInfo::Common::Common()
    {

    }

    MemcachedServerInfo::Common::Common(const std::string& common_text)
    {
        const std::string &src = common_text;
        size_t pos = 0;
        size_t start = 0;

        while((pos = src.find(("\r\n"), start)) != std::string::npos){
            std::string line = src.substr(start, pos-start);
            size_t delem = line.find_first_of(':');
            std::string field = line.substr(0, delem);
            std::string value = line.substr(delem + 1);
            if(field == MEMCACHED_PID_LABEL){
                pid_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_UPTIME_LABEL){
                uptime_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_TIME_LABEL){
                time_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_VERSION_LABEL){
                version_ = value;
            }
            else if(field == MEMCACHED_POINTER_SIZE_LABEL){
                pointer_size_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_RUSAGE_USER_LABEL){
                rusage_user_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_RUSAGE_SYSTEM_LABEL){
                rusage_system_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_CURR_ITEMS_LABEL){
                curr_items_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_TOTAL_ITEMS_LABEL){
                total_items_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_BYTES_LABEL){
                bytes_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_CURR_CONNECTIONS_LABEL){
                curr_connections_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_TOTAL_CONNECTIONS_LABEL){
                total_connections_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_CONNECTION_STRUCTURES_LABEL){
                connection_structures_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_CMD_GET_LABEL){
                cmd_get_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_CMD_SET_LABEL){
                cmd_set_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_GET_HITS_LABEL){
                get_hits_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_GET_MISSES_LABEL){
                get_misses_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_EVICTIONS_LABEL){
                evictions_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_BYTES_READ_LABEL){
                bytes_read_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_BYTES_WRITTEN_LABEL){
                bytes_written_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_LIMIT_MAXBYTES_LABEL){
                limit_maxbytes_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == MEMCACHED_THREADS_LABEL){
                threads_ = common::convertFromString<uint32_t>(value);
            }
            start = pos + 2;
        }
    }

    common::Value* MemcachedServerInfo::Common::valueByIndex(unsigned char index) const
    {
        switch (index) {
        case 0:
            return new common::FundamentalValue(pid_);
        case 1:
            return new common::FundamentalValue(uptime_);
        case 2:
            return new common::FundamentalValue(time_);
        case 3:
            return new common::StringValue(version_);
        case 4:
            return new common::FundamentalValue(pointer_size_);
        case 5:
            return new common::FundamentalValue(rusage_user_);
        case 6:
            return new common::FundamentalValue(rusage_system_);
        case 7:
            return new common::FundamentalValue(curr_items_);
        case 8:
            return new common::FundamentalValue(total_items_);
        case 9:
            return new common::FundamentalValue(bytes_);
        case 10:
            return new common::FundamentalValue(curr_connections_);
        case 11:
            return new common::FundamentalValue(total_connections_);
        case 12:
            return new common::FundamentalValue(connection_structures_);
        case 13:
            return new common::FundamentalValue(cmd_get_);
        case 14:
            return new common::FundamentalValue(cmd_set_);
        case 15:
            return new common::FundamentalValue(get_hits_);
        case 16:
            return new common::FundamentalValue(get_misses_);
        case 17:
            return new common::FundamentalValue(evictions_);
        case 18:
            return new common::FundamentalValue(bytes_read_);
        case 19:
            return new common::FundamentalValue(bytes_written_);
        case 20:
            return new common::FundamentalValue(limit_maxbytes_);
        case 21:
            return new common::FundamentalValue(threads_);
        default:
            NOTREACHED();
            break;
        }
        return NULL;
    }

    MemcachedServerInfo::MemcachedServerInfo()
        : ServerInfo(MEMCACHED)
    {

    }

    MemcachedServerInfo::MemcachedServerInfo(const Common& common)
        : ServerInfo(MEMCACHED), common_(common)
    {

    }

    common::Value* MemcachedServerInfo::valueByIndexes(unsigned char property, unsigned char field) const
    {
        switch (property) {
        case 0:
            return common_.valueByIndex(field);
        default:
            NOTREACHED();
            break;
        }
        return NULL;
    }

    std::ostream& operator<<(std::ostream& out, const MemcachedServerInfo::Common& value)
    {
        return out << MEMCACHED_PID_LABEL":" << value.pid_ << ("\r\n")
                    << MEMCACHED_UPTIME_LABEL":" << value.uptime_ << ("\r\n")
                    << MEMCACHED_TIME_LABEL":" << value.time_ << ("\r\n")
                    << MEMCACHED_VERSION_LABEL":" << value.version_ << ("\r\n")
                    << MEMCACHED_POINTER_SIZE_LABEL":" << value.pointer_size_ << ("\r\n")
                    << MEMCACHED_RUSAGE_USER_LABEL":" << value.rusage_user_ << ("\r\n")
                    << MEMCACHED_RUSAGE_SYSTEM_LABEL":" << value.rusage_system_ << ("\r\n")
                    << MEMCACHED_CURR_ITEMS_LABEL":" << value.curr_items_ << ("\r\n")
                    << MEMCACHED_TOTAL_ITEMS_LABEL":" << value.total_items_ << ("\r\n")
                    << MEMCACHED_BYTES_LABEL":" << value.bytes_ << ("\r\n")
                    << MEMCACHED_CURR_CONNECTIONS_LABEL":" << value.curr_connections_ << ("\r\n")
                    << MEMCACHED_TOTAL_CONNECTIONS_LABEL":" << value.total_connections_ << ("\r\n")
                    << MEMCACHED_CONNECTION_STRUCTURES_LABEL":" << value.connection_structures_ << ("\r\n")
                    << MEMCACHED_CMD_GET_LABEL":" << value.cmd_get_ << ("\r\n")
                    << MEMCACHED_CMD_SET_LABEL":" << value.cmd_set_ << ("\r\n")
                    << MEMCACHED_GET_HITS_LABEL":" << value.get_hits_ << ("\r\n")
                    << MEMCACHED_GET_MISSES_LABEL":" << value.get_misses_ << ("\r\n")
                    << MEMCACHED_EVICTIONS_LABEL":" << value.evictions_ << ("\r\n")
                    << MEMCACHED_BYTES_READ_LABEL":" << value.bytes_read_ << ("\r\n")
                    << MEMCACHED_BYTES_WRITTEN_LABEL":" << value.bytes_written_ << ("\r\n")
                    << MEMCACHED_LIMIT_MAXBYTES_LABEL":" << value.limit_maxbytes_ << ("\r\n")
                    << MEMCACHED_THREADS_LABEL":" << value.threads_ << ("\r\n");
    }

    std::ostream& operator<<(std::ostream& out, const MemcachedServerInfo& value)
    {
        return out << value.toString();
    }

    MemcachedServerInfo* makeMemcachedServerInfo(const std::string &content)
    {
        if(content.empty()){
            return NULL;
        }

        MemcachedServerInfo* result = new MemcachedServerInfo;

        int j = 0;
        std::string word;
        size_t pos = 0;
        const std::vector<std::string> headers = DBTraits<MEMCACHED>::infoHeaders();

        for(int i = 0; i < content.size(); ++i)
        {
            char ch = content[i];
            word += ch;
            if(word == headers[j]){
                if(j+1 != headers.size()){
                    pos = content.find(headers[j+1], pos);
                }
                else{
                    break;
                }

                if(pos != std::string::npos){
                    std::string part = content.substr(i + 1, pos - i - 1 );
                    switch(j)
                    {
                    case 0:
                        result->common_ = MemcachedServerInfo::Common(part);
                        break;
                    default:
                        break;
                    }
                    i = pos-1;
                    ++j;
                }
                word.clear();
            }
        }

        return result;
    }


    std::string MemcachedServerInfo::toString() const
    {
        std::stringstream str;
        str << MEMCACHED_COMMON_LABEL"\r\n" << common_;
        return str.str();
    }

    uint32_t MemcachedServerInfo::version() const
    {
        return common::convertVersionNumberFromString(common_.version_);
    }

    MemcachedServerInfo* makeMemcachedServerInfo(FastoObject* root)
    {
        const std::string content = common::convertToString(root);
        return makeMemcachedServerInfo(content);
    }

    MemcachedDataBaseInfo::MemcachedDataBaseInfo(const std::string& name, bool isDefault, size_t size, const keys_cont_type &keys)
        : DataBaseInfo(name, isDefault, MEMCACHED, size, keys)
    {

    }

    DataBaseInfo* MemcachedDataBaseInfo::clone() const
    {
        return new MemcachedDataBaseInfo(*this);
    }

    MemcachedCommand::MemcachedCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr)
        : FastoObjectCommand(parent, cmd, delemitr)
    {

    }

    bool MemcachedCommand::isReadOnly() const
    {
        std::string key = inputCmd();
        if(key.empty()){
            return true;
        }

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        return key != "get";
    }
}
