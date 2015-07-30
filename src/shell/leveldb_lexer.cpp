#include "shell/leveldb_lexer.h"

#include "core/leveldb/leveldb_driver.h"

namespace
{
    const QString help("help");
}

namespace fastonosql
{
    LeveldbApi::LeveldbApi(QsciLexer *lexer)
        : QsciAbstractAPIs(lexer)
    {
    }

    void LeveldbApi::updateAutoCompletionList(const QStringList& context, QStringList& list)
    {
        for(QStringList::const_iterator it = context.begin(); it != context.end(); ++it){
            QString val = *it;
            for(int i = 0; i < SIZEOFMASS(leveldbCommandsKeywords); ++i){
                QString jval = leveldbCommandsKeywords[i];
                if(jval.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                    list.append(jval + "?1");
                }
            }

            if(help.startsWith(val, Qt::CaseInsensitive) || (val == ALL_COMMANDS && context.size() == 1) ){
                list.append(help + "?2");
            }
        }
    }

    QStringList LeveldbApi::callTips(const QStringList& context, int commas, QsciScintilla::CallTipsStyle style, QList<int>& shifts)
    {
        return QStringList();
    }

    LeveldbLexer::LeveldbLexer(QObject* parent)
        : QsciLexerCustom(parent)
    {
        setAPIs(new LeveldbApi(this));
    }

    const char *LeveldbLexer::language() const
    {
        return "Ssdb";
    }

    const char* LeveldbLexer::version()
    {
        return LeveldbDriver::versionApi();
    }

    QString LeveldbLexer::description(int style) const
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

    void LeveldbLexer::styleText(int start, int end)
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

    QColor LeveldbLexer::defaultColor(int style) const
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

    void LeveldbLexer::paintCommands(const QString& source, int start)
    {
        for(int i = 0; i < SIZEOFMASS(leveldbCommandsKeywords); ++i){
            QString word = leveldbCommandsKeywords[i];
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
