#include "core/idatabase.h"

#include "core/iserver.h"

namespace fastoredis
{
    IDatabase::IDatabase(IServerSPtr server, DataBaseInfoSPtr info)
        : info_(info), server_(server)
    {
        DCHECK(server);
        DCHECK(info);
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

    void IDatabase::loadContent(const std::string& pattern, uint32_t countKeys, uint32_t cursor)
    {
        server_->loadDatabaseContent(info_, pattern, countKeys, cursor);
    }

    void IDatabase::setDefault()
    {
        server_->setDefaultDb(info_);
    }

    DataBaseInfoSPtr IDatabase::info() const
    {
        return info_;
    }

    void IDatabase::setInfo(DataBaseInfoSPtr info)
    {
        info_ = info;
    }

    void IDatabase::removeKey(const NKey& key)
    {
        CommandKeySPtr com(new CommandDeleteKey(key));
        server_->executeCommand(info_, com);
    }

    void IDatabase::loadValue(const NKey& key)
    {
        CommandKeySPtr com(new CommandLoadKey(key));
        server_->executeCommand(info_, com);
    }

    void IDatabase::createKey(const NKey& key, FastoObjectIPtr value)
    {
        CommandKeySPtr com(new CommandCreateKey(key, value));
        server_->executeCommand(info_, com);
    }
}
