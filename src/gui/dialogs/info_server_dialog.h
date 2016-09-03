/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    FastoNoSQL is free software: you can redistribute it
   and/or modify
    it under the terms of the GNU General Public License as
   published by
    the Free Software Foundation, either version 3 of the
   License, or
    (at your option) any later version.

    FastoNoSQL is distributed in the hope that it will be
   useful,
    but WITHOUT ANY WARRANTY; without even the implied
   warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General
   Public License
    along with FastoNoSQL.  If not, see
   <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QDialog>

#include "core/core_fwd.h"

class QTextEdit;

namespace fasto {
namespace qt {
namespace gui {
class GlassWidget;
}
}
}  // lines 59-59
namespace fastonosql {
namespace core {
namespace events_info {
class ServerInfoResponce;
}
}
}
namespace fastonosql {
namespace core {
namespace events_info {
struct ServerInfoRequest;
}
}
}

namespace fastonosql {
namespace core {
namespace leveldb {
class ServerInfo;
}
}
}
namespace fastonosql {
namespace core {
namespace lmdb {
class ServerInfo;
}
}
}
namespace fastonosql {
namespace core {
namespace memcached {
class ServerInfo;
}
}
}
namespace fastonosql {
namespace core {
namespace redis {
struct ServerInfo;
}
}
}
namespace fastonosql {
namespace core {
namespace rocksdb {
class ServerInfo;
}
}
}
namespace fastonosql {
namespace core {
namespace ssdb {
class ServerInfo;
}
}
}
namespace fastonosql {
namespace core {
namespace unqlite {
class ServerInfo;
}
}
}

namespace fastonosql {
namespace gui {

class InfoServerDialog : public QDialog {
  Q_OBJECT
 public:
  explicit InfoServerDialog(core::IServerSPtr server, QWidget* parent = 0);
  enum { min_width = 420, min_height = 560 };

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
  QTextEdit* serverTextInfo_;
  fasto::qt::gui::GlassWidget* glassWidget_;
  const core::IServerSPtr server_;
};

}  // namespace gui
}  // namespace fastonosql
