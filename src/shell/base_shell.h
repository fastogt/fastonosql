#pragma once

#include "gui/fasto_editor.h"
#include "core/connection_types.h"

namespace fastonosql
{
    class BaseShell
            : public FastoEditorShell
    {
        Q_OBJECT
    public:
        std::vector<uint32_t> supportedVersions() const;
        uint32_t commandsCount() const;
        QString version() const;
        QString basedOn() const;
        void setFilteredVersion(uint32_t version);

        static BaseShell* createFromType(connectionTypes type, bool showAutoCompl);

    protected:
        BaseShell(connectionTypes type, bool showAutoCompl, QWidget* parent = 0);
    };
}
