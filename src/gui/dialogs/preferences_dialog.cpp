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

#include "gui/dialogs/preferences_dialog.h"

#include <stddef.h>  // for size_t

#include <string>  // for string

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFontComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSplitter>
#include <QLineEdit>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for VERIFY, SIZEOFMASS

#include <common/qt/convert2string.h>             // for ConvertToString
#include <common/qt/gui/app_style.h>              // for applyFont, applyStyle, etc
#include <common/qt/translations/translations.h>  // for applyLanguage, etc

#include "proxy/settings_manager.h"  // for SettingsManager

#include "proxy/types.h"  // for viewsText, ConvertToString, etc

#include "gui/gui_factory.h"  // for GuiFactory

namespace {
const QString trPreferences = QObject::tr("Preferences " PROJECT_NAME_TITLE);
const QString trProfileSettings = QObject::tr("Profile settings");
const QString trLogin = QObject::tr("Login:");
const QString trGeneralSettings = QObject::tr("General settings");
const QString trAutoCheckUpd = QObject::tr("Automatically check for updates");
const QString trShowAutoCompletion = QObject::tr("Show autocompletion");
const QString trAutoOpenConsole = QObject::tr("Automatically open console");
const QString trAutoConnectDb = QObject::tr("Automatically connect to db");
const QString trFastViewValues = QObject::tr("Fast view values");
const QString trLanguage = QObject::tr("Language:");
const QString trSupportedUiStyles = QObject::tr("Supported UI styles:");
const QString trSupportedFonts = QObject::tr("Supported fonts:");
const QString trDefaultViews = QObject::tr("Default views:");
const QString trHistoryDirectory = QObject::tr("History directory:");
}  // namespace

namespace fastonosql {
namespace gui {

PreferencesDialog::PreferencesDialog(QWidget* parent) : QDialog(parent) {
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)
#ifndef IS_PUBLIC_BUILD
  profileBox_ = new QGroupBox;
  QHBoxLayout* profileLayout = new QHBoxLayout;
  loginLabel_ = new QLabel;
  loginText_ = new QLineEdit;
  loginText_->setText(USER_SPECIFIC_LOGIN);
  loginText_->setEnabled(false);
  profileLayout->addWidget(loginLabel_);
  profileLayout->addWidget(loginText_);
  profileBox_->setLayout(profileLayout);
#endif

  // ui settings
  generalBox_ = new QGroupBox;
  QGridLayout* generalLayout = new QGridLayout;

  autoCheckUpdates_ = new QCheckBox;
  generalLayout->addWidget(autoCheckUpdates_, 0, 0);
  fastViewKeys_ = new QCheckBox;
  generalLayout->addWidget(fastViewKeys_, 0, 1);

  autoOpenConsole_ = new QCheckBox;
  generalLayout->addWidget(autoOpenConsole_, 1, 0);
  autoComletionEnable_ = new QCheckBox;
  generalLayout->addWidget(autoComletionEnable_, 1, 1);

  autoConnectDB_ = new QCheckBox;
  generalLayout->addWidget(autoConnectDB_, 2, 0);

  stylesLabel_ = new QLabel;
  stylesComboBox_ = new QComboBox;
  stylesComboBox_->addItems(common::qt::gui::supportedStyles());
  generalLayout->addWidget(stylesLabel_, 3, 0);
  generalLayout->addWidget(stylesComboBox_, 3, 1);

  fontLabel_ = new QLabel;
  fontComboBox_ = new QFontComboBox;
  fontComboBox_->setEditable(false);
  fontSizeSpinBox_ = new QSpinBox;
  generalLayout->addWidget(fontLabel_, 4, 0);
  // fontLayout->addWidget(new QSplitter(Qt::Horizontal));
  QHBoxLayout* l = new QHBoxLayout;
  l->addWidget(fontComboBox_);
  l->addWidget(fontSizeSpinBox_);
  generalLayout->addLayout(l, 4, 1);

  langLabel_ = new QLabel;
  generalLayout->addWidget(langLabel_, 5, 0);
  languagesComboBox_ = new QComboBox;
  languagesComboBox_->addItems(common::qt::translations::supportedLanguages());
  generalLayout->addWidget(languagesComboBox_, 5, 1);

  defaultViewLabel_ = new QLabel;
  defaultViewComboBox_ = new QComboBox;
  for (size_t i = 0; i < SIZEOFMASS(proxy::viewsText); ++i) {
    std::string vstr = proxy::viewsText[i];
    QString qstr;
    if (common::ConvertFromString(vstr, &qstr)) {
      defaultViewComboBox_->addItem(qstr, static_cast<int>(i));
    }
  }
  generalLayout->addWidget(defaultViewLabel_, 6, 0);
  generalLayout->addWidget(defaultViewComboBox_, 6, 1);

