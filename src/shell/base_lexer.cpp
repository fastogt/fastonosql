#include "shell/base_lexer.h"

#include "common/convert2string.h"

namespace fastonosql
{
    BaseQsciApi::BaseQsciApi(QsciLexer* lexer)
        : QsciAbstractAPIs(lexer)
    {

    }

    BaseQsciLexer::BaseQsciLexer(QObject* parent)
        : QsciLexerCustom(parent)
    {
    }

    QString makeCallTip(const CommandInfo& info)
    {
        QString since_str = info.since_ == UNDEFINED_SINCE ? UNDEFINED_SINCE_STR :
                                                             common::convertFromString<QString>(common::convertVersionNumberToString(info.since_));
        return QString("Arguments: %1\nSummary: %2\nSince: %3\nExample: %4")
                .arg(common::convertFromString<QString>(info.params_))
                .arg(common::convertFromString<QString>(info.summary_))
                .arg(since_str)
                .arg(common::convertFromString<QString>(info.example_));
    }
}
