#pragma once

#include "shell/base_shell.h"

namespace fastonosql
{
    class MemcachedShell
            : public BaseShell
    {
        Q_OBJECT
    public:
        MemcachedShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