  logDirPath_ = new QLineEdit;
  logDirLabel_ = new QLabel;
  generalLayout->addWidget(logDirLabel_, 7, 0);
  generalLayout->addWidget(logDirPath_, 7, 1);
  generalBox_->setLayout(generalLayout);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout(this);
#ifndef IS_PUBLIC_BUILD
  layout->addWidget(profileBox_);
#endif
  layout->addWidget(generalBox_);

  QDialogButtonBox* buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  buttonBox->setOrientation(Qt::Horizontal);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::reject));
  layout->addWidget(buttonBox);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(layout);

  syncWithSettings();
  retranslateUi();
}

void PreferencesDialog::accept() {
  proxy::SettingsManager::Instance().SetAutoCheckUpdates(autoCheckUpdates_->isChecked());
  proxy::SettingsManager::Instance().SetAutoCompletion(autoComletionEnable_->isChecked());

  QString newLang = common::qt::translations::applyLanguage(languagesComboBox_->currentText());
  proxy::SettingsManager::Instance().SetCurrentLanguage(newLang);

  common::qt::gui::applyStyle(stylesComboBox_->currentText());
  proxy::SettingsManager::Instance().SetCurrentStyle(stylesComboBox_->currentText());

  QFont cf(fontComboBox_->currentText(), fontSizeSpinBox_->value());
  proxy::SettingsManager::Instance().SetCurrentFont(cf);
  common::qt::gui::applyFont(gui::GuiFactory::Instance().font());

  QVariant var = defaultViewComboBox_->currentData();
  proxy::supportedViews v = static_cast<proxy::supportedViews>(qvariant_cast<unsigned char>(var));
  proxy::SettingsManager::Instance().SetDefaultView(v);

  proxy::SettingsManager::Instance().SetLoggingDirectory(logDirPath_->text());
  proxy::SettingsManager::Instance().SetAutoOpenConsole(autoOpenConsole_->isChecked());
  proxy::SettingsManager::Instance().SetAutoConnectDB(autoConnectDB_->isChecked());
  proxy::SettingsManager::Instance().SetFastViewKeys(fastViewKeys_->isChecked());

  return QDialog::accept();
}

void PreferencesDialog::syncWithSettings() {
  autoCheckUpdates_->setChecked(proxy::SettingsManager::Instance().AutoCheckUpdates());
  autoComletionEnable_->setChecked(proxy::SettingsManager::Instance().AutoCompletion());
  languagesComboBox_->setCurrentText(proxy::SettingsManager::Instance().CurrentLanguage());
  stylesComboBox_->setCurrentText(proxy::SettingsManager::Instance().CurrentStyle());
  QFont cf = proxy::SettingsManager::Instance().CurrentFont();
  fontComboBox_->setCurrentFont(cf);
  fontSizeSpinBox_->setValue(cf.pointSize());
  proxy::supportedViews v = proxy::SettingsManager::Instance().DefaultView();
  std::string vstr = proxy::viewsText[v];
  QString qstr;
  if (common::ConvertFromString(vstr, &qstr)) {
    defaultViewComboBox_->setCurrentText(qstr);
  }
  logDirPath_->setText(proxy::SettingsManager::Instance().LoggingDirectory());
  autoOpenConsole_->setChecked(proxy::SettingsManager::Instance().AutoOpenConsole());
  autoConnectDB_->setChecked(proxy::SettingsManager::Instance().AutoConnectDB());
  fastViewKeys_->setChecked(proxy::SettingsManager::Instance().FastViewKeys());
}

void PreferencesDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QDialog::changeEvent(e);
}

void PreferencesDialog::retranslateUi() {
  setWindowTitle(trPreferences);

  generalBox_->setTitle(trGeneralSettings);
#ifndef IS_PUBLIC_BUILD
  profileBox_->setTitle(trProfileSettings);
  loginLabel_->setText(trLogin);
#endif
  autoCheckUpdates_->setText(trAutoCheckUpd);
  autoComletionEnable_->setText(trShowAutoCompletion);
  autoOpenConsole_->setText(trAutoOpenConsole);
  autoConnectDB_->setText(trAutoConnectDb);
  fastViewKeys_->setText(trFastViewValues);
  langLabel_->setText(trLanguage);
  stylesLabel_->setText(trSupportedUiStyles);
  fontLabel_->setText(trSupportedFonts);
  defaultViewLabel_->setText(trDefaultViews);
  logDirLabel_->setText(trHistoryDirectory);
}

}  // namespace gui
}  // namespace fastonosql
