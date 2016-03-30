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

#include <vector>
#include <utility>
#include <string>

#include "global/global.h"

#include "core/connection_types.h"

#include "common/net/net.h"

#define UNDEFINED_SINCE 0x00000000U
#define UNDEFINED_SINCE_STR "Undefined"
#define UNDEFINED_EXAMPLE_STR "Unspecified"
#define UNDEFINED_STR_IN_PROGRESS "Undefined in progress"
#define INFINITE_COMMAND_ARGS UINT8_MAX

namespace fastonosql {
namespace core {

struct CommandInfo {
  CommandInfo(const std::string& name, const std::string& params,
              const std::string& summary, uint32_t since,
              const std::string& example, uint8_t required_arguments_count,
              uint8_t optional_arguments_count);

  uint16_t maxArgumentsCount() const;
  uint8_t minArgumentsCount() const;

  const std::string name;
  const std::string params;
  const std::string summary;
  const uint32_t since;
  const std::string example;

  const uint8_t required_arguments_count;
  const uint8_t optional_arguments_count;
};

class CommandHandler;
class CommandHolder
    : public CommandInfo {
 public:
  typedef std::function<common::Error(CommandHandler*, int, char**, FastoObject*)> function_type;

  CommandHolder(const std::string& name, const std::string& params,
                const std::string& summary, uint32_t since,
                const std::string& example, uint8_t required_arguments_count,
                uint8_t optional_arguments_count, function_type func);
  bool isCommand(const std::string& cmd);
  common::Error execute(CommandHandler* handler, int argc, char** argv, FastoObject* out);

 private:
  const function_type func_;
};

class CommandHandler {
 public:
  typedef CommandHolder commands_type;
  explicit CommandHandler(const std::vector<commands_type>& commands);
  common::Error execute(int argc, char** argv, FastoObject* out);

  static common::Error notSupported(const char* cmd);
 private:
  const std::vector<commands_type> commands_;
};

template<typename C>
class StaticDbApiRaw
    : public CommandHandler {
public:
  bool isConnected() const {
    return isConnectedImpl();
  }
  common::Error connect(const C& config) WARN_UNUSED_RESULT {
    if (isConnected()) {
      return common::Error();
    }

    common::Error err = connectImpl(config);
    if (err && err->isError()) {
      return err;
    }

    config_ = config;
    return common::Error();
  }
  common::Error disconnect() WARN_UNUSED_RESULT {
    if (!isConnected()) {
      return common::Error();
    }

    common::Error err = disconnectImpl();
    if (err && err->isError()) {
      return err;
    }

    config_ = C();
    return common::Error();
  }
  C config() const {
    return config_;
  }
  std::string delimiter() const {
    return config_.delimiter;
  }

protected:
  explicit StaticDbApiRaw(const std::vector<commands_type>& commands)
    : CommandHandler(commands) {
  }

private:
  virtual bool isConnectedImpl() const = 0;
  virtual common::Error connectImpl(const C& config) = 0;
  virtual common::Error disconnectImpl() = 0;

  C config_;
};

std::string convertVersionNumberToReadableString(uint32_t version);

struct NKey {
  explicit NKey(const std::string& key, int32_t ttl_sec = -1);

  std::string key;
  int32_t ttl_sec;
};

typedef common::ValueSPtr NValue;

class NDbKValue {
 public:
  NDbKValue(const NKey& key, NValue value);

  NKey key() const;
  NValue value() const;
  common::Value::Type type() const;

  void setTTL(int32_t ttl);
  void setValue(NValue value);

  std::string keyString() const;

 private:
  NKey key_;
  NValue value_;
};

class ServerDiscoveryInfo {
 public:
  virtual ~ServerDiscoveryInfo();

  connectionTypes connectionType() const;
  serverTypes type() const;
  bool self() const;

  std::string name() const;
  void setName(const std::string& name);

  common::net::hostAndPort host() const;
  void setHost(const common::net::hostAndPort& host);

 protected:
  ServerDiscoveryInfo(connectionTypes ctype, serverTypes type, bool self);
  common::net::hostAndPort host_;
  std::string name_;

 private:
  const bool self_;
  const serverTypes type_;
  const connectionTypes ctype_;
  DISALLOW_COPY_AND_ASSIGN(ServerDiscoveryInfo);
};

typedef common::shared_ptr<ServerDiscoveryInfo> ServerDiscoveryInfoSPtr;

class IServerInfo {
 public:
  explicit IServerInfo(connectionTypes type);
  virtual ~IServerInfo();

