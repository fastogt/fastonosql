#pragma once

#include "fasto/qt/gui/base/table_model.h"

#include "core/types.h"

namespace fastonosql
{
    struct PropertyTableItem
            : public fasto::qt::gui::TableItem
    {
        enum eColumn
        {
            eKey = 0,
            eValue = 1,
            eCountColumns = 2
        };
        PropertyTableItem(const QString& key, const QString& value);

        QString key_;
        QString value_;
    };

    class PropertyTableModel
            : public fasto::qt::gui::TableModel
    {
        Q_OBJECT
    public:
        PropertyTableModel(QObject* parent = 0);
        ~PropertyTableModel();

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

        void changeProperty(const PropertyType& pr);

    Q_SIGNALS:
        void changedProperty(const PropertyType& pr);
    };
}


