#pragma once

#include "shell/base_lexer.h"

namespace fastonosql
{
    class RedisApi
            : public BaseQsciApi
    {
        Q_OBJECT
    public:
        RedisApi(QsciLexer* lexer);

        virtual void updateAutoCompletionList(const QStringList& context, QStringList& list);
        virtual QStringList callTips(const QStringList& context, int commas, QsciScintilla::CallTipsStyle style, QList<int>& shifts);
    };

    class RedisLexer
            : public BaseQsciLexer
    {
        Q_OBJECT
    public:
        enum
        {
            Default = 0,
            Command = 1,
            Sentinel = 2,
            HelpKeyword
        };

        RedisLexer(QObject* parent = 0);
        virtual const char* language() const;

        const char* version() const;
        std::vector<uint32_t> supportedVersions() const;

        virtual QString description(int style) const;
        virtual void styleText(int start, int end);
        virtual QColor defaultColor(int style) const;

    private:
        void paintCommands(const QString& source, int start);
        void paintSentinelCommands(const QString& source, int start);
    };
}
