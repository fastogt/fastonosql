#include "gui/dialogs/create_dbkey_dialog.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QLabel>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QMenu>
#include <QGridLayout>
#include <QGroupBox>
#include <QEvent>

#include "common/qt/convert_string.h"

#include "gui/gui_factory.h"
#include "gui/dialogs/input_dialog.h"

#include "translations/global.h"
#ifdef BUILD_WITH_REDIS
#include "core/redis/redis_infos.h"
#endif

#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/memcached_infos.h"
#endif

#ifdef BUILD_WITH_SSDB
#include "core/ssdb/ssdb_infos.h"
#endif

#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/leveldb_infos.h"
#endif

namespace fastonosql
{
    CreateDbKeyDialog::CreateDbKeyDialog(const QString &title, connectionTypes type, QWidget* parent)
        : QDialog(parent), type_(type), value_()
    {
        using namespace translations;

        setWindowIcon(GuiFactory::instance().icon(type_));
        setWindowTitle(title);

        QGridLayout *kvLayout = new QGridLayout;

        kvLayout->addWidget(new QLabel(tr("Type:")), 0, 0);
        typesCombo_ = new QComboBox;
#ifdef BUILD_WITH_REDIS
        if(type_ == REDIS){
            for(int i = 0; i < DBTraits<REDIS>::supportedTypes.size(); ++i){
                common::Value::Type t = DBTraits<REDIS>::supportedTypes[i];
                QString type = common::convertFromString<QString>(common::Value::toString(t));
                typesCombo_->addItem(GuiFactory::instance().icon(t), type, t);
            }
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type_ == MEMCACHED){
            for(int i = 0; i < DBTraits<MEMCACHED>::supportedTypes.size(); ++i){
                common::Value::Type t = DBTraits<MEMCACHED>::supportedTypes[i];
                QString type = common::convertFromString<QString>(common::Value::toString(t));
                typesCombo_->addItem(GuiFactory::instance().icon(t), type, t);
            }
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type_ == SSDB){
            for(int i = 0; i < DBTraits<SSDB>::supportedTypes.size(); ++i){
                common::Value::Type t = DBTraits<SSDB>::supportedTypes[i];
                QString type = common::convertFromString<QString>(common::Value::toString(t));
                typesCombo_->addItem(GuiFactory::instance().icon(t), type, t);
            }
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type_ == LEVELDB){
            for(int i = 0; i < DBTraits<LEVELDB>::supportedTypes.size(); ++i){
                common::Value::Type t = DBTraits<LEVELDB>::supportedTypes[i];
                QString type = common::convertFromString<QString>(common::Value::toString(t));
                typesCombo_->addItem(GuiFactory::instance().icon(t), type, t);
            }
        }
#endif
        typedef void (QComboBox::*ind)(int);
        VERIFY(connect(typesCombo_, static_cast<ind>(&QComboBox::currentIndexChanged), this, &CreateDbKeyDialog::typeChanged));
        kvLayout->addWidget(typesCombo_, 0, 1);

        //key layout

        kvLayout->addWidget(new QLabel(tr("Key:")), 1, 0);
        keyEdit_ = new QLineEdit;
        kvLayout->addWidget(keyEdit_, 1, 1);

        //value layout

        kvLayout->addWidget(new QLabel(tr("Value:")), 2, 0);
        valueEdit_ = new QLineEdit;
        kvLayout->addWidget(valueEdit_, 2, 1);
        valueEdit_->setVisible(true);

        valueListEdit_ = new QListWidget;
        valueListEdit_->setContextMenuPolicy(Qt::ActionsContextMenu);
        valueListEdit_->setSelectionMode(QAbstractItemView::SingleSelection);
        valueListEdit_->setSelectionBehavior(QAbstractItemView::SelectRows);

        QAction* addItem = new QAction(trAddItem, this);
        VERIFY(connect(addItem, &QAction::triggered, this, &CreateDbKeyDialog::addItem));
        valueListEdit_->addAction(addItem);

        QAction* removeItem = new QAction(trRemoveItem, this);
        VERIFY(connect(removeItem, &QAction::triggered, this, &CreateDbKeyDialog::removeItem));
        valueListEdit_->addAction(removeItem);

        kvLayout->addWidget(valueListEdit_, 2, 1);
        valueListEdit_->setVisible(false);

        valueTableEdit_ = new QTableWidget(0, 2);
        valueTableEdit_->setContextMenuPolicy(Qt::ActionsContextMenu);
        valueTableEdit_->setSelectionBehavior(QAbstractItemView::SelectRows);
        valueTableEdit_->verticalHeader()->hide();
        valueTableEdit_->horizontalHeader()->hide();

        valueTableEdit_->addAction(addItem);
        valueTableEdit_->addAction(removeItem);

        kvLayout->addWidget(valueTableEdit_, 2, 1);
        valueTableEdit_->setVisible(false);

        generalBox_ = new QGroupBox;
        generalBox_->setLayout(kvLayout);

        // main layout
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(generalBox_);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        VERIFY(connect(buttonBox, &QDialogButtonBox::accepted, this, &CreateDbKeyDialog::accept));
        VERIFY(connect(buttonBox, &QDialogButtonBox::rejected, this, &CreateDbKeyDialog::reject));
        layout->addWidget(buttonBox);

        typeChanged(0);
        setMinimumSize(QSize(min_width, min_height));
        setLayout(layout);
        retranslateUi();
    }

