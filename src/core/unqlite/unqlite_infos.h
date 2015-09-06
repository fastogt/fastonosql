#pragma once

#include "core/types.h"

#define UNQLITE_STATS_LABEL "# Stats"

#define UNQLITE_CAMPACTIONS_LEVEL_LABEL "compactions_level"
#define UNQLITE_FILE_SIZE_MB_LABEL "file_size_mb"
#define UNQLITE_TIME_SEC_LABEL "time_sec"
#define UNQLITE_READ_MB_LABEL "read_mb"
#define UNQLITE_WRITE_MB_LABEL "write_mb"

namespace fastonosql
{
    extern const std::vector<std::string> unqliteHeaders;
    extern const std::vector<std::vector<Field> > unqliteFields;

    class UnqliteServerInfo
            : public ServerInfo
    {
    public:
        //Compactions\nLevel  Files Size(MB) Time(sec) Read(MB) Write(MB)\n
        struct Stats
                : FieldByIndex
        {
            Stats();
            explicit Stats(const std::string& common_text);
            common::Value* valueByIndex(unsigned char index) const;

            uint32_t compactions_level_;
            uint32_t file_size_mb_;
            uint32_t time_sec_;
            uint32_t read_mb_;
            uint32_t write_mb_;
        } stats_;

        UnqliteServerInfo();
        explicit UnqliteServerInfo(const Stats& stats);
        virtual common::Value* valueByIndexes(unsigned char property, unsigned char field) const;
        virtual std::string toString() const;
        virtual uint32_t version() const;
    };

    std::ostream& operator << (std::ostream& out, const UnqliteServerInfo& value);

    UnqliteServerInfo* makeUnqliteServerInfo(const std::string &content);
    UnqliteServerInfo* makeUnqliteServerInfo(FastoObject *root);

    class UnqliteDataBaseInfo
            : public DataBaseInfo
    {
    public:
        UnqliteDataBaseInfo(const std::string& name, size_t size, bool isDefault);
        virtual DataBaseInfo* clone() const;
    };

    template<>
    struct DBTraits<UNQLITE>
    {
        static const std::vector<common::Value::Type> supportedTypes;
    };

    class UnqliteCommand
            : public FastoObjectCommand
    {
    public:
        UnqliteCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr);
        virtual bool isReadOnly() const;
    };
}
