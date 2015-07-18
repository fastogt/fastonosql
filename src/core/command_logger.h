#pragma once

#include <QObject>

#include "common/patterns/singleton_pattern.h"

#include "global/types.h"

namespace fastoredis
{
    class CommandLogger
        : public QObject, public common::patterns::LazySingleton<CommandLogger>
    {
        friend class common::patterns::LazySingleton<CommandLogger>;
        Q_OBJECT

    public:
        void print(const Command& command);

    Q_SIGNALS:
        void printed(const Command& mess);

    private:
        CommandLogger();
    };

    inline void LOG_COMMAND(const Command& command)
    {
        return CommandLogger::instance().print(command);
    }
}
