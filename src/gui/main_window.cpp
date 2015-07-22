#include "gui/main_window.h"

#include <QAction>
#include <QMenuBar>
#include <QDockWidget>
#include <QMessageBox>
#include <QThread>
#include <QApplication>

#ifdef OS_ANDROID
#include <QGestureEvent>
#endif

#include "fasto/qt/gui/app_style.h"
#include "fasto/qt/translations/translations.h"
#include "fasto/qt/logger.h"
#include "common/qt/convert_string.h"
#include "common/net/socket_tcp.h"

#include "gui/shortcuts.h"
#include "gui/gui_factory.h"
#include "gui/dialogs/about_dialog.h"
#include "gui/dialogs/preferences_dialog.h"
#include "gui/dialogs/connections_dialog.h"
#include "gui/widgets/log_tab_widget.h"
#include "gui/widgets/main_widget.h"
#include "gui/explorer/explorer_tree_view.h"
#include "gui/dialogs/encode_decode_dialog.h"

#include "core/servers_manager.h"
#include "core/settings_manager.h"
#include "core/command_logger.h"
#include "core/icluster.h"

#include "server_config_daemon/server_config.h"

#include "translations/global.h"

namespace
{
    bool isNeededUpdate(const QString& serverVersion)
    {
        if(serverVersion.isEmpty()){
            return false;
        }

        QString curVer;
        int pos = 0;
        uint serMaj = 0;
        uint serMin = 0;
        uint serPatch = 0;

        for(int i = 0; i < serverVersion.length(); ++i){
            QChar ch = serverVersion[i];
            if(ch == '.'){
                if(pos == 0){
                    serMaj = curVer.toUInt();
                }
                else if(pos == 1){
                    serMin = curVer.toUInt();
                }
                else if(pos == 2){
                    serPatch = curVer.toUInt();
                }

                ++pos;
                curVer.clear();
            }
            else{
                curVer += ch;
            }
        }

        if(pos != 3){
            return false;
        }

        if(PROJECT_VERSION_MAJOR < serMaj){
            return true;
        }

        if(PROJECT_VERSION_MINOR < serMin){
            return true;
        }

        return PROJECT_VERSION_PATCH < serPatch;
    }

    const QKeySequence logsKeySequence = Qt::CTRL + Qt::Key_L;
    const QKeySequence explorerKeySequence = Qt::CTRL + Qt::Key_T;
}

