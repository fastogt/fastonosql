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

        explicit KeyTableItem(const NDbValue& key);

        QString key() const;
        QString typeText() const;
        int32_t TTL() const;
        common::Value::Type type() const;

        NDbValue dbv() const;
        void setDbv(const NDbValue& val);

    private:
        NDbValue key_;
    };

    class KeysTableModel
            : public fasto::qt::gui::TableModel
    {
        Q_OBJECT
    public:
        explicit KeysTableModel(QObject *parent = 0);
        virtual ~KeysTableModel();

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        virtual int columnCount(const QModelIndex& parent) const;
        void clear();

        void changeValue(const NDbValue& value);

    Q_SIGNALS:
        void changedValue(CommandKeySPtr cmd);
    };
}


