#pragma once

#include "shell/base_shell.h"

namespace fastonosql
{
    class LeveldbShell
            : public BaseShell
    {
        Q_OBJECT
    public:
        LeveldbShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