    FastoObjectIPtr CreateDbKeyDialog::value() const
    {
        return value_;
    }

    NKey CreateDbKeyDialog::key() const
    {
        int index = typesCombo_->currentIndex();
        QVariant var = typesCombo_->itemData(index);
        common::Value::Type t = (common::Value::Type)qvariant_cast<unsigned char>(var);
        return NKey(common::convertToString(keyEdit_->text()), t);
    }

    void CreateDbKeyDialog::accept()
    {
        if(validateAndApply()){
            QDialog::accept();
        }
    }

    void CreateDbKeyDialog::typeChanged(int index)
    {
        QVariant var = typesCombo_->itemData(index);
        common::Value::Type t = (common::Value::Type)qvariant_cast<unsigned char>(var);
        valueEdit_->clear();

        if(t == common::Value::TYPE_ARRAY || t == common::Value::TYPE_SET){
            valueListEdit_->setVisible(true);
            valueEdit_->setVisible(false);
            valueTableEdit_->setVisible(false);
        }
        else if(t == common::Value::TYPE_ZSET || t == common::Value::TYPE_HASH){
            valueTableEdit_->setVisible(true);
            valueEdit_->setVisible(false);
            valueListEdit_->setVisible(false);
        }
        else{            
            valueEdit_->setVisible(true);
            valueListEdit_->setVisible(false);
            valueTableEdit_->setVisible(false);
            if(t == common::Value::TYPE_INTEGER || t == common::Value::TYPE_UINTEGER){
                QRegExp rx("\\d+");//(0-65554)
                valueEdit_->setValidator(new QRegExpValidator(rx, this));
            }
            else if(t == common::Value::TYPE_BOOLEAN){
                QRegExp rx("true|false");//
                valueEdit_->setValidator(new QRegExpValidator(rx, this));
            }
            else{
                QRegExp rx("*");//
                valueEdit_->setValidator(new QRegExpValidator(rx, this));
            }
        }
    }

    void CreateDbKeyDialog::addItem()
    {
        using namespace translations;

        if(valueListEdit_->isVisible()){
            InputDialog diag(this, trAddItem, InputDialog::SingleLine, trValue);
            int result = diag.exec();
            if(result != QDialog::Accepted){
                return;
            }

            QString text = diag.firstText();
            if(!text.isEmpty()){
                QListWidgetItem* nitem = new QListWidgetItem(text, valueListEdit_);
                nitem->setFlags(nitem->flags() | Qt::ItemIsEditable);
                valueListEdit_->addItem(nitem);
            }
        }
        else{
            int index = typesCombo_->currentIndex();
            QVariant var = typesCombo_->itemData(index);
            common::Value::Type t = (common::Value::Type)qvariant_cast<unsigned char>(var);

            InputDialog diag(this, trAddItem, InputDialog::DoubleLine, t == common::Value::TYPE_HASH ? trField : trScore, trValue);
            int result = diag.exec();
            if(result != QDialog::Accepted){
                return;
            }

            QString ftext = diag.firstText();
            QString stext = diag.secondText();

            if(!ftext.isEmpty() && !stext.isEmpty()){
                QTableWidgetItem* fitem = new QTableWidgetItem(ftext);
                fitem->setFlags(fitem->flags() | Qt::ItemIsEditable);

                QTableWidgetItem* sitem = new QTableWidgetItem(stext);
                sitem->setFlags(sitem->flags() | Qt::ItemIsEditable);

                valueTableEdit_->insertRow(0);
                valueTableEdit_->setItem(0, 0, fitem);
                valueTableEdit_->setItem(0, 1, sitem);
            }
        }
    }

