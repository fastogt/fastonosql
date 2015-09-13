#pragma once

#include "core/types.h"

#define SSDB_COMMON_LABEL "# Common"

#define SSDB_VERSION_LABEL "version"
#define SSDB_LINKS_LABEL "links"
#define SSDB_TOTAL_CALLS_LABEL "total_calls"
#define SSDB_DBSIZE_LABEL "dbsize"
#define SSDB_BINLOGS_LABEL "binlogs"

namespace fastonosql
{
    class SsdbServerInfo
            : public ServerInfo
    {
    public:
        struct Common
                : FieldByIndex
        {
            Common();
            explicit Common(const std::string& common_text);
            common::Value* valueByIndex(unsigned char index) const;

            std::string version_;
            uint32_t links_;
            uint32_t total_calls_;
            uint32_t dbsize_;
            std::string binlogs_;
        } common_;

        SsdbServerInfo();
        explicit SsdbServerInfo(const Common& common);
        virtual common::Value* valueByIndexes(unsigned char property, unsigned char field) const;
        virtual std::string toString() const;
        virtual uint32_t version() const;
    };

    std::ostream& operator << (std::ostream& out, const SsdbServerInfo& value);

    SsdbServerInfo* makeSsdbServerInfo(const std::string &content);
    SsdbServerInfo* makeSsdbServerInfo(FastoObject *root);

    class SsdbDataBaseInfo
            : public DataBaseInfo
    {
    public:
        SsdbDataBaseInfo(const std::string& name, bool isDefault, size_t size, const keys_cont_type& keys = keys_cont_type());
        virtual DataBaseInfo* clone() const;
    };

    class SsdbCommand
            : public FastoObjectCommand
    {
    public:
        SsdbCommand(FastoObject* parent, common::CommandValue* cmd, const std::string &delemitr);
        virtual bool isReadOnly() const;
    };
}
