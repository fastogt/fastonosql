#include "shell/redis_lexer.h"

#include "core/redis/redis_driver.h"

namespace
{
    const QString help("help");
}

namespace fastonosql
{
    RedisApi::RedisApi(QsciLexer* lexer)
        : BaseQsciApi(lexer)
    {
    }

    void RedisApi::updateAutoCompletionList(const QStringList& context, QStringList& list)
    {
        for(QStringList::const_iterator it = context.begin(); it != context.end(); ++it){
            QString val = *it;
            for(int i = 0; i < SIZEOFMASS(redisCommands); ++i){
                CommandInfo cmd = redisCommands[i];
                QString jval = common::convertFromString<QString>(cmd.name_);
                if(jval.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                    list.append(jval + "?1");
                }
            }

            for(int i = 0; i < SIZEOFMASS(redisSentinelCommands); ++i){
                CommandInfo cmd = redisSentinelCommands[i];
                QString jval = common::convertFromString<QString>(cmd.name_);
                if(jval.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                    list.append(jval + "?2");
                }
            }

            if(help.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                list.append(help + "?3");
            }
        }
    }

    QStringList RedisApi::callTips(const QStringList& context, int commas, QsciScintilla::CallTipsStyle style, QList<int>& shifts)
    {
        for(QStringList::const_iterator it = context.begin(); it != context.end() - 1; ++it){
            QString val = *it;
            for(int i = 0; i < SIZEOFMASS(redisCommands); ++i){
                CommandInfo cmd = redisCommands[i];
                QString jval = common::convertFromString<QString>(cmd.name_);
                if(QString::compare(jval, val, Qt::CaseInsensitive) == 0){
                    return QStringList() << makeCallTip(cmd);
                }
            }

            for(int i = 0; i < SIZEOFMASS(redisSentinelCommands); ++i){
                CommandInfo cmd = redisSentinelCommands[i];
                QString jval = common::convertFromString<QString>(cmd.name_);
                if(QString::compare(jval, val, Qt::CaseInsensitive) == 0){
                    return QStringList() << makeCallTip(cmd);
                }
            }
        }

        return QStringList();
    }

    RedisLexer::RedisLexer(QObject* parent)
        : BaseQsciLexer(parent)
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

    const char *RedisLexer::wordCharacters() const
    {
        return "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    }

    QString RedisLexer::description(int style) const
    {
        switch (style)
        {
        case Default:
             return "Default";
        case Command:
            return "Command";
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
            case Sentinel:
                return Qt::darkGreen;
            case HelpKeyword:
                return Qt::red;
        }

        return Qt::black;
    }

    void RedisLexer::paintCommands(const QString& source, int start)
    {
        for(int i = 0; i < SIZEOFMASS(redisCommands); ++i){
            CommandInfo cmd = redisCommands[i];
            QString word = common::convertFromString<QString>(cmd.name_);
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

    void RedisLexer::paintSentinelCommands(const QString& source, int start)
    {
        for(int i = 0; i < SIZEOFMASS(redisSentinelCommands); ++i){
            CommandInfo cmd = redisSentinelCommands[i];
            QString word = common::convertFromString<QString>(cmd.name_);
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
