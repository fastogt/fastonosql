#include "gui/dialogs/info_server_dialog.h"

#include <QLabel>
#include <QHBoxLayout>

#include "gui/gui_factory.h"
#include "fasto/qt/gui/glass_widget.h"

#include "translations/global.h"

namespace
{
    const QString redisTextServerTemplate = QObject::tr("<h2>Server:</h2><br/>"
                                                  "Redis version: %1<br/>"
                                                  "Redis git_sha1: %2<br/>"
                                                  "Redis git_dirty: %3<br/>"
                                                  "Redis mode: %4<br/>"
                                                  "Os: %5<br/>"
                                                  "Arch: %6<br/>"
                                                  "Multiplexing Api: %7<br/>"
                                                  "Gcc version: %8<br/>"
                                                  "Process id: %9<br/>"
                                                  "Run id: %10<br/>"
                                                  "Tcp port: %11<br/>"
                                                  "Uptime sec: %12<br/>"
                                                  "Uptime days: %13<br/>"
                                                  "Hz: %14<br/>"
                                                  "Lru clock: %15");

    const QString redisTextClientsTemplate = QObject::tr("<h2>Clients:</h2><br/>"
                                                         "Connected clients_: %1<br/>"
                                                         "Client longest output list: %2<br/>"
                                                         "Client biggest input buf: %3<br/>"
                                                         "Blocked clients: %4");

    const QString redisTextMemoryTemplate = QObject::tr("<h2>Memory:</h2><br/>"
                                                  "Used memory: %1<br/>"
                                                  "Used memory human: %2<br/>"
                                                  "Used memory rss: %3<br/>"
                                                  "Used memory peak: %4<br/>"
                                                  "Used memory peak human: %5<br/>"
                                                  "Used memory lua: %6<br/>"
                                                  "Mem fragmentation ratio: %7<br/>"
                                                  "Mem allocator: %8");

    const QString redisTextPersistenceTemplate = QObject::tr("<h2>Persistence:</h2><br/>"
                                                  "Loading: %1<br/>"
                                                  "Rdb changes since last save: %2<br/>"
                                                  "Rdb bgsave in_progress: %3<br/>"
                                                  "Rdb last save_time: %4<br/>"
                                                  "Rdb last bgsave_status: %5<br/>"
                                                  "Rdb last bgsave time sec: %6<br/>"
                                                  "Rdb current bgsave time sec: %7<br/>"
                                                  "Aof enabled: %8<br/>"
                                                  "Aof rewrite in progress: %9<br/>"
                                                  "Aof rewrite scheduled: %10<br/>"
                                                  "Aof last rewrite time sec: %11<br/>"
                                                  "Aof current rewrite time sec: %12<br/>"
                                                  "Aof last bgrewrite status: %13<br/>"
                                                  "Aof last write status: %14");

    const QString redisTextStatsTemplate = QObject::tr("<h2>Stats:</h2><br/>"
                                                  "Total connections received: %1<br/>"
                                                  "Total commands processed: %2<br/>"
                                                  "Instantaneous ops per sec: %3<br/>"
                                                  "Rejected connections: %4<br/>"
                                                  "Sync full: %5<br/>"
                                                  "Sync partial ok: %6<br/>"
                                                  "Sync partial err: %7<br/>"
                                                  "Expired keys: %8<br/>"
                                                  "Evicted keys: %9<br/>"
                                                  "Keyspace hits: %10<br/>"
                                                  "Keyspace misses: %11<br/>"
                                                  "Pubsub channels: %12<br/>"
                                                  "Pubsub patterns: %13<br/>"
                                                  "Latest fork usec: %14");

    const QString redisTextReplicationTemplate = QObject::tr("<h2>Replication:</h2><br/>"
                                                   "Role: %1<br/>"
                                                   "Connected slaves: %2<br/>"
                                                   "Master reply offset: %3<br/>"
                                                   "Backlog active: %4<br/>"
                                                   "Backlog size: %5<br/>"
                                                   "Backlog first byte offset: %6<br/>"
                                                   "Backlog histen: %7");

    const QString redisTextCpuTemplate = QObject::tr("<h2>Cpu:</h2><br/>"
                                                         "Used cpu sys: %1<br/>"
                                                         "Used cpu user: %2<br/>"
                                                         "Used cpu sys children: %3<br/>"
                                                         "Used cpu user children: %4");

