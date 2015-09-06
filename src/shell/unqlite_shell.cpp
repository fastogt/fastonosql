#include "shell/unqlite_shell.h"

#include "shell/unqlite_lexer.h"

#include "gui/gui_factory.h"

namespace fastonosql
{
    UnqliteShell::UnqliteShell(bool showAutoCompl, QWidget* parent)
        : BaseShell(showAutoCompl, parent)
    {
        UnqliteLexer* red = new UnqliteLexer(this);

        registerImage(UnqliteLexer::Command, GuiFactory::instance().commandIcon(UNQLITE).pixmap(QSize(64,64)));
        registerImage(UnqliteLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);
    }
}

