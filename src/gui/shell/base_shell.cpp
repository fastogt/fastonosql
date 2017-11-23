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

#include "gui/shell/base_shell.h"

#include <QIcon>

#include "gui/gui_factory.h"  // for GuiFactory

#ifdef BUILD_WITH_REDIS
#include "gui/db/redis/lexer.h"
#endif

#ifdef BUILD_WITH_MEMCACHED
#include "gui/db/memcached/lexer.h"
#endif

#ifdef BUILD_WITH_SSDB
#include "gui/db/ssdb/lexer.h"
#endif

#ifdef BUILD_WITH_LEVELDB
#include "gui/db/leveldb/lexer.h"
#endif

#ifdef BUILD_WITH_ROCKSDB
#include "gui/db/rocksdb/lexer.h"
#endif

#ifdef BUILD_WITH_UNQLITE
#include "gui/db/unqlite/lexer.h"
#endif

#ifdef BUILD_WITH_LMDB
#include "gui/db/lmdb/lexer.h"
#endif

#ifdef BUILD_WITH_UPSCALEDB
#include "gui/db/upscaledb/lexer.h"
#endif

#ifdef BUILD_WITH_FORESTDB
#include "gui/db/forestdb/lexer.h"
#endif

namespace {
const QSize image_size(64, 64);
}

namespace fastonosql {
namespace gui {

BaseShell::BaseShell(core::connectionTypes type, bool showAutoCompl, QWidget* parent)
    : gui::FastoEditorShell(showAutoCompl, parent) {
  VERIFY(connect(this, &BaseShell::customContextMenuRequested, this, &BaseShell::showContextMenu));
  BaseQsciLexer* lex = nullptr;
#ifdef BUILD_WITH_REDIS
  if (type == core::REDIS) {
    lex = new redis::Lexer(this);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if (type == core::MEMCACHED) {
    lex = new memcached::Lexer(this);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if (type == core::SSDB) {
    lex = new ssdb::Lexer(this);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if (type == core::LEVELDB) {
    lex = new leveldb::Lexer(this);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if (type == core::ROCKSDB) {
    lex = new rocksdb::Lexer(this);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if (type == core::UNQLITE) {
    lex = new unqlite::Lexer(this);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if (type == core::LMDB) {
    lex = new lmdb::Lexer(this);
  }
#endif
#ifdef BUILD_WITH_UPSCALEDB
  if (type == core::UPSCALEDB) {
    lex = new upscaledb::Lexer(this);
  }
#endif
#ifdef BUILD_WITH_FORESTDB
  if (type == core::FORESTDB) {
    lex = new forestdb::Lexer(this);
  }
#endif
  const QIcon& ic = gui::GuiFactory::GetInstance().GetCommandIcon(type);
  QPixmap pix = ic.pixmap(image_size);
  registerImage(BaseQsciLexer::Command, pix);
  registerImage(BaseQsciLexer::ExCommand, pix);

  if (!lex) {
    NOTREACHED();
    return;
  }

  setLexer(lex);
  lex->setFont(gui::GuiFactory::GetInstance().GetFont());
}

BaseQsciLexer* BaseShell::lexer() const {
  BaseQsciLexer* lex = dynamic_cast<BaseQsciLexer*>(gui::FastoEditorShell::lexer());  // +
  CHECK(lex);
  return lex;
}

QString BaseShell::version() const {
  BaseQsciLexer* lex = lexer();
  return QString(lex->version());
}

QString BaseShell::basedOn() const {
  BaseQsciLexer* lex = lexer();
  return QString(lex->basedOn());
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

BaseShell* BaseShell::createFromType(core::connectionTypes type, bool showAutoCompl) {
  return new BaseShell(type, showAutoCompl);
}

}  // namespace gui
}  // namespace fastonosql
