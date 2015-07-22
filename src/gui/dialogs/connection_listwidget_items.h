#pragma once

#include <QTreeWidgetItem>

#include "core/connection_settings.h"

namespace fastonosql
{
    class ConnectionListWidgetItem
            : public QTreeWidgetItem
    {
    public:
        ConnectionListWidgetItem(IConnectionSettingsBaseSPtr connection);
        void setConnection(IConnectionSettingsBaseSPtr cons);
        IConnectionSettingsBaseSPtr connection() const;

    private:
        IConnectionSettingsBaseSPtr connection_;
    };

    class ConnectionListWidgetItemEx
            : public ConnectionListWidgetItem
    {
    public:
        ConnectionListWidgetItemEx(IConnectionSettingsBaseSPtr connection, serverTypes st);
    };

    class ClusterConnectionListWidgetItem
            : public QTreeWidgetItem
    {
    public:
        ClusterConnectionListWidgetItem(IClusterSettingsBaseSPtr connection);
        void setConnection(IClusterSettingsBaseSPtr cons);
        IClusterSettingsBaseSPtr connection() const;

    private:
        IClusterSettingsBaseSPtr connection_;
    };
}
