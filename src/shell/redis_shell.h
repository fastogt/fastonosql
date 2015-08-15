#pragma once

#include "shell/base_shell.h"

namespace fastonosql
{
    class RedisShell
            : public BaseShell
    {
        Q_OBJECT
    public:
        RedisShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
