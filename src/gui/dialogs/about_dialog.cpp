/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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
#include <QTreeWidget>

#include <Qsci/qsciglobal.h>
#include <libssh2.h>
#include <openssl/opensslv.h>

#include <common/macros.h>  // for STRINGIZE, VERIFY
#include <common/qt/convert2string.h>

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"

namespace {
const QString trDescription = QObject::tr(
#if defined(PROJECT_BUILD_TYPE_VERSION) && defined(PROJECT_BUILD_RELEASE)
    "<h3>" PROJECT_NAME_TITLE " " PROJECT_VERSION "<br/>Revision:" PROJECT_GIT_VERSION "</h3>"
#else
    "<h3>" PROJECT_NAME_TITLE " " PROJECT_VERSION " " PROJECT_BUILD_TYPE_VERSION STRINGIZE(
        PROJECT_VERSION_TWEAK) "<br/>Revision:" PROJECT_GIT_VERSION "</h3>"
#endif
    PROJECT_SUMMARY
    "<br/>"
    "<br/>"
    "Visit our website: <a href=\"" PROJECT_DOMAIN "\">" PROJECT_NAME_TITLE
    "</a> <br/>"
    "<br/>"
    "<a href=\"" PROJECT_GITHUB_FORK
    "\">Fork</a> project or <a "
    "href=" PROJECT_GITHUB_ISSUES
    ">submit</a> issues/proposals on GitHub.  <br/>"
    "<br/>"
    "Copyright 2014-2016 <a "
    "href=\"" PROJECT_COMPANYNAME_DOMAIN "\">" PROJECT_COMPANYNAME
    "</a>. All rights reserved.<br/>"
    "<br/>"
    "The program is provided AS IS with NO WARRANTY OF ANY "
    "KIND, "
    "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND "
    "FITNESS FOR A "
    "PARTICULAR PURPOSE.<br/>");

const QString tAboutTitle = QObject::tr("About " PROJECT_NAME_TITLE);

void addDBItem(QTreeWidget* dblist_widget, const std::string& name, const char* lib_name, const char* version) {
  QTreeWidgetItem* treeItem = new QTreeWidgetItem;
  QString qname;
  if (common::ConvertFromString(name, &qname)) {
    treeItem->setText(0, qname);
  }
  treeItem->setText(1, lib_name);
  treeItem->setText(2, version);
  dblist_widget->addTopLevelItem(treeItem);
}

void addLibItem(QTreeWidget* libs_list_widget, const std::string& name, const char* version) {
  QTreeWidgetItem* treeItem = new QTreeWidgetItem;
  QString qname;
  if (common::ConvertFromString(name, &qname)) {
    treeItem->setText(0, qname);
  }
  treeItem->setText(1, version);
  libs_list_widget->addTopLevelItem(treeItem);
}
}  // namespace

namespace fastonosql {
namespace gui {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle(tAboutTitle);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  QVBoxLayout* glayout = new QVBoxLayout;
  QLabel* copyRightLabel = new QLabel(trDescription);
  copyRightLabel->setWordWrap(true);
  copyRightLabel->setOpenExternalLinks(true);
  copyRightLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton* closeButton = buttonBox->button(QDialogButtonBox::Close);
  buttonBox->addButton(closeButton,
                       QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &AboutDialog::reject));

  QIcon icon = GuiFactory::GetInstance().mainWindowIcon();
  QPixmap iconPixmap = icon.pixmap(48, 48);

  QGridLayout* copy_rights_layout = new QGridLayout;
  QLabel* logoLabel = new QLabel;
  logoLabel->setPixmap(iconPixmap);
  copy_rights_layout->addWidget(logoLabel, 0, 0, 1, 1);
  copy_rights_layout->addWidget(copyRightLabel, 0, 1, 4, 4);

  QTabWidget* main_tab = new QTabWidget;
  QTreeWidget* dblist_widget = new QTreeWidget;
  dblist_widget->setIndentation(5);

