#pragma once

#include "core/core_fwd.h"

namespace fastoredis
{
    class IDatabase
    {
    public:
        virtual ~IDatabase();

        connectionTypes type() const;
        IServerSPtr server() const;
        bool isDefault() const;
        std::string name() const;

        void loadContent(const std::string& pattern, uint32_t countKeys, uint32_t cursor = 0);
        void setDefault();

        DataBaseInfoSPtr info() const;
        void setInfo(DataBaseInfoSPtr info);

        void removeKey(const NKey& key);
        void loadValue(const NKey& key);
        void createKey(const NKey& key, FastoObjectIPtr value);

    protected:
        IDatabase(IServerSPtr server, DataBaseInfoSPtr info);

        DataBaseInfoSPtr info_;
        const IServerSPtr server_;
    };
}
