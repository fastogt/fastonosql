#pragma once

#include "gui/fasto_editor.h"

namespace fastonosql
{
    class BaseQsciLexer;

    class BaseShell
            : public FastoEditorShell
    {
        Q_OBJECT
    public:
        BaseShell(bool showAutoCompl, QWidget* parent = 0);

        std::vector<uint32_t> supportedVersions() const;
        QString version() const;
        void setFilteredVersion(uint32_t version);
    };
}
