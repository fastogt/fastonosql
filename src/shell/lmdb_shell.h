#pragma once

#include "shell/base_shell.h"

namespace fastonosql
{
    class LmdbShell
            : public BaseShell
    {
        Q_OBJECT
    public:
        LmdbShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
