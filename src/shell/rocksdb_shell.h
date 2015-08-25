#pragma once

#include "shell/base_shell.h"

namespace fastonosql
{
    class RocksdbShell
            : public BaseShell
    {
        Q_OBJECT
    public:
        RocksdbShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
