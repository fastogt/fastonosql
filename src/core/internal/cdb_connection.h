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

#pragma once

#include <stddef.h>  // for size_t
#include <inttypes.h>
#include <stdint.h>  // for uint64_t, UINT64_MAX
#include <vector>    // for vector

#include <common/error.h>   // for Error, make_error_value
#include <common/macros.h>  // for DNOTREACHED, etc
#include <common/value.h>   // for ErrorValue, etc
#include <common/sprintf.h>

#include "core/command_info.h"
#include "core/connection_types.h"     // for connectionTypes
#include "core/db_key.h"               // for NDbKValue, NKey, etc
#include "core/icommand_translator.h"  // for translator_t, etc

#include "core/database/idatabase_info.h"

#include "core/internal/command_handler.h"  // for CommandHandler, etc
#include "core/internal/cdb_connection_client.h"
#include "core/internal/db_connection.h"  // for DBConnection

#define ALL_KEYS_PATTERNS "*"
#define NO_KEYS_LIMIT UINT64_MAX

#define GET_KEYS_PATTERN_3ARGS_ISI "SCAN %" PRIu64 " MATCH %s COUNT %" PRIu64

namespace fastonosql {
namespace core {
class CommandHolder;
}
}

namespace fastonosql {
namespace core {
namespace internal {

template <typename NConnection, typename Config, connectionTypes ContType>
class CDBConnection : public DBConnection<NConnection, Config, ContType>, public CommandHandler {
 public:
  typedef DBConnection<NConnection, Config, ContType> db_base_class;

  CDBConnection(CDBConnectionClient* client, ICommandTranslator* translator)
      : db_base_class(), CommandHandler(translator), client_(client) {}
  virtual ~CDBConnection() {}

  static std::vector<CommandHolder> Commands();
  static const char* BasedOn();
  static const char* VersionApi();

  std::string CurrentDBName() const;                                                        //
  common::Error Help(int argc, const char** argv, std::string* answer) WARN_UNUSED_RESULT;  //

  common::Error Scan(uint64_t cursor_in,
                     std::string pattern,
                     uint64_t count_keys,
                     std::vector<std::string>* keys_out,
                     uint64_t* cursor_out) WARN_UNUSED_RESULT;  // nvi
  common::Error Keys(const std::string& key_start,
                     const std::string& key_end,
                     uint64_t limit,
                     std::vector<std::string>* ret) WARN_UNUSED_RESULT;                    // nvi
  common::Error DBkcount(size_t* size) WARN_UNUSED_RESULT;                                 // nvi
  common::Error FlushDB() WARN_UNUSED_RESULT;                                              // nvi
  common::Error Select(const std::string& name, IDataBaseInfo** info) WARN_UNUSED_RESULT;  // nvi
  common::Error Delete(const NKeys& keys, NKeys* deleted_keys) WARN_UNUSED_RESULT;         // nvi
  common::Error Set(const NDbKValue& key, NDbKValue* added_key) WARN_UNUSED_RESULT;        // nvi
  common::Error Get(const NKey& key, NDbKValue* loaded_key) WARN_UNUSED_RESULT;            // nvi
  common::Error Rename(const NKey& key, const std::string& new_key) WARN_UNUSED_RESULT;    // nvi
  common::Error SetTTL(const NKey& key, ttl_t ttl) WARN_UNUSED_RESULT;                     // nvi
  common::Error GetTTL(const NKey& key, ttl_t* ttl) WARN_UNUSED_RESULT;                    // nvi
  common::Error Quit() WARN_UNUSED_RESULT;                                                 // nvi

 protected:
  CDBConnectionClient* client_;

