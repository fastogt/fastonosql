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
#include <QLineEdit>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for VERIFY, SIZEOFMASS

#include <common/qt/convert2string.h>             // for ConvertToString
#include <common/qt/gui/app_style.h>              // for applyFont, applyStyle, etc
#include <common/qt/translations/translations.h>  // for applyLanguage, etc

#include "core/settings_manager.h"  // for SettingsManager

#include "global/types.h"  // for viewsText, ConvertToString, etc

#include "gui/gui_factory.h"  // for GuiFactory

namespace {
const QString trPreferences = QObject::tr("Preferences " PROJECT_NAME_TITLE);
const QString trProfileSettings = QObject::tr("Profile settings");
const QString trLogin = QObject::tr("Login:");
const QString trGeneralSettings = QObject::tr("General settings");
const QString trAutoCheckUpd = QObject::tr("Automatically check for updates");
const QString trShowAutoCompletion = QObject::tr("Show autocompletion");
const QString trAutoOpenConsole = QObject::tr("Automatically open console");
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

  QHBoxLayout* styleswLayout = new QHBoxLayout;
  stylesLabel_ = new QLabel;
  stylesComboBox_ = new QComboBox;
  stylesComboBox_->addItems(common::qt::gui::supportedStyles());
  styleswLayout->addWidget(stylesLabel_);
  styleswLayout->addWidget(stylesComboBox_);

  QHBoxLayout* fontLayout = new QHBoxLayout;
  fontLabel_ = new QLabel;
  fontComboBox_ = new QFontComboBox;
  fontComboBox_->setEditable(false);
  fontLayout->addWidget(fontLabel_);
  fontLayout->addWidget(fontComboBox_);

  QHBoxLayout* langLayout = new QHBoxLayout;
  langLabel_ = new QLabel;
  langLayout->addWidget(langLabel_);
  languagesComboBox_ = new QComboBox;
  languagesComboBox_->addItems(common::qt::translations::supportedLanguages());
  langLayout->addWidget(languagesComboBox_);

  QVBoxLayout* generalLayout = new QVBoxLayout;
  autoCheckUpdates_ = new QCheckBox;
  generalLayout->addWidget(autoCheckUpdates_);
  autoComletionEnable_ = new QCheckBox;
  generalLayout->addWidget(autoComletionEnable_);
  autoOpenConsole_ = new QCheckBox;
  generalLayout->addWidget(autoOpenConsole_);
  fastViewKeys_ = new QCheckBox;
  generalLayout->addWidget(fastViewKeys_);
  generalLayout->addLayout(styleswLayout);
  generalLayout->addLayout(fontLayout);
  generalLayout->addLayout(langLayout);

  generalBox_->setLayout(generalLayout);

  QHBoxLayout* defaultViewLayaut = new QHBoxLayout;
  defaultViewLabel_ = new QLabel;
  defaultViewComboBox_ = new QComboBox;
  for (size_t i = 0; i < SIZEOFMASS(viewsText); ++i) {
    std::string vstr = viewsText[i];
    supportedViews sv = common::ConvertFromString<supportedViews>(vstr);
    defaultViewComboBox_->addItem(common::ConvertFromString<QString>(vstr), sv);
  }
  defaultViewLayaut->addWidget(defaultViewLabel_);
  defaultViewLayaut->addWidget(defaultViewComboBox_);

  logDirPath_ = new QLineEdit;
  QHBoxLayout* logLayout = new QHBoxLayout;
  logDirLabel_ = new QLabel;
  logLayout->addWidget(logDirLabel_);
  logLayout->addWidget(logDirPath_);

  generalLayout->addLayout(defaultViewLayaut);
  generalLayout->addLayout(logLayout);

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
  core::SettingsManager::instance().SetAutoCheckUpdates(autoCheckUpdates_->isChecked());
  core::SettingsManager::instance().SetAutoCompletion(autoComletionEnable_->isChecked());

  QString newLang = common::qt::translations::applyLanguage(languagesComboBox_->currentText());
  core::SettingsManager::instance().SetCurrentLanguage(newLang);

  common::qt::gui::applyStyle(stylesComboBox_->currentText());
  core::SettingsManager::instance().SetCurrentStyle(stylesComboBox_->currentText());

  core::SettingsManager::instance().SetCurrentFontName(fontComboBox_->currentText());
  common::qt::gui::applyFont(gui::GuiFactory::instance().font());

  QVariant var = defaultViewComboBox_->currentData();
  supportedViews v = static_cast<supportedViews>(qvariant_cast<unsigned char>(var));
  core::SettingsManager::instance().SetDefaultView(v);

  core::SettingsManager::instance().SetLoggingDirectory(logDirPath_->text());
  core::SettingsManager::instance().SetAutoOpenConsole(autoOpenConsole_->isChecked());
  core::SettingsManager::instance().SetFastViewKeys(fastViewKeys_->isChecked());

  return QDialog::accept();
}

void PreferencesDialog::syncWithSettings() {
  autoCheckUpdates_->setChecked(core::SettingsManager::instance().AutoCheckUpdates());
  autoComletionEnable_->setChecked(core::SettingsManager::instance().AutoCompletion());
  languagesComboBox_->setCurrentText(core::SettingsManager::instance().CurrentLanguage());
  stylesComboBox_->setCurrentText(core::SettingsManager::instance().CurrentStyle());
  fontComboBox_->setCurrentText(core::SettingsManager::instance().CurrentFontName());
  supportedViews v = core::SettingsManager::instance().DefaultView();
  std::string vstr = viewsText[v];
  defaultViewComboBox_->setCurrentText(common::ConvertFromString<QString>(vstr));
  logDirPath_->setText(core::SettingsManager::instance().LoggingDirectory());
  autoOpenConsole_->setChecked(core::SettingsManager::instance().AutoOpenConsole());
  fastViewKeys_->setChecked(core::SettingsManager::instance().FastViewKeys());
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
  fastViewKeys_->setText(trFastViewValues);
  langLabel_->setText(trLanguage);
  stylesLabel_->setText(trSupportedUiStyles);
  fontLabel_->setText(trSupportedFonts);
  defaultViewLabel_->setText(trDefaultViews);
  logDirLabel_->setText(trHistoryDirectory);
}

}  // namespace gui
}  // namespace fastonosql
