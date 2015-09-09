#pragma once

#include <QDialog>

class QComboBox;
class QLineEdit;
class QDialogButtonBox;
class QPushButton;
class QCheckBox;
class QSpinBox;

#include "core/connection_settings.h"

class QTreeWidget;
class QToolBar;

namespace fastonosql
{
    class ClusterDialog
            : public QDialog
    {
        Q_OBJECT

    public:
        typedef std::vector<IConnectionSettingsBaseSPtr> cluster_connection_type;
        ClusterDialog(QWidget* parent, IClusterSettingsBase* connection = NULL); //get ownerships connection
        IClusterSettingsBaseSPtr connection() const;

    public Q_SLOTS:
        virtual void accept();

    private Q_SLOTS:
        void typeConnectionChange(int index);
        void loggingStateChange(int value);
        void testConnection();
        void discoveryCluster();
        void showContextMenu(const QPoint& point);

        void setStartNode();

        void add();
        void remove();
        void edit();

        void itemSelectionChanged();

    protected:
        virtual void changeEvent(QEvent* );

    private:
        void retranslateUi();
        bool validateAndApply();
        void addConnection(IConnectionSettingsBaseSPtr con);

        IClusterSettingsBaseSPtr cluster_connection_;
        QLineEdit* connectionName_;
        QComboBox* typeConnection_;
        QCheckBox* logging_;
        QSpinBox* loggingMsec_;

        QToolBar* savebar_;
        QTreeWidget* listWidget_;

        QPushButton* testButton_;
        QPushButton* discoveryButton_;
        QDialogButtonBox* buttonBox_;
        QAction* setDefault_;
    };
}
