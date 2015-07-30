#pragma once

#include "gui/fasto_editor.h"

namespace fastonosql
{
    class LeveldbShell
            : public FastoEditorShell
    {
        Q_OBJECT
    public:
        LeveldbShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
