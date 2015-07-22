#pragma once

#include <QDialog>

#include "core/connection_settings.h"
#include "core/types.h"

class QMovie;
class QLabel;
class QTreeWidget;

namespace fasto
{
    namespace qt
    {
        namespace gui
        {
            class GlassWidget;
        }
    }
}

namespace fastonosql
{
    class DiscoveryConnection
            : public QObject
    {
        Q_OBJECT
    public:
        DiscoveryConnection(IConnectionSettingsBaseSPtr conn, QObject* parent = 0);

    Q_SIGNALS:
        void connectionResult(bool suc, qint64 msTimeExecute, const QString& resultText, std::vector<ServerDiscoveryInfoSPtr> infos);

    public Q_SLOTS:
        void routine();

    private:
        IConnectionSettingsBaseSPtr connection_;
        common::time64_t startTime_;
    };

    class DiscoveryDiagnosticDialog
            : public QDialog
    {
        Q_OBJECT

    public:
        enum
        {
            fix_height = 320,
            fix_width = 480
        };

        DiscoveryDiagnosticDialog(QWidget* parent, IConnectionSettingsBaseSPtr connection, IClusterSettingsBaseSPtr cluster);
        std::vector<IConnectionSettingsBaseSPtr> selectedConnections() const;

    private Q_SLOTS:
        void connectionResult(bool suc, qint64 mstimeExecute, const QString &resultText, std::vector<ServerDiscoveryInfoSPtr> infos);

    protected:
        virtual void showEvent(QShowEvent* e);

    private:
        void testConnection(IConnectionSettingsBaseSPtr connection);

        fasto::qt::gui::GlassWidget *glassWidget_;
        QLabel* executeTimeLabel_;
        QLabel* statusLabel_;
        QTreeWidget* listWidget_;
        QLabel* iconLabel_;
        IClusterSettingsBaseSPtr cluster_;
    };
}
