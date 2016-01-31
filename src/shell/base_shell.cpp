/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    This file is part of FastoNoSQL.

    SiteOnYourDevice is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SiteOnYourDevice is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SiteOnYourDevice.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "shell/base_shell.h"

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

namespace fastonosql {

BaseShell::BaseShell(connectionTypes type, bool showAutoCompl, QWidget* parent)
  : FastoEditorShell(showAutoCompl, parent) {
  VERIFY(connect(this, &BaseShell::customContextMenuRequested, this, &BaseShell::showContextMenu));
  BaseQsciLexer* lex = NULL;
#ifdef BUILD_WITH_REDIS
  if(type == REDIS){
    lex = new RedisLexer(this);
  }
#endif
#ifdef BUILD_WITH_MEMCACHED
  if(type == MEMCACHED){
    lex = new MemcachedLexer(this);
  }
#endif
#ifdef BUILD_WITH_SSDB
  if(type == SSDB){
    lex = new SsdbLexer(this);
  }
#endif
#ifdef BUILD_WITH_LEVELDB
  if(type == LEVELDB){
    lex = new LeveldbLexer(this);
  }
#endif
#ifdef BUILD_WITH_ROCKSDB
  if(type == ROCKSDB){
    lex = new RocksdbLexer(this);
  }
#endif
#ifdef BUILD_WITH_UNQLITE
  if(type == UNQLITE){
    lex = new UnqliteLexer(this);
  }
#endif
#ifdef BUILD_WITH_LMDB
  if(type == LMDB){
    lex = new LmdbLexer(this);
  }
#endif
  registerImage(BaseQsciLexer::Command, GuiFactory::instance().commandIcon(type).pixmap(QSize(64,64)));
  registerImage(BaseQsciLexer::HelpKeyword, GuiFactory::instance().messageBoxQuestionIcon().pixmap(QSize(64,64)));

  DCHECK(lex);
  setLexer(lex);
  lex->setFont(font());
}

QString BaseShell::version() const {
  BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
  DCHECK(red);
  if(red){
    return common::convertFromString<QString>(red->version());
  }

  return QString();
}

QString BaseShell::basedOn() const {
  BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
  DCHECK(red);
  if(red){
    return common::convertFromString<QString>(red->basedOn());
  }

  return QString();
}

std::vector<uint32_t> BaseShell::supportedVersions() const {
  BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
  DCHECK(red);
  if(red){
    return red->supportedVersions();
  }

  return std::vector<uint32_t>();
}

uint32_t BaseShell::commandsCount() const {
  BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
  DCHECK(red);
  if (red) {
    return red->commandsCount();
  }

  return 0;
}

void BaseShell::setFilteredVersion(uint32_t version) {
  BaseQsciLexer* red = dynamic_cast<BaseQsciLexer*>(lexer());
  DCHECK(red);
  if(!red){
    return;
  }

  BaseQsciApi * api = dynamic_cast<BaseQsciApi*>(red->apis());
  if (!api) {
    return;
  }

  api->setFilteredVersion(version);
}

BaseShell* BaseShell::createFromType(connectionTypes type, bool showAutoCompl) {
  return new BaseShell(type, showAutoCompl);
}

}

