#pragma once

#include "gui/fasto_editor.h"

namespace fastonosql
{
    class SsdbShell
            : public FastoEditorShell
    {
        Q_OBJECT
    public:
        SsdbShell(bool showAutoCompl, QWidget* parent = 0);
    };
}
