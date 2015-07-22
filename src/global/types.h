#pragma once

#include "common/value.h"
#include "common/convert2string.h"

namespace fastonosql
{
    enum supportedViews
    {
        Tree = 0,
        Table,
        Text
    };

    static const std::string viewsText[] = { "Tree", "Table", "Text" };

    class Command
    {
    public:
        Command();
        Command(const std::string& mess, common::Value::CommandType commandT);
        const std::string& message() const;
        common::Value::CommandType type() const;

    private:
        const std::string message_;
        const common::Value::CommandType type_;
    };
}

namespace common
{
    std::string convertToString(fastonosql::supportedViews v);
}
