#pragma once

#include <Qsci/qsciabstractapis.h>
#include <Qsci/qscilexercustom.h>

#include "core/types.h"

#define ALL_COMMANDS "ALL_COMMANDS"

namespace fastonosql
{
    class BaseQsciApi
            : public QsciAbstractAPIs
    {
        Q_OBJECT
    public:
        BaseQsciApi(QsciLexer* lexer);
    };

    class BaseQsciLexer
            : public QsciLexerCustom
    {
        Q_OBJECT
    protected:
        BaseQsciLexer(QObject* parent = 0);
    };

    QString makeCallTip(const CommandInfo& info);
}
