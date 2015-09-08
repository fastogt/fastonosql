#pragma  once

#include <QDialog>

#include "core/events/events_info.h"

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

#ifdef BUILD_WITH_UNQLITE
#include "core/unqlite/unqlite_infos.h"
#endif

#ifdef BUILD_WITH_LMDB
#include "core/lmdb/lmdb_infos.h"
#endif

class QLabel;

namespace fasto
{
    namespace qt
    {
        namespace gui
        {
            class GlassWidget;
        }
    }
}

namespace fastonosql
{
    class InfoServerDialog
            : public QDialog
    {
        Q_OBJECT
    public:
        explicit InfoServerDialog(const QString& title, connectionTypes type, QWidget* parent = 0);
        enum
        {
            min_height = 320,
            min_width = 240
        };

    Q_SIGNALS:
        void showed();

    public Q_SLOTS:
        void startServerInfo(const EventsInfo::ServerInfoRequest& req);
        void finishServerInfo(const EventsInfo::ServerInfoResponce& res);

    protected:
        virtual void showEvent(QShowEvent* e);

    private:
#ifdef BUILD_WITH_REDIS
        void updateText(const RedisServerInfo& serv);
#endif
#ifdef BUILD_WITH_MEMCACHED
        void updateText(const MemcachedServerInfo& serv);
#endif
#ifdef BUILD_WITH_SSDB
        void updateText(const SsdbServerInfo& serv);
#endif
#ifdef BUILD_WITH_LEVELDB
        void updateText(const LeveldbServerInfo& serv);
#endif
#ifdef BUILD_WITH_ROCKSDB
        void updateText(const RocksdbServerInfo& serv);
#endif
#ifdef BUILD_WITH_UNQLITE
        void updateText(const UnqliteServerInfo& serv);
#endif
#ifdef BUILD_WITH_LMDB
        void updateText(const LmdbServerInfo& serv);
#endif
        QLabel* serverTextInfo_;
        QLabel* hardwareTextInfo_;
        fasto::qt::gui::GlassWidget* glassWidget_;
        const connectionTypes type_;
    };
}
