/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#pragma once

#include <QDialog>

#include "proxy/proxy_fwd.h"

class QTextEdit;

namespace common {
namespace qt {
namespace gui {
class GlassWidget;
}
}  // namespace qt
}  // namespace common

namespace fastonosql {
namespace core {
namespace leveldb {
class ServerInfo;
}
namespace lmdb {
class ServerInfo;
}
namespace memcached {
class ServerInfo;
}
namespace redis {
class ServerInfo;
}
namespace rocksdb {
class ServerInfo;
}
namespace ssdb {
class ServerInfo;
}
namespace unqlite {
class ServerInfo;
}
namespace upscaledb {
class ServerInfo;
}
namespace forestdb {
class ServerInfo;
}
namespace pika {
class ServerInfo;
}
}  // namespace core
}  // namespace fastonosql

namespace fastonosql {
namespace proxy {
namespace events_info {
class ServerInfoResponce;
struct ServerInfoRequest;
}  // namespace events_info
}  // namespace proxy
namespace gui {

class InfoServerDialog : public QDialog {
  Q_OBJECT
 public:
  explicit InfoServerDialog(proxy::IServerSPtr server, QWidget* parent = Q_NULLPTR);
  enum { min_width = 420, min_height = 560 };

 private Q_SLOTS:
  void startServerInfo(const proxy::events_info::ServerInfoRequest& req);
  void finishServerInfo(const proxy::events_info::ServerInfoResponce& res);

 protected:
  virtual void changeEvent(QEvent* e) override;
  virtual void showEvent(QShowEvent* e) override;

 private:
  void retranslateUi();
#ifdef BUILD_WITH_REDIS
  void updateText(const core::redis::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_MEMCACHED
  void updateText(const core::memcached::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_SSDB
  void updateText(const core::ssdb::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_LEVELDB
  void updateText(const core::leveldb::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_ROCKSDB
  void updateText(const core::rocksdb::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_UNQLITE
  void updateText(const core::unqlite::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_LMDB
  void updateText(const core::lmdb::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_UPSCALEDB
  void updateText(const core::upscaledb::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_FORESTDB
  void updateText(const core::forestdb::ServerInfo& serv);
#endif
#ifdef BUILD_WITH_PIKA
  void updateText(const core::pika::ServerInfo& serv);
#endif
  QTextEdit* server_text_info_;
  common::qt::gui::GlassWidget* glass_widget_;
  const proxy::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
