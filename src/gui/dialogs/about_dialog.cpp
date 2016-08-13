/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/about_dialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "common/macros.h"              // for STRINGIZE, VERIFY
#include "gui/gui_factory.h"            // for GuiFactory

namespace {
  const QString trDescription = QObject::tr(
#if defined(PROJECT_BUILD_TYPE_VERSION) && defined(PROJECT_BUILD_RELEASE)
    "<h3>" PROJECT_NAME_TITLE " " PROJECT_VERSION "<br/>Revision:" PROJECT_GIT_VERSION "</h3>"
#else
    "<h3>" PROJECT_NAME_TITLE " " PROJECT_VERSION " " PROJECT_BUILD_TYPE_VERSION STRINGIZE(PROJECT_VERSION_TWEAK) "<br/>Revision:" PROJECT_GIT_VERSION "</h3>"
#endif
    PROJECT_SUMMARY
    "<br/>"
    "<br/>"
    "Visit " PROJECT_NAME_TITLE " website: <a href=\"http://" PROJECT_DOMAIN "\">" PROJECT_DOMAIN "</a> <br/>"
    "<br/>"
    "<a href=\"https://" PROJECT_GITHUB_FORK "\">Fork</a> project or <a href=\"https://" PROJECT_GITHUB_ISSUES "\">submit</a> issues/proposals on GitHub.  <br/>"
    "<br/>"
    "Copyright 2014-2016 <a href=\"http://" PROJECT_COMPANYNAME_DOMAIN "\">" PROJECT_COMPANYNAME "</a>. All rights reserved.<br/>"
    "<br/>"
    "The program is provided AS IS with NO WARRANTY OF ANY KIND, "
    "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A "
    "PARTICULAR PURPOSE.<br/>");

  const QString tAboutTitle = QObject::tr("About " PROJECT_NAME_TITLE);
}  // namespace

namespace fastonosql {
namespace gui {

AboutDialog::AboutDialog(QWidget* parent)
  : QDialog(parent) {
  setWindowTitle(tAboutTitle);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  QGridLayout* glayout = new QGridLayout;

  QLabel* copyRightLabel = new QLabel(trDescription);
  copyRightLabel->setWordWrap(true);
  copyRightLabel->setOpenExternalLinks(true);
  copyRightLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton* closeButton = buttonBox->button(QDialogButtonBox::Close);
  buttonBox->addButton(closeButton, QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &AboutDialog::reject));

  QIcon icon = GuiFactory::instance().mainWindowIcon();
  QPixmap iconPixmap = icon.pixmap(48, 48);

  QLabel* logoLabel = new QLabel;
  logoLabel->setPixmap(iconPixmap);
  glayout->addWidget(logoLabel, 0, 0, 1, 1);
  glayout->addWidget(copyRightLabel, 0, 1, 4, 4);
  glayout->addWidget(buttonBox, 4, 0, 1, 5);
  glayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(glayout);
}

}  // namespace gui
}  // namespace fastonosql
