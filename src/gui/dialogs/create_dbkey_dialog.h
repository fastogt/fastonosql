#pragma once

#include <QDialog>

#include "core/types.h"

class QLineEdit;
class QComboBox;
class QListWidget;
class QTableWidget;
class QGroupBox;

namespace fastonosql
{
    class CreateDbKeyDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        enum
        {
            min_height = 200,
            min_width = 320
        };

        explicit CreateDbKeyDialog(const QString& title, connectionTypes type, QWidget* parent = 0);
        common::ValueSPtr value() const;
        NKey key() const;

    public Q_SLOTS:
        virtual void accept();

    private Q_SLOTS:
        void typeChanged(int index);
        void addItem();
        void removeItem();

    protected:
        virtual void changeEvent(QEvent* );

    private:
        bool validateAndApply();
        void retranslateUi();

        common::Value *getItem() const;
        const connectionTypes type_;
        QGroupBox* generalBox_;
        QLineEdit* keyEdit_;
        QComboBox* typesCombo_;
        QLineEdit* valueEdit_;
        QListWidget* valueListEdit_;
        QTableWidget* valueTableEdit_;
        common::ValueSPtr value_;
    };
}
