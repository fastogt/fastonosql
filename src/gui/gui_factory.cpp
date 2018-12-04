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

#include "gui/gui_factory.h"

#include <string>

#include <QApplication>
#include <QIcon>
#include <QStyle>

#include <common/file_system/string_path_utils.h>
#include <common/qt/convert2string.h>

#include <fastonosql/core/value.h>

#include "proxy/settings_manager.h"  // for SettingsManager

#define CONNECT_GIF_PATH_RELATIVE "share/resources/help/connect.gif"
#define INDIVIDUAL_BUILDS_GIF_PATH_RELATIVE "share/resources/help/individual_builds.gif"
#define WORKFLOW_GIF_PATH_RELATIVE "share/resources/help/workflow.gif"

namespace fastonosql {
namespace gui {

GuiFactory::GuiFactory() {}

const QIcon& GuiFactory::directoryIcon() const {
  static QIcon open = qApp->style()->standardIcon(QStyle::SP_DirIcon);
  return open;
}

const QIcon& GuiFactory::connectDBIcon() const {
  static QIcon servers(":" PROJECT_NAME_LOWERCASE "/images/64x64/connect_db.png");
  return servers;
}

const QIcon& GuiFactory::openIcon() const {
  static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/open.png");
  return open;
}

const QIcon& GuiFactory::logoIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/logo.png");
  return main;
}

const QIcon& GuiFactory::mainWindowIcon() const {
  return logoIcon();
}

const QIcon& GuiFactory::connectIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/connect.png");
  return main;
}

const QIcon& GuiFactory::disConnectIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/disconnect.png");
  return main;
}

const QIcon& GuiFactory::serverIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/server.png");
  return main;
}

const QIcon& GuiFactory::addIcon() const {
  static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/add.png");
  return open;
}

const QIcon& GuiFactory::removeIcon() const {
  static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/remove.png");
  return open;
}

const QIcon& GuiFactory::editIcon() const {
  static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/edit.png");
  return open;
}

const QIcon& GuiFactory::cloneIcon() const {
  static QIcon clone(":" PROJECT_NAME_LOWERCASE "/images/64x64/clone.png");
  return clone;
}

const QIcon& GuiFactory::messageBoxInformationIcon() const {
  static QIcon open = qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation);
  return open;
}

const QIcon& GuiFactory::messageBoxQuestionIcon() const {
  static QIcon open = qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion);
  return open;
}

const QIcon& GuiFactory::validateIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/validate.png");
  return start;
}

const QIcon& GuiFactory::executeIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/execute.png");
  return start;
}

const QIcon& GuiFactory::helpIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/help.png");
  return start;
}

const QIcon& GuiFactory::timeIcon() const {
  static QIcon time(":" PROJECT_NAME_LOWERCASE "/images/64x64/time.png");
  return time;
}

const QIcon& GuiFactory::stopIcon() const {
  static QIcon stop(":" PROJECT_NAME_LOWERCASE "/images/64x64/stop.png");
  return stop;
}

const QIcon& GuiFactory::databaseIcon() const {
  static QIcon db(":" PROJECT_NAME_LOWERCASE "/images/64x64/database.png");
  return db;
}

const QIcon& GuiFactory::moduleIcon() const {
  static QIcon md(":" PROJECT_NAME_LOWERCASE "/images/64x64/module.png");
  return md;
}

const QIcon& GuiFactory::keyIcon() const {
  static QIcon db(":" PROJECT_NAME_LOWERCASE "/images/64x64/key.png");
  return db;
}

const QIcon& GuiFactory::keyTTLIcon() const {
  static QIcon db(":" PROJECT_NAME_LOWERCASE "/images/64x64/key_ttl.png");
  return db;
}

