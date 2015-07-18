#pragma once

#include "gui/fasto_editor.h"

namespace fastoredis
{
    class RedisShell
            : public FastoEditorShell
    {
        Q_OBJECT
    public:
        RedisShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
