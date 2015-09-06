#pragma once

#include "shell/base_lexer.h"

namespace fastonosql
{
    class UnqliteApi
            : public BaseQsciApi
    {
        Q_OBJECT
    public:
        explicit UnqliteApi(QsciLexer* lexer);

        virtual void updateAutoCompletionList(const QStringList& context, QStringList& list);
        virtual QStringList callTips(const QStringList& context, int commas, QsciScintilla::CallTipsStyle style, QList<int>& shifts);
    };

    class UnqliteLexer
            : public BaseQsciLexer
    {
        Q_OBJECT
    public:
        enum
        {
            Default = 0,
            Command = 1,
            HelpKeyword
        };

        explicit UnqliteLexer(QObject* parent = 0);
        virtual const char* language() const;

        virtual const char* version() const;
        virtual std::vector<uint32_t> supportedVersions() const;
        virtual uint32_t commandsCount() const;

        virtual QString description(int style) const;
        virtual void styleText(int start, int end);
        virtual QColor defaultColor(int style) const;

    private:
        void paintCommands(const QString& source, int start);
    };
}
