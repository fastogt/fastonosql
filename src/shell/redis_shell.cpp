#include "shell/redis_shell.h"

#include "shell/redis_lexer.h"

#include "gui/gui_factory.h"

namespace fastonosql
{
    RedisShell::RedisShell(bool showAutoCompl, QWidget* parent)
        : BaseShell(showAutoCompl, parent)
    {
        RedisLexer* red = new RedisLexer(this);

        registerImage(RedisLexer::Command, GuiFactory::instance().commandIcon(REDIS).pixmap(QSize(64,64)));
        registerImage(RedisLexer::Sentinel, GuiFactory::instance().commandIcon(REDIS).pixmap(QSize(64,64)));
        registerImage(RedisLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);

        VERIFY(connect(this, &RedisShell::customContextMenuRequested, this, &RedisShell::showContextMenu));
    }
}

