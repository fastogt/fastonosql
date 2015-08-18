#include "global/types.h"

namespace fastonosql
{
    Command::Command()
        : message_(), type_(common::Value::C_INNER)
    {

    }

    Command::Command(const std::string &mess, common::Value::CommandLoggingType commandT)
        : message_(mess), type_(commandT)
    {

    }

    const std::string &Command::message() const
    {
        return message_;
    }

    common::Value::CommandLoggingType Command::type() const
    {
        return type_;
    }
}

namespace common
{
    std::string convertToString(fastonosql::supportedViews v)
    {
        return fastonosql::viewsText[v];
    }

    template<>
    fastonosql::supportedViews convertFromString(const std::string& from)
    {
        for(int i = 0; i < SIZEOFMASS(fastonosql::viewsText); ++i){
            if(from == fastonosql::viewsText[i]){
                return static_cast<fastonosql::supportedViews>(i);
            }
        }

        return fastonosql::Tree;
    }
}
