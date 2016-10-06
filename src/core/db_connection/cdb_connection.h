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

#include <string>  // for string

#include <common/error.h>   // for Error
#include <common/macros.h>  // for DNOTREACHED, WARN_UNUSED_RESULT

#include "core/db_connection/cdb_connection_client.h"  // for CDBConnectionClient
#include "core/command/command_handler.h"              // for CommandHandler, CommandHandl...
#include "core/connection_types.h"                     // for connectionTypes
#include "core/db_connection/db_connection.h"          // for DBConnection
#include "core/db_key.h"                               // for key_and_value_t, key_t, keys_t
#include "core/translator/icommand_translator.h"       // for translator_t, ICommandTransl...

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

  common::Error select(const std::string& name, IDataBaseInfo** info) WARN_UNUSED_RESULT;
  common::Error del(const keys_t& keys, keys_t* deleted_keys) WARN_UNUSED_RESULT;
  common::Error set(const key_and_value_t& key, key_and_value_t* added_key) WARN_UNUSED_RESULT;
  common::Error get(const key_t& key, key_and_value_t* loaded_key) WARN_UNUSED_RESULT;
  common::Error rename(const key_t& key, const std::string& new_key) WARN_UNUSED_RESULT;
  common::Error setTTL(const key_t& key, ttl_t ttl) WARN_UNUSED_RESULT;

  translator_t translator() const { return translator_; }

 private:
  virtual common::Error selectImpl(const std::string& name, IDataBaseInfo** info) = 0;
  virtual common::Error delImpl(const keys_t& keys, keys_t* deleted_keys) = 0;
  virtual common::Error setImpl(const key_and_value_t& key, key_and_value_t* added_key) = 0;
  virtual common::Error getImpl(const key_t& key, key_and_value_t* loaded_key) = 0;
  virtual common::Error renameImpl(const key_t& key, const std::string& new_key) = 0;
  virtual common::Error setTTLImpl(const key_t& key, ttl_t ttl) = 0;

  CDBConnectionClient* client_;
  translator_t translator_;
};

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::select(const std::string& name,
                                                                   IDataBaseInfo** info) {
  if (!CDBConnection<NConnection, Config, ContType>::isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  IDataBaseInfo* linfo = nullptr;
  common::Error err = selectImpl(name, &linfo);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->onCurrentDataBaseChanged(linfo);
  }

  if (info) {
    *info = linfo;
  } else {
    delete linfo;
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::del(const keys_t& keys,
                                                                keys_t* deleted_keys) {
  if (!deleted_keys) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = delImpl(keys, deleted_keys);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->onKeysRemoved(*deleted_keys);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::set(const key_and_value_t& key,
                                                                key_and_value_t* added_key) {
  if (!added_key) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = setImpl(key, added_key);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->onKeyAdded(*added_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::get(const key_t& key,
                                                                key_and_value_t* loaded_key) {
  if (!loaded_key) {
    DNOTREACHED();
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  if (!CDBConnection<NConnection, Config, ContType>::isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = getImpl(key, loaded_key);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->onKeyLoaded(*loaded_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::rename(const key_t& key,
                                                                   const std::string& new_key) {
  if (!CDBConnection<NConnection, Config, ContType>::isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = renameImpl(key, new_key);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->onKeyRenamed(key, new_key);
  }

  return common::Error();
}

template <typename NConnection, typename Config, connectionTypes ContType>
common::Error CDBConnection<NConnection, Config, ContType>::setTTL(const key_t& key, ttl_t ttl) {
  if (!CDBConnection<NConnection, Config, ContType>::isConnected()) {
    DNOTREACHED();
    return common::make_error_value("Not connected", common::Value::E_ERROR);
  }

  common::Error err = setTTLImpl(key, ttl);
  if (err && err->isError()) {
    return err;
  }

  if (client_) {
    client_->onKeyTTLChanged(key, ttl);
  }

  return common::Error();
}

}  // namespace core
}  // namespace fastonosql
