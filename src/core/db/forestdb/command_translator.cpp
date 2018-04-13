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

#include "core/db/forestdb/command_translator.h"

#include "core/connection_types.h"

#define FORESTDB_GET_KEY_COMMAND DB_GET_KEY_COMMAND
#define FORESTDB_SET_KEY_COMMAND DB_SET_KEY_COMMAND
#define FORESTDB_DELETE_KEY_COMMAND DB_DELETE_KEY_COMMAND
#define FORESTDB_RENAME_KEY_COMMAND DB_RENAME_KEY_COMMAND

namespace fastonosql {
namespace core {
namespace forestdb {

CommandTranslator::CommandTranslator(const std::vector<CommandHolder>& commands) : ICommandTranslatorBase(commands) {}

common::Error CommandTranslator::CreateKeyCommandImpl(const NDbKValue& key, command_buffer_t* cmdstring) const {
  NKey cur = key.GetKey();
  key_t key_str = cur.GetKey();
  NValue value = key.GetValue();
  value_t value_str = value.GetValue();
  command_buffer_writer_t wr;
  wr << FORESTDB_SET_KEY_COMMAND " " << key_str.GetForCommandLine() << " " << value_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::LoadKeyCommandImpl(const NKey& key,
                                                    common::Value::Type type,
                                                    command_buffer_t* cmdstring) const {
  UNUSED(type);

  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << FORESTDB_GET_KEY_COMMAND " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::DeleteKeyCommandImpl(const NKey& key, command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << FORESTDB_DELETE_KEY_COMMAND " " << key_str.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

common::Error CommandTranslator::RenameKeyCommandImpl(const NKey& key,
                                                      const key_t& new_name,
                                                      command_buffer_t* cmdstring) const {
  key_t key_str = key.GetKey();
  command_buffer_writer_t wr;
  wr << FORESTDB_RENAME_KEY_COMMAND " " << key_str.GetForCommandLine() << " " << new_name.GetForCommandLine();
  *cmdstring = wr.str();
  return common::Error();
}

bool CommandTranslator::IsLoadKeyCommandImpl(const CommandInfo& cmd) const {
  return cmd.IsEqualName(FORESTDB_GET_KEY_COMMAND);
}

const char* CommandTranslator::GetDBName() const {
  return ConnectionTraits<FORESTDB>::GetDBName();
}

}  // namespace forestdb
}  // namespace core
}  // namespace fastonosql
