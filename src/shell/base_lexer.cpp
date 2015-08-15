#include "shell/base_lexer.h"

#include "common/convert2string.h"

namespace fastonosql
{
    BaseQsciApi::BaseQsciApi(QsciLexer* lexer)
        : QsciAbstractAPIs(lexer), filtered_version_(UNDEFINED_SINCE)
    {

    }

    bool BaseQsciApi::canSkipCommand(const CommandInfo& info) const
    {
        if(filtered_version_ == UNDEFINED_SINCE){
            return false;
        }

        if(info.since_ == UNDEFINED_SINCE){
            return false;
        }

        return info.since_ > filtered_version_;
    }

    void BaseQsciApi::setFilteredVersion(uint32_t version)
    {
        filtered_version_ = version;
    }

    BaseQsciLexer::BaseQsciLexer(QObject* parent)
        : QsciLexerCustom(parent)
    {
    }

    QString makeCallTip(const CommandInfo& info)
    {
        QString since_str = common::convertFromString<QString>(convertVersionNumberToReadableString(info.since_));
        return QString("Arguments: %1\nSummary: %2\nSince: %3\nExample: %4")
                .arg(common::convertFromString<QString>(info.params_))
                .arg(common::convertFromString<QString>(info.summary_))
                .arg(since_str)
                .arg(common::convertFromString<QString>(info.example_));
    }
}
