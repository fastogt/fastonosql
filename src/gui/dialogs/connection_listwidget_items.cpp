#include "gui/dialogs/connection_listwidget_items.h"

#include "gui/gui_factory.h"

namespace fastoredis
{
    ConnectionListWidgetItem::ConnectionListWidgetItem(IConnectionSettingsBaseSPtr connection)
        : connection_()
    {
        setConnection(connection);
    }

    void ConnectionListWidgetItem::setConnection(IConnectionSettingsBaseSPtr cons)
    {
        if(!cons){
            return;
        }

        connection_ = cons;
        setText(0, common::convertFromString<QString>(connection_->connectionName()));
        connectionTypes conType = connection_->connectionType();
        setIcon(0, GuiFactory::instance().icon(conType));
        setText(1, common::convertFromString<QString>(connection_->fullAddress()));
    }

    IConnectionSettingsBaseSPtr ConnectionListWidgetItem::connection() const
    {
        return connection_;
    }

    ConnectionListWidgetItemEx::ConnectionListWidgetItemEx(IConnectionSettingsBaseSPtr connection, serverTypes st)
        : ConnectionListWidgetItem(connection)
    {
        std::string sert = common::convertToString(st);
        setText(2, common::convertFromString<QString>(sert));
    }

    ClusterConnectionListWidgetItem::ClusterConnectionListWidgetItem(IClusterSettingsBaseSPtr connection)
        : connection_(connection)
    {
        setText(0, common::convertFromString<QString>(connection_->connectionName()));
        setIcon(0, GuiFactory::instance().clusterIcon());

        IClusterSettingsBase::cluster_connection_type servers = connection_->nodes();

        for(int i = 0; i < servers.size(); ++i){
            IConnectionSettingsBaseSPtr con = servers[i];
            ConnectionListWidgetItem* item = new ConnectionListWidgetItem(con);
            addChild(item);
        }
    }

    void ClusterConnectionListWidgetItem::setConnection(IClusterSettingsBaseSPtr cons)
    {
        if(!cons){
            return;
        }

        connection_ = cons;
        setText(0, common::convertFromString<QString>(connection_->connectionName()));
        setIcon(0, GuiFactory::instance().clusterIcon());
    }

    IClusterSettingsBaseSPtr ClusterConnectionListWidgetItem::connection() const
    {
        return connection_;
    }
}
