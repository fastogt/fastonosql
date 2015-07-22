#pragma once

#include <QDialog>

#include "core/connection_settings.h"

class QTreeWidget;

namespace fastonosql
{
    class ConnectionsDialog
            : public QDialog
    {
        Q_OBJECT

    public:
        enum
        {
            min_height = 320,
            min_width = 480
        };

        ConnectionsDialog(QWidget* parent = 0);
        IConnectionSettingsBaseSPtr selectedConnection() const;
        IClusterSettingsBaseSPtr selectedCluster() const;

        virtual void accept();

    private Q_SLOTS:
        void add();
        void addCls();
        void remove();
        void edit();
        void connectionSelectChange();

    protected:
        virtual void changeEvent(QEvent* );

    private:
        void retranslateUi();
        void addConnection(IConnectionSettingsBaseSPtr con);
        void addCluster(IClusterSettingsBaseSPtr con);

        QTreeWidget* listWidget_;
        QPushButton* acButton_;
    };
}
