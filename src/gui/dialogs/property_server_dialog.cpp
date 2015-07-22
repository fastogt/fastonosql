#include "gui/dialogs/property_server_dialog.h"

#include <QHBoxLayout>
#include <QTableView>

#include "fasto/qt/gui/glass_widget.h"

#include "gui/gui_factory.h"
#include "gui/property_table_model.h"

#include "translations/global.h"

namespace fastonosql
{
    PropertyServerDialog::PropertyServerDialog(const QString& title, connectionTypes type, QWidget* parent)
        : QDialog(parent), type_(type)
    {
        using namespace translations;
        setWindowIcon(GuiFactory::instance().icon(type_));
        setWindowTitle(title);

        PropertyTableModel* mod = new PropertyTableModel(this);
        propertyes_table_ = new QTableView;
        VERIFY(connect(mod, &PropertyTableModel::changedProperty, this, &PropertyServerDialog::changedProperty));
        propertyes_table_->setModel(mod);

        QHBoxLayout *mainL = new QHBoxLayout;
        mainL->addWidget(propertyes_table_);
        setLayout(mainL);

        glassWidget_ = new fasto::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(), trLoading, 0.5, QColor(111, 111, 100), this);
    }

    void PropertyServerDialog::startServerProperty(const EventsInfo::ServerPropertyInfoRequest& req)
    {
        glassWidget_->start();
    }

    void PropertyServerDialog::finishServerProperty(const EventsInfo::ServerPropertyInfoResponce& res)
    {
        glassWidget_->stop();
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        if(type_ == REDIS){
            ServerPropertyInfo inf = res.info_;
            PropertyTableModel *model = qobject_cast<PropertyTableModel*>(propertyes_table_->model());
            for(int i = 0; i < inf.propertyes_.size(); ++i)
            {
                PropertyType it = inf.propertyes_[i];
                model->insertItem(new PropertyTableItem(common::convertFromString<QString>(it.first), common::convertFromString<QString>(it.second)));
            }
        }
    }

    void PropertyServerDialog::startServerChangeProperty(const EventsInfo::ChangeServerPropertyInfoRequest& req)
    {

    }

    void PropertyServerDialog::finishServerChangeProperty(const EventsInfo::ChangeServerPropertyInfoResponce& res)
    {
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        if(type_ == REDIS){
            PropertyType pr = res.newItem_;
            if(res.isChange_){
                PropertyTableModel *model = qobject_cast<PropertyTableModel*>(propertyes_table_->model());
                model->changeProperty(pr);
            }
        }
    }

    void PropertyServerDialog::showEvent(QShowEvent* e)
    {
        QDialog::showEvent(e);
        emit showed();
    }
}
