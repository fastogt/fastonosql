#include "shell/redis_lexer.h"

#include "core/redis/redis_driver.h"

namespace
{
    const QString help("help");
}

namespace fastonosql
{
    RedisApi::RedisApi(QsciLexer* lexer)
        : QsciAbstractAPIs(lexer)
    {
    }

    void RedisApi::updateAutoCompletionList(const QStringList& context, QStringList& list)
    {
        for(QStringList::const_iterator it = context.begin(); it != context.end(); ++it){
            QString val = *it;
            for(std::vector<QString>::const_iterator jt = redisCommandsKeywords.begin(); jt != redisCommandsKeywords.end(); ++jt){
                QString jval = *jt;
                if(jval.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                    list.append(jval + "?1");
                }
            }

            for(std::vector<QString>::const_iterator jt = redisTypesKeywords.begin(); jt != redisTypesKeywords.end(); ++jt){
                QString jval = *jt;
                if(jval.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                    list.append(jval + "?2");
                }
            }

            for(std::vector<QString>::const_iterator jt = redisSentinelKeywords.begin(); jt != redisSentinelKeywords.end(); ++jt){
                QString jval = *jt;
                if(jval.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                    list.append(jval + "?3");
                }
            }

            if(help.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                list.append(help + "?4");
            }
        }
        NOOP();
    }

    QStringList RedisApi::callTips(const QStringList& context, int commas, QsciScintilla::CallTipsStyle style, QList<int>& shifts)
    {
        return QStringList();
    }

    RedisLexer::RedisLexer(QObject* parent)
        : QsciLexerCustom(parent)
    {
        setAPIs(new RedisApi(this));
    }

    const char* RedisLexer::language() const
    {
        return "Redis";
    }

    const char* RedisLexer::version()
    {
        return RedisDriver::versionApi();
    }

    QString RedisLexer::description(int style) const
    {
        switch (style)
        {
        case Default:
             return "Default";
        case Command:
            return "Command";
        case Types:
            return "Types";
        case Sentinel:
            return "Sentinel commands";
        case HelpKeyword:
            return "HelpKeyword";
        }

        return QString(style);
    }

    void RedisLexer::styleText(int start, int end)
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
        paintTypes(source, start);
        paintSentinelCommands(source, start);

        int index = 0;
        int begin = 0;
        while( (begin = source.indexOf(help, index, Qt::CaseInsensitive)) != -1){
            index = begin + help.length();

            startStyling(start + begin);
            setStyling(help.length(), HelpKeyword);
            startStyling(start + begin);
        }
    }

    QColor RedisLexer::defaultColor(int style) const
    {
        switch(style) {
            case Default:
                return Qt::black;
            case Command:
                return Qt::red;
            case Types:
                return Qt::blue;
            case Sentinel:
                return Qt::darkGreen;
            case HelpKeyword:
                return Qt::red;
        }

        return Qt::black;
    }

    void RedisLexer::paintCommands(const QString& source, int start)
    {
        for(std::vector<QString>::const_iterator it = redisCommandsKeywords.begin(); it != redisCommandsKeywords.end(); ++it){
            QString word = (*it);
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

    void RedisLexer::paintTypes(const QString& source, int start)
    {
        for(std::vector<QString>::const_iterator it = redisTypesKeywords.begin(); it != redisTypesKeywords.end(); ++it){
            QString word = *it;
            int index = 0;
            int begin = 0;
            while( (begin = source.indexOf(word, index, Qt::CaseInsensitive)) != -1){
                index = begin + word.length();

                startStyling(start + begin);
                setStyling(word.length(), Types);
                startStyling(start + begin);
            }
        }
    }

    void RedisLexer::paintSentinelCommands(const QString& source, int start)
    {
        for(std::vector<QString>::const_iterator it = redisSentinelKeywords.begin(); it != redisSentinelKeywords.end(); ++it){
            QString word = *it;
            int index = 0;
            int begin = 0;
            while( (begin = source.indexOf(word, index, Qt::CaseInsensitive)) != -1){
                index = begin + word.length();

                startStyling(start + begin);
                setStyling(word.length(), Sentinel);
                startStyling(start + begin);
            }
        }
    }
}
