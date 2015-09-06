#pragma once

#include "shell/base_shell.h"

namespace fastonosql
{
    class UnqliteShell
            : public BaseShell
    {
        Q_OBJECT
    public:
        UnqliteShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
