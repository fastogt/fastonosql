#include "shell/redis_shell.h"

#include "shell/redis_lexer.h"

#include "gui/gui_factory.h"

namespace fastoredis
{
    RedisShell::RedisShell(bool showAutoCompl, QWidget* parent)
        : FastoEditorShell(common::convertFromString<QString>(RedisLexer::version()), showAutoCompl, parent)
    {
        RedisLexer* red = new RedisLexer(this);

        registerImage(RedisLexer::Command, GuiFactory::instance().commandIcon(REDIS).pixmap(QSize(64,64)));
        registerImage(RedisLexer::Types, GuiFactory::instance().typeIcon(REDIS).pixmap(QSize(64,64)));
        registerImage(RedisLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);
        setAllCommands(ALL_COMMANDS);

        VERIFY(connect(this, &RedisShell::customContextMenuRequested, this, &RedisShell::showContextMenu));
    }
}