namespace fastonosql
{
    MainWindow::MainWindow()
        : QMainWindow(), isCheckedInSession_(false)
    {
#ifdef OS_ANDROID
        setAttribute(Qt::WA_AcceptTouchEvents);
        //setAttribute(Qt::WA_StaticContents);

        //grabGesture(Qt::TapGesture); //click
        grabGesture(Qt::TapAndHoldGesture); //long tap

        //grabGesture(Qt::SwipeGesture); //swipe
        //grabGesture(Qt::PanGesture); // drag and drop
        //grabGesture(Qt::PinchGesture); //zoom
#endif
        using namespace common;
        QString lang = SettingsManager::instance().currentLanguage();
        QString newLang = fasto::qt::translations::applyLanguage(lang);
        SettingsManager::instance().setCurrentLanguage(newLang);

        QString style = SettingsManager::instance().currentStyle();
        fasto::qt::gui::applyStyle(style);

        setWindowTitle(PROJECT_NAME_TITLE " " PROJECT_VERSION);

        openAction_ = new QAction(this);
        openAction_->setIcon(GuiFactory::instance().openIcon());
        openAction_->setShortcut(openKey);
        VERIFY(connect(openAction_, &QAction::triggered, this, &MainWindow::open));

        // Exit action
        exitAction_ = new QAction(this);
        exitAction_->setShortcut(quitKey);
        VERIFY(connect(exitAction_, &QAction::triggered, this, &MainWindow::close));

        // File menu
        QMenu *fileMenu = new QMenu(this);
        fileAction_ = menuBar()->addMenu(fileMenu);
        fileMenu->addAction(openAction_);
        QMenu *recentMenu = new QMenu(this);
        recentConnections_ = fileMenu->addMenu(recentMenu);
        for (int i = 0; i < MaxRecentConnections; ++i) {
            recentConnectionsActs_[i] = new QAction(this);
            VERIFY(connect(recentConnectionsActs_[i], &QAction::triggered, this, &MainWindow::openRecentConnection));
            recentMenu->addAction(recentConnectionsActs_[i]);
        }

        clearMenu_ = new QAction(this);
        recentMenu->addSeparator();
        VERIFY(connect(clearMenu_, &QAction::triggered, this, &MainWindow::clearRecentConnectionsMenu));
        recentMenu->addAction(clearMenu_);

        fileMenu->addSeparator();
        fileMenu->addAction(exitAction_);
        updateRecentConnectionActions();

        preferencesAction_ = new QAction(this);
        preferencesAction_->setIcon(GuiFactory::instance().preferencesIcon());
        VERIFY(connect(preferencesAction_, &QAction::triggered, this, &MainWindow::openPreferences));

        QMenu *editMenu = new QMenu(this);
        editAction_ = menuBar()->addMenu(editMenu);
        editMenu->addAction(preferencesAction_);

        //tools menu
        QMenu *tools = new QMenu(this);
        toolsAction_ = menuBar()->addMenu(tools);

        encodeDecodeDialogAction_ = new QAction(this);
        encodeDecodeDialogAction_->setIcon(GuiFactory::instance().encodeDecodeIcon());
        VERIFY(connect(encodeDecodeDialogAction_, &QAction::triggered, this, &MainWindow::openEncodeDecodeDialog));
        tools->addAction(encodeDecodeDialogAction_);

        //window menu
        QMenu *window = new QMenu(this);
        windowAction_ = menuBar()->addMenu(window);
        fullScreanAction_ = new QAction(this);
        fullScreanAction_->setShortcut(fullScreenKey);
        VERIFY(connect(fullScreanAction_, &QAction::triggered, this, &MainWindow::enterLeaveFullScreen));
        window->addAction(fullScreanAction_);

        QMenu *views = new QMenu(translations::trViews, this);
        window->addMenu(views);

        QMenu *helpMenu = new QMenu(this);
        aboutAction_ = new QAction(this);
        VERIFY(connect(aboutAction_, &QAction::triggered, this, &MainWindow::about));

        helpAction_ = menuBar()->addMenu(helpMenu);

        checkUpdateAction_ = new QAction(this);
        VERIFY(connect(checkUpdateAction_, &QAction::triggered, this, &MainWindow::checkUpdate));

        reportBugAction_ = new QAction(this);
        VERIFY(connect(reportBugAction_, &QAction::triggered, this, &MainWindow::reportBug));

        helpMenu->addAction(checkUpdateAction_);
        helpMenu->addSeparator();
        helpMenu->addAction(reportBugAction_);
        helpMenu->addSeparator();
        helpMenu->addAction(aboutAction_);

        MainWidget *mainW = new MainWidget;
        setCentralWidget(mainW);

        exp_ = new ExplorerTreeView(this);
        VERIFY(connect(exp_, &ExplorerTreeView::openedConsole, mainW, &MainWidget::openConsole));
        VERIFY(connect(exp_, &ExplorerTreeView::closeServer, &ServersManager::instance(), &ServersManager::closeServer));
        VERIFY(connect(exp_, &ExplorerTreeView::closeCluster, &ServersManager::instance(), &ServersManager::closeCluster));
        expDock_ = new QDockWidget(this);
        explorerAction_ = expDock_->toggleViewAction();
        explorerAction_->setShortcut(explorerKeySequence);
        explorerAction_->setChecked(true);
        views->addAction(explorerAction_);

        expDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
        expDock_->setWidget(exp_);
        expDock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
        expDock_->setVisible(true);
        addDockWidget(Qt::LeftDockWidgetArea, expDock_);

        LogTabWidget *log = new LogTabWidget(this);
        VERIFY(connect(&fasto::qt::Logger::instance(), &fasto::qt::Logger::printed, log, &LogTabWidget::addLogMessage));
        VERIFY(connect(&CommandLogger::instance(), &CommandLogger::printed, log, &LogTabWidget::addCommand));
        logDock_ = new QDockWidget(this);
        logsAction_ = logDock_->toggleViewAction();
        logsAction_->setShortcut(logsKeySequence);
        logsAction_->setChecked(false);
        views->addAction(logsAction_);

        logDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
        logDock_->setWidget(log);
        logDock_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
        logDock_->setVisible(false);
        addDockWidget(Qt::BottomDockWidgetArea, logDock_);

        setMinimumSize(QSize(min_width, min_height));
        createStatusBar();
        retranslateUi();
    }

    MainWindow::~MainWindow()
    {
        ServersManager::instance().clear();
    }

    void MainWindow::changeEvent(QEvent* ev)
    {
        if(ev->type() == QEvent::LanguageChange){
            retranslateUi();
        }

        return QMainWindow::changeEvent(ev);
    }

    void MainWindow::showEvent(QShowEvent* ev)
    {
        QMainWindow::showEvent(ev);
        bool isA = SettingsManager::instance().autoCheckUpdates();
        if(isA && !isCheckedInSession_){
            isCheckedInSession_ = true;
            checkUpdate();
        }
    }

