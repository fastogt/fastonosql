#include "gui/gui_factory.h"

#include <QApplication>
#include <QStyle>

#include "core/settings_manager.h"

namespace fastonosql
{
    const QIcon& GuiFactory::homePageIcon() const
    {
        static QIcon homepage(":" PROJECT_NAME_LOWERCASE "/images/64x64/homepage.png");
        return homepage;
    }

    const QIcon& GuiFactory::facebookIcon() const
    {
        static QIcon facebook(":" PROJECT_NAME_LOWERCASE "/images/64x64/facebook.png");
        return facebook;
    }

    const QIcon& GuiFactory::twitterIcon() const
    {
        static QIcon twitter(":" PROJECT_NAME_LOWERCASE "/images/64x64/twitter.png");
        return twitter;
    }

    const QIcon& GuiFactory::githubIcon() const
    {
        static QIcon github(":" PROJECT_NAME_LOWERCASE "/images/64x64/github.png");
        return github;
    }

    const QIcon& GuiFactory::openIcon() const
    {
        static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/open.png");
        return open;
    }

    const QIcon& GuiFactory::logoIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/logo.png");
        return main;
    }

    const QIcon& GuiFactory::mainWindowIcon() const
    {
        return logoIcon();
    }

    const QIcon& GuiFactory::connectIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/connect.png");
        return main;
    }

    const QIcon& GuiFactory::disConnectIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/disconnect.png");
        return main;
    }

    const QIcon& GuiFactory::serverIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/server.png");
        return main;
    }

    const QIcon& GuiFactory::addIcon() const
    {
        static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/add.png");
        return open;
    }

    const QIcon& GuiFactory::removeIcon() const
    {
        static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/remove.png");
        return open;
    }

    const QIcon& GuiFactory::editIcon() const
    {
        static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/edit.png");
        return open;
    }

    const QIcon& GuiFactory::messageBoxInformationIcon() const
    {
        static QIcon open = qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation);
        return open;
    }

    const QIcon& GuiFactory::messageBoxQuestionIcon() const
    {
        static QIcon open = qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion);
        return open;
    }

    const QIcon& GuiFactory::executeIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/execute.png");
        return start;
    }

    const QIcon& GuiFactory::timeIcon() const
    {
        static QIcon time(":" PROJECT_NAME_LOWERCASE "/images/64x64/time.png");
        return time;
    }

    const QIcon& GuiFactory::stopIcon() const
    {
        static QIcon stop(":" PROJECT_NAME_LOWERCASE "/images/64x64/stop.png");
        return stop;
    }

    const QIcon& GuiFactory::databaseIcon() const
    {
        static QIcon db(":" PROJECT_NAME_LOWERCASE "/images/64x64/database.png");
        return db;
    }

    const QIcon& GuiFactory::keyIcon() const
    {
        static QIcon db(":" PROJECT_NAME_LOWERCASE "/images/64x64/key.png");
        return db;
    }

    const QIcon& GuiFactory::icon(connectionTypes type) const
    {
        if(type == REDIS){
            return redisConnectionIcon();
        }
        else if(type == MEMCACHED){
            return memcachedConnectionIcon();
        }
        else if(type == SSDB){
            return ssdbConnectionIcon();
        }
        else if(type == LEVELDB){
            return leveldbConnectionIcon();
        }
        else if(type == ROCKSDB){
            return rocksdbConnectionIcon();
        }
        else if(type == UNQLITE){
            return unqliteConnectionIcon();
        }
        else if(type == LMDB){
            return lmdbConnectionIcon();
        }
        else{
            return serverIcon();
        }
    }

    const QIcon& GuiFactory::modeIcon(ConnectionMode mode) const
    {
        if(mode == LatencyMode){
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/latency_mode.png");
            return i;
        }
        else if(mode == SlaveMode){
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/slave_mode.png");
            return i;
        }
        else if(mode == GetRDBMode){
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/rdb_mode.png");
            return i;
        }
        else if(mode == PipeMode){
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/pipe_mode.png");
            return i;
        }
        else if(mode == FindBigKeysMode){
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/bigkeys_mode.png");
            return i;
        }
        else if(mode == StatMode){
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/stat_mode.png");
            return i;
        }
        else if(mode == ScanMode){
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/scan_mode.png");
            return i;
        }
        else if(mode == IntaractiveMode){
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/interactive_mode.png");
            return i;
        }
        else{
            static QIcon err(":" PROJECT_NAME_LOWERCASE "/images/64x64/error.png");
            NOTREACHED();
            return err;
        }
    }

    const QIcon& GuiFactory::icon(common::Value::Type type) const
    {
        switch(type)
        {
        case common::Value::TYPE_NULL:
            static QIcon u(":" PROJECT_NAME_LOWERCASE "/images/64x64/null.png");
            return u;
        case common::Value::TYPE_BOOLEAN:
            static QIcon b(":" PROJECT_NAME_LOWERCASE "/images/64x64/bool.png");
            return b;
        case common::Value::TYPE_STRING:
            static QIcon s(":" PROJECT_NAME_LOWERCASE "/images/64x64/string.png");
            return s;
        case common::Value::TYPE_SET:
        case common::Value::TYPE_ARRAY:
            static QIcon a(":" PROJECT_NAME_LOWERCASE "/images/64x64/array.png");
            return a;
        case common::Value::TYPE_HASH:
            static QIcon h(":" PROJECT_NAME_LOWERCASE "/images/64x64/hash.png");
            return h;
        case common::Value::TYPE_ZSET:
            static QIcon z(":" PROJECT_NAME_LOWERCASE "/images/64x64/zset.png");
            return z;
        case common::Value::TYPE_INTEGER:
        case common::Value::TYPE_DOUBLE:
        case common::Value::TYPE_UINTEGER:
            static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/integer.png");
            return i;
        case common::Value::TYPE_ERROR:
            static QIcon er(":" PROJECT_NAME_LOWERCASE "/images/64x64/error.png");
            return er;
        default:
            static QIcon err(":" PROJECT_NAME_LOWERCASE "/images/64x64/error.png");
            NOTREACHED();
            return err;
        }
    }

    const QIcon& GuiFactory::importIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/import.png");
        return start;
    }

    const QIcon& GuiFactory::exportIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/export.png");
        return start;
    }

    const QIcon& GuiFactory::loadIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/load.png");
        return start;
    }

    const QIcon& GuiFactory::clusterIcon() const
    {
        static QIcon cluster(":" PROJECT_NAME_LOWERCASE "/images/64x64/cluster.png");
        return cluster;
    }

    const QIcon& GuiFactory::saveIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/save.png");
        return start;
    }

    const QIcon& GuiFactory::saveAsIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/saveas.png");
        return start;
    }

    const QIcon& GuiFactory::textIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/text.png");
        return start;
    }

    const QIcon& GuiFactory::tableIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/table.png");
        return start;
    }

    const QIcon& GuiFactory::treeIcon() const
    {
        static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/tree.png");
        return start;
    }

    const QIcon& GuiFactory::loggingIcon() const
    {
        static QIcon logg(":" PROJECT_NAME_LOWERCASE "/images/64x64/logging.png");
        return logg;
    }

    const QIcon& GuiFactory::discoveryIcon() const
    {
        static QIcon discovery(":" PROJECT_NAME_LOWERCASE "/images/64x64/discovery.png");
        return discovery;
    }

    const QIcon& GuiFactory::commandIcon() const
    {
        static QIcon comm(":" PROJECT_NAME_LOWERCASE "/images/64x64/command.png");
        return comm;
    }

    const QIcon& GuiFactory::encodeDecodeIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/encode_decode.png");
        return main;
    }

    const QIcon& GuiFactory::preferencesIcon() const
    {
        static QIcon pref(":" PROJECT_NAME_LOWERCASE "/images/64x64/preferences.png");
        return pref;
    }

    const QIcon& GuiFactory::leftIcon() const
    {
        static QIcon left = qApp->style()->standardIcon(QStyle::SP_ArrowLeft);
        return left;
    }

    const QIcon& GuiFactory::rightIcon() const
    {
        static QIcon right = qApp->style()->standardIcon(QStyle::SP_ArrowRight);
        return right;
    }

    const QIcon& GuiFactory::close16Icon() const
    {
        static QIcon close(":" PROJECT_NAME_LOWERCASE "/images/16x16/close.png");
        return close;
    }

    const QIcon& GuiFactory::commandIcon(connectionTypes type) const
    {
        if(type == REDIS){
            return redisConnectionIcon();
        }
        else if(type == MEMCACHED){
            return memcachedConnectionIcon();
        }
        else if(type == SSDB){
            return ssdbConnectionIcon();
        }
        else if(type == LEVELDB){
            return leveldbConnectionIcon();
        }
        else if(type == ROCKSDB){
            return rocksdbConnectionIcon();
        }
        else if(type == UNQLITE){
            return unqliteConnectionIcon();
        }
        else if(type == LMDB){
            return lmdbConnectionIcon();
        }
        else{
            return serverIcon();
        }
    }

    const QIcon& GuiFactory::successIcon() const
    {
        static QIcon suc(":" PROJECT_NAME_LOWERCASE "/images/64x64/success.png");
        return suc;
    }

    const QIcon& GuiFactory::failIcon() const
    {
        static QIcon fail(":" PROJECT_NAME_LOWERCASE "/images/64x64/fail.png");
        return fail;
    }

    const QIcon& GuiFactory::unknownIcon() const
    {
        static QIcon unknown(":" PROJECT_NAME_LOWERCASE "/images/64x64/unknown.png");
        return unknown;
    }

    QFont GuiFactory::font() const
    {
        return QFont(SettingsManager::instance().currentFontName());
    }

    const QString &GuiFactory::pathToLoadingGif() const
    {
        static QString path(":" PROJECT_NAME_LOWERCASE "/images/loading.gif");
        return path;
    }

    const QIcon& GuiFactory::redisConnectionIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/redis.png");
        return main;
    }

    const QIcon& GuiFactory::memcachedConnectionIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/memcached.png");
        return main;
    }

    const QIcon& GuiFactory::ssdbConnectionIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/ssdb.png");
        return main;
    }

    const QIcon& GuiFactory::leveldbConnectionIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/leveldb.png");
        return main;
    }

    const QIcon& GuiFactory::rocksdbConnectionIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/rocksdb.png");
        return main;
    }

    const QIcon& GuiFactory::unqliteConnectionIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/unqlite.png");
        return main;
    }

    const QIcon& GuiFactory::lmdbConnectionIcon() const
    {
        static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/lmdb.png");
        return main;
    }
}
