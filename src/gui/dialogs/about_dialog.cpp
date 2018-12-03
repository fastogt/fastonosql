/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <string>

#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeWidget>

#include <Qsci/qsciglobal.h>
#include <json-c/json_c_version.h>
#include <libssh2.h>
#include <openssl/opensslv.h>
#include <snappy-stubs-public.h>

#include <common/config.h>
#include <common/qt/convert2string.h>

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"

#define SNAPPY_VERSION_TEXT STRINGIZE(SNAPPY_MAJOR) "." STRINGIZE(SNAPPY_MINOR) "." STRINGIZE(SNAPPY_PATCHLEVEL)

namespace {
const QString trDescription = QObject::tr("<h3>" PROJECT_NAME_TITLE " " PROJECT_VERSION
                                          "<br/>Revision:" PROJECT_VERSION_GIT "</h3>" PROJECT_SUMMARY
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
                                          "Copyright 2014-2018 <a "
                                          "href=\"" PROJECT_COMPANYNAME_DOMAIN "\">" PROJECT_COMPANYNAME
                                          "</a>. All rights reserved.<br/>"
                                          "<br/>"
                                          "The program is provided AS IS with NO WARRANTY OF ANY "
                                          "KIND, "
                                          "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND "
                                          "FITNESS FOR A "
                                          "PARTICULAR PURPOSE.<br/>");

const QString trAbout = QObject::tr("About");
const QString trBasedOn = QObject::tr("Based on");
const QString trVersion = QObject::tr("Version");
const QString trAvailibleDatabases = QObject::tr("Availible databases");
const QString trExternalLibraries = QObject::tr("External libraries");
const QString trLicenseAgreement = QObject::tr("License agreement");

QTreeWidgetItem* createDbItem(const std::string& name, const char* lib_name, const char* version) {
  QTreeWidgetItem* tree_item = new QTreeWidgetItem;
  QString qname;
  if (common::ConvertFromString(name, &qname)) {
    tree_item->setText(0, qname);
  }
  tree_item->setText(1, lib_name);
  tree_item->setText(2, version);
  return tree_item;
}

QTreeWidgetItem* createLibItem(const std::string& name, const char* version) {
  QTreeWidgetItem* tree_item = new QTreeWidgetItem;
  QString qname;
  if (common::ConvertFromString(name, &qname)) {
    tree_item->setText(0, qname);
  }
  tree_item->setText(1, version);
  return tree_item;
}
}  // namespace

namespace fastonosql {
namespace gui {

AboutDialog::AboutDialog(QWidget* parent) : base_class(trAbout + " " PROJECT_NAME_TITLE, parent) {
  QTabWidget* about_tabs = new QTabWidget;
  QWidget* about_tab = new QWidget;

  // about content
  QVBoxLayout* glayout = new QVBoxLayout;
  about_tab->setLayout(glayout);
  QLabel* copy_right_label = new QLabel(trDescription);
  copy_right_label->setWordWrap(true);
  copy_right_label->setOpenExternalLinks(true);
  copy_right_label->setTextInteractionFlags(Qt::TextBrowserInteraction);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton* close_button = button_box->button(QDialogButtonBox::Close);
  button_box->addButton(close_button,
                        QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &AboutDialog::reject));

  QIcon icon = GuiFactory::GetInstance().mainWindowIcon();
  QPixmap iconPixmap = icon.pixmap(48, 48);

  QGridLayout* copy_rights_layout = new QGridLayout;
  QLabel* logoLabel = new QLabel;
  logoLabel->setPixmap(iconPixmap);
  copy_rights_layout->addWidget(logoLabel, 0, 0, 1, 1);
  copy_rights_layout->addWidget(copy_right_label, 0, 1, 4, 4);

  QTabWidget* main_tab = new QTabWidget;
  QTreeWidget* dblist_widget = new QTreeWidget;
  dblist_widget->setIndentation(5);

