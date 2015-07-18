#pragma once

#include "common/smart_ptr.h"
#include "common/net/net.h"

#include "core/connection_types.h"
#include "core/ssh_info.h"

namespace fastoredis
{
    class IConnectionSettings
    {
    public:
        virtual ~IConnectionSettings();

        std::string connectionName() const;
        void setConnectionName(const std::string& name);

        connectionTypes connectionType() const;

        bool loggingEnabled() const;
        void setLoggingEnabled(bool isLogging);

        uint32_t loggingMsTimeInterval() const;
        void setLoggingMsTimeInterval(uint32_t mstime);

        virtual std::string toString() const;
        virtual IConnectionSettings* clone() const = 0;

    protected:
        IConnectionSettings(const std::string& connectionName, connectionTypes type);

        std::string connectionName_;
        bool logging_enabled_;
        const connectionTypes type_;
        uint32_t msinterval_;
    };

    class IConnectionSettingsBase
            : public IConnectionSettings
    {
    public:
        virtual ~IConnectionSettingsBase();
        std::string hash() const;

        std::string loggingPath() const;

        virtual void setHost(const common::net::hostAndPort& host) = 0;
        virtual common::net::hostAndPort host() const = 0;

        void setConnectionNameAndUpdateHash(const std::string& name);

        virtual std::string commandLine() const = 0;
        virtual void setCommandLine(const std::string& line) = 0;

        std::string fullAddress() const;

        static IConnectionSettingsBase* createFromType(connectionTypes type, const std::string& conName = std::string());
        static IConnectionSettingsBase* fromString(const std::string& val);

        virtual std::string toString() const;

        SSHInfo sshInfo() const;
        void setSshInfo(const SSHInfo& info);

    protected:
        virtual std::string toCommandLine() const = 0;
        virtual void initFromCommandLine(const std::string& val) = 0;
        IConnectionSettingsBase(const std::string& connectionName, connectionTypes type);

    private:
        using IConnectionSettings::setConnectionName;

        std::string hash_;        
        SSHInfo sshInfo_;
    };

    const char *useHelpText(connectionTypes type);
    std::string defaultCommandLine(connectionTypes type);

    typedef common::shared_ptr<IConnectionSettingsBase> IConnectionSettingsBaseSPtr;

    class IClusterSettingsBase
            : public IConnectionSettings
    {
    public:
        typedef std::vector<IConnectionSettingsBaseSPtr> cluster_connection_type;
        cluster_connection_type nodes() const;
        IConnectionSettingsBaseSPtr root() const;

        void addNode(IConnectionSettingsBaseSPtr node);

        static IClusterSettingsBase* createFromType(connectionTypes type, const std::string& conName = std::string());
        static IClusterSettingsBase* fromString(const std::string& val);

        virtual std::string toString() const;

        IConnectionSettingsBaseSPtr findSettingsByHost(const common::net::hostAndPort& host) const;

    protected:
        IClusterSettingsBase(const std::string& connectionName, connectionTypes type);

    private:
        cluster_connection_type clusters_nodes_; //first element is root!!!
    };

    typedef common::shared_ptr<IClusterSettingsBase> IClusterSettingsBaseSPtr;
}
