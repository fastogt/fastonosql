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

#include <string>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFontComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>

#include "common/qt/convert_string.h"

#include "fasto/qt/gui/app_style.h"
#include "fasto/qt/translations/translations.h"

#include "core/settings_manager.h"
#include "core/servers_manager.h"

#include "gui/gui_factory.h"

namespace {
  const QString trPreferences = QObject::tr("Preferences " PROJECT_NAME_TITLE);
  const QString trGeneralSettings = QObject::tr("General settings");
  const QString trAutoCheckUpd = QObject::tr("Automatically check for updates");
  const QString trShowAutoCompletion = QObject::tr("Show autocompletion");
  const QString trAutoOpenConsole = QObject::tr("Automatically open console");
  const QString trFastViewValues = QObject::tr("Fast view values");
  const QString trLanguage = QObject::tr("Language:");
  const QString trSupportedUiStyles = QObject::tr("Supported UI styles:");
  const QString trSupportedFonts = QObject::tr("Supported fonts:");
  const QString trServerGlobalSettings = QObject::tr("Servers global settings");
  const QString trDefaultViews = QObject::tr("Default views:");
  const QString trLoggingDirectory = QObject::tr("Logging directory:");
}  //

namespace fastonosql {
namespace gui {

PreferencesDialog::PreferencesDialog(QWidget* parent)
  : QDialog(parent) {
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

//      ui settings
  generalBox_ = new QGroupBox;

  QHBoxLayout* styleswLayout = new QHBoxLayout;
  stylesLabel_ = new QLabel;
  stylesComboBox_ = new QComboBox;
  stylesComboBox_->addItems(fasto::qt::gui::supportedStyles());
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
  languagesComboBox_  = new QComboBox;
  languagesComboBox_->addItems(fasto::qt::translations::supportedLanguages());
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

  // servers settings
  serverSettingsBox_ = new QGroupBox;

  QHBoxLayout* defaultViewLayaut = new QHBoxLayout;
  defaultViewLabel_ = new QLabel;
  defaultViewComboBox_ = new QComboBox;
  for (size_t i = 0; i < SIZEOFMASS(viewsText); ++i) {
    defaultViewComboBox_->addItem(common::ConvertFromString<QString>(viewsText[i]));
  }
  defaultViewLayaut->addWidget(defaultViewLabel_);
  defaultViewLayaut->addWidget(defaultViewComboBox_);

  logDirPath_ = new QLineEdit;
  QHBoxLayout* logLayout = new QHBoxLayout;
  logDirLabel_ = new QLabel;
  logLayout->addWidget(logDirLabel_);
  logLayout->addWidget(logDirPath_);

  QVBoxLayout* serverSettingsLayout = new QVBoxLayout;
  serverSettingsLayout->addLayout(defaultViewLayaut);
  serverSettingsLayout->addLayout(logLayout);
  serverSettingsBox_->setLayout(serverSettingsLayout);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(generalBox_);
  layout->addWidget(serverSettingsBox_);

  QDialogButtonBox* buttonBox = new QDialogButtonBox;
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::accept));
  VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::reject));
  layout->addWidget(buttonBox);
  setMinimumSize(QSize(min_width, min_height));
  setLayout(layout);

  syncWithSettings();
  retranslateUi();
}

void PreferencesDialog::accept() {
  core::SettingsManager::instance().setAutoCheckUpdates(autoCheckUpdates_->isChecked());
  core::SettingsManager::instance().setAutoCompletion(autoComletionEnable_->isChecked());

  QString newLang = fasto::qt::translations::applyLanguage(languagesComboBox_->currentText());
  core::SettingsManager::instance().setCurrentLanguage(newLang);

  fasto::qt::gui::applyStyle(stylesComboBox_->currentText());
  core::SettingsManager::instance().setCurrentStyle(stylesComboBox_->currentText());

  core::SettingsManager::instance().setCurrentFontName(fontComboBox_->currentText());
  fasto::qt::gui::applyFont(gui::GuiFactory::instance().font());

  std::string defCombo = common::ConvertToString(defaultViewComboBox_->currentText());
  supportedViews v = common::ConvertFromString<supportedViews>(defCombo);
  core::SettingsManager::instance().setDefaultView(v);

  core::SettingsManager::instance().setLoggingDirectory(logDirPath_->text());
  core::SettingsManager::instance().setAutoOpenConsole(autoOpenConsole_->isChecked());
  core::SettingsManager::instance().setFastViewKeys(fastViewKeys_->isChecked());

  return QDialog::accept();
}

void PreferencesDialog::syncWithSettings() {
  autoCheckUpdates_->setChecked(core::SettingsManager::instance().autoCheckUpdates());
  autoComletionEnable_->setChecked(core::SettingsManager::instance().autoCompletion());
  languagesComboBox_->setCurrentText(core::SettingsManager::instance().currentLanguage());
  stylesComboBox_->setCurrentText(core::SettingsManager::instance().currentStyle());
  fontComboBox_->setCurrentText(core::SettingsManager::instance().currentFontName());
  std::string defaultViewText = common::ConvertToString(core::SettingsManager::instance().defaultView());
  defaultViewComboBox_->setCurrentText(common::ConvertFromString<QString>(defaultViewText));
  logDirPath_->setText(core::SettingsManager::instance().loggingDirectory());
  autoOpenConsole_->setChecked(core::SettingsManager::instance().autoOpenConsole());
  fastViewKeys_->setChecked(core::SettingsManager::instance().fastViewKeys());
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
  autoCheckUpdates_->setText(trAutoCheckUpd);
  autoComletionEnable_->setText(trShowAutoCompletion);
  autoOpenConsole_->setText(trAutoOpenConsole);
  fastViewKeys_->setText(trFastViewValues);
  langLabel_->setText(trLanguage);
  stylesLabel_->setText(trSupportedUiStyles);
  fontLabel_->setText(trSupportedFonts);

  serverSettingsBox_->setTitle(trServerGlobalSettings);
  defaultViewLabel_->setText(trDefaultViews);
  logDirLabel_->setText(trLoggingDirectory);
}

}  // namespace gui
}  // namespace fastonosql
