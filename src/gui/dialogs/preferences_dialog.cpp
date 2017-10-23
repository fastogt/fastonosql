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

#include "gui/dialogs/preferences_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFontComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include <common/qt/convert2string.h>             // for ConvertToString
#include <common/qt/gui/app_style.h>              // for applyFont, applyStyle, etc
#include <common/qt/translations/translations.h>  // for applyLanguage, etc

#include "proxy/settings_manager.h"  // for SettingsManager

#include "gui/gui_factory.h"  // for GuiFactory

namespace {
const QString trPreferences = QObject::tr(PROJECT_NAME_TITLE " preferences");
const QString trProfileSettings = QObject::tr("Profile settings");
const QString trLogin = QObject::tr("Login:");
const QString trGeneralSettings = QObject::tr("General settings");
const QString trSendStatistic = QObject::tr("Send statistic");
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
  profile_box_ = new QGroupBox;
  QHBoxLayout* profile_layout = new QHBoxLayout;
  login_label_ = new QLabel;
  login_text_ = new QLineEdit;
  login_text_->setText(USER_SPECIFIC_LOGIN);
  login_text_->setEnabled(false);
  profile_layout->addWidget(login_label_);
  profile_layout->addWidget(login_text_);
  profile_box_->setLayout(profile_layout);
#endif

  // ui settings
  general_box_ = new QGroupBox;
  QGridLayout* generalLayout = new QGridLayout;

  send_statitsic_ = new QCheckBox;
  generalLayout->addWidget(send_statitsic_, 0, 0);
  auto_check_updates_ = new QCheckBox;
  generalLayout->addWidget(auto_check_updates_, 0, 1);

  auto_open_console_ = new QCheckBox;
  generalLayout->addWidget(auto_open_console_, 1, 0);
  auto_comletion_ = new QCheckBox;
  generalLayout->addWidget(auto_comletion_, 1, 1);

  fast_view_keys_ = new QCheckBox;
  generalLayout->addWidget(fast_view_keys_, 2, 0);
  auto_connect_db_ = new QCheckBox;
  generalLayout->addWidget(auto_connect_db_, 2, 1);

  styles_label_ = new QLabel;
  styles_combo_box_ = new QComboBox;
  styles_combo_box_->addItems(common::qt::gui::supportedStyles());
  generalLayout->addWidget(styles_label_, 3, 0);
  generalLayout->addWidget(styles_combo_box_, 3, 1);

  font_label_ = new QLabel;
  font_combo_box_ = new QFontComboBox;
  font_combo_box_->setEditable(false);
  font_size_spin_box_ = new QSpinBox;
  generalLayout->addWidget(font_label_, 4, 0);
  // fontLayout->addWidget(new QSplitter(Qt::Horizontal));
  QHBoxLayout* l = new QHBoxLayout;
  l->addWidget(font_combo_box_);
  l->addWidget(font_size_spin_box_);
  generalLayout->addLayout(l, 4, 1);

  languages_label_ = new QLabel;
  generalLayout->addWidget(languages_label_, 5, 0);
  languages_combo_box_ = new QComboBox;
  languages_combo_box_->addItems(common::qt::translations::supportedLanguages());
  generalLayout->addWidget(languages_combo_box_, 5, 1);

  default_view_label_ = new QLabel;
  default_view_combo_box_ = new QComboBox;
  for (uint32_t i = 0; i < proxy::supported_views_text.size(); ++i) {
    default_view_combo_box_->addItem(proxy::supported_views_text[i], i);
  }
  generalLayout->addWidget(default_view_label_, 6, 0);
  generalLayout->addWidget(default_view_combo_box_, 6, 1);

  log_dir_path_ = new QLineEdit;
  log_dir_label_ = new QLabel;
  generalLayout->addWidget(log_dir_label_, 7, 0);
  generalLayout->addWidget(log_dir_path_, 7, 1);
  general_box_->setLayout(generalLayout);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout(this);
#ifndef IS_PUBLIC_BUILD
  layout->addWidget(profile_box_);
#endif
  layout->addWidget(general_box_);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
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
  proxy::SettingsManager::GetInstance()->SetSendStatistic(send_statitsic_->isChecked());
  proxy::SettingsManager::GetInstance()->SetAutoCheckUpdates(auto_check_updates_->isChecked());
  proxy::SettingsManager::GetInstance()->SetAutoCompletion(auto_comletion_->isChecked());

