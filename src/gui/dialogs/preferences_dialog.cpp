#include "gui/dialogs/preferences_dialog.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "fasto/qt/gui/app_style.h"
#include "fasto/qt/translations/translations.h"
#include "common/qt/convert_string.h"

#include "gui/gui_factory.h"

#include "core/settings_manager.h"
#include "core/servers_manager.h"

namespace fastonosql
{
    PreferencesDialog::PreferencesDialog(QWidget* parent)
        : QDialog(parent)
    {
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

//      ui settings
        generalBox_ = new QGroupBox;

        QHBoxLayout *styleswLayout = new QHBoxLayout;
        stylesLabel_ = new QLabel;
        stylesComboBox_ = new QComboBox;
        stylesComboBox_->addItems(fasto::qt::gui::getSupportedStyles());
        styleswLayout->addWidget(stylesLabel_);
        styleswLayout->addWidget(stylesComboBox_);

        QHBoxLayout *langLayout = new QHBoxLayout;
        langLabel_ = new QLabel;
        langLayout->addWidget(langLabel_);
        languagesComboBox_  = new QComboBox;
        languagesComboBox_->addItems(fasto::qt::translations::supportedLanguages());
        langLayout->addWidget(languagesComboBox_);

        QVBoxLayout *generalLayout = new QVBoxLayout;
        autoCheckUpdates_ = new QCheckBox;
        generalLayout->addWidget(autoCheckUpdates_);
        autoComletionEnable_ = new QCheckBox;
        generalLayout->addWidget(autoComletionEnable_);
        autoOpneConsole_ = new QCheckBox;
        generalLayout->addWidget(autoOpneConsole_);
        generalLayout->addLayout(styleswLayout);
        generalLayout->addLayout(langLayout);

        generalBox_->setLayout(generalLayout);

//      servers settings
        serverSettingsBox_ = new QGroupBox;

        QHBoxLayout* defaultViewLayaut = new QHBoxLayout;
        defaultViewLabel_ = new QLabel;
        defaultViewComboBox_ = new QComboBox;
        for(int i = 0; i < SIZEOFMASS(viewsText); ++i){
            defaultViewComboBox_->addItem(common::convertFromString<QString>(viewsText[i]));
        }
        defaultViewLayaut->addWidget(defaultViewLabel_);
        defaultViewLayaut->addWidget(defaultViewComboBox_);

        syncTabs_ = new QCheckBox;
        logDirPath_ = new QLineEdit;
        QHBoxLayout *logLayout = new QHBoxLayout;
        logDirLabel_ = new QLabel;
        logLayout->addWidget(logDirLabel_);
        logLayout->addWidget(logDirPath_);

        QVBoxLayout *serverSettingsLayout = new QVBoxLayout;
        serverSettingsLayout->addLayout(defaultViewLayaut);
        serverSettingsLayout->addWidget(syncTabs_);
        serverSettingsLayout->addLayout(logLayout);
        serverSettingsBox_->setLayout(serverSettingsLayout);

//      main layout
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(generalBox_);
        layout->addWidget(serverSettingsBox_);

        QDialogButtonBox *buttonBox = new QDialogButtonBox;
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

    void PreferencesDialog::accept()
    {
        SettingsManager::instance().setAutoCheckUpdates(autoCheckUpdates_->isChecked());
        SettingsManager::instance().setAutoCompletion(autoComletionEnable_->isChecked());

        QString newLang = fasto::qt::translations::applyLanguage(languagesComboBox_->currentText());
        SettingsManager::instance().setCurrentLanguage(newLang);

        fasto::qt::gui::applyStyle(stylesComboBox_->currentText());
        SettingsManager::instance().setCurrentStyle(stylesComboBox_->currentText());

        const std::string defCombo = common::convertToString(defaultViewComboBox_->currentText());
        const fastonosql::supportedViews v = common::convertFromString<fastonosql::supportedViews>(defCombo);
        SettingsManager::instance().setDefaultView(v);

        ServersManager::instance().setSyncServers(syncTabs_->isChecked());
        SettingsManager::instance().setSyncTabs(syncTabs_->isChecked());
        SettingsManager::instance().setLoggingDirectory(logDirPath_->text());
        SettingsManager::instance().setAutoOpenConsole(autoOpneConsole_->isChecked());

        return QDialog::accept();
    }

    void PreferencesDialog::syncWithSettings()
    {
        autoCheckUpdates_->setChecked(SettingsManager::instance().autoCheckUpdates());
        autoComletionEnable_->setChecked(SettingsManager::instance().autoCompletion());
        languagesComboBox_->setCurrentText(SettingsManager::instance().currentLanguage());
        stylesComboBox_->setCurrentText(SettingsManager::instance().currentStyle());
        defaultViewComboBox_->setCurrentText(common::convertFromString<QString>(common::convertToString(SettingsManager::instance().defaultView())));
        syncTabs_->setChecked(SettingsManager::instance().syncTabs());
        logDirPath_->setText(SettingsManager::instance().loggingDirectory());
        autoOpneConsole_->setChecked(SettingsManager::instance().autoOpenConsole());
    }

    void PreferencesDialog::changeEvent(QEvent* e)
    {
        if(e->type() == QEvent::LanguageChange){
            retranslateUi();
        }
        QDialog::changeEvent(e);
    }

    void PreferencesDialog::retranslateUi()
    {
        setWindowTitle(tr("Preferences " PROJECT_NAME_TITLE));

        generalBox_->setTitle(tr("General settings"));
        autoCheckUpdates_->setText(tr("Automatically check for updates"));
        autoComletionEnable_->setText(tr("Show autocompletion"));
        autoOpneConsole_->setText(tr("Automatically open console"));
        langLabel_->setText(tr("Language:"));
        stylesLabel_->setText(tr("Supported UI styles:"));

        serverSettingsBox_->setTitle(tr("Servers global settings"));
        defaultViewLabel_->setText(tr("Default views:"));
        syncTabs_->setText(tr("Sync tabs"));
        logDirLabel_->setText(tr("Logging directory:"));
    }
}