const QIcon& GuiFactory::icon(core::ConnectionType type) const {
#if defined(BUILD_WITH_REDIS)
  if (type == core::REDIS) {
    static QIcon redis(":" PROJECT_NAME_LOWERCASE "/images/64x64/redis.png");
    return redis;
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (type == core::MEMCACHED) {
    static QIcon mem(":" PROJECT_NAME_LOWERCASE "/images/64x64/memcached.png");
    return mem;
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (type == core::SSDB) {
    static QIcon ssdb(":" PROJECT_NAME_LOWERCASE "/images/64x64/ssdb.png");
    return ssdb;
  }
#endif
#if defined(BUILD_WITH_LEVELDB)
  if (type == core::LEVELDB) {
    static QIcon level(":" PROJECT_NAME_LOWERCASE "/images/64x64/leveldb.png");
    return level;
  }
#endif
#if defined(BUILD_WITH_ROCKSDB)
  if (type == core::ROCKSDB) {
    static QIcon rock(":" PROJECT_NAME_LOWERCASE "/images/64x64/rocksdb.png");
    return rock;
  }
#endif
#if defined(BUILD_WITH_UNQLITE)
  if (type == core::UNQLITE) {
    static QIcon unq(":" PROJECT_NAME_LOWERCASE "/images/64x64/unqlite.png");
    return unq;
  }
#endif
#if defined(BUILD_WITH_LMDB)
  if (type == core::LMDB) {
    static QIcon lmdb(":" PROJECT_NAME_LOWERCASE "/images/64x64/lmdb.png");
    return lmdb;
  }
#endif
#if defined(BUILD_WITH_UPSCALEDB)
  if (type == core::UPSCALEDB) {
    static QIcon ups(":" PROJECT_NAME_LOWERCASE "/images/64x64/upscaledb.png");
    return ups;
  }
#endif
#if defined(BUILD_WITH_FORESTDB)
  if (type == core::FORESTDB) {
    static QIcon forestdb(":" PROJECT_NAME_LOWERCASE "/images/64x64/forestdb.png");
    return forestdb;
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (type == core::PIKA) {
    static QIcon pika(":" PROJECT_NAME_LOWERCASE "/images/64x64/pika.png");
    return pika;
  }
#endif
#if defined(BUILD_WITH_DYNOMITEDB)
  if (type == core::DYNOMITEDB) {
    static QIcon dynomitedb(":" PROJECT_NAME_LOWERCASE "/images/64x64/dynomitedb.png");
    return dynomitedb;
  }
#endif

  NOTREACHED() << "Unhandled type: " << type;
  return serverIcon();
}

const QIcon& GuiFactory::modeIcon(core::ConnectionMode mode) const {
  if (mode == core::InteractiveMode) {
    static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/interactive_mode.png");
    return i;
  }

  static QIcon err(":" PROJECT_NAME_LOWERCASE "/images/64x64/error.png");
  DNOTREACHED();
  return err;
}

const QIcon& GuiFactory::icon(common::Value::Type type) const {
  const uint8_t ctype = type;
  switch (ctype) {
    case common::Value::TYPE_NULL: {
      static QIcon u(":" PROJECT_NAME_LOWERCASE "/images/64x64/null.png");
      return u;
    }
    case common::Value::TYPE_BOOLEAN: {
      static QIcon b(":" PROJECT_NAME_LOWERCASE "/images/64x64/bool.png");
      return b;
    }
    case common::Value::TYPE_STRING: {
      static QIcon s(":" PROJECT_NAME_LOWERCASE "/images/64x64/string.png");
      return s;
    }
    case common::Value::TYPE_BYTE_ARRAY: {
      static QIcon by(":" PROJECT_NAME_LOWERCASE "/images/64x64/byte.png");
      return by;
    }
    case common::Value::TYPE_SET:
    case common::Value::TYPE_ARRAY: {
      static QIcon a(":" PROJECT_NAME_LOWERCASE "/images/64x64/array.png");
      return a;
    }
    case common::Value::TYPE_HASH: {
      static QIcon h(":" PROJECT_NAME_LOWERCASE "/images/64x64/hash.png");
      return h;
    }
    case common::Value::TYPE_ZSET: {
      static QIcon z(":" PROJECT_NAME_LOWERCASE "/images/64x64/zset.png");
      return z;
    }
    case common::Value::TYPE_INTEGER:
    case common::Value::TYPE_UINTEGER:
    case common::Value::TYPE_LONG_INTEGER:
    case common::Value::TYPE_ULONG_INTEGER:
    case common::Value::TYPE_LONG_LONG_INTEGER:
    case common::Value::TYPE_ULONG_LONG_INTEGER:
    case common::Value::TYPE_DOUBLE: {
      static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/integer.png");
      return i;
    }
    case core::StreamValue::TYPE_STREAM: {
      static QIcon g(":" PROJECT_NAME_LOWERCASE "/images/64x64/stream.png");
      return g;
    }
    case core::JsonValue::TYPE_JSON: {
      static QIcon j(":" PROJECT_NAME_LOWERCASE "/images/64x64/json.png");
      return j;
    }
    case core::GraphValue::TYPE_GRAPH: {
      static QIcon g(":" PROJECT_NAME_LOWERCASE "/images/64x64/graph.png");
      return g;
    }
    case core::BloomValue::TYPE_BLOOM: {
      static QIcon g(":" PROJECT_NAME_LOWERCASE "/images/64x64/bloom.png");
      return g;
    }
    case core::SearchValue::TYPE_FT_INDEX: {
      static QIcon g(":" PROJECT_NAME_LOWERCASE "/images/64x64/index.png");
      return g;
    }
    case core::SearchValue::TYPE_FT_TERM: {
      static QIcon g(":" PROJECT_NAME_LOWERCASE "/images/64x64/index.png");
      return g;
    }
  }

  static QIcon err(":" PROJECT_NAME_LOWERCASE "/images/64x64/error.png");
  DNOTREACHED();
  return err;
}

const QIcon& GuiFactory::importIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/import.png");
  return start;
}

const QIcon& GuiFactory::exportIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/export.png");
  return start;
}

const QIcon& GuiFactory::loadIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/load.png");
  return start;
}