    void CreateDbKeyDialog::removeItem()
    {
        if(valueListEdit_->isVisible()){
            QListWidgetItem* ritem = valueListEdit_->currentItem();
            delete ritem;
        }
        else{
            int row = valueTableEdit_->currentRow();
            valueTableEdit_->removeRow(row);
        }
    }

    void CreateDbKeyDialog::changeEvent(QEvent* e)
    {
        if(e->type() == QEvent::LanguageChange){
            retranslateUi();
        }
        QDialog::changeEvent(e);
    }

    bool CreateDbKeyDialog::validateAndApply()
    {
        if(keyEdit_->text().isEmpty()){
            return false;
        }

        FastoObject* obj = getItem();
        if(!obj){
            return false;
        }

        value_.reset(obj);
        return true;
    }

    void CreateDbKeyDialog::retranslateUi()
    {
        generalBox_->setTitle(tr("Key/Value input"));
    }

    FastoObject* CreateDbKeyDialog::getItem() const
    {
        int index = typesCombo_->currentIndex();
        QVariant var = typesCombo_->itemData(index);
        common::Value::Type t = (common::Value::Type)qvariant_cast<unsigned char>(var);
        if(t == common::Value::TYPE_ARRAY){
            if(valueListEdit_->count() == 0) {
                return NULL;
            }
            common::ArrayValue* ar = common::Value::createArrayValue();
            for(int i = 0; i < valueListEdit_->count(); ++i){
                std::string val = common::convertToString(valueListEdit_->item(i)->text());
                ar->appendString(val);
            }

            return new FastoObjectArray(NULL, ar, " ");
        }
        else if(t == common::Value::TYPE_SET){
            if(valueListEdit_->count() == 0) {
                return NULL;
            }
            common::SetValue* ar = common::Value::createSetValue();
            for(int i = 0; i < valueListEdit_->count(); ++i){
                std::string val = common::convertToString(valueListEdit_->item(i)->text());
                ar->insert(val);
            }

            return new FastoObjectSet(NULL, ar, " ");
        }
        else if(t == common::Value::TYPE_ZSET){
            if(valueTableEdit_->rowCount() == 0) {
                return NULL;
            }

            common::ZSetValue* ar = common::Value::createZSetValue();
            for(int i = 0; i < valueTableEdit_->rowCount(); ++i){
                QTableWidgetItem* kitem = valueTableEdit_->item(i, 0);
                QTableWidgetItem* vitem = valueTableEdit_->item(i, 0);

                std::string key = common::convertToString(kitem->text());
                std::string val = common::convertToString(vitem->text());
                ar->insert(key, val);
            }

            return new FastoObjectZSet(NULL, ar, " ");
        }
        else if(t == common::Value::TYPE_HASH){
            if(valueTableEdit_->rowCount() == 0) {
                return NULL;
            }

            common::HashValue* ar = common::Value::createHashValue();
            for(int i = 0; i < valueTableEdit_->rowCount(); ++i){
                QTableWidgetItem* kitem = valueTableEdit_->item(i, 0);
                QTableWidgetItem* vitem = valueTableEdit_->item(i, 0);

                std::string key = common::convertToString(kitem->text());
                std::string val = common::convertToString(vitem->text());
                ar->insert(key, val);
            }

            return new FastoObjectHash(NULL, ar, " ");
        }
        else{
            QString text = valueEdit_->text();
            if(text.isEmpty()){
                return NULL;
            }

            return new FastoObject(NULL, common::Value::createStringValue(common::convertToString(text)), std::string());
        }
    }
}