    const QString memcachedTextServerTemplate = QObject::tr("<h2>Common:</h2><br/>"
                                                            "Pid: %1<br/>"
                                                            "Update time: %2<br/>"
                                                            "Time: %3<br/>"
                                                            "Version: %4<br/>"
                                                            "Pointer size: %5<br/>"
                                                            "Usage user: %6<br/>"
                                                            "Usage system: %7<br/>"
                                                            "Current items: %8<br/>"
                                                            "Total items: %9<br/>"
                                                            "Bytes: %10<br/>"
                                                            "Current connections: %11<br/>"
                                                            "Total connections: %12<br/>"
                                                            "Connection structures: %13<br/>"
                                                            "Cmd get: %14<br/>"
                                                            "Cmd set: %15<br/>"
                                                            "Get hits: %16<br/>"
                                                            "Get misses: %17<br/>"
                                                            "Evictions: %18<br/>"
                                                            "Bytes read: %19<br/>"
                                                            "Bytes written: %20<br/>"
                                                            "Limit max bytes: %21<br/>"
                                                            "Threads: %22");

    const QString ssdbTextServerTemplate = QObject::tr("<h2>Common:</h2><br/>"
                                                            "Version: %1<br/>"
                                                            "Links: %2<br/>"
                                                            "Total calls: %3<br/>"
                                                            "Dbsize: %4<br/>"
                                                            "Binlogs: %5");
}

namespace fastonosql
{
    InfoServerDialog::InfoServerDialog(const QString& title, connectionTypes type, QWidget* parent)
        : QDialog(parent), type_(type)
    {
        using namespace translations;
        setWindowIcon(GuiFactory::instance().icon(type_));
        setWindowTitle(title);

        serverTextInfo_ = new QLabel;
        hardwareTextInfo_ = new QLabel;
        QHBoxLayout *mainL = new QHBoxLayout;
        mainL->addWidget(serverTextInfo_);
        mainL->addWidget(hardwareTextInfo_);
        setLayout(mainL);

        setMinimumSize(QSize(min_height, min_width));

        glassWidget_ = new fasto::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(), trLoading, 0.5, QColor(111, 111, 100), this);
#ifdef BUILD_WITH_REDIS
        if(type_ == REDIS){
            updateText(RedisServerInfo());
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type_ == MEMCACHED){
            updateText(MemcachedServerInfo());
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type_ == SSDB){
            updateText(SsdbServerInfo());
        }
#endif
    }

    void InfoServerDialog::startServerInfo(const EventsInfo::ServerInfoRequest& req)
    {
        glassWidget_->start();
    }

    void InfoServerDialog::finishServerInfo(const EventsInfo::ServerInfoResponce& res)
    {
        glassWidget_->stop();
        common::ErrorValueSPtr er = res.errorInfo();
        if(er && er->isError()){
            return;
        }

        ServerInfoSPtr inf = res.info();
        if(!inf){
            return;
        }

        DCHECK(type_ == inf->type());
#ifdef BUILD_WITH_REDIS
        if(type_ == REDIS){
            RedisServerInfo* infr = dynamic_cast<RedisServerInfo*>(inf.get());
            if(infr){
                updateText(*infr);
            }
        }
#endif
#ifdef BUILD_WITH_MEMCACHED
        if(type_ == MEMCACHED){
            MemcachedServerInfo* infr = dynamic_cast<MemcachedServerInfo*>(inf.get());
            if(infr){
                updateText(*infr);
            }
        }
#endif
#ifdef BUILD_WITH_SSDB
        if(type_ == SSDB){
            SsdbServerInfo* infr = dynamic_cast<SsdbServerInfo*>(inf.get());
            if(infr){
                updateText(*infr);
            }
        }
#endif
    }

    void InfoServerDialog::showEvent(QShowEvent* e)
    {
        QDialog::showEvent(e);
        emit showed();
    }