  connectionTypes type() const;
  virtual std::string toString() const = 0;
  virtual uint32_t version() const = 0;
  virtual common::Value* valueByIndexes(unsigned char property, unsigned char field) const = 0;

 private:
  const connectionTypes type_;
  DISALLOW_COPY_AND_ASSIGN(IServerInfo);
};

struct FieldByIndex {
  virtual common::Value* valueByIndex(unsigned char index) const = 0;
};

struct Field {
  Field(const std::string& name, common::Value::Type type);

  bool isIntegral() const;
  std::string name;
  common::Value::Type type;
};

template<connectionTypes ct>
struct DBTraits {
  static std::vector<common::Value::Type> supportedTypes();
  static std::vector<std::string> infoHeaders();
  static std::vector< std::vector<Field> > infoFields();
};

std::vector<common::Value::Type> supportedTypesFromType(connectionTypes type);
std::vector<std::string> infoHeadersFromType(connectionTypes type);
std::vector< std::vector<Field> > infoFieldsFromType(connectionTypes type);

typedef common::shared_ptr<IServerInfo> IServerInfoSPtr;

struct ServerInfoSnapShoot {
  ServerInfoSnapShoot();
  ServerInfoSnapShoot(common::time64_t msec, IServerInfoSPtr info);
  bool isValid() const;

  common::time64_t msec;
  IServerInfoSPtr info;
};

typedef std::pair<std::string, std::string> PropertyType;

struct ServerPropertyInfo {
  ServerPropertyInfo();
  std::vector<PropertyType> propertyes;
};

ServerPropertyInfo makeServerProperty(const FastoObjectArray* array);

class IDataBaseInfo {
 public:
  typedef std::vector<NDbKValue> keys_cont_type;
  connectionTypes type() const;
  std::string name() const;
  size_t sizeDB() const;
  void setSizeDB(size_t size);
  size_t loadedSize() const;

  bool isDefault() const;
  void setIsDefault(bool isDef);

  virtual IDataBaseInfo* clone() const = 0;
  virtual ~IDataBaseInfo();

  keys_cont_type keys() const;
  void setKeys(const keys_cont_type& keys);
  void clearKeys();
 protected:
  IDataBaseInfo(const std::string& name, bool isDefault, connectionTypes type,
               size_t size, const keys_cont_type& keys);

 private:
  const std::string name_;
  bool is_default_;
  size_t size_;
  keys_cont_type keys_;

  const connectionTypes type_;
};

typedef common::shared_ptr<IDataBaseInfo> IDataBaseInfoSPtr;

class CommandKey {
 public:
  enum cmdtype {
    C_DELETE,
    C_LOAD,
    C_CREATE,
    C_CHANGE_TTL
  };

  cmdtype type() const;
  NDbKValue key() const;

  virtual ~CommandKey();

 protected:
  CommandKey(const NDbKValue& key, cmdtype type);

  const cmdtype type_;
  const NDbKValue key_;
};

class CommandDeleteKey
      : public CommandKey {
 public:
  explicit CommandDeleteKey(const NDbKValue& key);
};

class CommandLoadKey
  : public CommandKey {
 public:
  explicit CommandLoadKey(const NDbKValue& key);
};

class CommandCreateKey
  : public CommandKey {
 public:
  explicit CommandCreateKey(const NDbKValue& dbv);
  NValue value() const;
};

class CommandChangeTTL
  : public CommandKey {
 public:
  CommandChangeTTL(const NDbKValue& dbv, int32_t newTTL);
  int32_t newTTL() const;
  NDbKValue newKey() const;

 private:
  int32_t new_ttl_;
};

typedef common::shared_ptr<CommandKey> CommandKeySPtr;

template<typename Command>
FastoObjectCommand* createCommand(FastoObject* parent, const std::string& input,
                                  common::Value::CommandLoggingType ct) {
  if (!parent) {
    DNOTREACHED();
    return nullptr;
  }

  std::string stable_input = stableCommand(input);
  if (stable_input.empty()) {
    DNOTREACHED();
    return nullptr;
  }

  common::CommandValue* cmd = common::Value::createCommand(stable_input, ct);
  FastoObjectCommand* fs = new Command(parent, cmd, parent->delemitr());
  parent->addChildren(fs);
  return fs;
}

template<typename Command>
FastoObjectCommand* createCommand(FastoObjectIPtr parent, const std::string& input,
                                  common::Value::CommandLoggingType ct) {
  return createCommand<Command>(parent.get(), input, ct);
}

}  // namespace core
}  // namespace fastonosql
