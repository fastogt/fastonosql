#include "gui/dialogs/history_server_dialog.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QComboBox>

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

#ifdef BUILD_WITH_ROCKSDB
#include "core/rocksdb/rocksdb_infos.h"
#endif

#include "fasto/qt/gui/base/graph_widget.h"
#include "gui/gui_factory.h"
#include "fasto/qt/gui/glass_widget.h"

#include "translations/global.h"

namespace fastonosql
{
    ServerHistoryDialog::ServerHistoryDialog(const QString& title, connectionTypes type, QWidget* parent)
        : QDialog(parent, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint ), type_(type)
    {
        using namespace translations;

        setWindowIcon(GuiFactory::instance().icon(type_));
        setWindowTitle(title);

        graphWidget_ = new fasto::qt::gui::GraphWidget;
        settingsGraph_ = new QWidget;
        QHBoxLayout *mainL = new QHBoxLayout;

        QSplitter *splitter = new QSplitter;
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(1);
        splitter->setContentsMargins(0, 0, 0, 0);

        mainL->addWidget(splitter);
        splitter->addWidget(settingsGraph_);

        serverInfoGroupsNames_ = new QComboBox;
        serverInfoFields_ = new QComboBox;

        typedef void (QComboBox::*curc)(int);
        VERIFY(connect(serverInfoGroupsNames_, static_cast<curc>(&QComboBox::currentIndexChanged), this, &ServerHistoryDialog::refreshInfoFields ));
        VERIFY(connect(serverInfoFields_, static_cast<curc>(&QComboBox::currentIndexChanged), this, &ServerHistoryDialog::refreshGraph ));
#ifdef BUILD_WITH_REDIS
        if(type_ == REDIS){
            for(int i = 0; i < redisHeaders.size(); ++i){
                serverInfoGroupsNames_->addItem(common::convertFromString<QString>(redisHeaders[i]));
            }
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type_ == MEMCACHED){
            for(int i = 0; i < memcachedHeaders.size(); ++i){
                serverInfoGroupsNames_->addItem(common::convertFromString<QString>(memcachedHeaders[i]));
            }
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type_ == SSDB){
            for(int i = 0; i < ssdbHeaders.size(); ++i){
                serverInfoGroupsNames_->addItem(common::convertFromString<QString>(ssdbHeaders[i]));
            }
        }
#endif
#ifdef BUILD_WITH_LEVELDB
        if(type_ == LEVELDB){
            for(int i = 0; i < leveldbHeaders.size(); ++i){
                serverInfoGroupsNames_->addItem(common::convertFromString<QString>(leveldbHeaders[i]));
            }
        }
#endif
#ifdef BUILD_WITH_ROCKSDB
        if(type_ == ROCKSDB){
            for(int i = 0; i < rocksdbHeaders.size(); ++i){
                serverInfoGroupsNames_->addItem(common::convertFromString<QString>(rocksdbHeaders[i]));
            }
        }
#endif
        QVBoxLayout *setingsLayout = new QVBoxLayout;
        setingsLayout->addWidget(serverInfoGroupsNames_);
        setingsLayout->addWidget(serverInfoFields_);
        settingsGraph_->setLayout(setingsLayout);

        splitter->addWidget(graphWidget_);
        setLayout(mainL);

        glassWidget_ = new fasto::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(), trLoading, 0.5, QColor(111, 111, 100), this);
    }

    void ServerHistoryDialog::startLoadServerHistoryInfo(const EventsInfo::ServerInfoHistoryRequest& req)
    {
        glassWidget_->start();
    }

    void ServerHistoryDialog::finishLoadServerHistoryInfo(const EventsInfo::ServerInfoHistoryResponce& res)
    {
        glassWidget_->stop();
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        infos_ = res.infos();
        reset();
    }

    void ServerHistoryDialog::snapShotAdd(ServerInfoSnapShoot snapshot)
    {
        infos_.push_back(snapshot);
        reset();
    }

    void ServerHistoryDialog::refreshInfoFields(int index)
    {
        if(index == -1){
            return;
        }

        serverInfoFields_->clear();

        std::vector<Field> field;
#ifdef BUILD_WITH_REDIS
        if(type_ == REDIS){
            field = redisFields[index];
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type_ == MEMCACHED){
            field = memcachedFields[index];
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type_ == SSDB){
            field = ssdbFields[index];
        }
#endif
#ifdef BUILD_WITH_LEVELDB
        if(type_ == LEVELDB){
            field = leveldbFields[index];
        }
#endif
#ifdef BUILD_WITH_ROCKSDB
        if(type_ == ROCKSDB){
            field = rocksdbFields[index];
        }
#endif
        DCHECK(!field.empty());
        for(int i = 0; i < field.size(); ++i){
            Field fl = field[i];
            if(fl.isIntegral()){
                serverInfoFields_->addItem(common::convertFromString<QString>(fl.name_), i);
            }
        }
    }

    void ServerHistoryDialog::refreshGraph(int index)
    {
        if(index == -1){
            return;
        }

        unsigned char serverIndex = serverInfoGroupsNames_->currentIndex();
        QVariant var = serverInfoFields_->itemData(index);
        unsigned char indexIn = qvariant_cast<unsigned char>(var);
        fasto::qt::gui::GraphWidget::nodes_container_type nodes;
        for(EventsInfo::ServerInfoHistoryResponce::infos_container_type::iterator it = infos_.begin(); it != infos_.end(); ++it){
            EventsInfo::ServerInfoHistoryResponce::infos_container_type::value_type val = *it;
            if(!val.isValid()){
                continue;
            }

            common::Value* value = val.info_->valueByIndexes(serverIndex, indexIn); //allocate
            if(value){
                qreal graphY = 0.0f;
                if(value->getAsDouble(&graphY)){
                    nodes.push_back(std::make_pair(val.msec_, graphY));
                }
            }
            delete value;
        }

        graphWidget_->setNodes(nodes);
    }

    void ServerHistoryDialog::showEvent(QShowEvent* e)
    {
        QDialog::showEvent(e);
        emit showed();
    }

    void ServerHistoryDialog::reset()
    {
        refreshGraph(serverInfoFields_->currentIndex());
    }
}
