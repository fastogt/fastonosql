#pragma once

#include <QIcon>
#include <QFont>

#include "core/connection_types.h"

#include "common/value.h"
#include "common/patterns/singleton_pattern.h"

namespace fastonosql
{
    class GuiFactory
            : public common::patterns::LazySingleton<GuiFactory>
    {
    public:
        friend class common::patterns::LazySingleton<GuiFactory>;

        const QIcon& openIcon() const;
        const QIcon& logoIcon() const;
        const QIcon& mainWindowIcon() const;
        const QIcon& connectIcon() const;
        const QIcon& disConnectIcon() const;
        const QIcon& serverIcon() const;
        const QIcon& addIcon() const;
        const QIcon& removeIcon() const;
        const QIcon& editIcon() const;
        const QIcon& messageBoxInformationIcon() const;
        const QIcon& messageBoxQuestionIcon() const;
        const QIcon& executeIcon() const;        
        const QIcon& timeIcon() const;
        const QIcon& stopIcon() const;
        const QIcon& databaseIcon() const;
        const QIcon& keyIcon() const;

        const QIcon& icon(connectionTypes type) const;
        const QIcon& modeIcon(ConnectionMode mode) const;
        const QIcon& icon(common::Value::Type type) const;

        const QIcon& loadIcon() const;
        const QIcon& clusterIcon() const;
        const QIcon& saveIcon() const;
        const QIcon& saveAsIcon() const;
        const QIcon& textIcon() const;
        const QIcon& tableIcon() const;
        const QIcon& treeIcon() const;        
        const QIcon& loggingIcon() const;
        const QIcon& discoveryIcon() const;
        const QIcon& commandIcon() const;
        const QIcon& encodeDecodeIcon() const;
        const QIcon& preferencesIcon() const;

        const QIcon& leftIcon() const;
        const QIcon& rightIcon() const;

        const QIcon& close16Icon() const;

        const QIcon& commandIcon(connectionTypes type) const;
        const QIcon& typeIcon(connectionTypes type) const;

        const QIcon& successIcon() const;
        const QIcon& failIcon() const;

        QFont font() const;
        const QString& pathToLoadingGif() const;

    private:
        const QIcon& redisConnectionIcon() const;
        const QIcon& memcachedConnectionIcon() const;
        const QIcon& ssdbConnectionIcon() const;
    };
}
