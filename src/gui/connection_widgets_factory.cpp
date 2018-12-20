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

#include "gui/connection_widgets_factory.h"

#if defined(BUILD_WITH_REDIS)
#include "gui/db/redis/connection_widget.h"
#endif
#if defined(BUILD_WITH_MEMCACHED)
#include "gui/db/memcached/connection_widget.h"
#endif
#if defined(BUILD_WITH_SSDB)
#include "gui/db/ssdb/connection_widget.h"
#endif
#if defined(BUILD_WITH_LEVELDB)
#include "gui/db/leveldb/connection_widget.h"
#endif
#if defined(BUILD_WITH_ROCKSDB)
#include "gui/db/rocksdb/connection_widget.h"
#endif
#if defined(BUILD_WITH_UNQLITE)
#include "gui/db/unqlite/connection_widget.h"
#endif
#if defined(BUILD_WITH_LMDB)
#include "gui/db/lmdb/connection_widget.h"
#endif
#if defined(BUILD_WITH_UPSCALEDB)
#include "gui/db/upscaledb/connection_widget.h"
#endif
#if defined(BUILD_WITH_FORESTDB)
#include "gui/db/forestdb/connection_widget.h"
#endif
#if defined(BUILD_WITH_PIKA)
#include "gui/db/pika/connection_widget.h"
#endif
#if defined(BUILD_WITH_DYNOMITE)
#include "gui/db/dynomite/connection_widget.h"
#endif

namespace fastonosql {
namespace gui {
namespace {
ConnectionBaseWidget* createWidgetImpl(core::ConnectionType type, QWidget* parent) {
#if defined(BUILD_WITH_REDIS)
  if (type == core::REDIS) {
    return createWidget<redis::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (type == core::MEMCACHED) {
    return createWidget<memcached::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (type == core::SSDB) {
    return createWidget<ssdb::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_LEVELDB)
  if (type == core::LEVELDB) {
    return createWidget<leveldb::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_ROCKSDB)
  if (type == core::ROCKSDB) {
    return createWidget<rocksdb::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_UNQLITE)
  if (type == core::UNQLITE) {
    return createWidget<unqlite::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_LMDB)
  if (type == core::LMDB) {
    return createWidget<lmdb::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_UPSCALEDB)
  if (type == core::UPSCALEDB) {
    return createWidget<upscaledb::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_FORESTDB)
  if (type == core::FORESTDB) {
    return createWidget<forestdb::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (type == core::PIKA) {
    return createWidget<pika::ConnectionWidget>(parent);
  }
#endif
#if defined(BUILD_WITH_DYNOMITE)
  if (type == core::DYNOMITE) {
    return createWidget<dynomite::ConnectionWidget>(parent);
  }
#endif

  NOTREACHED() << "Not handled type: " << type;
  return nullptr;
}
}  // namespace

ConnectionBaseWidget* ConnectionWidgetsFactory::createWidget(proxy::IConnectionSettingsBase* connection,
                                                             QWidget* parent) {
  CHECK(connection);

  core::ConnectionType type = connection->GetType();
  ConnectionBaseWidget* widget = createWidgetImpl(type, parent);
  widget->syncControls(connection);
  return widget;
}

}  // namespace gui
}  // namespace fastonosql
