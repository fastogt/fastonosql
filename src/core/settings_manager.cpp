#include "core/settings_manager.h"

#include <QSettings>

#include "fasto/qt/translations/translations.h"
#include "fasto/qt/gui/app_style.h"

#include "common/file_system.h"
#include "common/qt/convert_string.h"
#include "common/utils.h"

#define PREFIX "settings/"

#define LANGUAGE PREFIX"language"
#define STYLE PREFIX"style"
#define CONNECTIONS PREFIX"connections"
#define CLUSTERS PREFIX"clusters"
#define VIEW PREFIX"view"
#define SYNCTABS PREFIX"synctabs"
#define LOGGINGDIR PREFIX"loggingdir"
#define CHECKUPDATES PREFIX"checkupdates"
#define AUTOCOMPLETION PREFIX"autocompletion"
#define RCONNECTIONS PREFIX"rconnections"
#define AUTOOPENCONSOLE PREFIX"autoopenconsole"

namespace
{
    const std::string iniPath("~/.config/" PROJECT_NAME "/config.ini");
}


namespace fastoredis
{
    SettingsManager::SettingsManager()
        : views_(), curStyle_(), curLanguage_(), connections_(), syncTabs_(), loggingDir_(), autoCheckUpdate_(), autoCompletion_(), autoOpenConsole_()
    {
       load();
    }


    SettingsManager::~SettingsManager()
    {
        save();
    }

    supportedViews SettingsManager::defaultView() const
    {
        return views_;
    }

    void SettingsManager::setDefaultView(supportedViews view)
    {
        views_ = view;
    }

    QString SettingsManager::currentStyle() const
    {
        return curStyle_;
    }

    void SettingsManager::setCurrentStyle(const QString &st)
    {
        curStyle_ = st;
    }

    QString SettingsManager::currentLanguage() const
    {
        return curLanguage_;
    }

    void SettingsManager::setCurrentLanguage(const QString &lang)
    {
        curLanguage_ = lang;
    }

    void SettingsManager::addConnection(IConnectionSettingsBaseSPtr connection)
    {
        if(connection){
            ConnectionSettingsContainerType::iterator it = std::find(connections_.begin(),connections_.end(),connection);
            if (it == connections_.end()) {
                connections_.push_back(connection);
            }
        }
    }

    void SettingsManager::removeConnection(IConnectionSettingsBaseSPtr connection)
    {
        if(connection){
            ConnectionSettingsContainerType::iterator it = std::find(connections_.begin(),connections_.end(),connection);
            if (it != connections_.end()) {
                connections_.erase(it);
            }
        }
    }

    SettingsManager::ConnectionSettingsContainerType SettingsManager::connections() const
    {
        return connections_;
    }

    void SettingsManager::addCluster(IClusterSettingsBaseSPtr cluster)
    {
        if(cluster){
            ClusterSettingsContainerType::iterator it = std::find(clusters_.begin(),clusters_.end(),cluster);
            if (it == clusters_.end()) {
                clusters_.push_back(cluster);
            }
        }
    }

    void SettingsManager::removeCluster(IClusterSettingsBaseSPtr cluster)
    {
        if(cluster){
            ClusterSettingsContainerType::iterator it = std::find(clusters_.begin(), clusters_.end(), cluster);
            if (it != clusters_.end()) {
                clusters_.erase(it);
            }
        }
    }

    SettingsManager::ClusterSettingsContainerType SettingsManager::clusters() const
    {
        return clusters_;
    }

    void SettingsManager::addRConnection(const QString& connection)
    {
        if(!connection.isEmpty()){
            QStringList::iterator it = std::find(recentConnections_.begin(), recentConnections_.end(), connection);
            if (it == recentConnections_.end()) {
                recentConnections_.push_front(connection);
            }
        }
    }

    void SettingsManager::removeRConnection(const QString& connection)
    {
        if(!connection.isEmpty()){
            QStringList::iterator it = std::find(recentConnections_.begin(), recentConnections_.end(), connection);
            if (it != recentConnections_.end()) {
                recentConnections_.erase(it);
            }
        }
    }

    QStringList SettingsManager::recentConnections() const
    {
        return recentConnections_;
    }

    void SettingsManager::clearRConnections()
    {
        recentConnections_.clear();
    }

    bool SettingsManager::syncTabs() const
    {
        return syncTabs_;
    }

    void SettingsManager::setSyncTabs(bool sync)
    {
        syncTabs_ = sync;
    }

    QString SettingsManager::loggingDirectory() const
    {
        return loggingDir_;
    }

    void SettingsManager::setLoggingDirectory(const QString &dir)
    {
        loggingDir_ = dir;
    }

    bool SettingsManager::autoCheckUpdates() const
    {
        return autoCheckUpdate_;
    }

    void SettingsManager::setAutoCheckUpdates(bool isCheck)
    {
        autoCheckUpdate_ = isCheck;
    }

    bool SettingsManager::autoCompletion() const
    {
        return autoCompletion_;
    }

