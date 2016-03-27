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

#include "shell/base_shell.h"

#include <vector>

#include "gui/gui_factory.h"

#ifdef BUILD_WITH_REDIS
#include "shell/redis_lexer.h"
#endif

#ifdef BUILD_WITH_MEMCACHED
#include "shell/memcached_lexer.h"
#endif

#ifdef BUILD_WITH_SSDB
#include "shell/ssdb_lexer.h"
#endif

#ifdef BUILD_WITH_LEVELDB
#include "shell/leveldb_lexer.h"
#endif

#ifdef BUILD_WITH_ROCKSDB
#include "shell/rocksdb_lexer.h"
#endif

#ifdef BUILD_WITH_UNQLITE
#include "shell/unqlite_lexer.h"
#endif

#ifdef BUILD_WITH_LMDB
#include "shell/lmdb_lexer.h"
#endif

namespace {
  const QSize image_size(64, 64);
}

namespace fastonosql {
namespace shell {

BaseShell::BaseShell(core::connectionTypes type, bool showAutoCompl, QWidget* parent)
  : gui::FastoEditorShell(showAutoCompl, parent) {
  VERIFY(connect(this, &BaseShell::customContextMenuRequested, this, &BaseShell::showContextMenu));
  BaseQsciLexer* lex = nullptr;
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    lex = new RedisLexer(this);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    lex = new MemcachedLexer(this);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    lex = new SsdbLexer(this);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    lex = new LeveldbLexer(this);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    lex = new RocksdbLexer(this);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    lex = new UnqliteLexer(this);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    lex = new LmdbLexer(this);
  }
#endif
  registerImage(BaseQsciLexer::Command,
                gui::GuiFactory::instance().commandIcon(type).pixmap(image_size));

  CHECK(lex);
  if (lex) {
    setLexer(lex);
    lex->setFont(font());
  }
}

QString BaseShell::version() const {
  BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
  if (red) {
    return common::convertFromString<QString>(red->version());
  }

  DNOTREACHED();
  return QString();
}

QString BaseShell::basedOn() const {
  BaseQsciLexer* lex = dynamic_cast<BaseQsciLexer*>(lexer());
  if (!lex) {
    NOTREACHED();
    return QString();
  }

  return common::convertFromString<QString>(lex->basedOn());
}

std::vector<uint32_t> BaseShell::supportedVersions() const {
  BaseQsciLexer* lex = dynamic_cast<BaseQsciLexer*>(lexer());
  if (!lex) {
    NOTREACHED();
    return std::vector<uint32_t>();
  }

  return lex->supportedVersions();
}

size_t BaseShell::commandsCount() const {
  BaseQsciLexer* lex = dynamic_cast<BaseQsciLexer*>(lexer());
  if (!lex) {
    NOTREACHED();
    return 0;
  }

  return lex->commandsCount();
}

void BaseShell::setFilteredVersion(uint32_t version) {
  BaseQsciLexer* lex = dynamic_cast<BaseQsciLexer*>(lexer());
  if (!lex) {
    NOTREACHED();
    return;
  }

  BaseQsciApi* api = dynamic_cast<BaseQsciApi*>(lex->apis());
  if (!api) {
    NOTREACHED();
    return;
  }

  api->setFilteredVersion(version);
}

BaseShell* BaseShell::createFromType(core::connectionTypes type, bool showAutoCompl) {
  return new BaseShell(type, showAutoCompl);
}

}  // namespace shell
}  // namespace fastonosql
