#include "shell/base_lexer.h"

#include "common/convert2string.h"

namespace fastonosql
{
    BaseQsciApi::BaseQsciApi(QsciLexer* lexer)
        : QsciAbstractAPIs(lexer)
    {

    }

    QString makeCallTip(const CommandInfo& info)
    {
        return QString("Arguments: %1\nSummary: %2\nSince: %3\nExample: %4")
                .arg(common::convertFromString<QString>(info.params_))
                .arg(common::convertFromString<QString>(info.summary_))
                .arg(common::convertFromString<QString>(info.since_))
                .arg(common::convertFromString<QString>(info.example_));
    }

    BaseQsciLexer::BaseQsciLexer(QObject* parent)
        : QsciLexerCustom(parent)
    {
    }
}
