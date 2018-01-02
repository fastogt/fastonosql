/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <QApplication>
#include <QIcon>
#include <QStyle>

#include "core/value.h"

#include "proxy/settings_manager.h"  // for SettingsManager

namespace fastonosql {
namespace gui {

GuiFactory::GuiFactory() {}

const QIcon& GuiFactory::GetDirectoryIcon() const {
  static QIcon open = qApp->style()->standardIcon(QStyle::SP_DirIcon);
  return open;
}

const QIcon& GuiFactory::GetConnectDBIcon() const {
  static QIcon servers(":" PROJECT_NAME_LOWERCASE "/images/64x64/connect_db.png");
  return servers;
}

const QIcon& GuiFactory::GetOpenIcon() const {
  static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/open.png");
  return open;
}

const QIcon& GuiFactory::GetLogoIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/logo.png");
  return main;
}

const QIcon& GuiFactory::GetMainWindowIcon() const {
  return GetLogoIcon();
}

const QIcon& GuiFactory::GetConnectIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/connect.png");
  return main;
}

const QIcon& GuiFactory::GetDisConnectIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/disconnect.png");
  return main;
}

const QIcon& GuiFactory::GetServerIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/server.png");
  return main;
}

const QIcon& GuiFactory::GetAddIcon() const {
  static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/add.png");
  return open;
}

const QIcon& GuiFactory::GetRemoveIcon() const {
  static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/remove.png");
  return open;
}

const QIcon& GuiFactory::GetEditIcon() const {
  static QIcon open(":" PROJECT_NAME_LOWERCASE "/images/64x64/edit.png");
  return open;
}

const QIcon& GuiFactory::GetCloneIcon() const {
  static QIcon clone(":" PROJECT_NAME_LOWERCASE "/images/64x64/clone.png");
  return clone;
}

const QIcon& GuiFactory::GetMessageBoxInformationIcon() const {
  static QIcon open = qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation);
  return open;
}

const QIcon& GuiFactory::GetMessageBoxQuestionIcon() const {
  static QIcon open = qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion);
  return open;
}

const QIcon& GuiFactory::GetValidateIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/validate.png");
  return start;
}

const QIcon& GuiFactory::GetExecuteIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/execute.png");
  return start;
}

const QIcon& GuiFactory::GetHelpIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/help.png");
  return start;
}

const QIcon& GuiFactory::GetTimeIcon() const {
  static QIcon time(":" PROJECT_NAME_LOWERCASE "/images/64x64/time.png");
  return time;
}

const QIcon& GuiFactory::stopIcon() const {
  static QIcon stop(":" PROJECT_NAME_LOWERCASE "/images/64x64/stop.png");
  return stop;
}

const QIcon& GuiFactory::GetDatabaseIcon() const {
  static QIcon db(":" PROJECT_NAME_LOWERCASE "/images/64x64/database.png");
  return db;
}

const QIcon& GuiFactory::GetModuleIcon() const {
  static QIcon md(":" PROJECT_NAME_LOWERCASE "/images/64x64/module.png");
  return md;
}

const QIcon& GuiFactory::GetKeyIcon() const {
  static QIcon db(":" PROJECT_NAME_LOWERCASE "/images/64x64/key.png");
  return db;
}

const QIcon& GuiFactory::GetKeyTTLIcon() const {
  static QIcon db(":" PROJECT_NAME_LOWERCASE "/images/64x64/key_ttl.png");
  return db;
}

const QIcon& GuiFactory::GetIcon(core::connectionTypes type) const {
  if (type == core::REDIS) {
    return redisConnectionIcon();
  } else if (type == core::MEMCACHED) {
    return memcachedConnectionIcon();
  } else if (type == core::SSDB) {
    return ssdbConnectionIcon();
  } else if (type == core::LEVELDB) {
    return leveldbConnectionIcon();
  } else if (type == core::ROCKSDB) {
    return rocksdbConnectionIcon();
  } else if (type == core::UNQLITE) {
    return unqliteConnectionIcon();
  } else if (type == core::LMDB) {
    return lmdbConnectionIcon();
  } else if (type == core::UPSCALEDB) {
    return upscaledbConnectionIcon();
  } else if (type == core::FORESTDB) {
    return forestdbConnectionIcon();
  } else {
    return GetServerIcon();
  }
}

const QIcon& GuiFactory::GetModeIcon(core::ConnectionMode mode) const {
  if (mode == core::InteractiveMode) {
    static QIcon i(":" PROJECT_NAME_LOWERCASE "/images/64x64/interactive_mode.png");
    return i;
  }

  static QIcon err(":" PROJECT_NAME_LOWERCASE "/images/64x64/error.png");
  DNOTREACHED();
  return err;
}

const QIcon& GuiFactory::GetIcon(common::Value::Type type) const {
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

const QIcon& GuiFactory::GetImportIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/import.png");
  return start;
}

const QIcon& GuiFactory::GetExportIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/export.png");
  return start;
}

const QIcon& GuiFactory::GetLoadIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/load.png");
  return start;
}

const QIcon& GuiFactory::GetClusterIcon() const {
  static QIcon cluster(":" PROJECT_NAME_LOWERCASE "/images/64x64/cluster.png");
  return cluster;
}

const QIcon& GuiFactory::GetSentinelIcon() const {
  static QIcon sentinel(":" PROJECT_NAME_LOWERCASE "/images/64x64/sentinel.png");
  return sentinel;
}

const QIcon& GuiFactory::GetSaveIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/save.png");
  return start;
}

