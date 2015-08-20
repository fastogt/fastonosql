#include "core/ssdb/ssdb_infos.h"

#include <ostream>
#include <sstream>

namespace
{
    using namespace fastonosql;

    const std::vector<Field> SsdbCommonFields =
    {
        Field(SSDB_VERSION_LABEL, common::Value::TYPE_STRING),
        Field(SSDB_LINKS_LABEL, common::Value::TYPE_UINTEGER),
        Field(SSDB_TOTAL_CALLS_LABEL, common::Value::TYPE_UINTEGER),
        Field(SSDB_DBSIZE_LABEL, common::Value::TYPE_UINTEGER),
        Field(SSDB_BINLOGS_LABEL, common::Value::TYPE_STRING)
    };
}

namespace fastonosql
{   
    const std::vector<common::Value::Type> DBTraits<SSDB>::supportedTypes = {
                                            common::Value::TYPE_BOOLEAN,
                                            common::Value::TYPE_INTEGER,
                                            common::Value::TYPE_UINTEGER,
                                            common::Value::TYPE_DOUBLE,
                                            common::Value::TYPE_STRING,
                                            common::Value::TYPE_ARRAY,
                                            common::Value::TYPE_SET,
                                            common::Value::TYPE_ZSET,
                                            common::Value::TYPE_HASH
                                           };

    const std::vector<std::string> ssdbHeaders =
    {
        SSDB_COMMON_LABEL
    };

    const std::vector<std::vector<Field> > SsdbFields =
    {
        SsdbCommonFields
    };

    SsdbServerInfo::Common::Common()
    {

    }

    SsdbServerInfo::Common::Common(const std::string& common_text)
    {
        const std::string &src = common_text;
        size_t pos = 0;
        size_t start = 0;

        while((pos = src.find(("\r\n"), start)) != std::string::npos){
            std::string line = src.substr(start, pos-start);
            size_t delem = line.find_first_of(':');
            std::string field = line.substr(0, delem);
            std::string value = line.substr(delem + 1);
            if(field == SSDB_VERSION_LABEL){
                version_ = value;
            }
            else if(field == SSDB_LINKS_LABEL){
                links_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == SSDB_TOTAL_CALLS_LABEL){
                total_calls_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == SSDB_DBSIZE_LABEL){
                dbsize_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == SSDB_BINLOGS_LABEL){
                binlogs_ = value;
            }
            start = pos + 2;
        }
    }

    common::Value* SsdbServerInfo::Common::valueByIndex(unsigned char index) const
    {
        switch (index) {
        case 0:
            return new common::StringValue(version_);
        case 1:
            return new common::FundamentalValue(links_);
        case 2:
            return new common::FundamentalValue(total_calls_);
        case 3:
            return new common::FundamentalValue(dbsize_);
        case 4:
            return new common::StringValue(binlogs_);
        default:
            NOTREACHED();
            break;
        }
        return NULL;
    }

    SsdbServerInfo::SsdbServerInfo()
        : ServerInfo(SSDB)
    {

    }

    SsdbServerInfo::SsdbServerInfo(const Common& common)
        : ServerInfo(SSDB), common_(common)
    {

    }

    common::Value* SsdbServerInfo::valueByIndexes(unsigned char property, unsigned char field) const
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

    std::ostream& operator<<(std::ostream& out, const SsdbServerInfo::Common& value)
    {
        return out << SSDB_VERSION_LABEL":" << value.version_ << ("\r\n")
                    << SSDB_LINKS_LABEL":" << value.links_ << ("\r\n")
                    << SSDB_TOTAL_CALLS_LABEL":" << value.total_calls_ << ("\r\n")
                    << SSDB_DBSIZE_LABEL":" << value.dbsize_ << ("\r\n")
                    << SSDB_BINLOGS_LABEL":" << value.binlogs_ << ("\r\n");
    }

    std::ostream& operator<<(std::ostream& out, const SsdbServerInfo& value)
    {
        return out << value.toString();
    }

    SsdbServerInfo* makeSsdbServerInfo(const std::string &content)
    {
        if(content.empty()){
            return NULL;
        }

        SsdbServerInfo* result = new SsdbServerInfo;

        std::string word;
        DCHECK(ssdbHeaders.size() == 1);
        for(int i = 0; i < content.size(); ++i)
        {
            word += content[i];
            if(word == ssdbHeaders[0]){
                std::string part = content.substr(i + 1);
                result->common_ = SsdbServerInfo::Common(part);
                break;
            }
        }

        return result;
    }


    std::string SsdbServerInfo::toString() const
    {
        std::stringstream str;
        str << SSDB_COMMON_LABEL"\r\n" << common_;
        return str.str();
    }

    uint32_t SsdbServerInfo::version() const
    {
        return common::convertVersionNumberFromString(common_.version_);
    }

    SsdbServerInfo* makeSsdbServerInfo(FastoObject* root)
    {
        const std::string content = common::convertToString(root);
        return makeSsdbServerInfo(content);
    }

    SsdbDataBaseInfo::SsdbDataBaseInfo(const std::string& name, size_t size, bool isDefault)
        : DataBaseInfo(name, size, isDefault, SSDB)
    {

    }

    DataBaseInfo* SsdbDataBaseInfo::clone() const
    {
        return new SsdbDataBaseInfo(*this);
    }


    SsdbCommand::SsdbCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr)
        : FastoObjectCommand(parent, cmd, delemitr)
    {

    }

    bool SsdbCommand::isReadOnly() const
    {
        std::string key = inputCmd();
        if(key.empty()){
            return true;
        }

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        return key != "get";
    }
}
