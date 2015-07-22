#pragma once

#include "gui/fasto_editor.h"

namespace fastonosql
{
    class MemcachedShell
            : public FastoEditorShell
    {
        Q_OBJECT
    public:
        MemcachedShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
