#pragma once

#include <QString>

#include "core/types.h"
#include "fasto/qt/gui/base/tree_item.h"

namespace fastonosql
{
    class FastoCommonItem
            : public fasto::qt::gui::TreeItem
    {
    public:
        enum eColumn
        {
            eKey = 0,
            eValue = 1,
            eType = 2,
            eCountColumns = 3
        };
        FastoCommonItem(const QString& key, NValue value, bool isReadOnly, TreeItem* parent, void* internalPointer);

        QString key() const;
        QString value() const;        
        common::Value::Type type() const;

        bool isReadOnly() const;

        void setValue(NValue val);

    private:
        QString key_;
        NValue value_;
        bool isReadOnly_;
    };

    QString toJson(FastoCommonItem* item);
    QString toRaw(FastoCommonItem* item);
    QString toHex(FastoCommonItem* item);
    QString toCsv(FastoCommonItem* item, const QString &delemitr);

    QString fromGzip(FastoCommonItem* item);
    QString fromHexMsgPack(FastoCommonItem* item);
}