 private:
  virtual common::Error ScanImpl(uint64_t cursor_in,
                                 std::string pattern,
                                 uint64_t count_keys,
                                 std::vector<std::string>* keys_out,
                                 uint64_t* cursor_out) = 0;
  virtual common::Error KeysImpl(const std::string& key_start,
                                 const std::string& key_end,
                                 uint64_t limit,
                                 std::vector<std::string>* ret) = 0;
  virtual common::Error DBkcountImpl(size_t* size) = 0;
  virtual common::Error FlushDBImpl() = 0;
  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) = 0;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) = 0;
  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) = 0;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) = 0;
  virtual common::Error RenameImpl(const NKey& key, const std::string& new_key) = 0;
  virtual common::Error SetTTLImpl(const NKey& key, ttl_t ttl) = 0;
  virtual common::Error GetTTLImpl(const NKey& key, ttl_t* ttl) = 0;
  virtual common::Error QuitImpl() = 0;
};

template <typename NConnection, typename Config, connectionTypes ContType>
std::string CDBConnection<NConnection, Config, ContType>::CurrentDBName() const {
  return "default";
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Help(int argc,
                                                                 const char** argv,
                                                                 std::string* answer) {
  if (!answer || argc < 0) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (argc == 0) {
    *answer = common::MemSPrintf(PROJECT_NAME_TITLE
                                 " based on %s %s \r\n"
                                 "Type: \"help <command>\" for help on <command>\r\n",
                                 BasedOn(), VersionApi());

    return common::Error();
  }

  const CommandHolder* cmd = nullptr;
  size_t off = 0;
  translator_t tran = Translator();
  common::Error err = tran->FindCommand(argc, argv, &cmd, &off);
  if (err && err->isError()) {
    return err;
  }

  *answer = common::MemSPrintf(
      "name: %s\n"
      "summary: %s\n"
      "params: %s\n"
      "since: %s\n"
      "example: %s\r\n",
      cmd->name, cmd->summary, cmd->params, ConvertVersionNumberToReadableString(cmd->since),
      cmd->example);
  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Scan(uint64_t cursor_in,
                                                                 std::string pattern,
                                                                 uint64_t count_keys,
                                                                 std::vector<std::string>* keys_out,
                                                                 uint64_t* cursor_out) {
  if (!keys_out || !cursor_out) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = ScanImpl(cursor_in, pattern, count_keys, keys_out, cursor_out);
  if (err && err->isError()) {
    return err;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Keys(const std::string& key_start,
                                                                 const std::string& key_end,
                                                                 uint64_t limit,
                                                                 std::vector<std::string>* ret) {
  if (!ret) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = KeysImpl(key_start, key_end, limit, ret);
  if (err && err->isError()) {
    return err;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::DBkcount(size_t* size) {
  if (!size) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = DBkcountImpl(size);
  if (err && err->isError()) {
    return err;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::FlushDB() {
  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = FlushDBImpl();
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnFlushedCurrentDB();
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Select(const std::string& name,
                                                                   IDataBaseInfo** info) {
  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  IDataBaseInfo* linfo = nullptr;
  common::Error err = SelectImpl(name, &linfo);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnCurrentDataBaseChanged(linfo);
  }

  if (info) {
    *info = linfo;
  } else {
    delete linfo;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Delete(const NKeys& keys,
                                                                   NKeys* deleted_keys) {
  if (!deleted_keys) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = DeleteImpl(keys, deleted_keys);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnKeysRemoved(*deleted_keys);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Set(const NDbKValue& key,
                                                                NDbKValue* added_key) {
  if (!added_key) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = SetImpl(key, added_key);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnKeyAdded(*added_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Get(const NKey& key,
                                                                NDbKValue* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = GetImpl(key, loaded_key);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnKeyLoaded(*loaded_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Rename(const NKey& key,
                                                                   const std::string& new_key) {
  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = RenameImpl(key, new_key);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnKeyRenamed(key, new_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::SetTTL(const NKey& key, ttl_t ttl) {
  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = SetTTLImpl(key, ttl);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnKeyTTLChanged(key, ttl);
  }

  return common::Error();
}
template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::GetTTL(const NKey& key, ttl_t* ttl) {
  if (!ttl) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = GetTTLImpl(key, ttl);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnKeyTTLLoaded(key, *ttl);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::Quit() {
  if (!CDBConnection<NConnection, Config, ContType>::IsConnected()) {
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = QuitImpl();
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->OnQuited();
  }

  return common::Error();
}
}
}  // namespace core
}  // namespace fastonosql
