/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/preferences_dialog.h"

#include <string>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include <common/qt/convert2string.h>
#include <common/qt/gui/app_style.h>
#include <common/qt/translations/translations.h>

#include "proxy/settings_manager.h"

#include "gui/widgets/path_widget.h"

#include "translations/global.h"

namespace {
const QString trProfileSettings = QObject::tr("Profile settings");
const QString trGeneralSettings = QObject::tr("General settings");
const QString trSendStatistic = QObject::tr("Send statistic");
const QString trAutoCheckUpd = QObject::tr("Automatically check for updates");
const QString trShowAutoCompletion = QObject::tr("Show autocompletion");
const QString trAutoOpenConsole = QObject::tr("Automatically open console");
const QString trAutoConnectDb = QObject::tr("Automatically connect to DB");
const QString trShowWelcomePage = QObject::tr("Show welcome page");
const QString trLanguage = QObject::tr("Language");
const QString trUiStyle = QObject::tr("UI style");
const QString trFont = QObject::tr("Font");
const QString trDefaultView = QObject::tr("Default view");
const QString trHistoryDirectory = QObject::tr("History directory");
const QString trGeneral = QObject::tr("General");
const QString trExternal = QObject::tr("External");

const QString trSelectPythonPath = QObject::tr("Select python executable");
const QString trPythonExecutable = QObject::tr("Python executable (*)");
const QString trPythonPath = QObject::tr("Python path:");

const QString trSelectModulesPath = QObject::tr("Select modules path");
const QString trModulesPath = QObject::tr("Modules path:");

const QString trPreferences = QObject::tr(PROJECT_NAME_TITLE " preferences");
}  // namespace

