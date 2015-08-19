#include "core/idatabase.h"

#include "core/iserver.h"

namespace fastonosql
{
    IDatabase::IDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : info_(info), server_(server)
    {
        DCHECK(server);
        DCHECK(info);
        DCHECK(server->type() == info->type());
    }

    IDatabase::~IDatabase()
    {

    }

    connectionTypes IDatabase::type() const
    {
        return server_->type();
    }

    IServerSPtr IDatabase::server() const
    {
        return server_;
    }

    bool IDatabase::isDefault() const
    {
        return info_->isDefault();
    }

    std::string IDatabase::name() const
    {
        return info_->name();
    }

    void IDatabase::loadContent(const EventsInfo::LoadDatabaseContentRequest& req)
    {
        DCHECK(req.inf_ == info_);
        server_->loadDatabaseContent(req);
    }

    void IDatabase::setDefault(const EventsInfo::SetDefaultDatabaseRequest& req)
    {
        DCHECK(req.inf_ == info_);
        server_->setDefaultDb(req);
    }

    DataBaseInfoSPtr IDatabase::info() const
    {
        return info_;
    }

    void IDatabase::setInfo(DataBaseInfoSPtr info)
    {
        info_ = info;
    }

    void IDatabase::executeCommand(const EventsInfo::CommandRequest& req)
    {
        DCHECK(req.inf_ == info_);
        server_->executeCommand(req);
    }
}
