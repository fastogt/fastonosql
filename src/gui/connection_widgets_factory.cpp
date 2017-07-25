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

#include "gui/connection_widgets_factory.h"

#ifdef BUILD_WITH_REDIS
#include "gui/db/redis/connection_widget.h"
#endif
#ifdef BUILD_WITH_MEMCACHED
#include "gui/db/memcached/connection_widget.h"
#endif
#ifdef BUILD_WITH_SSDB
#include "gui/db/ssdb/connection_widget.h"
#endif
#ifdef BUILD_WITH_LEVELDB
#include "gui/db/leveldb/connection_widget.h"
#endif
#ifdef BUILD_WITH_ROCKSDB
#include "gui/db/rocksdb/connection_widget.h"
#endif
#ifdef BUILD_WITH_UNQLITE
#include "gui/db/unqlite/connection_widget.h"
#endif
#ifdef BUILD_WITH_LMDB
#include "gui/db/lmdb/connection_widget.h"
#endif
#ifdef BUILD_WITH_UPSCALEDB
#include "gui/db/upscaledb/connection_widget.h"
#endif
#ifdef BUILD_WITH_FORESTDB
#include "gui/db/forestdb/connection_widget.h"
#endif

namespace fastonosql {
namespace gui {
namespace {
ConnectionBaseWidget* createWidgetImpl(core::connectionTypes type, QWidget* parent) {
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    return new redis::ConnectionWidget(parent);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    return new memcached::ConnectionWidget(parent);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    return new ssdb::ConnectionWidget(parent);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    return new leveldb::ConnectionWidget(parent);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    return new rocksdb::ConnectionWidget(parent);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    return new unqlite::ConnectionWidget(parent);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    return new lmdb::ConnectionWidget(parent);
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == core::UPSCALEDB) {
    return new upscaledb::ConnectionWidget(parent);
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (type == core::FORESTDB) {
    return new forestdb::ConnectionWidget(parent);
  }
#endif
  NOTREACHED();
  return nullptr;
}
}  // namespace

ConnectionBaseWidget* ConnectionWidgetsFactory::createWidget(proxy::IConnectionSettingsBase* connection,
                                                             QWidget* parent) {
  if (!connection) {
    NOTREACHED();
    return nullptr;
  }

  core::connectionTypes type = connection->Type();
  ConnectionBaseWidget* widget = createWidgetImpl(type, parent);
  widget->syncControls(connection);
  widget->retranslateUi();
  return widget;
}

}  // namespace gui
}  // namespace fastonosql
