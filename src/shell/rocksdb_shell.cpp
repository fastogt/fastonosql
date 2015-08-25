#include "shell/rocksdb_shell.h"

#include "shell/rocksdb_lexer.h"

#include "gui/gui_factory.h"

namespace fastonosql
{
    RocksdbShell::RocksdbShell(bool showAutoCompl, QWidget* parent)
        : BaseShell(showAutoCompl, parent)
    {
        RocksdbLexer* red = new RocksdbLexer(this);

        registerImage(RocksdbLexer::Command, GuiFactory::instance().commandIcon(ROCKSDB).pixmap(QSize(64,64)));
        registerImage(RocksdbLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

        setLexer(red);
    }
}

