#pragma once

#include <QMainWindow>

#include "core/connection_settings.h"

class QAction;
class QDockWidget;

#ifdef OS_ANDROID
class QGestureEvent;
class QSwipeGesture;
class QTapAndHoldGesture;
#endif

namespace fastonosql
{
    class MainWidget;
    class ExplorerTreeView;

    class MainWindow
            : public QMainWindow
    {
        Q_OBJECT
    public:
        enum
        {
            min_height = 480,
            min_width = 640,
            MaxRecentConnections = 5
        };

        MainWindow();
        ~MainWindow();

    protected:
        virtual void changeEvent(QEvent* ev);
        virtual void showEvent(QShowEvent* ev);

    private Q_SLOTS:
        void open();
        void about();
        void openPreferences();
        void checkUpdate();
        void reportBug();
        void enterLeaveFullScreen();
        void openEncodeDecodeDialog();
        void openRecentConnection();

        void versionAvailible(bool succesResult, const QString& version);

    protected:
#ifdef OS_ANDROID
        virtual bool event(QEvent *event);
        bool gestureEvent(QGestureEvent *event);
        void swipeTriggered(QSwipeGesture* swipeEvent);
        void tapAndHoldTriggered(QTapAndHoldGesture* tapEvent);
#endif

    private:
        void createStatusBar();
        void retranslateUi();
        void updateRecentConnectionActions();
        void clearRecentConnectionsMenu();
        void createServer(IConnectionSettingsBaseSPtr settings);
        void createCluster(IClusterSettingsBaseSPtr settings);

        QAction* openAction_;
        QAction* exitAction_;
        QAction* preferencesAction_;
        QAction* fullScreanAction_;
        QAction* windowAction_;
        QAction* aboutAction_;
        QAction* reportBugAction_;
        QAction* fileAction_;
        QAction* editAction_;
        QAction* checkUpdateAction_;
        QAction* toolsAction_;
        QAction* encodeDecodeDialogAction_;
        QAction* helpAction_;
        QAction* explorerAction_;
        QAction* logsAction_;
        QAction* recentConnections_;
        QAction* clearMenu_;
        QAction* recentConnectionsActs_[MaxRecentConnections];

        ExplorerTreeView* exp_;
        QDockWidget* expDock_;
        QDockWidget* logDock_;
        bool isCheckedInSession_;
    };

    class UpdateChecker
            : public QObject
    {
        Q_OBJECT
    public:
        UpdateChecker(QObject* parent = 0);

    Q_SIGNALS:
        void versionAvailibled(bool succesResult, const QString& version);

    public Q_SLOTS:
        void routine();
    };
}
