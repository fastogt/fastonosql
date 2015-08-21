#pragma once

#include <QDialog>

#include "core/events/events_info.h"

class QLineEdit;
class QSpinBox;
class QLabel;

namespace fastonosql
{
    class FastoTableView;
    class KeysTableModel;

    class ViewKeysDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        enum
        {
            min_height = 200,
            min_width = 320,
            min_key_on_page = 1,
            max_key_on_page = 100,
            defaults_key = 10,
            step_keys_on_page = defaults_key
        };

        explicit ViewKeysDialog(const QString& title, IDatabaseSPtr db, QWidget* parent = 0);

    private Q_SLOTS:
        void startLoadDatabaseContent(const EventsInfo::LoadDatabaseContentRequest& req);
        void finishLoadDatabaseContent(const EventsInfo::LoadDatabaseContentResponce& res);

        void startExecuteCommand(const EventsInfo::CommandRequest& req);
        void finishExecuteCommand(const EventsInfo::CommandResponce& res);

        void executeCommand(CommandKeySPtr cmd);

        void searchLineChanged(const QString& text);
        void leftPageClicked();
        void rightPageClicked();

    protected:
        virtual void changeEvent(QEvent* );

    private:
        void search(bool forward);
        void retranslateUi();
        void updateControls();
        size_t keysCount() const;

        std::vector<uint32_t> cursorStack_;
        uint32_t curPos_;
        QLineEdit* searchBox_;
        QLabel* keyCountLabel_;
        QSpinBox* countSpinEdit_;

        QPushButton* searchButton_;
        QPushButton* leftButtonList_;
        QSpinBox* currentKey_;
        QSpinBox* countKey_;
        QPushButton* rightButtonList_;
        FastoTableView* keysTable_;
        KeysTableModel* keysModel_;
        IDatabaseSPtr db_;
    };
}