#ifdef BUILD_WITH_REDIS
    void InfoServerDialog::updateText(const RedisServerInfo& serv)
    {
        using namespace common;
        RedisServerInfo::Server ser = serv.server_;
        QString textServ = redisTextServerTemplate.arg(convertFromString<QString>(ser.redis_version_))
                .arg(convertFromString<QString>(ser.redis_git_sha1_))
                .arg(convertFromString<QString>(ser.redis_git_dirty_))
                .arg(convertFromString<QString>(ser.redis_mode_))
                .arg(convertFromString<QString>(ser.os_))
                .arg(ser.arch_bits_)
                .arg(convertFromString<QString>(ser.multiplexing_api_))
                .arg(convertFromString<QString>(ser.gcc_version_))
                .arg(ser.process_id_)
                .arg(convertFromString<QString>(ser.run_id_))
                .arg(ser.tcp_port_)
                .arg(ser.uptime_in_seconds_)
                .arg(ser.uptime_in_days_)
                .arg(ser.hz_)
                .arg(ser.lru_clock_);

        RedisServerInfo::Clients cl = serv.clients_;
        QString textCl = redisTextClientsTemplate.arg(cl.connected_clients_)
                .arg(cl.client_longest_output_list_)
                .arg(cl.client_biggest_input_buf_)
                .arg(cl.blocked_clients_);

        RedisServerInfo::Memory mem = serv.memory_;
        QString textMem = redisTextMemoryTemplate.arg(mem.used_memory_)
                .arg(convertFromString<QString>(mem.used_memory_human_))
                .arg(mem.used_memory_rss_)
                .arg(mem.used_memory_peak_)
                .arg(convertFromString<QString>(mem.used_memory_peak_human_))
                .arg(mem.used_memory_lua_)
                .arg(mem.mem_fragmentation_ratio_)
                .arg(convertFromString<QString>(mem.mem_allocator_));

        RedisServerInfo::Persistence per = serv.persistence_;
        QString textPer = redisTextPersistenceTemplate.arg(per.loading_)
                .arg(per.rdb_changes_since_last_save_)
                .arg(per.rdb_bgsave_in_progress_)
                .arg(per.rdb_last_save_time_)
                .arg(convertFromString<QString>(per.rdb_last_bgsave_status_))
                .arg(per.rdb_last_bgsave_time_sec_)
                .arg(per.rdb_current_bgsave_time_sec_)
                .arg(per.aof_enabled_)
                .arg(per.aof_rewrite_in_progress_)
                .arg(per.aof_rewrite_scheduled_)
                .arg(per.aof_last_rewrite_time_sec_)
                .arg(per.aof_current_rewrite_time_sec_)
                .arg(convertFromString<QString>(per.aof_last_bgrewrite_status_))
                .arg(convertFromString<QString>(per.aof_last_write_status_));

        RedisServerInfo::Stats stat = serv.stats_;
        QString textStat = redisTextStatsTemplate.arg(stat.total_connections_received_)
                .arg(stat.total_commands_processed_)
                .arg(stat.instantaneous_ops_per_sec_)
                .arg(stat.rejected_connections_)
                .arg(stat.sync_full_)
                .arg(stat.sync_partial_ok_)
                .arg(stat.sync_partial_err_)
                .arg(stat.expired_keys_)
                .arg(stat.evicted_keys_)
                .arg(stat.keyspace_hits_)
                .arg(stat.keyspace_misses_)
                .arg(stat.pubsub_channels_)
                .arg(stat.pubsub_patterns_)
                .arg(stat.latest_fork_usec_);

        RedisServerInfo::Replication repl = serv.replication_;
        QString textRepl = redisTextReplicationTemplate.arg(convertFromString<QString>(repl.role_))
                .arg(repl.connected_slaves_)
                .arg(repl.master_repl_offset_)
                .arg(repl.backlog_active_)
                .arg(repl.backlog_size_)
                .arg(repl.backlog_first_byte_offset_)
                .arg(repl.backlog_histen_);

        RedisServerInfo::Cpu cpu = serv.cpu_;
        QString textCpu = redisTextCpuTemplate.arg(cpu.used_cpu_sys_)
                .arg(cpu.used_cpu_user_)
                .arg(cpu.used_cpu_sys_children_)
                .arg(cpu.used_cpu_user_children_);

        serverTextInfo_->setText(textServ + textMem + textCpu);
        hardwareTextInfo_->setText(textCl + textPer + textStat + textRepl);
    }
#endif

#ifdef BUILD_WITH_MEMCACHED
    void InfoServerDialog::updateText(const MemcachedServerInfo& serv)
    {
        using namespace common;
        MemcachedServerInfo::Common com = serv.common_;

        QString textServ = memcachedTextServerTemplate.arg(com.pid_)
                .arg(com.uptime_)
                .arg(com.time_)
                .arg(convertFromString<QString>(com.version_))
                .arg(com.pointer_size_)
                .arg(com.rusage_user_)
                .arg(com.rusage_system_)
                .arg(com.curr_items_)
                .arg(com.total_items_)
                .arg(com.bytes_)
                .arg(com.curr_connections_)
                .arg(com.total_connections_)
                .arg(com.connection_structures_)
                .arg(com.cmd_get_)
                .arg(com.cmd_set_)
                .arg(com.get_hits_)
                .arg(com.get_misses_)
                .arg(com.evictions_)
                .arg(com.bytes_read_)
                .arg(com.bytes_written_)
                .arg(com.limit_maxbytes_)
                .arg(com.threads_);

        //QString textHard = memcachedTextHardwareTemplate;
        serverTextInfo_->setText(textServ);
        //hardwareTextInfo_->setText(textHard);
    }
#endif

#ifdef BUILD_WITH_SSDB
    void InfoServerDialog::updateText(const SsdbServerInfo& serv)
    {
        using namespace common;
        SsdbServerInfo::Common com = serv.common_;
        QString textServ = ssdbTextServerTemplate.arg(convertFromString<QString>(com.version_))
                .arg(com.links_)
                .arg(com.total_calls_)
                .arg(com.dbsize_)
                .arg(convertFromString<QString>(com.binlogs_));

        serverTextInfo_->setText(textServ);
    }
#endif
}
