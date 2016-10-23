/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoNoSQL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/dialogs/info_server_dialog.h"

#include <stddef.h>  // for offsetof
#include <string.h>  // for strcmp

#include <memory>  // for __shared_ptr

#include <QHBoxLayout>
#include <QTextEdit>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/error.h>           // for Error
#include <common/macros.h>          // for CHECK, VERIFY, UNUSED
#include <common/value.h>           // for ErrorValue

#ifdef BUILD_WITH_REDIS
#include "core/redis/server_info.h"
#endif

#ifdef BUILD_WITH_MEMCACHED
#include "core/memcached/server_info.h"
#endif

#ifdef BUILD_WITH_SSDB
#include "core/ssdb/server_info.h"
#endif

#ifdef BUILD_WITH_LEVELDB
#include "core/leveldb/server_info.h"
#endif

#ifdef BUILD_WITH_ROCKSDB
#include "core/rocksdb/server_info.h"
#endif

#ifdef BUILD_WITH_UNQLITE
#include "core/unqlite/server_info.h"
#endif

#ifdef BUILD_WITH_LMDB
#include "core/lmdb/server_info.h"
#endif

#include "core/connection_types.h"    // for connectionTypes, etc
#include "core/events/events_info.h"  // for ServerInfoResponce, etc
#include "core/server/iserver.h"      // for IServer

#include <common/qt/gui/glass_widget.h>  // for GlassWidget

#include "gui/gui_factory.h"  // for GuiFactory

#include "translations/global.h"  // for trLoading