const QIcon& GuiFactory::GetSaveAsIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/saveas.png");
  return start;
}

const QIcon& GuiFactory::GetTextIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/text.png");
  return start;
}

const QIcon& GuiFactory::GetTableIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/table.png");
  return start;
}

const QIcon& GuiFactory::GetTreeIcon() const {
  static QIcon start(":" PROJECT_NAME_LOWERCASE "/images/64x64/tree.png");
  return start;
}

const QIcon& GuiFactory::GetLoggingIcon() const {
  static QIcon logg(":" PROJECT_NAME_LOWERCASE "/images/64x64/logging.png");
  return logg;
}

const QIcon& GuiFactory::GetDiscoveryIcon() const {
  static QIcon discovery(":" PROJECT_NAME_LOWERCASE "/images/64x64/discovery.png");
  return discovery;
}

const QIcon& GuiFactory::GetChannelIcon() const {
  static QIcon channel(":" PROJECT_NAME_LOWERCASE "/images/64x64/channel.png");
  return channel;
}

const QIcon& GuiFactory::GetCommandIcon() const {
  static QIcon comm(":" PROJECT_NAME_LOWERCASE "/images/64x64/command.png");
  return comm;
}

const QIcon& GuiFactory::GetEncodeDecodeIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/encode_decode.png");
  return main;
}

const QIcon& GuiFactory::GetPreferencesIcon() const {
  static QIcon pref(":" PROJECT_NAME_LOWERCASE "/images/64x64/preferences.png");
  return pref;
}

const QIcon& GuiFactory::GetLeftIcon() const {
  static QIcon left = qApp->style()->standardIcon(QStyle::SP_ArrowLeft);
  return left;
}

const QIcon& GuiFactory::GetRightIcon() const {
  static QIcon right = qApp->style()->standardIcon(QStyle::SP_ArrowRight);
  return right;
}

const QIcon& GuiFactory::GetClose16Icon() const {
  static QIcon close(":" PROJECT_NAME_LOWERCASE "/images/16x16/close.png");
  return close;
}

const QIcon& GuiFactory::GetSearch16Icon() const {
  static QIcon search(":" PROJECT_NAME_LOWERCASE "/images/16x16/search.png");
  return search;
}

const QIcon& GuiFactory::GetCommandIcon(core::connectionTypes type) const {
  if (type == core::REDIS) {
    return redisConnectionIcon();
  } else if (type == core::MEMCACHED) {
    return memcachedConnectionIcon();
  } else if (type == core::SSDB) {
    return ssdbConnectionIcon();
  } else if (type == core::LEVELDB) {
    return leveldbConnectionIcon();
  } else if (type == core::ROCKSDB) {
    return rocksdbConnectionIcon();
  } else if (type == core::UNQLITE) {
    return unqliteConnectionIcon();
  } else if (type == core::LMDB) {
    return lmdbConnectionIcon();
  } else if (type == core::UPSCALEDB) {
    return upscaledbConnectionIcon();
  } else if (type == core::UPSCALEDB) {
    return forestdbConnectionIcon();
  } else {
    return GetServerIcon();
  }
}

const QIcon& GuiFactory::GetSuccessIcon() const {
  static QIcon suc(":" PROJECT_NAME_LOWERCASE "/images/64x64/success.png");
  return suc;
}

const QIcon& GuiFactory::GetFailIcon() const {
  static QIcon fail(":" PROJECT_NAME_LOWERCASE "/images/64x64/fail.png");
  return fail;
}

const QIcon& GuiFactory::GetUnknownIcon() const {
  static QIcon unknown(":" PROJECT_NAME_LOWERCASE "/images/64x64/unknown.png");
  return unknown;
}

QFont GuiFactory::GetFont() const {
  return proxy::SettingsManager::GetInstance()->GetCurrentFont();
}

const QString& GuiFactory::GetPathToLoadingGif() const {
  static QString path(":" PROJECT_NAME_LOWERCASE "/images/loading.gif");
  return path;
}

const QString& GuiFactory::GetPathToIndividualBuilds() const {
  static QString path(":" PROJECT_NAME_LOWERCASE "/images/help/individual_builds.gif");
  return path;
}

const QString& GuiFactory::GetPathToConnectGif() const {
  static QString path(":" PROJECT_NAME_LOWERCASE "/images/help/connect.gif");
  return path;
}

const QString& GuiFactory::GetPathToWorkflowGif() const {
  static QString path(":" PROJECT_NAME_LOWERCASE "/images/help/workflow.gif");
  return path;
}

const QIcon& GuiFactory::redisConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/redis.png");
  return main;
}

const QIcon& GuiFactory::memcachedConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/memcached.png");
  return main;
}

const QIcon& GuiFactory::ssdbConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/ssdb.png");
  return main;
}

const QIcon& GuiFactory::leveldbConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/leveldb.png");
  return main;
}

const QIcon& GuiFactory::rocksdbConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/rocksdb.png");
  return main;
}

const QIcon& GuiFactory::unqliteConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/unqlite.png");
  return main;
}

const QIcon& GuiFactory::lmdbConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/lmdb.png");
  return main;
}

const QIcon& GuiFactory::upscaledbConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/upscaledb.png");
  return main;
}

const QIcon& GuiFactory::forestdbConnectionIcon() const {
  static QIcon main(":" PROJECT_NAME_LOWERCASE "/images/64x64/forestdb.png");
  return main;
}

}  // namespace gui
}  // namespace fastonosql
