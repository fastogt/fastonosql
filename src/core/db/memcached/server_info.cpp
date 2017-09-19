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

#include "core/db/memcached/server_info.h"

#include <stddef.h>  // for size_t
#include <sstream>   // for operator<<, basic_ostream, etc
#include <string>    // for operator==, char_traits, etc
#include <utility>   // for make_pair
#include <vector>    // for vector

#include <common/convert2string.h>  // for ConvertFromString, etc
#include <common/macros.h>          // for NOTREACHED
#include <common/value.h>           // for FundamentalValue, Value, etc

#include "core/connection_types.h"  // for connectionTypes::MEMCACHED
#include "core/db_traits.h"

#define MARKER "\r\n"

namespace fastonosql {
namespace core {

namespace {
const std::vector<Field> g_memcached_common_fields = {
    Field(MEMCACHED_PID_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_UPTIME_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_TIME_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_VERSION_LABEL, common::Value::TYPE_STRING),
    Field(MEMCACHED_POINTER_SIZE_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_RUSAGE_USER_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_RUSAGE_SYSTEM_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_CURR_ITEMS_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_TOTAL_ITEMS_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_BYTES_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_CURR_CONNECTIONS_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_TOTAL_CONNECTIONS_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_CONNECTION_STRUCTURES_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_CMD_GET_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_CMD_SET_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_GET_HITS_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_GET_MISSES_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_EVICTIONS_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_BYTES_READ_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_BYTES_WRITTEN_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_LIMIT_MAXBYTES_LABEL, common::Value::TYPE_UINTEGER),
    Field(MEMCACHED_THREADS_LABEL, common::Value::TYPE_UINTEGER)};
}  // namespace

template <>
std::vector<common::Value::Type> DBTraits<MEMCACHED>::GetSupportedTypes() {
  return {common::Value::TYPE_BOOLEAN, common::Value::TYPE_INTEGER, common::Value::TYPE_UINTEGER,
          common::Value::TYPE_DOUBLE, common::Value::TYPE_STRING};
}

template <>
std::vector<info_field_t> DBTraits<MEMCACHED>::GetInfoFields() {
  return {std::make_pair(MEMCACHED_COMMON_LABEL, g_memcached_common_fields)};
}

namespace memcached {

ServerInfo::Stats::Stats() {}

ServerInfo::Stats::Stats(const std::string& common_text) {
  size_t pos = 0;
  size_t start = 0;

  while ((pos = common_text.find(MARKER, start)) != std::string::npos) {
    std::string line = common_text.substr(start, pos - start);
    size_t delem = line.find_first_of(':');
    std::string field = line.substr(0, delem);
    std::string value = line.substr(delem + 1);
    if (field == MEMCACHED_PID_LABEL) {
      uint32_t lpid;
      if (common::ConvertFromString(value, &lpid)) {
        pid = lpid;
      }
    } else if (field == MEMCACHED_UPTIME_LABEL) {
      uint32_t luptime;
      if (common::ConvertFromString(value, &luptime)) {
        uptime = luptime;
      }
    } else if (field == MEMCACHED_TIME_LABEL) {
      uint32_t ltime;
      if (common::ConvertFromString(value, &ltime)) {
        time = ltime;
      }
    } else if (field == MEMCACHED_VERSION_LABEL) {
      version = value;
    } else if (field == MEMCACHED_POINTER_SIZE_LABEL) {
      uint32_t lpointer_size;
      if (common::ConvertFromString(value, &lpointer_size)) {
        pointer_size = lpointer_size;
      }
    } else if (field == MEMCACHED_RUSAGE_USER_LABEL) {
      uint32_t lrusage_user;
      if (common::ConvertFromString(value, &lrusage_user)) {
        rusage_user = lrusage_user;
      }
    } else if (field == MEMCACHED_RUSAGE_SYSTEM_LABEL) {
      uint32_t lrusage_system;
      if (common::ConvertFromString(value, &lrusage_system)) {
        rusage_system = lrusage_system;
      }
    } else if (field == MEMCACHED_CURR_ITEMS_LABEL) {
      uint32_t lcurr_items;
      if (common::ConvertFromString(value, &lcurr_items)) {
        curr_items = lcurr_items;
      }
    } else if (field == MEMCACHED_TOTAL_ITEMS_LABEL) {
      uint32_t ltotal_items;
      if (common::ConvertFromString(value, &ltotal_items)) {
        total_items = ltotal_items;
      }
    } else if (field == MEMCACHED_BYTES_LABEL) {
      uint32_t lbytes;
      if (common::ConvertFromString(value, &lbytes)) {
        bytes = lbytes;
      }
    } else if (field == MEMCACHED_CURR_CONNECTIONS_LABEL) {
      uint32_t lcurr_connections;
      if (common::ConvertFromString(value, &lcurr_connections)) {
        curr_connections = lcurr_connections;
      }
    } else if (field == MEMCACHED_TOTAL_CONNECTIONS_LABEL) {
      uint32_t ltotal_connections;
      if (common::ConvertFromString(value, &ltotal_connections)) {
        curr_connections = ltotal_connections;
      }
    } else if (field == MEMCACHED_CONNECTION_STRUCTURES_LABEL) {
      uint32_t lconnection_structures;
      if (common::ConvertFromString(value, &lconnection_structures)) {
        connection_structures = lconnection_structures;
      }
    } else if (field == MEMCACHED_CMD_GET_LABEL) {
      uint32_t lcmd_get;
      if (common::ConvertFromString(value, &lcmd_get)) {
        cmd_get = lcmd_get;
      }
    } else if (field == MEMCACHED_CMD_SET_LABEL) {
      uint32_t lcmd_set;
      if (common::ConvertFromString(value, &lcmd_set)) {
        cmd_set = lcmd_set;
      }
    } else if (field == MEMCACHED_GET_HITS_LABEL) {
      uint32_t lget_hits;
      if (common::ConvertFromString(value, &lget_hits)) {
        get_hits = lget_hits;
      }
    } else if (field == MEMCACHED_GET_MISSES_LABEL) {
      uint32_t lget_misses;
      if (common::ConvertFromString(value, &lget_misses)) {
        get_misses = lget_misses;
      }
    } else if (field == MEMCACHED_EVICTIONS_LABEL) {
      uint32_t levictions;
      if (common::ConvertFromString(value, &levictions)) {
        evictions = levictions;
      }
    } else if (field == MEMCACHED_BYTES_READ_LABEL) {
      uint32_t lbytes_read;
      if (common::ConvertFromString(value, &lbytes_read)) {
        bytes_read = lbytes_read;
      }
    } else if (field == MEMCACHED_BYTES_WRITTEN_LABEL) {
      uint32_t lbytes_written;
      if (common::ConvertFromString(value, &lbytes_written)) {
        bytes_written = lbytes_written;
      }
    } else if (field == MEMCACHED_LIMIT_MAXBYTES_LABEL) {
      uint32_t llimit_maxbytes;
      if (common::ConvertFromString(value, &llimit_maxbytes)) {
        limit_maxbytes = llimit_maxbytes;
      }
    } else if (field == MEMCACHED_THREADS_LABEL) {
      uint32_t lthreads;
      if (common::ConvertFromString(value, &lthreads)) {
        threads = lthreads;
      }
    }
    start = pos + 2;
  }
}

common::Value* ServerInfo::Stats::GetValueByIndex(unsigned char index) const {
  switch (index) {
    case 0:
      return new common::FundamentalValue(pid);
    case 1:
      return new common::FundamentalValue(uptime);
    case 2:
      return new common::FundamentalValue(time);
    case 3:
      return new common::StringValue(version);
    case 4:
      return new common::FundamentalValue(pointer_size);
    case 5:
      return new common::FundamentalValue(rusage_user);
    case 6:
      return new common::FundamentalValue(rusage_system);
    case 7:
      return new common::FundamentalValue(curr_items);
    case 8:
      return new common::FundamentalValue(total_items);
    case 9:
      return new common::FundamentalValue(bytes);
    case 10:
      return new common::FundamentalValue(curr_connections);
    case 11:
      return new common::FundamentalValue(total_connections);
    case 12:
      return new common::FundamentalValue(connection_structures);
    case 13:
      return new common::FundamentalValue(cmd_get);
    case 14:
      return new common::FundamentalValue(cmd_set);
    case 15:
      return new common::FundamentalValue(get_hits);
    case 16:
      return new common::FundamentalValue(get_misses);
    case 17:
      return new common::FundamentalValue(evictions);
    case 18:
      return new common::FundamentalValue(bytes_read);
    case 19:
      return new common::FundamentalValue(bytes_written);
    case 20:
      return new common::FundamentalValue(limit_maxbytes);
    case 21:
      return new common::FundamentalValue(threads);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

ServerInfo::ServerInfo() : IServerInfo(MEMCACHED) {}

ServerInfo::ServerInfo(const Stats& common) : IServerInfo(MEMCACHED), stats_(common) {}

common::Value* ServerInfo::GetValueByIndexes(unsigned char property, unsigned char field) const {
  switch (property) {
    case 0:
      return stats_.GetValueByIndex(field);
    default:
      break;
  }

  NOTREACHED();
  return nullptr;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo::Stats& value) {
  return out << MEMCACHED_PID_LABEL ":" << value.pid << MARKER << MEMCACHED_UPTIME_LABEL ":" << value.uptime << MARKER
             << MEMCACHED_TIME_LABEL ":" << value.time << MARKER << MEMCACHED_VERSION_LABEL ":" << value.version
             << MARKER << MEMCACHED_POINTER_SIZE_LABEL ":" << value.pointer_size << MARKER
             << MEMCACHED_RUSAGE_USER_LABEL ":" << value.rusage_user << MARKER << MEMCACHED_RUSAGE_SYSTEM_LABEL ":"
             << value.rusage_system << MARKER << MEMCACHED_CURR_ITEMS_LABEL ":" << value.curr_items << MARKER
             << MEMCACHED_TOTAL_ITEMS_LABEL ":" << value.total_items << MARKER << MEMCACHED_BYTES_LABEL ":"
             << value.bytes << MARKER << MEMCACHED_CURR_CONNECTIONS_LABEL ":" << value.curr_connections << MARKER
             << MEMCACHED_TOTAL_CONNECTIONS_LABEL ":" << value.total_connections << MARKER
             << MEMCACHED_CONNECTION_STRUCTURES_LABEL ":" << value.connection_structures << MARKER
             << MEMCACHED_CMD_GET_LABEL ":" << value.cmd_get << MARKER << MEMCACHED_CMD_SET_LABEL ":" << value.cmd_set
             << MARKER << MEMCACHED_GET_HITS_LABEL ":" << value.get_hits << MARKER << MEMCACHED_GET_MISSES_LABEL ":"
             << value.get_misses << MARKER << MEMCACHED_EVICTIONS_LABEL ":" << value.evictions << MARKER
             << MEMCACHED_BYTES_READ_LABEL ":" << value.bytes_read << MARKER << MEMCACHED_BYTES_WRITTEN_LABEL ":"
             << value.bytes_written << MARKER << MEMCACHED_LIMIT_MAXBYTES_LABEL ":" << value.limit_maxbytes << MARKER
             << MEMCACHED_THREADS_LABEL ":" << value.threads << MARKER;
}

std::ostream& operator<<(std::ostream& out, const ServerInfo& value) {
  return out << value.ToString();
}

ServerInfo* MakeMemcachedServerInfo(const std::string& content) {
  if (content.empty()) {
    return nullptr;
  }

  ServerInfo* result = new ServerInfo;
  size_t j = 0;
  std::string word;
  size_t pos = 0;
  static const std::vector<info_field_t> fields = DBTraits<MEMCACHED>::GetInfoFields();
  for (size_t i = 0; i < content.size(); ++i) {
    char ch = content[i];
    word += ch;
    if (word == fields[j].first) {
      if (j + 1 != fields.size()) {
        pos = content.find(fields[j + 1].first, pos);
      } else {
        break;
      }

      if (pos != std::string::npos) {
        std::string part = content.substr(i + 1, pos - i - 1);
        switch (j) {
          case 0:
            result->stats_ = ServerInfo::Stats(part);
            break;
          default:
            break;
        }
        i = pos - 1;
        ++j;
      }
      word.clear();
    }
  }

  return result;
}

std::string ServerInfo::ToString() const {
  std::stringstream str;
  str << MEMCACHED_COMMON_LABEL MARKER << stats_;
  return str.str();
}

uint32_t ServerInfo::GetVersion() const {
  return common::ConvertVersionNumberFromString(stats_.version);
}

}  // namespace memcached
}  // namespace core
}  // namespace fastonosql
