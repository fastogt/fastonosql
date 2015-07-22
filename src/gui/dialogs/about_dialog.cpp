#include "gui/dialogs/about_dialog.h"

#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGridLayout>

#include "gui/gui_factory.h"

namespace
{
    const QString description = QObject::tr(
#if defined(PROJECT_BUILD_TYPE) && defined(PROJECT_BUILD_RELEASE)
        "<h3>" PROJECT_NAME_TITLE " " PROJECT_VERSION "<br/>Revision:" PROJECT_GIT_VERSION "</h3>"
#else
        "<h3>" PROJECT_NAME_TITLE " " PROJECT_VERSION " " PROJECT_BUILD_TYPE STRINGIZE(PROJECT_VERSION_TWEAK) "<br/>Revision:" PROJECT_GIT_VERSION "</h3>"
#endif
        "Cross-platform open source Redis, Memcached, SSDB management tool."
        "<br/>"
        "<br/>"
        "Visit " PROJECT_NAME_TITLE " website: <a href=\"http://" PROJECT_DOMAIN "\">" PROJECT_DOMAIN "</a> <br/>"
        "<br/>"
        "<a href=\"https://" PROJECT_GITHUB_FORK "\">Fork</a> project or <a href=\"https://" PROJECT_GITHUB_ISSUES "\">submit</a> issues/proposals on GitHub.  <br/>"
        "<br/>"
        "Copyright 2014-2015 <a href=\"http://" PROJECT_COMPANYNAME_DOMAIN "\">" PROJECT_COMPANYNAME "</a>. All rights reserved.<br/>"
        "<br/>"
        "The program is provided AS IS with NO WARRANTY OF ANY KIND, "
        "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A "
        "PARTICULAR PURPOSE.<br/>");
}

namespace fastonosql
{
    AboutDialog::AboutDialog(QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle("About " PROJECT_NAME_TITLE);
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        QGridLayout* glayout = new QGridLayout;

        QLabel* copyRightLabel = new QLabel(description);
        copyRightLabel->setWordWrap(true);
        copyRightLabel->setOpenExternalLinks(true);
        copyRightLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        QPushButton *closeButton = buttonBox->button(QDialogButtonBox::Close);
        buttonBox->addButton(closeButton, QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
        VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &AboutDialog::reject));

        QIcon icon = GuiFactory::instance().mainWindowIcon();
        QPixmap iconPixmap = icon.pixmap(48, 48);

        QLabel* logoLabel = new QLabel;
        logoLabel->setPixmap(iconPixmap);
        glayout->addWidget(logoLabel, 0, 0, 1, 1);
        glayout->addWidget(copyRightLabel, 0, 1, 4, 4);
        glayout->addWidget(buttonBox, 4, 0, 1, 5);
        setLayout(glayout);
        glayout->setSizeConstraint(QLayout::SetFixedSize);
    }
}
