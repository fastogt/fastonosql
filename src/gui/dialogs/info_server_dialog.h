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

#pragma  once

#include <QDialog>

#include "core/core_fwd.h"
#include "core/events/events_info.h"

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

class QLabel;

namespace fasto {
namespace qt {
namespace gui {
class GlassWidget;
}
}
}

namespace fastonosql {
namespace gui {

class InfoServerDialog
  : public QDialog {
  Q_OBJECT
 public:
  explicit InfoServerDialog(core::IServerSPtr server, QWidget* parent = 0);
  enum {
    min_height = 320,
    min_width = 240
  };

 Q_SIGNALS:
  void showed();

 private Q_SLOTS:
  void startServerInfo(const core::events_info::ServerInfoRequest& req);
  void finishServerInfo(const core::events_info::ServerInfoResponce& res);

 protected:
  virtual void changeEvent(QEvent* e);
  virtual void showEvent(QShowEvent* e);

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
  QLabel* serverTextInfo_;
  QLabel* hardwareTextInfo_;
  fasto::qt::gui::GlassWidget* glassWidget_;
  const core::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
