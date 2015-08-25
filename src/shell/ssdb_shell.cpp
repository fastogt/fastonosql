#include "shell/ssdb_shell.h"

#include "shell/ssdb_lexer.h"

#include "gui/gui_factory.h"

namespace fastonosql
{
    SsdbShell::SsdbShell(bool showAutoCompl, QWidget* parent)
        : BaseShell(showAutoCompl, parent)
    {
        SsdbLexer* red = new SsdbLexer(this);

        registerImage(SsdbLexer::Command, GuiFactory::instance().commandIcon(SSDB).pixmap(QSize(64,64)));
        registerImage(SsdbLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);
    }
}
