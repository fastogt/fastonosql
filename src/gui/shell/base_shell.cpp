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
    along with FastoNoSQL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/shell/base_shell.h"

#include <string>
#include <vector>

#include <QIcon>

#include "gui/gui_factory.h"  // for GuiFactory

#if defined(BUILD_WITH_REDIS)
#include "gui/db/redis/lexer.h"
#endif

#if defined(BUILD_WITH_MEMCACHED)
#include "gui/db/memcached/lexer.h"
#endif

#if defined(BUILD_WITH_SSDB)
#include "gui/db/ssdb/lexer.h"
#endif

#if defined(BUILD_WITH_LEVELDB)
#include "gui/db/leveldb/lexer.h"
#endif

#if defined(BUILD_WITH_ROCKSDB)
#include "gui/db/rocksdb/lexer.h"
#endif

#if defined(BUILD_WITH_UNQLITE)
#include "gui/db/unqlite/lexer.h"
#endif

#if defined(BUILD_WITH_LMDB)
#include "gui/db/lmdb/lexer.h"
#endif

#if defined(BUILD_WITH_FORESTDB)
#include "gui/db/forestdb/lexer.h"
#endif

#if defined(BUILD_WITH_PIKA)
#include "gui/db/pika/lexer.h"
#endif

#if defined(BUILD_WITH_DYNOMITE)
#include "gui/db/dynomite/lexer.h"
#endif

namespace {
const QSize kImageSize = QSize(32, 32);
}

namespace fastonosql {
namespace gui {
namespace {
BaseQsciLexer* createLexer(core::ConnectionType type, QObject* parent) {
#if defined(BUILD_WITH_REDIS)
  if (type == core::REDIS) {
    return new redis::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_MEMCACHED)
  if (type == core::MEMCACHED) {
    return new memcached::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_SSDB)
  if (type == core::SSDB) {
    return new ssdb::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_LEVELDB)
  if (type == core::LEVELDB) {
    return new leveldb::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_ROCKSDB)
  if (type == core::ROCKSDB) {
    return new rocksdb::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_UNQLITE)
  if (type == core::UNQLITE) {
    return new unqlite::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_LMDB)
  if (type == core::LMDB) {
    return new lmdb::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_FORESTDB)
  if (type == core::FORESTDB) {
    return new forestdb::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_PIKA)
  if (type == core::PIKA) {
    return new pika::Lexer(parent);
  }
#endif
#if defined(BUILD_WITH_DYNOMITE)
  if (type == core::DYNOMITE) {
    return new dynomite::Lexer(parent);
  }
#endif

  NOTREACHED() << "Not handled type: " << type;
  return nullptr;
}
}  // namespace
BaseShell::BaseShell(core::ConnectionType type, bool show_auto_complete, QWidget* parent)
    : gui::FastoEditorShell(show_auto_complete, parent) {
  VERIFY(connect(this, &BaseShell::customContextMenuRequested, this, &BaseShell::showContextMenu));
  const QIcon& ic = gui::GuiFactory::GetInstance().commandIcon(type);
  QPixmap pix = ic.pixmap(kImageSize);
  registerImage(BaseQsciLexer::Command, pix);
  registerImage(BaseQsciLexer::ExCommand, pix);

  BaseQsciLexer* lex = createLexer(type, this);
  setLexer(lex);
  lex->setFont(gui::GuiFactory::GetInstance().font());
}

BaseQsciLexer* BaseShell::lexer() const {
  BaseQsciLexer* lex = dynamic_cast<BaseQsciLexer*>(gui::FastoEditorShell::lexer());  // +
  CHECK(lex);
  return lex;
}

QString BaseShell::version() const {
  BaseQsciLexer* lex = lexer();
  return lex->version();
}

QString BaseShell::basedOn() const {
  BaseQsciLexer* lex = lexer();
  return lex->basedOn();
}

std::vector<uint32_t> BaseShell::supportedVersions() const {
  BaseQsciLexer* lex = lexer();
  return lex->supportedVersions();
}

size_t BaseShell::commandsCount() const {
  BaseQsciLexer* lex = lexer();
  return lex->commandsCount();
}

size_t BaseShell::validateCommandsCount() const {
  return 0;
}

void BaseShell::setFilteredVersion(uint32_t version) {
  BaseQsciLexer* lex = lexer();
  BaseQsciApi* api = lex->apis();
  api->setFilteredVersion(version);
}

BaseShell* BaseShell::createFromType(core::ConnectionType type, bool show_auto_complete) {
  return createWidget<BaseShell>(type, show_auto_complete);
}

}  // namespace gui
}  // namespace fastonosql
