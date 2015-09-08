#include "shell/lmdb_shell.h"

#include "shell/lmdb_lexer.h"

#include "gui/gui_factory.h"

namespace fastonosql
{
    LmdbShell::LmdbShell(bool showAutoCompl, QWidget* parent)
        : BaseShell(showAutoCompl, parent)
    {
        LmdbLexer* red = new LmdbLexer(this);

        registerImage(LmdbLexer::Command, GuiFactory::instance().commandIcon(LMDB).pixmap(QSize(64,64)));
        registerImage(LmdbLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);
    }
}