    void MainWindow::open()
    {
        ConnectionsDialog dlg(this);
        int result = dlg.exec();
        if(result == QDialog::Accepted){
            if(IConnectionSettingsBaseSPtr con = dlg.selectedConnection()){
                createServer(con);
            }
            else if(IClusterSettingsBaseSPtr clus = dlg.selectedCluster()){
                createCluster(clus);
            }
        }
    }

    void MainWindow::about()
    {
        AboutDialog dlg(this);
        dlg.exec();
    }

    void MainWindow::openPreferences()
    {
        PreferencesDialog dlg(this);
        dlg.exec();
    }

    void MainWindow::checkUpdate()
    {
        QThread* th = new QThread;
        UpdateChecker* cheker = new UpdateChecker;
        cheker->moveToThread(th);
        VERIFY(connect(th, &QThread::started, cheker, &UpdateChecker::routine));
        VERIFY(connect(cheker, &UpdateChecker::versionAvailibled, this, &MainWindow::versionAvailible));
        VERIFY(connect(cheker, &UpdateChecker::versionAvailibled, th, &QThread::quit));
        VERIFY(connect(th, &QThread::finished, cheker, &UpdateChecker::deleteLater));
        VERIFY(connect(th, &QThread::finished, th, &QThread::deleteLater));
        th->start();
    }

    void MainWindow::reportBug()
    {
        QDesktopServices::openUrl(QUrl(PROJECT_GITHUB_ISSUES));
    }

    void MainWindow::enterLeaveFullScreen()
    {
        using namespace translations;
        if(isFullScreen()){
            showNormal();
            fullScreanAction_->setText(trEnterFullScreen);
        }
        else{
            showFullScreen();
            fullScreanAction_->setText(trExitFullScreen);
        }
    }

    void MainWindow::openEncodeDecodeDialog()
    {
        EncodeDecodeDialog dlg(this);
        dlg.exec();
    }

    void MainWindow::openRecentConnection()
    {
        QAction *action = qobject_cast<QAction *>(sender());
        if (!action){
            return;
        }

        QString rcon = action->text();
        std::string srcon = common::convertToString(rcon);
        SettingsManager::ConnectionSettingsContainerType conns = SettingsManager::instance().connections();
        for(SettingsManager::ConnectionSettingsContainerType::const_iterator it = conns.begin(); it != conns.end(); ++it){
            IConnectionSettingsBaseSPtr con = *it;
            if(con && con->connectionName() == srcon){
                createServer(con);
                return;
            }
        }
    }

    void MainWindow::versionAvailible(bool succesResult, const QString& version)
    {
        using namespace translations;
        if(!succesResult){
            QMessageBox::information(this, trCheckVersion, trConnectionErrorText);
            checkUpdateAction_->setEnabled(true);
        }
        else{
            bool isn = isNeededUpdate(version);
            if(isn){
                QMessageBox::information(this, trCheckVersion,
                    QObject::tr("Availible new version: %1")
                        .arg(version));
            }
            else{
                QMessageBox::information(this, trCheckVersion,
                    QObject::tr("<h3>You're' up-to-date!</h3>" PROJECT_NAME_TITLE " %1 is currently the newest version available.")
                        .arg(version));
            }

            checkUpdateAction_->setEnabled(isn);
        }
    }

#ifdef OS_ANDROID
    bool MainWindow::event(QEvent *event)
    {
        if (event->type() == QEvent::Gesture){
            QGestureEvent *gest = static_cast<QGestureEvent*>(event);
            if(gest){
                return gestureEvent(gest);
            }
        }
        return QMainWindow::event(event);
    }

    bool MainWindow::gestureEvent(QGestureEvent *event)
    {

        /*if (QGesture *qpan = event->gesture(Qt::PanGesture)){
            QPanGesture* pan = static_cast<QPanGesture*>(qpan);
        }
        if (QGesture *qpinch = event->gesture(Qt::PinchGesture)){
            QPinchGesture* pinch = static_cast<QPinchGesture*>(qpinch);
        }
        if (QGesture *qtap = event->gesture(Qt::TapGesture)){
            QTapGesture* tap = static_cast<QTapGesture*>(qtap);
        }*/

        if (QGesture *qswipe = event->gesture(Qt::SwipeGesture)){
            QSwipeGesture* swipe = static_cast<QSwipeGesture*>(qswipe);
            swipeTriggered(swipe);
        }
        else if (QGesture *qtapandhold = event->gesture(Qt::TapAndHoldGesture)){
            QTapAndHoldGesture* tapandhold = static_cast<QTapAndHoldGesture*>(qtapandhold);
            tapAndHoldTriggered(tapandhold);
            event->accept();
        }

        return true;
    }

