#include "gui/fasto_common_item.h"

#include "common/qt/convert_string.h"

#include "common/text_decoders/compress_edcoder.h"
#include "common/text_decoders/msgpack_edcoder.h"
#include "common/text_decoders/hex_edcoder.h"

namespace fastonosql
{
    FastoCommonItem::FastoCommonItem(const QString& key, const QString& value, common::Value::Type type, TreeItem *parent, void* internalPointer)
        : TreeItem(parent, internalPointer), key_(key), value_(value), type_(type)
    {

    }

    QString FastoCommonItem::key() const
    {
        return key_;
    }

    QString FastoCommonItem::value() const
    {
        return value_;
    }

    void FastoCommonItem::setValue(const QString& val)
    {
        value_ = val;
    }

    common::Value::Type FastoCommonItem::type() const
    {
        return type_;
    }

    bool FastoCommonItem::isReadOnly() const
    {
        return changeCommand_.empty();
    }

    void FastoCommonItem::setChangeCommand(const std::string& cmd)
    {
        changeCommand_ = cmd;
    }

    std::string FastoCommonItem::changeCommand() const
    {
        return changeCommand_;
    }

    QString toJson(FastoCommonItem* item)
    {
        if(!item){
            return "{  }";
        }

        if(!item->childrenCount()){
            return QString("{ %1 : %2 }").arg(item->key()).arg(item->value());
        }

        QString value;
        for(int i = 0; i < item->childrenCount(); ++i){
            value += toJson(dynamic_cast<FastoCommonItem*>(item->child(i)));
        }

        return QString("{ %1 : %2 }").arg(item->key()).arg(value);
    }

    QString toRaw(FastoCommonItem* item)
    {
        if(!item){
            return "";
        }

        if(!item->childrenCount()){
            return item->value();
        }

        QString value;
        for(int i = 0; i < item->childrenCount(); ++i){
            value += toRaw(dynamic_cast<FastoCommonItem*>(item->child(i)));
        }

        return value;
    }

    QString toHex(FastoCommonItem* item)
    {
        if(!item){
            return "";
        }

        if(!item->childrenCount()){
            QString val = item->value();
            std::string sval = common::convertToString(val);

            std::string hexstr;
            common::HexEDcoder hex;
            common::ErrorValueSPtr er = hex.encode(sval, hexstr);
            if(er){
                return QString();
            }
            return common::convertFromString<QString>(hexstr);
        }

        QString value;
        for(int i = 0; i < item->childrenCount(); ++i){
            value += toHex(dynamic_cast<FastoCommonItem*>(item->child(i)));
        }

        return value;
    }

    QString toCsv(FastoCommonItem* item, const QString& delemitr)
    {
        if(!item){
            return "";
        }

        if(!item->childrenCount()){
            return item->value().replace(delemitr, ",");
        }

        QString value;
        for(int i = 0; i < item->childrenCount(); ++i){
            value += toCsv(dynamic_cast<FastoCommonItem*>(item->child(i)), delemitr);
            if(i != item->childrenCount() - 1){
                value += ",";
            }
        }

        return value;
    }

    QString fromGzip(FastoCommonItem* item)
    {
        if(!item){
            return QString();
        }

        if(!item->childrenCount()){
            QString val = item->value();
            std::string sval = common::convertToString(val);
            std::string out;
            common::CompressEDcoder enc;
            common::ErrorValueSPtr er = enc.decode(sval, out);
            if(er){
                return QString();
            }
            else{
                return common::convertFromString<QString>(out);
            }
        }

        QString value;
        for(int i = 0; i < item->childrenCount(); ++i){
            value += fromGzip(dynamic_cast<FastoCommonItem*>(item->child(i)));
        }

        return value;
    }

    QString fromHexMsgPack(FastoCommonItem* item)
    {
        if(!item){
            return QString();
        }

        if(!item->childrenCount()){
            std::string sval = common::convertToString(item->value());

            common::HexEDcoder hex;
            std::string hexstr;
            common::ErrorValueSPtr er = hex.decode(sval, hexstr);
            if(er){
                return QString();
            }

            common::MsgPackEDcoder msg;
            std::string upack;
            er = msg.decode(hexstr, upack);
            if(er){
                return QString();
            }
            return common::convertFromString<QString>(upack);
        }

        QString value;
        for(int i = 0; i < item->childrenCount(); ++i){
            value += fromHexMsgPack(dynamic_cast<FastoCommonItem*>(item->child(i)));
        }

        return value;
    }
}
