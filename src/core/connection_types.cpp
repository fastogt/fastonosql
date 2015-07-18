#include "core/connection_types.h"

namespace
{
    const std::string connnectionMode[] = { "Latency mode", "Slave mode", "Get RDB mode", "Pipe mode",  "Find big keys mode", "Stat mode", "Scan mode", "Intaractive mode" };
    const std::string serverTypes[] = { "Master", "Slave" };
}

namespace common
{
    template<>
    fastoredis::connectionTypes convertFromString(const std::string& text)
    {
        for (uint32_t i = 0; i < SIZEOFMASS(fastoredis::connnectionType); ++i){
            if (text == fastoredis::connnectionType[i]){
                return static_cast<fastoredis::connectionTypes>(i);
            }
        }

        return fastoredis::DBUNKNOWN;
    }

    std::string convertToString(fastoredis::connectionTypes t)
    {
        return fastoredis::connnectionType[t];
    }

    template<>
    fastoredis::serverTypes convertFromString(const std::string& text)
    {
        for (uint32_t i = 0; i < SIZEOFMASS(serverTypes); ++i){
            if (text == serverTypes[i]){
                return static_cast<fastoredis::serverTypes>(i);
            }
        }

        return fastoredis::MASTER;
    }

    std::string convertToString(fastoredis::serverTypes st)
    {
        return serverTypes[st];
    }

    std::string convertToString(fastoredis::ConnectionMode t)
    {
        return connnectionMode[t];
    }
}
