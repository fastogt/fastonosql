#pragma once

#include "core/core_fwd.h"

#include "core/iserver.h"

namespace fastonosql
{
    class ICluster
            : public IServerBase
    {
    public:
        typedef std::vector<IServerSPtr> nodes_type;

        QString name() const;
        nodes_type nodes() const;
        void addServer(IServerSPtr serv);

        IServerSPtr root() const;

    protected:
        explicit ICluster(const std::string& name);

    private:
        const std::string name_;
        nodes_type nodes_;
    };
}
