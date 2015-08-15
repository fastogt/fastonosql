#include "shell/base_shell.h"

#include "shell/base_lexer.h"

namespace fastonosql
{
    BaseShell::BaseShell(bool showAutoCompl, QWidget* parent)
        : FastoEditorShell(showAutoCompl, parent)
    {
    }

    QString BaseShell::version() const
    {
        BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
        DCHECK(red);
        if(red){
            return common::convertFromString<QString>(red->version());
        }

        return QString();
    }

    std::vector<uint32_t> BaseShell::supportedVersions() const
    {
        BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
        DCHECK(red);
        if(red){
            return red->supportedVersions();
        }

        return std::vector<uint32_t>();
    }

    void BaseShell::setFilteredVersion(uint32_t version)
    {
        BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
        DCHECK(red);
        if(!red){
            return;
        }

        BaseQsciApi * api = dynamic_cast<BaseQsciApi*>(red->apis());
        if(!api){
            return;
        }

        api->setFilteredVersion(version);
    }
}