namespace fastonosql {
namespace gui {

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : base_class(trPreferences, parent),
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
      profile_box_(nullptr),
      first_name_label_(nullptr),
      first_name_text_(nullptr),
      last_name_label_(nullptr),
      last_name_text_(nullptr),
      login_label_(nullptr),
      login_text_(nullptr),
      type_label_(nullptr),
      type_text_(nullptr),
#endif
      general_box_(nullptr),
      send_statitsic_(nullptr),
      auto_check_updates_(nullptr),
      auto_comletion_(nullptr),
      languages_label_(nullptr),
      languages_combo_box_(nullptr),
      styles_label_(nullptr),
      styles_combo_box_(nullptr),
      font_label_(nullptr),
      font_combo_box_(nullptr),
      font_size_spin_box_(nullptr),
      default_view_label_(nullptr),
      default_view_combo_box_(nullptr),
      log_dir_label_(nullptr),
      log_dir_path_(nullptr),
      auto_open_console_(nullptr),
      auto_connect_db_(nullptr),
      show_welcome_page_(nullptr),
      external_box_(nullptr),
      python_path_widget_(nullptr),
      modules_path_widget_(nullptr) {
  QTabWidget* preferences_tabs = new QTabWidget;
  QWidget* general_tab = createMainTab();
  QWidget* external_tab = createExternalTab();
  preferences_tabs->addTab(general_tab, trGeneral);
  preferences_tabs->addTab(external_tab, trExternal);

  QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
  button_box->setOrientation(Qt::Horizontal);
  VERIFY(connect(button_box, &QDialogButtonBox::accepted, this, &PreferencesDialog::accept));
  VERIFY(connect(button_box, &QDialogButtonBox::rejected, this, &PreferencesDialog::reject));

  QVBoxLayout* main_layout = new QVBoxLayout;
  main_layout->addWidget(preferences_tabs);
  main_layout->addWidget(button_box);
  setLayout(main_layout);
  setMinimumSize(QSize(min_width, min_height));

  syncWithSettings();
}

void PreferencesDialog::accept() {
  proxy::SettingsManager::GetInstance()->SetSendStatistic(send_statitsic_->isChecked());
  proxy::SettingsManager::GetInstance()->SetAutoCheckUpdates(auto_check_updates_->isChecked());
  proxy::SettingsManager::GetInstance()->SetAutoCompletion(auto_comletion_->isChecked());

  QString new_lang = common::qt::translations::applyLanguage(languages_combo_box_->currentText());
  proxy::SettingsManager::GetInstance()->SetCurrentLanguage(new_lang);

  common::qt::gui::applyStyle(styles_combo_box_->currentText());
  proxy::SettingsManager::GetInstance()->SetCurrentStyle(styles_combo_box_->currentText());

  const QFont current_font(font_combo_box_->currentText(), font_size_spin_box_->value());
  proxy::SettingsManager::GetInstance()->SetCurrentFont(current_font);
  common::qt::gui::applyFont(current_font);

  const QVariant view_var = default_view_combo_box_->currentData();
  proxy::SupportedView view = static_cast<proxy::SupportedView>(qvariant_cast<unsigned char>(view_var));
  proxy::SettingsManager::GetInstance()->SetDefaultView(view);

  proxy::SettingsManager::GetInstance()->SetLoggingDirectory(log_dir_path_->text());
  proxy::SettingsManager::GetInstance()->SetAutoOpenConsole(auto_open_console_->isChecked());
  proxy::SettingsManager::GetInstance()->SetAutoConnectDB(auto_connect_db_->isChecked());
  proxy::SettingsManager::GetInstance()->SetShowWelcomePage(show_welcome_page_->isChecked());
  proxy::SettingsManager::GetInstance()->SetPythonPath(python_path_widget_->path());

  return base_class::accept();
}

void PreferencesDialog::syncWithSettings() {
  send_statitsic_->setChecked(proxy::SettingsManager::GetInstance()->GetSendStatistic());
  auto_check_updates_->setChecked(proxy::SettingsManager::GetInstance()->GetAutoCheckUpdates());
  auto_comletion_->setChecked(proxy::SettingsManager::GetInstance()->GetAutoCompletion());
  languages_combo_box_->setCurrentText(proxy::SettingsManager::GetInstance()->GetCurrentLanguage());
  styles_combo_box_->setCurrentText(proxy::SettingsManager::GetInstance()->GetCurrentStyle());
  QFont current_font = proxy::SettingsManager::GetInstance()->GetCurrentFont();
  font_combo_box_->setCurrentFont(current_font);
  font_size_spin_box_->setValue(current_font.pointSize());
  proxy::SupportedView v = proxy::SettingsManager::GetInstance()->GetDefaultView();
  QString view_text = proxy::g_supported_views_text[v];
  default_view_combo_box_->setCurrentText(view_text);
  log_dir_path_->setText(proxy::SettingsManager::GetInstance()->GetLoggingDirectory());
  auto_open_console_->setChecked(proxy::SettingsManager::GetInstance()->AutoOpenConsole());
  auto_connect_db_->setChecked(proxy::SettingsManager::GetInstance()->GetAutoConnectDB());
  show_welcome_page_->setChecked(proxy::SettingsManager::GetInstance()->GetShowWelcomePage());
  QString python_path = proxy::SettingsManager::GetInstance()->GetPythonPath();
  python_path_widget_->setPath(python_path);

  QString modules_path;
  common::ConvertFromString(proxy::SettingsManager::GetInstance()->GetModulesPath(), &modules_path);
  modules_path_widget_->setPath(modules_path);
}

QWidget* PreferencesDialog::createMainTab() {
  QWidget* main = new QWidget;
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  proxy::UserInfo uinfo = proxy::SettingsManager::GetInstance()->GetUserInfo();
  profile_box_ = new QGroupBox;
  QVBoxLayout* profile_layout = new QVBoxLayout;
  QHBoxLayout* first_last_name_layout = new QHBoxLayout;
  QHBoxLayout* login_type_layout = new QHBoxLayout;

  QHBoxLayout* first_name_layout = new QHBoxLayout;
  first_name_label_ = new QLabel;
  first_name_text_ = new QLineEdit;
  QString qfirst_name;
  common::ConvertFromString(uinfo.GetFirstName(), &qfirst_name);
  first_name_text_->setText(qfirst_name);
  first_name_text_->setEnabled(false);
  first_name_layout->addWidget(first_name_label_);
  first_name_layout->addWidget(first_name_text_);
  first_last_name_layout->addLayout(first_name_layout);

  QHBoxLayout* last_name_layout = new QHBoxLayout;
  last_name_label_ = new QLabel;
  last_name_text_ = new QLineEdit;
  QString qlast_name;
  common::ConvertFromString(uinfo.GetLastName(), &qlast_name);
  last_name_text_->setText(qlast_name);
  last_name_text_->setEnabled(false);
  last_name_layout->addWidget(last_name_label_);
  last_name_layout->addWidget(last_name_text_);
  first_last_name_layout->addLayout(last_name_layout);

  QHBoxLayout* login_layout = new QHBoxLayout;
  login_label_ = new QLabel;
  login_text_ = new QLineEdit;
  QString qlogin;
  common::ConvertFromString(uinfo.GetLogin(), &qlogin);
  login_text_->setText(qlogin);
  login_text_->setEnabled(false);
  login_layout->addWidget(login_label_);
  login_layout->addWidget(login_text_);
  login_type_layout->addLayout(login_layout);

  QHBoxLayout* type_layout = new QHBoxLayout;
  type_label_ = new QLabel;
  type_text_ = new QLineEdit;

  std::string stype = common::ConvertToString(uinfo.GetType());
  QString qtype;
  common::ConvertFromString(stype, &qtype);
  type_text_->setText(qtype);
  type_text_->setEnabled(false);
  type_layout->addWidget(type_label_);
  type_layout->addWidget(type_text_);
  login_type_layout->addLayout(type_layout);

  profile_layout->addLayout(first_last_name_layout);
  profile_layout->addLayout(login_type_layout);
  profile_box_->setLayout(profile_layout);
#endif

  // ui settings
  general_box_ = new QGroupBox;
  QGridLayout* general_layout = new QGridLayout;

  send_statitsic_ = new QCheckBox;
  general_layout->addWidget(send_statitsic_, 0, 0);
  auto_check_updates_ = new QCheckBox;
  general_layout->addWidget(auto_check_updates_, 0, 1);

  auto_open_console_ = new QCheckBox;
  general_layout->addWidget(auto_open_console_, 1, 0);
  auto_comletion_ = new QCheckBox;
  general_layout->addWidget(auto_comletion_, 1, 1);

  show_welcome_page_ = new QCheckBox;
#if defined(COMMUNITY_VERSION)
  show_welcome_page_->setEnabled(false);
#endif

  general_layout->addWidget(show_welcome_page_, 2, 0);
  auto_connect_db_ = new QCheckBox;
  general_layout->addWidget(auto_connect_db_, 2, 1);

  styles_label_ = new QLabel;
  styles_combo_box_ = new QComboBox;
  styles_combo_box_->addItems(common::qt::gui::supportedStyles());
  general_layout->addWidget(styles_label_, 3, 0);
  general_layout->addWidget(styles_combo_box_, 3, 1);

  font_label_ = new QLabel;
  font_combo_box_ = new QFontComboBox;
  font_combo_box_->setEditable(false);
  font_size_spin_box_ = new QSpinBox;
  general_layout->addWidget(font_label_, 4, 0);
  // fontLayout->addWidget(new QSplitter(Qt::Horizontal));
  QHBoxLayout* l = new QHBoxLayout;
  l->addWidget(font_combo_box_);
  l->addWidget(font_size_spin_box_);
  general_layout->addLayout(l, 4, 1);

  languages_label_ = new QLabel;
  general_layout->addWidget(languages_label_, 5, 0);
  languages_combo_box_ = new QComboBox;
  languages_combo_box_->addItems(common::qt::translations::supportedLanguages());
  general_layout->addWidget(languages_combo_box_, 5, 1);

  default_view_label_ = new QLabel;
  default_view_combo_box_ = new QComboBox;
  for (uint32_t i = 0; i < proxy::g_supported_views_text.size(); ++i) {
    default_view_combo_box_->addItem(proxy::g_supported_views_text[i], i);
  }
  general_layout->addWidget(default_view_label_, 6, 0);
  general_layout->addWidget(default_view_combo_box_, 6, 1);

  log_dir_path_ = new QLineEdit;
  log_dir_label_ = new QLabel;
  general_layout->addWidget(log_dir_label_, 7, 0);
  general_layout->addWidget(log_dir_path_, 7, 1);
  general_box_->setLayout(general_layout);

  // main layout
  QVBoxLayout* layout = new QVBoxLayout;
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  layout->addWidget(profile_box_);
#endif
  layout->addWidget(general_box_);
  main->setLayout(layout);
  return main;
}

QWidget* PreferencesDialog::createExternalTab() {
  QWidget* external = new QWidget;

  external_box_ = new QGroupBox;
  QVBoxLayout* external_layout = new QVBoxLayout;
  python_path_widget_ = createWidget<FilePathWidget>(trPythonPath, trPythonExecutable, trSelectPythonPath);
  external_layout->addWidget(python_path_widget_);
  modules_path_widget_ = createWidget<DirectoryPathWidget>(trModulesPath, trSelectModulesPath);
  modules_path_widget_->setEnabled(false);
  external_layout->addWidget(modules_path_widget_);
  external_box_->setLayout(external_layout);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(external_box_);
  external->setLayout(layout);
  return external;
}

void PreferencesDialog::retranslateUi() {
  general_box_->setTitle(trGeneralSettings);
#if defined(PRO_VERSION) || defined(ENTERPRISE_VERSION)
  profile_box_->setTitle(trProfileSettings);
  first_name_label_->setText(translations::trFirstName + ":");
  last_name_label_->setText(translations::trLastName + ":");
  login_label_->setText(translations::trLogin + ":");
  type_label_->setText(translations::trType + ":");
#endif

  send_statitsic_->setText(trSendStatistic);
  auto_check_updates_->setText(trAutoCheckUpd);
  auto_comletion_->setText(trShowAutoCompletion);
  auto_open_console_->setText(trAutoOpenConsole);
  auto_connect_db_->setText(trAutoConnectDb);
  show_welcome_page_->setText(trShowWelcomePage);
  languages_label_->setText(trLanguage + ":");
  styles_label_->setText(trUiStyle + ":");
  font_label_->setText(trFont + ":");
  default_view_label_->setText(trDefaultView + ":");
  log_dir_label_->setText(trHistoryDirectory + ":");
  base_class::retranslateUi();
}

}  // namespace gui
}  // namespace fastonosql
