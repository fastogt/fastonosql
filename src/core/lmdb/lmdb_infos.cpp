#include "core/lmdb/lmdb_infos.h"

#include <ostream>
#include <sstream>

namespace
{
    using namespace fastonosql;

    const std::vector<Field> lmdbCommonFields =
    {
        Field(LMDB_CAMPACTIONS_LEVEL_LABEL, common::Value::TYPE_UINTEGER),
        Field(LMDB_FILE_SIZE_MB_LABEL, common::Value::TYPE_UINTEGER),
        Field(LMDB_TIME_SEC_LABEL, common::Value::TYPE_UINTEGER),
        Field(LMDB_READ_MB_LABEL, common::Value::TYPE_UINTEGER),
        Field(LMDB_WRITE_MB_LABEL, common::Value::TYPE_UINTEGER)
    };
}

namespace fastonosql
{
    template<>
    std::vector<common::Value::Type> DBTraits<LMDB>::supportedTypes()
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
    std::vector<std::string> DBTraits<LMDB>::infoHeaders()
    {
        return { LMDB_STATS_LABEL };
    }

    template<>
    std::vector<std::vector<Field> > DBTraits<LMDB>::infoFields()
    {
        return { lmdbCommonFields };
    }

    LmdbServerInfo::Stats::Stats()
        : compactions_level_(0), file_size_mb_(0), time_sec_(0), read_mb_(0), write_mb_(0)
    {

    }

    LmdbServerInfo::Stats::Stats(const std::string& common_text)
    {
        const std::string &src = common_text;
        size_t pos = 0;
        size_t start = 0;

        while((pos = src.find(("\r\n"), start)) != std::string::npos){
            std::string line = src.substr(start, pos-start);
            size_t delem = line.find_first_of(':');
            std::string field = line.substr(0, delem);
            std::string value = line.substr(delem + 1);
            if(field == LMDB_CAMPACTIONS_LEVEL_LABEL){
                compactions_level_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == LMDB_FILE_SIZE_MB_LABEL){
                file_size_mb_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == LMDB_TIME_SEC_LABEL){
                time_sec_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == LMDB_READ_MB_LABEL){
                read_mb_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == LMDB_WRITE_MB_LABEL){
                write_mb_ = common::convertFromString<uint32_t>(value);
            }
            start = pos + 2;
        }
    }

    common::Value* LmdbServerInfo::Stats::valueByIndex(unsigned char index) const
    {
        switch (index) {
        case 0:
            return new common::FundamentalValue(compactions_level_);
        case 1:
            return new common::FundamentalValue(file_size_mb_);
        case 2:
            return new common::FundamentalValue(time_sec_);
        case 3:
            return new common::FundamentalValue(read_mb_);
        case 4:
            return new common::FundamentalValue(write_mb_);
        default:
            NOTREACHED();
            break;
        }
        return NULL;
    }

    LmdbServerInfo::LmdbServerInfo()
        : ServerInfo(ROCKSDB)
    {

    }

    LmdbServerInfo::LmdbServerInfo(const Stats &stats)
        : ServerInfo(ROCKSDB), stats_(stats)
    {

    }

    common::Value* LmdbServerInfo::valueByIndexes(unsigned char property, unsigned char field) const
    {
        switch (property) {
        case 0:
            return stats_.valueByIndex(field);
        default:
            NOTREACHED();
            break;
        }
        return NULL;
    }

    std::ostream& operator<<(std::ostream& out, const LmdbServerInfo::Stats& value)
    {
        return out << LMDB_CAMPACTIONS_LEVEL_LABEL":" << value.compactions_level_ << ("\r\n")
                    << LMDB_FILE_SIZE_MB_LABEL":" << value.file_size_mb_ << ("\r\n")
                    << LMDB_TIME_SEC_LABEL":" << value.time_sec_ << ("\r\n")
                    << LMDB_READ_MB_LABEL":" << value.read_mb_ << ("\r\n")
                    << LMDB_WRITE_MB_LABEL":" << value.write_mb_ << ("\r\n");
    }

    std::ostream& operator<<(std::ostream& out, const LmdbServerInfo& value)
    {
        return out << value.toString();
    }

    LmdbServerInfo* makeLmdbServerInfo(const std::string &content)
    {
        if(content.empty()){
            return NULL;
        }

        LmdbServerInfo* result = new LmdbServerInfo;

        const std::vector<std::string> headers = DBTraits<LMDB>::infoHeaders();
        std::string word;
        DCHECK(headers.size() == 1);

        for(int i = 0; i < content.size(); ++i){
            word += content[i];
            if(word == headers[0]){
                std::string part = content.substr(i + 1);
                result->stats_ = LmdbServerInfo::Stats(part);
                break;
            }
        }

        return result;
    }


    std::string LmdbServerInfo::toString() const
    {
        std::stringstream str;
        str << LMDB_STATS_LABEL"\r\n" << stats_;
        return str.str();
    }

    uint32_t LmdbServerInfo::version() const
    {
        return 0;
    }

    LmdbServerInfo* makeLmdbServerInfo(FastoObject* root)
    {
        const std::string content = common::convertToString(root);
        return makeLmdbServerInfo(content);
    }

    LmdbDataBaseInfo::LmdbDataBaseInfo(const std::string& name, size_t size, bool isDefault)
        : DataBaseInfo(name, size, isDefault, LMDB)
    {

    }

    DataBaseInfo* LmdbDataBaseInfo::clone() const
    {
        return new LmdbDataBaseInfo(*this);
    }

    LmdbCommand::LmdbCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr)
        : FastoObjectCommand(parent, cmd, delemitr)
    {

    }

    bool LmdbCommand::isReadOnly() const
    {
        std::string key = inputCmd();
        if(key.empty()){
            return true;
        }

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        return key != "get";
    }
}
