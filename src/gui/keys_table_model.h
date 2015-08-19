#pragma once

#include "core/types.h"

#include "fasto/qt/gui/base/table_model.h"

namespace fastonosql
{
    class KeyTableItem
            : public fasto::qt::gui::TableItem
    {
    public:
        enum eColumn
        {
            kKey = 0,
            kType = 1,
            kTTL = 2,
            kCountColumns = 3
        };

        KeyTableItem(const NDbValue& key);

        QString key() const;
        QString typeText() const;
        int32_t msecTTL() const;
        common::Value::Type type() const;

    private:
        const NDbValue key_;
    };

    class KeysTableModel
            : public fasto::qt::gui::TableModel
    {
        Q_OBJECT
    public:
        KeysTableModel(QObject *parent = 0);
        virtual ~KeysTableModel();

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        virtual int columnCount(const QModelIndex& parent) const;
        void clear();
    };
}


