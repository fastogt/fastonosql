#include "global/types.h"

namespace fastoredis
{
    Command::Command()
        : message_(), type_(common::Value::C_INNER)
    {

    }

    Command::Command(const std::string &mess, common::Value::CommandType commandT)
        : message_(mess), type_(commandT)
    {

    }

    const std::string &Command::message() const
    {
        return message_;
    }

    common::Value::CommandType Command::type() const
    {
        return type_;
    }
}

namespace common
{
    std::string convertToString(fastoredis::supportedViews v)
    {
        return fastoredis::viewsText[v];
    }

    template<>
    fastoredis::supportedViews convertFromString(const std::string& from)
    {
        for(int i = 0; i < SIZEOFMASS(fastoredis::viewsText); ++i){
            if(from == fastoredis::viewsText[i]){
                return static_cast<fastoredis::supportedViews>(i);
            }
        }

        return fastoredis::Tree;
    }
}
