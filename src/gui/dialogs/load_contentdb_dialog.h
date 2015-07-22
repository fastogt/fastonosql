#pragma once

#include <QDialog>

#include "core/connection_types.h"

class QLineEdit;
class QSpinBox;

namespace fastonosql
{
    class LoadContentDbDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        enum
        {
            min_height = 120,
            min_width = 240,
            min_key_on_page = 1,
            max_key_on_page = 1000,
            defaults_key = 100,
            step_keys_on_page = defaults_key
        };

        explicit LoadContentDbDialog(const QString& title, connectionTypes type, QWidget* parent = 0);
        uint32_t count() const;
        QString pattern() const;

    public Q_SLOTS:
        virtual void accept();

    private:
        const connectionTypes type_;

        QLineEdit* patternEdit_;
        QSpinBox* countSpinEdit_;
    };
}
