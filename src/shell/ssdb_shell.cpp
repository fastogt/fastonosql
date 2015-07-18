#include "shell/ssdb_shell.h"

#include "shell/ssdb_lexer.h"

#include "gui/gui_factory.h"

namespace fastoredis
{
    SsdbShell::SsdbShell(bool showAutoCompl, QWidget* parent)
        : FastoEditorShell(common::convertFromString<QString>(SsdbLexer::version()), showAutoCompl, parent)
    {
        SsdbLexer* red = new SsdbLexer(this);

        registerImage(SsdbLexer::Command, GuiFactory::instance().commandIcon(SSDB).pixmap(QSize(64,64)));
        registerImage(SsdbLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);
        setAllCommands(ALL_COMMANDS);

        VERIFY(connect(this, &SsdbShell::customContextMenuRequested, this, &SsdbShell::showContextMenu));
    }
}

