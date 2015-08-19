#pragma once

#include "core/core_fwd.h"

#include "events/events_info.h"

namespace fastonosql
{
    class IDatabase
    {
    public:
        virtual ~IDatabase();

        connectionTypes type() const;
        IServerSPtr server() const;
        bool isDefault() const;
        std::string name() const;

        void loadContent(const EventsInfo::LoadDatabaseContentRequest &req);
        void setDefault(const EventsInfo::SetDefaultDatabaseRequest &req);

        DataBaseInfoSPtr info() const;
        void setInfo(DataBaseInfoSPtr info);

        void executeCommand(const EventsInfo::CommandRequest& req);

    protected:
        IDatabase(IServerSPtr server, DataBaseInfoSPtr info);

        DataBaseInfoSPtr info_;
        const IServerSPtr server_;
    };
}
