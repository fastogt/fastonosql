#include "shell/leveldb_shell.h"

#include "shell/leveldb_lexer.h"

#include "gui/gui_factory.h"

namespace fastonosql
{
    LeveldbShell::LeveldbShell(bool showAutoCompl, QWidget* parent)
        : FastoEditorShell(common::convertFromString<QString>(LeveldbLexer::version()), showAutoCompl, parent)
    {
        LeveldbLexer* red = new LeveldbLexer(this);

        registerImage(LeveldbLexer::Command, GuiFactory::instance().commandIcon(LEVELDB).pixmap(QSize(64,64)));
        registerImage(LeveldbLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);
        setAllCommands(ALL_COMMANDS);

        VERIFY(connect(this, &LeveldbShell::customContextMenuRequested, this, &LeveldbShell::showContextMenu));
    }
}

