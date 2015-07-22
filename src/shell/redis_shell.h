#pragma once

#include "gui/fasto_editor.h"

namespace fastonosql
{
    class RedisShell
            : public FastoEditorShell
    {
        Q_OBJECT
    public:
        RedisShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