  QStringList dbcolums;
  dbcolums << translations::trName << QObject::tr("Based on") << QObject::tr("Version");
  dblist_widget->setHeaderLabels(dbcolums);
#ifdef BUILD_WITH_REDIS
  typedef core::ConnectionTraits<core::REDIS> redis_traits_t;
  addDBItem(dblist_widget, redis_traits_t::GeDBName(), redis_traits_t::BasedOn(), redis_traits_t::VersionApi());
#endif
#ifdef BUILD_WITH_MEMCACHED
  typedef core::ConnectionTraits<core::MEMCACHED> memcached_traits_t;
  addDBItem(dblist_widget, memcached_traits_t::GeDBName(), memcached_traits_t::BasedOn(),
            memcached_traits_t::VersionApi());
#endif
#ifdef BUILD_WITH_SSDB
  typedef core::ConnectionTraits<core::SSDB> ssdb_traits_t;
  addDBItem(dblist_widget, ssdb_traits_t::GeDBName(), ssdb_traits_t::BasedOn(), ssdb_traits_t::VersionApi());
#endif
#ifdef BUILD_WITH_LEVELDB
  typedef core::ConnectionTraits<core::LEVELDB> leveldb_traits_t;
  addDBItem(dblist_widget, leveldb_traits_t::GeDBName(), leveldb_traits_t::BasedOn(), leveldb_traits_t::VersionApi());
#endif
#ifdef BUILD_WITH_ROCKSDB
  typedef core::ConnectionTraits<core::ROCKSDB> rocksdb_traits_t;
  addDBItem(dblist_widget, rocksdb_traits_t::GeDBName(), rocksdb_traits_t::BasedOn(), rocksdb_traits_t::VersionApi());
#endif
#ifdef BUILD_WITH_UNQLITE
  typedef core::ConnectionTraits<core::UNQLITE> unqlite_traits_t;
  addDBItem(dblist_widget, unqlite_traits_t::GeDBName(), unqlite_traits_t::BasedOn(), unqlite_traits_t::VersionApi());
#endif
#ifdef BUILD_WITH_LMDB
  typedef core::ConnectionTraits<core::LMDB> lmdb_traits_t;
  addDBItem(dblist_widget, lmdb_traits_t::GeDBName(), lmdb_traits_t::BasedOn(), lmdb_traits_t::VersionApi());
#endif
#ifdef BUILD_WITH_UPSCALEDB
  typedef core::ConnectionTraits<core::UPSCALEDB> upscaledb_traits_t;
  addDBItem(dblist_widget, upscaledb_traits_t::GeDBName(), upscaledb_traits_t::BasedOn(),
            upscaledb_traits_t::VersionApi());
#endif
#ifdef BUILD_WITH_FORESTDB
  typedef core::ConnectionTraits<core::FORESTDB> forestdb_traits_t;
  addDBItem(dblist_widget, forestdb_traits_t::GeDBName(), forestdb_traits_t::BasedOn(),
            forestdb_traits_t::VersionApi());
#endif
  main_tab->addTab(dblist_widget, QObject::tr("Availible databases"));

  QTreeWidget* libs_list_widget = new QTreeWidget;
  libs_list_widget->setIndentation(5);

  QStringList libscolums;
  libscolums << translations::trName << QObject::tr("Version");
  libs_list_widget->setHeaderLabels(libscolums);
  addLibItem(libs_list_widget, "Qt", QT_VERSION_STR);
  addLibItem(libs_list_widget, "QScintilla", QSCINTILLA_VERSION_STR);
  addLibItem(libs_list_widget, "libssh2", LIBSSH2_VERSION);
  addLibItem(libs_list_widget, "OpenSSL", OPENSSL_VERSION_TEXT);
  main_tab->addTab(libs_list_widget, QObject::tr("External libraries"));

  copy_rights_layout->addWidget(main_tab, 4, 1, 1, 5);
  glayout->addLayout(copy_rights_layout);

  glayout->addWidget(buttonBox);
  glayout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(glayout);
}

}  // namespace gui
}  // namespace fastonosql