    void MainWindow::swipeTriggered(QSwipeGesture* swipeEvent)
    {

    }

    void MainWindow::tapAndHoldTriggered(QTapAndHoldGesture* tapEvent)
    {
        QPoint pos = tapEvent->position().toPoint();
        QContextMenuEvent* cont = new QContextMenuEvent(QContextMenuEvent::Mouse, pos, mapToGlobal(pos));
        QWidget* rec = childAt(pos);
        QApplication::postEvent(rec, cont);
    }
#endif

    void MainWindow::createStatusBar()
    {
    }

    void MainWindow::retranslateUi()
    {
        using namespace translations;
        openAction_->setText(trOpen);
        exitAction_->setText(trExit);
        fileAction_->setText(trFile);
        toolsAction_->setText(trTools);
        encodeDecodeDialogAction_->setText(trEncodeDecode);
        preferencesAction_->setText(trPreferences);
        checkUpdateAction_->setText(trCheckUpdate);
        editAction_->setText(trEdit);
        windowAction_->setText(trWindow);
        fullScreanAction_->setText(trEnterFullScreen);
        reportBugAction_->setText(trReportBug);
        aboutAction_->setText(tr("About %1...").arg(PROJECT_NAME));
        helpAction_->setText(trHelp);
        explorerAction_->setText(trExpTree);
        logsAction_->setText(trLogs);
        recentConnections_->setText(trRecentConnections);
        clearMenu_->setText(trClearMenu);
        expDock_->setWindowTitle(trExpTree);
        logDock_->setWindowTitle(trLogs);
    }

    void MainWindow::updateRecentConnectionActions()
    {
        QStringList connections = SettingsManager::instance().recentConnections();

        int numRecentFiles = qMin(connections.size(), static_cast<int>(MaxRecentConnections));

        for (int i = 0; i < numRecentFiles; ++i) {
            QString text = connections[i];
            recentConnectionsActs_[i]->setText(text);
            recentConnectionsActs_[i]->setVisible(true);
        }

        for (int j = numRecentFiles; j < MaxRecentConnections; ++j){
            recentConnectionsActs_[j]->setVisible(false);
        }

        bool isHaveItem = numRecentFiles > 0;
        clearMenu_->setVisible(isHaveItem);
        recentConnections_->setEnabled(isHaveItem);
    }

    void MainWindow::clearRecentConnectionsMenu()
    {
        SettingsManager::instance().clearRConnections();
        updateRecentConnectionActions();
    }

    void MainWindow::createServer(IConnectionSettingsBaseSPtr settings)
    {
        if(!settings){
            return;
        }

        QString rcon = common::convertFromString<QString>(settings->connectionName());
        SettingsManager::instance().removeRConnection(rcon);
        IServerSPtr server = ServersManager::instance().createServer(settings);
        exp_->addServer(server);
        SettingsManager::instance().addRConnection(rcon);
        updateRecentConnectionActions();
        if(SettingsManager::instance().autoOpenConsole()){
            MainWidget* mwidg = qobject_cast<MainWidget*>(centralWidget());
            if(mwidg){
                mwidg->openConsole(server, "");
            }
        }
    }

    void MainWindow::createCluster(IClusterSettingsBaseSPtr settings)
    {
        if(!settings){
            return;
        }

        if(!settings->root()){
            QMessageBox::critical(this, QObject::tr("Cluster open failed"),
                                  QObject::tr("Imposible open cluster \"%1\" without connections!").arg(common::convertFromString<QString>(settings->connectionName())));
            return;
        }

        IClusterSPtr cl = ServersManager::instance().createCluster(settings);
        if(!cl){
            return;
        }

        exp_->addCluster(cl);
        if(SettingsManager::instance().autoOpenConsole()){
            MainWidget* mwidg = qobject_cast<MainWidget*>(centralWidget());
            if(mwidg){
                mwidg->openConsole(cl->root(), "");
            }
        }
    }

    UpdateChecker::UpdateChecker(QObject* parent)
        : QObject(parent)
    {

    }

    void UpdateChecker::routine()
    {
        common::net::ClientSocketTcp s(SITE_URL, SERV_PORT);
        bool res = s.connect();
        if(!res){
            emit versionAvailibled(res, QString());
            return;
        }

        res = s.write(GET_VERSION, sizeof(GET_VERSION));
        if(!res){
            emit versionAvailibled(res, QString());
            s.close();
            return;
        }

        char version[128] = {0};
        res = s.read(version, 128);

        QString vers = common::convertFromString<QString>(version);
        emit versionAvailibled(res, vers);

        s.close();
        return;
    }
}