namespace {
const QString trRedisTextServerTemplate = QObject::tr(
    "<h3>Server:</h3>"
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

const QString trRedisTextClientsTemplate = QObject::tr(
    "<h3>Clients:</h3>"
    "Connected clients_: %1<br/>"
    "Client longest output list: %2<br/>"
    "Client biggest input buf: %3<br/>"
    "Blocked clients: %4");

const QString trRedisTextMemoryTemplate = QObject::tr(
    "<h3>Memory:</h3>"
    "Used memory: %1<br/>"
    "Used memory human: %2<br/>"
    "Used memory rss: %3<br/>"
    "Used memory peak: %4<br/>"
    "Used memory peak human: %5<br/>"
    "Used memory lua: %6<br/>"
    "Mem fragmentation ratio: %7<br/>"
    "Mem allocator: %8");

const QString trRedisTextPersistenceTemplate = QObject::tr(
    "<h3>Persistence:</h3>"
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

const QString trRedisTextStatsTemplate = QObject::tr(
    "<h3>Stats:</h3>"
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

const QString trRedisTextReplicationTemplate = QObject::tr(
    "<h3>Replication:</h3>"
    "Role: %1<br/>"
    "Connected slaves: %2<br/>"
    "Master reply offset: %3<br/>"
    "Backlog active: %4<br/>"
    "Backlog size: %5<br/>"
    "Backlog first byte offset: %6<br/>"
    "Backlog histen: %7");

const QString trRedisTextCpuTemplate = QObject::tr(
    "<h3>Cpu:</h3>"
    "Used cpu sys: %1<br/>"
    "Used cpu user: %2<br/>"
    "Used cpu sys children: %3<br/>"
    "Used cpu user children: %4");

const QString trMemcachedTextServerTemplate = QObject::tr(
    "<b>Common:</b><br/>"
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

const QString trSsdbTextServerTemplate = QObject::tr(
    "<b>Common:</b><br/>"
    "Version: %1<br/>"
    "Links: %2<br/>"
    "Total calls: %3<br/>"
    "Dbsize: %4<br/>"
    "Binlogs: %5");

const QString trLeveldbTextServerTemplate = QObject::tr(
    "<b>Stats:</b><br/>"
    "Compactions level: %1<br/>"
    "File size mb: %2<br/>"
    "Time sec: %3<br/>"
    "Read mb: %4<br/>"
    "Write mb: %5");

const QString trRocksdbTextServerTemplate = QObject::tr(
    "<b>Stats:</b><br/>"
    "Compactions level: %1<br/>"
    "File size mb: %2<br/>"
    "Time sec: %3<br/>"
    "Read mb: %4<br/>"
    "Write mb: %5");

const QString trUnqliteTextServerTemplate = QObject::tr(
    "<b>Stats:</b><br/>"
    "File path: %1<br/>");

const QString trLmdbTextServerTemplate = QObject::tr(
    "<b>Stats:</b><br/>"
    "Db path: %1<br/>");
}  // namespace

namespace fastonosql {
namespace gui {
InfoServerDialog::InfoServerDialog(core::IServerSPtr server, QWidget* parent)
    : QDialog(parent), server_(server) {
  CHECK(server_);

  core::connectionTypes type = server->type();
  setWindowIcon(GuiFactory::instance().icon(type));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  // Remove help
                                                                     // button (?)

  serverTextInfo_ = new QTextEdit;
  serverTextInfo_->setReadOnly(true);
  QHBoxLayout* mainL = new QHBoxLayout;
  mainL->setContentsMargins(0, 0, 0, 0);
  mainL->addWidget(serverTextInfo_);

  setMinimumSize(QSize(min_width, min_height));
  setLayout(mainL);

  glassWidget_ =
      new common::qt::gui::GlassWidget(GuiFactory::instance().pathToLoadingGif(),
                                       translations::trLoading, 0.5, QColor(111, 111, 100), this);
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    updateText(core::redis::ServerInfo());
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    updateText(core::memcached::ServerInfo());
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    updateText(core::ssdb::ServerInfo());
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    updateText(core::leveldb::ServerInfo());
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    updateText(core::rocksdb::ServerInfo());
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    updateText(core::unqlite::ServerInfo());
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    updateText(core::lmdb::ServerInfo());
  }
#endif

  VERIFY(connect(server.get(), &core::IServer::startedLoadServerInfo, this,
                 &InfoServerDialog::startServerInfo));
  VERIFY(connect(server.get(), &core::IServer::finishedLoadServerInfo, this,
                 &InfoServerDialog::finishServerInfo));
  retranslateUi();
}

void InfoServerDialog::startServerInfo(const core::events_info::ServerInfoRequest& req) {
  UNUSED(req);

  glassWidget_->start();
}

void InfoServerDialog::finishServerInfo(const core::events_info::ServerInfoResponce& res) {
  glassWidget_->stop();
  common::Error er = res.errorInfo();
  if (er && er->isError()) {
    return;
  }

  core::IServerInfoSPtr inf = res.info();
  if (!inf) {
    return;
  }

  core::connectionTypes type = server_->type();
  CHECK(type == inf->type());
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    core::redis::ServerInfo* infr = static_cast<core::redis::ServerInfo*>(inf.get());
    CHECK(infr);
    updateText(*infr);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    core::memcached::ServerInfo* infr = static_cast<core::memcached::ServerInfo*>(inf.get());
    CHECK(infr);
    updateText(*infr);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    core::ssdb::ServerInfo* infr = static_cast<core::ssdb::ServerInfo*>(inf.get());
    CHECK(infr);
    updateText(*infr);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    core::leveldb::ServerInfo* infr = static_cast<core::leveldb::ServerInfo*>(inf.get());
    CHECK(infr);
    updateText(*infr);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    core::rocksdb::ServerInfo* infr = static_cast<core::rocksdb::ServerInfo*>(inf.get());
    CHECK(infr);
    updateText(*infr);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    core::unqlite::ServerInfo* infr = static_cast<core::unqlite::ServerInfo*>(inf.get());
    CHECK(infr);
    updateText(*infr);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    core::lmdb::ServerInfo* infr = static_cast<core::lmdb::ServerInfo*>(inf.get());
    CHECK(infr);
    updateText(*infr);
  }
#endif
}

void InfoServerDialog::showEvent(QShowEvent* e) {
  QDialog::showEvent(e);
  core::events_info::ServerInfoRequest req(this);
  server_->loadServerInfo(req);
}

void InfoServerDialog::changeEvent(QEvent* e) {
  if (e->type() == QEvent::LanguageChange) {
    retranslateUi();
  }
  QDialog::changeEvent(e);
}

void InfoServerDialog::retranslateUi() {
  QString name = common::ConvertFromString<QString>(server_->Name());
  setWindowTitle(tr("%1 info").arg(name));
}

#ifdef BUILD_WITH_REDIS
void InfoServerDialog::updateText(const core::redis::ServerInfo& serv) {
  core::redis::ServerInfo::Server ser = serv.server_;
  QString textServ = trRedisTextServerTemplate
                         .arg(common::ConvertFromString<QString>(ser.redis_version_),
                              common::ConvertFromString<QString>(ser.redis_git_sha1_),
                              common::ConvertFromString<QString>(ser.redis_git_dirty_),
                              common::ConvertFromString<QString>(ser.redis_mode_),
                              common::ConvertFromString<QString>(ser.os_))
                         .arg(ser.arch_bits_)
                         .arg(common::ConvertFromString<QString>(ser.multiplexing_api_),
                              common::ConvertFromString<QString>(ser.gcc_version_))
                         .arg(ser.process_id_)
                         .arg(common::ConvertFromString<QString>(ser.run_id_))
                         .arg(ser.tcp_port_)
                         .arg(ser.uptime_in_seconds_)
                         .arg(ser.uptime_in_days_)
                         .arg(ser.hz_)
                         .arg(ser.lru_clock_);

  core::redis::ServerInfo::Clients cl = serv.clients_;
  QString textCl = trRedisTextClientsTemplate.arg(cl.connected_clients_)
                       .arg(cl.client_longest_output_list_)
                       .arg(cl.client_biggest_input_buf_)
                       .arg(cl.blocked_clients_);

  core::redis::ServerInfo::Memory mem = serv.memory_;
  QString textMem = trRedisTextMemoryTemplate.arg(mem.used_memory_)
                        .arg(common::ConvertFromString<QString>(mem.used_memory_human_))
                        .arg(mem.used_memory_rss_)
                        .arg(mem.used_memory_peak_)
                        .arg(common::ConvertFromString<QString>(mem.used_memory_peak_human_))
                        .arg(mem.used_memory_lua_)
                        .arg(mem.mem_fragmentation_ratio_)
                        .arg(common::ConvertFromString<QString>(mem.mem_allocator_));

  core::redis::ServerInfo::Persistence per = serv.persistence_;
  QString textPer = trRedisTextPersistenceTemplate.arg(per.loading_)
                        .arg(per.rdb_changes_since_last_save_)
                        .arg(per.rdb_bgsave_in_progress_)
                        .arg(per.rdb_last_save_time_)
                        .arg(common::ConvertFromString<QString>(per.rdb_last_bgsave_status_))
                        .arg(per.rdb_last_bgsave_time_sec_)
                        .arg(per.rdb_current_bgsave_time_sec_)
                        .arg(per.aof_enabled_)
                        .arg(per.aof_rewrite_in_progress_)
                        .arg(per.aof_rewrite_scheduled_)
                        .arg(per.aof_last_rewrite_time_sec_)
                        .arg(per.aof_current_rewrite_time_sec_)
                        .arg(common::ConvertFromString<QString>(per.aof_last_bgrewrite_status_),
                             common::ConvertFromString<QString>(per.aof_last_write_status_));

  core::redis::ServerInfo::Stats stat = serv.stats_;
  QString textStat = trRedisTextStatsTemplate.arg(stat.total_connections_received_)
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

  core::redis::ServerInfo::Replication repl = serv.replication_;
  QString textRepl =
      trRedisTextReplicationTemplate.arg(common::ConvertFromString<QString>(repl.role_))
          .arg(repl.connected_slaves_)
          .arg(repl.master_repl_offset_)
          .arg(repl.backlog_active_)
          .arg(repl.backlog_size_)
          .arg(repl.backlog_first_byte_offset_)
          .arg(repl.backlog_histen_);

  core::redis::ServerInfo::Cpu cpu = serv.cpu_;
  QString textCpu = trRedisTextCpuTemplate.arg(cpu.used_cpu_sys_)
                        .arg(cpu.used_cpu_user_)
                        .arg(cpu.used_cpu_sys_children_)
                        .arg(cpu.used_cpu_user_children_);
  serverTextInfo_->setText(textServ + textMem + textCpu + textCl + textPer + textStat + textRepl);
}
#endif

#ifdef BUILD_WITH_MEMCACHED
void InfoServerDialog::updateText(const core::memcached::ServerInfo& serv) {
  core::memcached::ServerInfo::Stats com = serv.stats_;

  QString textServ = trMemcachedTextServerTemplate.arg(com.pid)
                         .arg(com.uptime)
                         .arg(com.time)
                         .arg(common::ConvertFromString<QString>(com.version))
                         .arg(com.pointer_size)
                         .arg(com.rusage_user)
                         .arg(com.rusage_system)
                         .arg(com.curr_items)
                         .arg(com.total_items)
                         .arg(com.bytes)
                         .arg(com.curr_connections)
                         .arg(com.total_connections)
                         .arg(com.connection_structures)
                         .arg(com.cmd_get)
                         .arg(com.cmd_set)
                         .arg(com.get_hits)
                         .arg(com.get_misses)
                         .arg(com.evictions)
                         .arg(com.bytes_read)
                         .arg(com.bytes_written)
                         .arg(com.limit_maxbytes)
                         .arg(com.threads);

  serverTextInfo_->setText(textServ);
}
#endif

#ifdef BUILD_WITH_SSDB
void InfoServerDialog::updateText(const core::ssdb::ServerInfo& serv) {
  core::ssdb::ServerInfo::Stats com = serv.stats_;
  QString textServ = trSsdbTextServerTemplate.arg(common::ConvertFromString<QString>(com.version))
                         .arg(com.links)
                         .arg(com.total_calls)
                         .arg(com.dbsize)
                         .arg(common::ConvertFromString<QString>(com.binlogs));

  serverTextInfo_->setText(textServ);
}
#endif

#ifdef BUILD_WITH_LEVELDB
void InfoServerDialog::updateText(const core::leveldb::ServerInfo& serv) {
  core::leveldb::ServerInfo::Stats stats = serv.stats_;
  QString textServ = trLeveldbTextServerTemplate.arg(stats.compactions_level)
                         .arg(stats.file_size_mb)
                         .arg(stats.time_sec)
                         .arg(stats.read_mb)
                         .arg(stats.write_mb);

  serverTextInfo_->setText(textServ);
}
#endif
#ifdef BUILD_WITH_ROCKSDB
void InfoServerDialog::updateText(const core::rocksdb::ServerInfo& serv) {
  core::rocksdb::ServerInfo::Stats stats = serv.stats_;
  QString textServ = trRocksdbTextServerTemplate.arg(stats.compactions_level)
                         .arg(stats.file_size_mb)
                         .arg(stats.time_sec)
                         .arg(stats.read_mb)
                         .arg(stats.write_mb);

  serverTextInfo_->setText(textServ);
}
#endif
#ifdef BUILD_WITH_UNQLITE
void InfoServerDialog::updateText(const core::unqlite::ServerInfo& serv) {
  core::unqlite::ServerInfo::Stats stats = serv.stats_;
  QString textServ =
      trUnqliteTextServerTemplate.arg(common::ConvertFromString<QString>(stats.file_name));

  serverTextInfo_->setText(textServ);
}
#endif
#ifdef BUILD_WITH_LMDB
void InfoServerDialog::updateText(const core::lmdb::ServerInfo& serv) {
  core::lmdb::ServerInfo::Stats stats = serv.stats_;
  QString textServ =
      trLmdbTextServerTemplate.arg(common::ConvertFromString<QString>(stats.db_path));

  serverTextInfo_->setText(textServ);
}
#endif
}  // namespace gui
}  // namespace fastonosql