  QString newLang = common::qt::translations::applyLanguage(languages_combo_box_->currentText());
  proxy::SettingsManager::GetInstance()->SetCurrentLanguage(newLang);

  common::qt::gui::applyStyle(styles_combo_box_->currentText());
  proxy::SettingsManager::GetInstance()->SetCurrentStyle(styles_combo_box_->currentText());

  QFont cf(font_combo_box_->currentText(), font_size_spin_box_->value());
  proxy::SettingsManager::GetInstance()->SetCurrentFont(cf);
  common::qt::gui::applyFont(gui::GuiFactory::GetInstance().font());

  QVariant var = default_view_combo_box_->currentData();
  proxy::supportedViews v = static_cast<proxy::supportedViews>(qvariant_cast<unsigned char>(var));
  proxy::SettingsManager::GetInstance()->SetDefaultView(v);

  proxy::SettingsManager::GetInstance()->SetLoggingDirectory(log_dir_path_->text());
  proxy::SettingsManager::GetInstance()->SetAutoOpenConsole(auto_open_console_->isChecked());
  proxy::SettingsManager::GetInstance()->SetAutoConnectDB(auto_connect_db_->isChecked());
  proxy::SettingsManager::GetInstance()->SetFastViewKeys(fast_view_keys_->isChecked());

  return QDialog::accept();
}

void PreferencesDialog::syncWithSettings() {
  send_statitsic_->setChecked(proxy::SettingsManager::GetInstance()->GetSendStatistic());
  auto_check_updates_->setChecked(proxy::SettingsManager::GetInstance()->GetAutoCheckUpdates());
  auto_comletion_->setChecked(proxy::SettingsManager::GetInstance()->GetAutoCompletion());
  languages_combo_box_->setCurrentText(proxy::SettingsManager::GetInstance()->GetCurrentLanguage());
  styles_combo_box_->setCurrentText(proxy::SettingsManager::GetInstance()->GetCurrentStyle());
  QFont cf = proxy::SettingsManager::GetInstance()->GetCurrentFont();
  font_combo_box_->setCurrentFont(cf);
  font_size_spin_box_->setValue(cf.pointSize());
  proxy::supportedViews v = proxy::SettingsManager::GetInstance()->GetDefaultView();
  QString qstr = proxy::supported_views_text[v];
  default_view_combo_box_->setCurrentText(qstr);
  log_dir_path_->setText(proxy::SettingsManager::GetInstance()->GetLoggingDirectory());
  auto_open_console_->setChecked(proxy::SettingsManager::GetInstance()->AutoOpenConsole());
  auto_connect_db_->setChecked(proxy::SettingsManager::GetInstance()->GetAutoConnectDB());
  fast_view_keys_->setChecked(proxy::SettingsManager::GetInstance()->GetFastViewKeys());
}

void PreferencesDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }

  QDialog::changeEvent(e);
}

void PreferencesDialog::retranslateUi() {
  setWindowTitle(trPreferences);

  general_box_->setTitle(trGeneralSettings);
#ifndef IS_PUBLIC_BUILD
  profile_box_->setTitle(trProfileSettings);
  login_label_->setText(trLogin);
#endif
  send_statitsic_->setText(trSendStatistic);
  auto_check_updates_->setText(trAutoCheckUpd);
  auto_comletion_->setText(trShowAutoCompletion);
  auto_open_console_->setText(trAutoOpenConsole);
  auto_connect_db_->setText(trAutoConnectDb);
  fast_view_keys_->setText(trFastViewValues);
  languages_label_->setText(trLanguage);
  styles_label_->setText(trSupportedUiStyles);
  font_label_->setText(trSupportedFonts);
  default_view_label_->setText(trDefaultViews);
  log_dir_label_->setText(trHistoryDirectory);
}

}  // namespace gui
}  // namespace fastonosql
