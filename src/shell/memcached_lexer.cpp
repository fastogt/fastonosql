#include "shell/memcached_lexer.h"

#include "core/memcached/memcached_driver.h"

namespace
{
    const QString help("help");
}

namespace fastonosql
{
    MemcachedApi::MemcachedApi(QsciLexer *lexer)
        : QsciAbstractAPIs(lexer)
    {
    }

    void MemcachedApi::updateAutoCompletionList(const QStringList& context, QStringList& list)
    {
        for(QStringList::const_iterator it = context.begin(); it != context.end(); ++it){
            QString val = *it;
            for(int i = 0; i < SIZEOFMASS(memcachedCommandsKeywords); ++i){
                QString jval = memcachedCommandsKeywords[i];
                if(jval.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                    list.append(jval + "?1");
                }
            }

            if(help.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                list.append(help + "?2");
            }
        }
    }

    QStringList MemcachedApi::callTips(const QStringList& context, int commas, QsciScintilla::CallTipsStyle style, QList<int>& shifts)
    {
        return QStringList();
    }

    MemcachedLexer::MemcachedLexer(QObject* parent)
        : QsciLexerCustom(parent)
    {
        setAPIs(new MemcachedApi(this));
    }

    const char *MemcachedLexer::language() const
    {
        return "Memcached";
    }

    const char* MemcachedLexer::version()
    {
        return MemcachedDriver::versionApi();
    }

    QString MemcachedLexer::description(int style) const
    {
        switch (style)
        {
        case Default:
             return "Default";
        case Command:
            return "Command";
        case HelpKeyword:
            return "HelpKeyword";
        }

        return QString(style);
    }

    void MemcachedLexer::styleText(int start, int end)
    {
        if(!editor()){
            return;
        }

        char *data = new char[end - start + 1];
        editor()->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start, end, data);
        QString source(data);
        delete [] data;

        if(source.isEmpty()){
            return;
        }

        paintCommands(source, start);

        int index = 0;
        int begin = 0;
        while( (begin = source.indexOf(help, index, Qt::CaseInsensitive)) != -1){
            index = begin + help.length();

            startStyling(start + begin);
            setStyling(help.length(), HelpKeyword);
            startStyling(start + begin);
        }
    }

    QColor MemcachedLexer::defaultColor(int style) const
    {
        switch(style) {
            case Default:
                return Qt::black;
            case Command:
                return Qt::red;
            case HelpKeyword:
                return Qt::red;
        }

        return Qt::black;
    }

    void MemcachedLexer::paintCommands(const QString& source, int start)
    {
        for(int i = 0; i < SIZEOFMASS(memcachedCommandsKeywords); ++i){
            QString word = memcachedCommandsKeywords[i];
            int index = 0;
            int begin = 0;
            while( (begin = source.indexOf(word, index, Qt::CaseInsensitive)) != -1){
                index = begin + word.length();

                startStyling(start + begin);
                setStyling(word.length(), Command);
                startStyling(start + begin);
            }
        }
    }
}
