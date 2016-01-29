/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/leveldb/leveldb_infos.h"

#include <ostream>
#include <sstream>

namespace
{
    using namespace fastonosql;

    const std::vector<Field> LeveldbCommonFields =
    {
        Field(LEVELDB_CAMPACTIONS_LEVEL_LABEL, common::Value::TYPE_UINTEGER),
        Field(LEVELDB_FILE_SIZE_MB_LABEL, common::Value::TYPE_UINTEGER),
        Field(LEVELDB_TIME_SEC_LABEL, common::Value::TYPE_UINTEGER),
        Field(LEVELDB_READ_MB_LABEL, common::Value::TYPE_UINTEGER),
        Field(LEVELDB_WRITE_MB_LABEL, common::Value::TYPE_UINTEGER)
    };
}

namespace fastonosql
{
    template<>
    std::vector<common::Value::Type> DBTraits<LEVELDB>::supportedTypes()
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
    std::vector<std::string> DBTraits<LEVELDB>::infoHeaders()
    {
        return  { LEVELDB_STATS_LABEL };
    }

    template<>
    std::vector<std::vector<Field> > DBTraits<LEVELDB>::infoFields()
    {
        return  { LeveldbCommonFields };
    }

    LeveldbServerInfo::Stats::Stats()
        : compactions_level_(0), file_size_mb_(0), time_sec_(0), read_mb_(0), write_mb_(0)
    {

    }

    LeveldbServerInfo::Stats::Stats(const std::string& common_text)
    {
        const std::string &src = common_text;
        size_t pos = 0;
        size_t start = 0;

        while((pos = src.find(("\r\n"), start)) != std::string::npos){
            std::string line = src.substr(start, pos-start);
            size_t delem = line.find_first_of(':');
            std::string field = line.substr(0, delem);
            std::string value = line.substr(delem + 1);
            if(field == LEVELDB_CAMPACTIONS_LEVEL_LABEL){
                compactions_level_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == LEVELDB_FILE_SIZE_MB_LABEL){
                file_size_mb_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == LEVELDB_TIME_SEC_LABEL){
                time_sec_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == LEVELDB_READ_MB_LABEL){
                read_mb_ = common::convertFromString<uint32_t>(value);
            }
            else if(field == LEVELDB_WRITE_MB_LABEL){
                write_mb_ = common::convertFromString<uint32_t>(value);
            }
            start = pos + 2;
        }
    }

    common::Value* LeveldbServerInfo::Stats::valueByIndex(unsigned char index) const
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

    LeveldbServerInfo::LeveldbServerInfo()
        : ServerInfo(LEVELDB)
    {

    }

    LeveldbServerInfo::LeveldbServerInfo(const Stats &stats)
        : ServerInfo(LEVELDB), stats_(stats)
    {

    }

    common::Value* LeveldbServerInfo::valueByIndexes(unsigned char property, unsigned char field) const
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

    std::ostream& operator<<(std::ostream& out, const LeveldbServerInfo::Stats& value)
    {
        return out << LEVELDB_CAMPACTIONS_LEVEL_LABEL":" << value.compactions_level_ << ("\r\n")
                    << LEVELDB_FILE_SIZE_MB_LABEL":" << value.file_size_mb_ << ("\r\n")
                    << LEVELDB_TIME_SEC_LABEL":" << value.time_sec_ << ("\r\n")
                    << LEVELDB_READ_MB_LABEL":" << value.read_mb_ << ("\r\n")
                    << LEVELDB_WRITE_MB_LABEL":" << value.write_mb_ << ("\r\n");
    }

    std::ostream& operator<<(std::ostream& out, const LeveldbServerInfo& value)
    {
        return out << value.toString();
    }

    LeveldbServerInfo* makeLeveldbServerInfo(const std::string &content)
    {
        if(content.empty()){
            return NULL;
        }

        LeveldbServerInfo* result = new LeveldbServerInfo;

        const std::vector<std::string> headers = DBTraits<LEVELDB>::infoHeaders();
        std::string word;
        DCHECK(headers.size() == 1);

        for(int i = 0; i < content.size(); ++i)
        {
            word += content[i];
            if(word == headers[0]){
                std::string part = content.substr(i + 1);
                result->stats_ = LeveldbServerInfo::Stats(part);
                break;
            }
        }

        return result;
    }


    std::string LeveldbServerInfo::toString() const
    {
        std::stringstream str;
        str << LEVELDB_STATS_LABEL"\r\n" << stats_;
        return str.str();
    }

    uint32_t LeveldbServerInfo::version() const
    {
        return 0;
    }

    LeveldbServerInfo* makeLeveldbServerInfo(FastoObject* root)
    {
        const std::string content = common::convertToString(root);
        return makeLeveldbServerInfo(content);
    }

    LeveldbDataBaseInfo::LeveldbDataBaseInfo(const std::string& name, bool isDefault, size_t size, const keys_cont_type &keys)
        : DataBaseInfo(name, isDefault, LEVELDB, size, keys)
    {

    }

    DataBaseInfo* LeveldbDataBaseInfo::clone() const
    {
        return new LeveldbDataBaseInfo(*this);
    }

    LeveldbCommand::LeveldbCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr)
        : FastoObjectCommand(parent, cmd, delemitr)
    {

    }

    bool LeveldbCommand::isReadOnly() const
    {
        std::string key = inputCmd();
        if(key.empty()){
            return true;
        }

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        return key != "get";
    }
}
