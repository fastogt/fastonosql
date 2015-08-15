#pragma once

#include "shell/base_shell.h"

namespace fastonosql
{
    class SsdbShell
            : public BaseShell
    {
        Q_OBJECT
    public:
        SsdbShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
