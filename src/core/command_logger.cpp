#include "core/command_logger.h"

namespace fastoredis
{
    CommandLogger::CommandLogger()
    {
        qRegisterMetaType<Command>("Command");
    }

    void CommandLogger::print(const Command &command)
    {
        emit printed(command);
    }
}
