#include "gui/property_table_model.h"

#include "common/qt/utils_qt.h"
#include "common/qt/convert_string.h"

#include "translations/global.h"

namespace fastoredis
{
    PropertyTableItem::PropertyTableItem(const QString& key, const QString& value)
        : key_(key), value_(value)
    {

    }

    PropertyTableModel::PropertyTableModel(QObject* parent)
        : TableModel(parent)
    {

    }

    PropertyTableModel::~PropertyTableModel()
    {

    }

    QVariant PropertyTableModel::data(const QModelIndex& index, int role) const
    {
        QVariant result;

        if (!index.isValid())
            return result;

        PropertyTableItem *node = common::utils_qt::item<PropertyTableItem*>(index);

        if (!node)
            return result;

        int col = index.column();

        if (role == Qt::DisplayRole) {
            if (col == PropertyTableItem::eKey) {
                result = node->key_;
            }
            else if (col == PropertyTableItem::eValue) {
                result = node->value_;
            }
        }
        return result;
    }

    bool PropertyTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (index.isValid() && role == Qt::EditRole) {
            int column = index.column();
            PropertyTableItem *node = common::utils_qt::item<PropertyTableItem*>(index);

            if (!node)
                return false;

            if (column == PropertyTableItem::eKey) {

            }
            else if (column == PropertyTableItem::eValue) {
                const QString &newValue = value.toString();
                if(newValue != node->value_){
                    PropertyType pr;
                    pr.first = common::convertToString(node->key_);
                    pr.second = common::convertToString(newValue);
                    emit changedProperty(pr);
                }
            }
        }

        return false;
    }

    Qt::ItemFlags PropertyTableModel::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags result = 0;
        if (index.isValid()) {
            result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            int col = index.column();
            if(col == PropertyTableItem::eValue){
                result |= Qt::ItemIsEditable;
            }
        }
        return result;
    }

    QVariant PropertyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        using namespace translations;
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == PropertyTableItem::eKey) {
                return trKey;
            }
            else if (section == PropertyTableItem::eValue) {
                return trValue;
            }
        }

        return TableModel::headerData(section, orientation, role);
    }

    int PropertyTableModel::columnCount(const QModelIndex& parent) const
    {
        return PropertyTableItem::eCountColumns;
    }


    void PropertyTableModel::changeProperty(const PropertyType& pr)
    {
        const QString key = common::convertFromString<QString>(pr.first);
        for(int i = 0; i < data_.size(); ++i)
        {
            PropertyTableItem *it = dynamic_cast<PropertyTableItem*>(data_[i]);
            if(it->key_ == key){
                it->value_ = common::convertFromString<QString>(pr.second);
                emit dataChanged(index(i,0), index(i,1));
                break;
            }
        }
    }
}
