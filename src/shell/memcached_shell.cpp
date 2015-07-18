#include "shell/memcached_shell.h"

#include "shell/memcached_lexer.h"

#include "gui/gui_factory.h"

namespace fastoredis
{
    MemcachedShell::MemcachedShell(bool showAutoCompl, QWidget* parent)
        : FastoEditorShell(common::convertFromString<QString>(MemcachedLexer::version()), showAutoCompl, parent)
    {
        MemcachedLexer* red = new MemcachedLexer(this);

        registerImage(MemcachedLexer::Command, GuiFactory::instance().commandIcon(MEMCACHED).pixmap(QSize(64,64)));
        registerImage(MemcachedLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);
        setAllCommands(ALL_COMMANDS);

        VERIFY(connect(this, &MemcachedShell::customContextMenuRequested, this, &MemcachedShell::showContextMenu));
    }
}