const QIcon& GuiFactory::clusterIcon() const {
  static QIcon cluster(":" PROJECT_NAME_LOWERCASE "/images/64x64/cluster.png");
  return cluster;
}

const QIcon& GuiFactory::sentinelIcon() const {
  static QIcon sentinel(":" PROJECT_NAME_LOWERCASE "/images/64x64/sentinel.png");
  return sentinel;
}

const QIcon& GuiFactory::saveIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/save.png");
  return start;
}

const QIcon& GuiFactory::saveAsIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/saveas.png");
  return start;
}

const QIcon& GuiFactory::textIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/text.png");
  return start;
}

const QIcon& GuiFactory::tableIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/table.png");
  return start;
}

const QIcon& GuiFactory::treeIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/tree.png");
  return start;
}

const QIcon& GuiFactory::loggingIcon() const {
  static QIcon logg(":" PROJECT_NAME_LOWERCASE "/images/64x64/logging.png");
  return logg;
}

const QIcon& GuiFactory::discoveryIcon() const {
  static QIcon discovery(":" PROJECT_NAME_LOWERCASE "/images/64x64/discovery.png");
  return discovery;
}

const QIcon& GuiFactory::channelIcon() const {
  static QIcon channel(":" PROJECT_NAME_LOWERCASE "/images/64x64/channel.png");
  return channel;
}

const QIcon& GuiFactory::commandIcon() const {
  static QIcon comm(":" PROJECT_NAME_LOWERCASE "/images/64x64/command.png");
  return comm;
}

const QIcon& GuiFactory::encodeDecodeIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/encode_decode.png");
  return main;
}

const QIcon& GuiFactory::preferencesIcon() const {
  static QIcon pref(":" PROJECT_NAME_LOWERCASE "/images/64x64/preferences.png");
  return pref;
}

const QIcon& GuiFactory::leftIcon() const {
  static QIcon left = qApp->style()->standardIcon(QStyle::SP_ArrowLeft);
  return left;
}

const QIcon& GuiFactory::rightIcon() const {
  static QIcon right = qApp->style()->standardIcon(QStyle::SP_ArrowRight);
  return right;
}

const QIcon& GuiFactory::close16Icon() const {
  static QIcon close(":" PROJECT_NAME_LOWERCASE "/images/16x16/close.png");
  return close;
}

const QIcon& GuiFactory::search16Icon() const {
  static QIcon search(":" PROJECT_NAME_LOWERCASE "/images/16x16/search.png");
  return search;
}

const QIcon& GuiFactory::commandIcon(core::ConnectionType type) const {
  return icon(type);
}

const QIcon& GuiFactory::successIcon() const {
  static QIcon suc(":" PROJECT_NAME_LOWERCASE "/images/64x64/success.png");
  return suc;
}

const QIcon& GuiFactory::failIcon() const {
  static QIcon fail(":" PROJECT_NAME_LOWERCASE "/images/64x64/fail.png");
  return fail;
}

const QIcon& GuiFactory::unknownIcon() const {
  static QIcon unknown(":" PROJECT_NAME_LOWERCASE "/images/64x64/unknown.png");
  return unknown;
}

const QIcon& GuiFactory::infoIcon() const {
  static QIcon unknown(":" PROJECT_NAME_LOWERCASE "/images/64x64/info.png");
  return unknown;
}

QFont GuiFactory::font() const {
  return proxy::SettingsManager::GetInstance()->GetCurrentFont();
}

const QString& GuiFactory::pathToLoadingGif() const {
  static QString path(":" PROJECT_NAME_LOWERCASE "/images/loading.gif");
  return path;
}

QString GuiFactory::pathToIndividualBuilds() const {
  const std::string absolute_source_dir = common::file_system::absolute_path_from_relative(RELATIVE_SOURCE_DIR);
  const std::string img_full_path =
      common::file_system::make_path(absolute_source_dir, INDIVIDUAL_BUILDS_GIF_PATH_RELATIVE);
  QString path;
  common::ConvertFromString(img_full_path, &path);
  return path;
}

QString GuiFactory::pathToConnectGif() const {
  const std::string absolute_source_dir = common::file_system::absolute_path_from_relative(RELATIVE_SOURCE_DIR);
  const std::string img_full_path = common::file_system::make_path(absolute_source_dir, CONNECT_GIF_PATH_RELATIVE);
  QString path;
  common::ConvertFromString(img_full_path, &path);
  return path;
}

QString GuiFactory::pathToWorkflowGif() const {
  const std::string absolute_source_dir = common::file_system::absolute_path_from_relative(RELATIVE_SOURCE_DIR);
  const std::string img_full_path = common::file_system::make_path(absolute_source_dir, WORKFLOW_GIF_PATH_RELATIVE);
  QString path;
  common::ConvertFromString(img_full_path, &path);
  return path;
}

}  // namespace gui
}  // namespace fastonosql
