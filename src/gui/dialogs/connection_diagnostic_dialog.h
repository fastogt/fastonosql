#pragma once

#include <QDialog>

#include "core/connection_settings.h"

class QMovie;
class QLabel;

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

namespace fastoredis
{
    class TestConnection
            : public QObject
    {
        Q_OBJECT
    public:
        TestConnection(IConnectionSettingsBaseSPtr conn, QObject* parent = 0);

    Q_SIGNALS:
        void connectionResult(bool suc, qint64 msTimeExecute, const QString& resultText);

    public Q_SLOTS:
        void routine();

    private:
        IConnectionSettingsBaseSPtr connection_;
        common::time64_t startTime_;
    };

    class ConnectionDiagnosticDialog
            : public QDialog
    {
        Q_OBJECT

    public:
        enum
        {
            fix_height = 160,
            fix_width = 240
        };

        ConnectionDiagnosticDialog(QWidget* parent, IConnectionSettingsBaseSPtr connection);

    private Q_SLOTS:
        void connectionResult(bool suc, qint64 mstimeExecute, const QString &resultText);

    protected:
        virtual void showEvent(QShowEvent* e);

    private:
        void testConnection(IConnectionSettingsBaseSPtr connection);
        fasto::qt::gui::GlassWidget *glassWidget_;
        QLabel* executeTimeLabel_;
        QLabel* statusLabel_;
        QLabel* iconLabel_;
    };
}
