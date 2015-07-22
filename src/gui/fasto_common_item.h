#pragma once

#include <QString>

#include "common/value.h"
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
        typedef std::vector<FastoCommonItem*> child_container_type;
        FastoCommonItem(const QString& key, const QString& value, common::Value::Type type, TreeItem* parent, void* internalPointer);

        QString key() const;
        QString value() const;
        void setValue(const QString& val);
        common::Value::Type type() const;

        bool isReadOnly() const;
        void setChangeCommand(const std::string& cmd);
        std::string changeCommand() const;

    private:
        QString key_;
        QString value_;
        std::string changeCommand_;
        common::Value::Type type_;
    };

    QString toJson(FastoCommonItem* item);
    QString toRaw(FastoCommonItem* item);
    QString toHex(FastoCommonItem* item);
    QString toCsv(FastoCommonItem* item, const QString &delemitr);

    QString fromGzip(FastoCommonItem* item);
    QString fromHexMsgPack(FastoCommonItem* item);
}

