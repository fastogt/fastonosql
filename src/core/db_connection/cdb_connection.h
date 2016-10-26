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

#include <memory>  // for __shared_ptr
#include <string>  // for string

#include <common/error.h>   // for Error, make_error_value
#include <common/macros.h>  // for DNOTREACHED, etc
#include <common/value.h>   // for ErrorValue, etc

#include "core/command/command_handler.h"  // for CommandHandler, etc
#include "core/connection_types.h"         // for connectionTypes
#include "core/db_connection/cdb_connection_client.h"
#include "core/db_connection/db_connection.h"     // for DBConnection
#include "core/db_key.h"                          // for NDbKValue, NKey, etc
#include "core/translator/icommand_translator.h"  // for translator_t, etc
#include "core/database/idatabase_info.h"

namespace fastonosql {
namespace core {

template <typename NConnection, typename Config, connectionTypes ContType>
class CDBConnection : public DBConnection<NConnection, Config, ContType>, public CommandHandler {
 public:
  typedef DBConnection<NConnection, Config, ContType> db_base_class;

  CDBConnection(const commands_t& commands,
                CDBConnectionClient* client,
                ICommandTranslator* translator)
      : db_base_class(), CommandHandler(commands), client_(client), translator_(translator) {}
  virtual ~CDBConnection() {}

  common::Error Select(const std::string& name, IDataBaseInfo** info) WARN_UNUSED_RESULT;
  common::Error Delete(const NKeys& keys, NKeys* deleted_keys) WARN_UNUSED_RESULT;
  common::Error Set(const NDbKValue& key, NDbKValue* added_key) WARN_UNUSED_RESULT;
  common::Error Get(const NKey& key, NDbKValue* loaded_key) WARN_UNUSED_RESULT;
  common::Error Rename(const NKey& key, const std::string& new_key) WARN_UNUSED_RESULT;
  common::Error SetTTL(const NKey& key, ttl_t ttl) WARN_UNUSED_RESULT;

  translator_t Translator() const { return translator_; }

 protected:
  CDBConnectionClient* client_;

 private:
  virtual common::Error SelectImpl(const std::string& name, IDataBaseInfo** info) = 0;
  virtual common::Error DeleteImpl(const NKeys& keys, NKeys* deleted_keys) = 0;
  virtual common::Error SetImpl(const NDbKValue& key, NDbKValue* added_key) = 0;
  virtual common::Error GetImpl(const NKey& key, NDbKValue* loaded_key) = 0;
  virtual common::Error RenameImpl(const NKey& key, const std::string& new_key) = 0;
  virtual common::Error SetTTLImpl(const NKey& key, ttl_t ttl) = 0;

  translator_t translator_;
};

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

}  // namespace core
}  // namespace fastonosql