    void SettingsManager::setAutoCompletion(bool enableAuto)
    {
        autoCompletion_ = enableAuto;
    }

    bool SettingsManager::autoOpenConsole() const
    {
        return autoOpenConsole_;
    }

    void SettingsManager::setAutoOpenConsole(bool enableAuto)
    {
        autoOpenConsole_ = enableAuto;
    }

    void SettingsManager::load()
    {
        QString inip = common::convertFromString<QString>(common::file_system::prepare_path(iniPath));
        QSettings settings(inip, QSettings::IniFormat);
        DCHECK(settings.status() == QSettings::NoError);

        curStyle_ = settings.value(STYLE, fasto::qt::gui::defStyle).toString();
        curLanguage_ = settings.value(LANGUAGE, fasto::qt::translations::defLanguage).toString();

        int view = settings.value(VIEW, fastoredis::Tree).toInt();
        views_ = static_cast<supportedViews>(view);

        QList<QVariant> clusters = settings.value(CLUSTERS, "").toList();
        for(QList<QVariant>::const_iterator it = clusters.begin(); it != clusters.end(); ++it){
            QVariant var = *it;
            QString string = var.toString();
            std::string encoded = common::convertToString(string);
            std::string raw = common::utils::base64::decode64(encoded);

            IClusterSettingsBaseSPtr sett(IClusterSettingsBase::fromString(raw));
            if(sett){
               clusters_.push_back(sett);
            }
        }

        QList<QVariant> connections = settings.value(CONNECTIONS, "").toList();
        for(QList<QVariant>::const_iterator it = connections.begin(); it != connections.end(); ++it){
            QVariant var = *it;
            QString string = var.toString();
            std::string encoded = common::convertToString(string);
            std::string raw = common::utils::base64::decode64(encoded);

            IConnectionSettingsBaseSPtr sett(IConnectionSettingsBase::fromString(raw));
            if(sett){
               connections_.push_back(sett);
            }
        }

        QStringList rconnections = settings.value(RCONNECTIONS, "").toStringList();
        for(QStringList::const_iterator it = rconnections.begin(); it != rconnections.end(); ++it){
            QString string = *it;
            std::string encoded = common::convertToString(string);
            std::string raw = common::utils::base64::decode64(encoded);

            QString qdata = common::convertFromString<QString>(raw);
            if(!qdata.isEmpty()){
               recentConnections_.push_back(qdata);
            }
        }

        syncTabs_= settings.value(SYNCTABS, false).toBool();
        std::string dir = common::file_system::get_dir_path(iniPath);
        loggingDir_ = settings.value(LOGGINGDIR, common::convertFromString<QString>(dir)).toString();
        autoCheckUpdate_ = settings.value(CHECKUPDATES, true).toBool();
        autoCompletion_ = settings.value(AUTOCOMPLETION, true).toBool();
        autoOpenConsole_ = settings.value(AUTOOPENCONSOLE, true).toBool();
    }

    void SettingsManager::save()
    {
        QSettings settings(common::convertFromString<QString>(common::file_system::prepare_path(iniPath)), QSettings::IniFormat);
        DCHECK(settings.status() == QSettings::NoError);

        settings.setValue(STYLE, curStyle_);
        settings.setValue(LANGUAGE, curLanguage_);
        settings.setValue(VIEW, views_);

        QList<QVariant> clusters;
        for(ClusterSettingsContainerType::const_iterator it = clusters_.begin(); it != clusters_.end(); ++it){
            IClusterSettingsBaseSPtr conn = *it;
            if(conn){
               std::string raw = conn->toString();
               std::string enc = common::utils::base64::encode64(raw);
               QString qdata = common::convertFromString<QString>(enc);
               clusters.push_back(qdata);
            }
        }
        settings.setValue(CLUSTERS, clusters);

        QList<QVariant> connections;
        for(ConnectionSettingsContainerType::const_iterator it = connections_.begin(); it != connections_.end(); ++it){
            IConnectionSettingsBaseSPtr conn = *it;
            if(conn){
               std::string raw = conn->toString();
               std::string enc = common::utils::base64::encode64(raw);
               QString qdata = common::convertFromString<QString>(enc);
               connections.push_back(qdata);
            }
        }
        settings.setValue(CONNECTIONS, connections);

        QStringList rconnections;
        for(QStringList::const_iterator it = recentConnections_.begin(); it != recentConnections_.end(); ++it){
            QString conn = *it;
            if(!conn.isEmpty()){
               std::string raw = common::convertToString(conn);
               std::string enc = common::utils::base64::encode64(raw);
               QString qdata = common::convertFromString<QString>(enc);
               rconnections.push_back(qdata);
            }
        }
        settings.setValue(RCONNECTIONS, rconnections);

        settings.setValue(SYNCTABS, syncTabs_);
        settings.setValue(LOGGINGDIR, loggingDir_);
        settings.setValue(CHECKUPDATES, autoCheckUpdate_);
        settings.setValue(AUTOCOMPLETION, autoCompletion_);
        settings.setValue(AUTOOPENCONSOLE, autoOpenConsole_);
    }
}
