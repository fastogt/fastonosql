#include <QApplication>
#include <QDesktopWidget>

#include "gui/main_window.h"
#include "gui/gui_factory.h"

#include "common/logger.h"

namespace
{
    const QSize preferedSize(1024, 768);
}

int main(int argc, char *argv[])
{    
    QApplication app(argc, argv);
    app.setOrganizationName(PROJECT_COMPANYNAME);
    app.setApplicationName(PROJECT_NAME);
    app.setApplicationVersion(PROJECT_VERSION);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setWindowIcon(fastonosql::GuiFactory::instance().logoIcon()); //default icon for app

#ifdef NDEBUG
    SET_LOG_LEVEL(common::logging::L_INFO);
#else
    SET_LOG_LEVEL(common::logging::L_NONE);
#endif

    fastonosql::MainWindow win;
    QRect screenGeometry = app.desktop()->availableGeometry();
    QSize screenSize(screenGeometry.width(), screenGeometry.height());

#ifdef OS_ANDROID
    win.resize(screenSize);
#else
    QSize size(screenGeometry.width()/2, screenGeometry.height()/2);
    if(preferedSize.height() < screenSize.height() && preferedSize.width() < screenSize.width()){
        win.resize(preferedSize);
    }
    else{
        win.resize(size);
    }

    QPoint center = screenGeometry.center();
    win.move(center.x() - win.width() * 0.5, center.y() - win.height() * 0.5);
#endif

    win.show();
    return app.exec();
}
