#pragma once

#include "core/icluster.h"

namespace fastonosql
{
    class RedisCluster
            : public ICluster
    {
        friend class ServersManager;
        Q_OBJECT
    public:

    private:
        RedisCluster(const std::string& name);
    };
}
