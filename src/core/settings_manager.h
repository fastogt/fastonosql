#pragma once

#include <QStringList>

#include "global/types.h"

#include "common/patterns/singleton_pattern.h"

#include "core/connection_settings.h"

namespace fastonosql
{
    class SettingsManager
            : public common::patterns::LazySingleton<SettingsManager>
    {
    public:
        typedef std::vector<IConnectionSettingsBaseSPtr> ConnectionSettingsContainerType;
        typedef std::vector<IClusterSettingsBaseSPtr> ClusterSettingsContainerType;
        friend class common::patterns::LazySingleton<SettingsManager>;

        static QString settingsDirPath();
        static std::string settingsFilePath();

        void setDefaultView(supportedViews view);
        supportedViews defaultView() const;

        QString currentStyle() const;
        void setCurrentStyle(const QString &style);

        QString currentLanguage() const;
        void setCurrentLanguage(const QString &lang);

        // connections
        void addConnection(IConnectionSettingsBaseSPtr connection);
        void removeConnection(IConnectionSettingsBaseSPtr connection);

        ConnectionSettingsContainerType connections() const;

        // clusters
        void addCluster(IClusterSettingsBaseSPtr cluster);
        void removeCluster(IClusterSettingsBaseSPtr cluster);

        ClusterSettingsContainerType clusters() const;

        void addRConnection(const QString& connection);
        void removeRConnection(const QString& connection);
        QStringList recentConnections() const;
        void clearRConnections();

        bool syncTabs() const;
        void setSyncTabs(bool sync);

        void setLoggingDirectory(const QString& dir);
        QString loggingDirectory() const;

        bool autoCheckUpdates() const;
        void setAutoCheckUpdates(bool isCheck);

        bool autoCompletion() const;
        void setAutoCompletion(bool enableAuto);

        bool autoOpenConsole() const;
        void setAutoOpenConsole(bool enableAuto);

        bool fastViewKeys() const;
        void setFastViewKeys(bool fastView);

        void reloadFromPath(const std::string& path, bool merge);

    private:
        void load();
        void save();

        SettingsManager();
        ~SettingsManager();

        supportedViews views_;
        QString curStyle_;
        QString curLanguage_;
        ConnectionSettingsContainerType connections_;
        ClusterSettingsContainerType clusters_;
        QStringList recentConnections_;
        bool syncTabs_;
        QString loggingDir_;
        bool autoCheckUpdate_;
        bool autoCompletion_;
        bool autoOpenConsole_;
        bool fastViewKeys_;
    };
}