  QStringList dbcolums;
  dbcolums << translations::trName << trBasedOn << trVersion;
  dblist_widget->setHeaderLabels(dbcolums);
#if defined(BUILD_WITH_REDIS) && defined(HAVE_REDIS)
  typedef core::ConnectionTraits<core::REDIS> redis_traits_t;
  dblist_widget->addTopLevelItem(
      createDbItem(redis_traits_t::GetDBName(), redis_traits_t::GetBasedOn(), redis_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_MEMCACHED) && defined(HAVE_MEMCACHED)
  typedef core::ConnectionTraits<core::MEMCACHED> memcached_traits_t;
  dblist_widget->addTopLevelItem(createDbItem(memcached_traits_t::GetDBName(), memcached_traits_t::GetBasedOn(),
                                              memcached_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_SSDB) && defined(HAVE_SSDB)
  typedef core::ConnectionTraits<core::SSDB> ssdb_traits_t;
  dblist_widget->addTopLevelItem(
      createDbItem(ssdb_traits_t::GetDBName(), ssdb_traits_t::GetBasedOn(), ssdb_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_LEVELDB) && defined(HAVE_LEVELDB)
  typedef core::ConnectionTraits<core::LEVELDB> leveldb_traits_t;
  dblist_widget->addTopLevelItem(
      createDbItem(leveldb_traits_t::GetDBName(), leveldb_traits_t::GetBasedOn(), leveldb_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_ROCKSDB) && defined(HAVE_ROCKSDB)
  typedef core::ConnectionTraits<core::ROCKSDB> rocksdb_traits_t;
  dblist_widget->addTopLevelItem(
      createDbItem(rocksdb_traits_t::GetDBName(), rocksdb_traits_t::GetBasedOn(), rocksdb_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_UNQLITE) && defined(HAVE_UNQLITE)
  typedef core::ConnectionTraits<core::UNQLITE> unqlite_traits_t;
  dblist_widget->addTopLevelItem(
      createDbItem(unqlite_traits_t::GetDBName(), unqlite_traits_t::GetBasedOn(), unqlite_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_LMDB) && defined(HAVE_LMDB)
  typedef core::ConnectionTraits<core::LMDB> lmdb_traits_t;
  dblist_widget->addTopLevelItem(
      createDbItem(lmdb_traits_t::GetDBName(), lmdb_traits_t::GetBasedOn(), lmdb_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_UPSCALEDB) && defined(HAVE_UPSCALEDB)
  typedef core::ConnectionTraits<core::UPSCALEDB> upscaledb_traits_t;
  dblist_widget->addTopLevelItem(createDbItem(upscaledb_traits_t::GetDBName(), upscaledb_traits_t::GetBasedOn(),
                                              upscaledb_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_FORESTDB) && defined(HAVE_FORESTDB)
  typedef core::ConnectionTraits<core::FORESTDB> forestdb_traits_t;
  dblist_widget->addTopLevelItem(createDbItem(forestdb_traits_t::GetDBName(), forestdb_traits_t::GetBasedOn(),
                                              forestdb_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_PIKA) && defined(HAVE_PIKA)
  typedef core::ConnectionTraits<core::PIKA> pika_traits_t;
  dblist_widget->addTopLevelItem(
      createDbItem(pika_traits_t::GetDBName(), pika_traits_t::GetBasedOn(), pika_traits_t::GetVersionApi()));
#endif
#if defined(BUILD_WITH_DYNOMITEDB) && defined(HAVE_DYNOMITE_REDIS)
  typedef core::ConnectionTraits<core::DYNOMITEDB> dynomite_redis_traits_t;
  dblist_widget->addTopLevelItem(createDbItem(dynomite_redis_traits_t::GetDBName(),
                                              dynomite_redis_traits_t::GetBasedOn(),
                                              dynomite_redis_traits_t::GetVersionApi()));
#endif
  main_tab->addTab(dblist_widget, trAvailibleDatabases);

  QTreeWidget* libs_list_widget = new QTreeWidget;
  libs_list_widget->setIndentation(5);

  QStringList libscolums;
  libscolums << translations::trName << trVersion;
  libs_list_widget->setHeaderLabels(libscolums);
  libs_list_widget->addTopLevelItem(createLibItem("Qt", QT_VERSION_STR));
  libs_list_widget->addTopLevelItem(createLibItem("QScintilla", QSCINTILLA_VERSION_STR));
  libs_list_widget->addTopLevelItem(createLibItem("libssh2", LIBSSH2_VERSION));
  libs_list_widget->addTopLevelItem(createLibItem("OpenSSL", OPENSSL_VERSION_TEXT));
  libs_list_widget->addTopLevelItem(createLibItem("FastoNoSQL core", FASTONOSQL_CORE_VERSION_TEXT));
  libs_list_widget->addTopLevelItem(createLibItem("common", COMMON_VERSION_TEXT));
  libs_list_widget->addTopLevelItem(createLibItem("Snappy", SNAPPY_VERSION_TEXT));
  libs_list_widget->addTopLevelItem(createLibItem("json-c", JSON_C_VERSION));
  main_tab->addTab(libs_list_widget, trExternalLibraries);

  copy_rights_layout->addWidget(main_tab, 4, 1, 1, 5);
  glayout->addLayout(copy_rights_layout);
  about_tabs->addTab(about_tab, trAbout);

  // license
  QTextEdit* license_tab = new QTextEdit;
  QFile file(":" PROJECT_NAME_LOWERCASE "/LICENSE");
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    license_tab->setHtml(file.readAll());
  }
  about_tabs->addTab(license_tab, trLicenseAgreement);

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(about_tabs);
  main_layout->addWidget(button_box);
  main_layout->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(main_layout);
}

}  // namespace gui
}  // namespace fastonosql
