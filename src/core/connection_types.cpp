#include "core/connection_types.h"

namespace
{
    const std::string connnectionMode[] = { "Latency mode", "Slave mode", "Get RDB mode", "Pipe mode",  "Find big keys mode", "Stat mode", "Scan mode", "Interactive mode" };
    const std::string serverTypes[] = { "Master", "Slave" };
}

namespace common
{
    template<>
    fastonosql::connectionTypes convertFromString(const std::string& text)
    {
        for (uint32_t i = 0; i < SIZEOFMASS(fastonosql::connnectionType); ++i){
            if (text == fastonosql::connnectionType[i]){
                return static_cast<fastonosql::connectionTypes>(i);
            }
        }

        return fastonosql::DBUNKNOWN;
    }

    std::string convertToString(fastonosql::connectionTypes t)
    {
        return fastonosql::connnectionType[t];
    }

    template<>
    fastonosql::serverTypes convertFromString(const std::string& text)
    {
        for (uint32_t i = 0; i < SIZEOFMASS(serverTypes); ++i){
            if (text == serverTypes[i]){
                return static_cast<fastonosql::serverTypes>(i);
            }
        }

        return fastonosql::MASTER;
    }

    std::string convertToString(fastonosql::serverTypes st)
    {
        return serverTypes[st];
    }

    std::string convertToString(fastonosql::ConnectionMode t)
    {
        return connnectionMode[t];
    }
}
