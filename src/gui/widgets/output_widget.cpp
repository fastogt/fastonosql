#include "gui/widgets/output_widget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>

#include "gui/fasto_text_view.h"
#include "gui/fasto_table_view.h"
#include "gui/fasto_tree_view.h"
#include "gui/fasto_common_model.h"
#include "gui/fasto_common_item.h"

#include "gui/gui_factory.h"
#include "fasto/qt/gui/icon_label.h"

#include "core/settings_manager.h"
#include "core/iserver.h"

#include "common/qt/convert_string.h"
#include "common/utf_string_conversions.h"
#include "common/time.h"

namespace
{
    fastonosql::FastoCommonItem* createItem(fasto::qt::gui::TreeItem* parent, const QString& key, fastonosql::FastoObject* item)
    {
        const std::string value = item->toString();
        return new fastonosql::FastoCommonItem(key, common::convertFromString<QString>(value), item->type(), parent, item);
    }
}

namespace fastonosql
{
    OutputWidget::OutputWidget(IServerSPtr server, QWidget* parent)
        : QWidget(parent)
    {
        commonModel_ = new FastoCommonModel(this);
        VERIFY(connect(commonModel_, &FastoCommonModel::changedValue, server.get(), &IServer::changeValue, Qt::DirectConnection));
        VERIFY(connect(server.get(), &IServer::startedChangeDbValue, this, &OutputWidget::startChangeDbValue, Qt::DirectConnection));
        VERIFY(connect(server.get(), &IServer::finishedChangeDbValue, this, &OutputWidget::finishChangeDbValue, Qt::DirectConnection));

        treeView_ = new FastoTreeView;
        treeView_->setModel(commonModel_);

        tableView_ = new FastoTableView;
        tableView_->setModel(commonModel_);

        textView_ = new FastoTextView(server->outputDelemitr());
        textView_->setModel(commonModel_);
        textView_->setReadOnly(true);

        timeLabel_ = new fasto::qt::gui::IconLabel(GuiFactory::instance().timeIcon(), "0", QSize(32, 32));

        QVBoxLayout* mainL = new QVBoxLayout;
        QHBoxLayout* topL = new QHBoxLayout;
        QSplitter* splitter = new QSplitter;
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(1);
        splitter->setContentsMargins(0, 0, 0, 0);

        treeButton_ = new QPushButton;
        tableButton_ = new QPushButton;
        textButton_ = new QPushButton;
        treeButton_->setIcon(GuiFactory::instance().treeIcon());
        VERIFY(connect(treeButton_, SIGNAL(clicked()), this, SLOT(setTreeView())));
        tableButton_->setIcon(GuiFactory::instance().tableIcon());
        VERIFY(connect(tableButton_, SIGNAL(clicked()), this, SLOT(setTableView())));
        textButton_->setIcon(GuiFactory::instance().textIcon());
        VERIFY(connect(textButton_, SIGNAL(clicked()), this, SLOT(setTextView())));

        topL->addWidget(treeButton_);
        topL->addWidget(tableButton_);
        topL->addWidget(textButton_);
        topL->addWidget(splitter);
        topL->addWidget(timeLabel_);

        mainL->addLayout(topL);
        mainL->addWidget(treeView_);
        mainL->addWidget(tableView_);
        mainL->addWidget(textView_);
        setLayout(mainL);
        syncWithSettings();
    }

    void OutputWidget::rootCreate(const EventsInfo::CommandRootCreatedInfo& res)
    {
        FastoObject* rootObj = res.root_.get();
        fastonosql::FastoCommonItem* root = createItem(NULL, "", rootObj);
        commonModel_->setRoot(root);
    }

    void OutputWidget::rootCompleate(const EventsInfo::CommandRootCompleatedInfo& res)
    {
        updateTimeLabel(res);
    }

    void OutputWidget::startChangeDbValue(const EventsInfo::ChangeDbValueRequest& req)
    {

    }

    void OutputWidget::finishChangeDbValue(const EventsInfo::ChangeDbValueResponce& res)
    {
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        commonModel_->changeValue(res.newItem_);
    }

    void OutputWidget::addChild(FastoObject* child)
    {
        DCHECK(child->parent());

        FastoObjectCommand* command = dynamic_cast<FastoObjectCommand*>(child);
        if(command){
            return;
        }

        command = dynamic_cast<FastoObjectCommand*>(child->parent());
        if(command){

            void* parentinner = command->parent();

            QModelIndex parent;
            bool isFound = commonModel_->findItem(parentinner, parent);
            if(!isFound){
                return;
            }

            fastonosql::FastoCommonItem* par = NULL;
            if(!parent.isValid()){
                par = static_cast<fastonosql::FastoCommonItem*>(commonModel_->root());
            }
            else{
                par = common::utils_qt::item<fastonosql::FastoCommonItem*>(parent);
            }

            DCHECK(par);
            if(!par){
                return;
            }

            const QString key = common::convertFromString<QString>(command->inputArgs());

            fastonosql::FastoCommonItem* comChild = createItem(par, key, child);
            comChild->setChangeCommand(command->oppositeCommand());
            commonModel_->insertItem(parent, comChild);
        }
        else{
            FastoObjectArray* arr = dynamic_cast<FastoObjectArray*>(child->parent());
            if(arr){
                QModelIndex parent;
                bool isFound = commonModel_->findItem(arr, parent);
                if(!isFound){
                    return;
                }

                fastonosql::FastoCommonItem* par = NULL;
                if(!parent.isValid()){
                    par = static_cast<fastonosql::FastoCommonItem*>(commonModel_->root());
                }
                else{
                    par = common::utils_qt::item<fastonosql::FastoCommonItem*>(parent);
                }

                DCHECK(par);
                if(!par){
                    return;
                }

                fastonosql::FastoCommonItem* comChild = createItem(par, QString(), child);
                commonModel_->insertItem(parent, comChild);
            }
            else{
                NOTREACHED();
            }
        }
    }

    void OutputWidget::itemUpdate(FastoObject* item, const QString& newValue)
    {
        QModelIndex index;
        bool isFound = commonModel_->findItem(item, index);
        if(!isFound){
            return;
        }

        fastonosql::FastoCommonItem* it = common::utils_qt::item<fastonosql::FastoCommonItem*>(index);
        if(!it){
            return;
        }

        it->setValue(newValue);
        commonModel_->updateItem(index.parent(), index);
    }

    void OutputWidget::setTreeView()
    {
        treeView_->setVisible(true);
        tableView_->setVisible(false);
        textView_->setVisible(false);
    }

    void OutputWidget::setTableView()
    {
        treeView_->setVisible(false);
        tableView_->setVisible(true);
        textView_->setVisible(false);
    }

    void OutputWidget::setTextView()
    {
        treeView_->setVisible(false);
        tableView_->setVisible(false);
        textView_->setVisible(true);
    }

    void OutputWidget::syncWithSettings()
    {
        supportedViews curV = SettingsManager::instance().defaultView();
        if(curV == Tree){
            setTreeView();
        }
        else if(curV == Table){
            setTableView();
        }
        else{
            setTextView();
        }
    }

    void OutputWidget::updateTimeLabel(const EventsInfo::EventInfoBase& evinfo)
    {
        timeLabel_->setText(QString("%1 msec").arg(evinfo.elapsedTime()));
    }
}
