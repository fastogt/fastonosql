#pragma  once

#include <QDialog>

class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QGroupBox;

namespace fastoredis
{
    class PreferencesDialog 
		: public QDialog
    {
        Q_OBJECT

    public:
        explicit PreferencesDialog(QWidget* parent);
        enum
        {
            min_height = 480,
            min_width = 640
        };

    public Q_SLOTS:
        virtual void accept();

    private:
        void syncWithSettings();

    protected:
        virtual void changeEvent(QEvent* );

    private:
        void retranslateUi();

        QGroupBox* generalBox_;
        QCheckBox* autoCheckUpdates_;
        QCheckBox* autoComletionEnable_;
        QLabel* langLabel_;
        QComboBox* languagesComboBox_;

        QLabel* stylesLabel_;
        QComboBox* stylesComboBox_;
        QLabel* defaultViewLabel_;
        QComboBox* defaultViewComboBox_;

        QGroupBox* serverSettingsBox_;
        QCheckBox* syncTabs_;
        QLabel* logDirLabel_;
        QLineEdit* logDirPath_;
        QCheckBox* autoOpneConsole_;
    };
}
