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

#include "core/value.h"

#include <algorithm>

#include <common/convert2string.h>
#include <common/utils.h>

namespace {

template<typename T>
std::string hex_string_impl(const T& value) {
  std::ostringstream wr;
  auto hexed = common::utils::hex::encode(value, true);
  for (size_t i = 0; i < hexed.size(); i += 2) {
    wr << "\\x";
    wr << hexed[i];
    wr << hexed[i + 1];
  }

  return wr.str();
}

}  // namespace

namespace fastonosql {
namespace core {
namespace detail {
bool have_space(const std::string& data) {
  auto it = std::find_if(data.begin(), data.end(), [](char c) { return std::isspace(c); });
  return it != data.end();
}

std::string hex_string(const common::buffer_t& value) {
  return hex_string_impl(value);
}

std::string hex_string(const std::string& value) {
  return hex_string_impl(value);
}
}  // namespace detail

std::string ConvertValue(common::Value* value, const std::string& delimiter, bool for_cmd) {
  if (!value) {
    return std::string();
  }

  common::Value::Type t = value->GetType();
  if (t == common::Value::TYPE_NULL) {
    return std::string();
  } else if (t == common::Value::TYPE_BOOLEAN) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_UINTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_ULONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_LONG_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_ULONG_LONG_INTEGER) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_DOUBLE) {
    return ConvertValue(static_cast<common::FundamentalValue*>(value), delimiter, for_cmd);

  } else if (t == common::Value::TYPE_STRING) {
    return ConvertValue(static_cast<common::StringValue*>(value), delimiter, for_cmd);

  } else if (t == common::Value::TYPE_ARRAY) {
    return ConvertValue(static_cast<common::ArrayValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_BYTE_ARRAY) {
    return ConvertValue(static_cast<common::ByteArrayValue*>(value), delimiter, for_cmd);

  } else if (t == common::Value::TYPE_SET) {
    return ConvertValue(static_cast<common::SetValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_ZSET) {
    return ConvertValue(static_cast<common::ZSetValue*>(value), delimiter, for_cmd);
  } else if (t == common::Value::TYPE_HASH) {
    return ConvertValue(static_cast<common::HashValue*>(value), delimiter, for_cmd);
  }

  DNOTREACHED();
  return std::string();
}

std::string ConvertValue(common::ArrayValue* array, const std::string& delimiter, bool for_cmd) {
  if (!array) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(array->end());
  for (auto it = array->begin(); it != array->end(); ++it) {
    common::Value* cur_val = *it;
    std::string val_str = ConvertValue(cur_val, delimiter, for_cmd);
    if (val_str.empty()) {
      continue;
    }

    result += val_str;
    if (lastIt != it) {
      result += delimiter;
    }
  }

  return result;
}

std::string ConvertValue(common::SetValue* set, const std::string& delimiter, bool for_cmd) {
  if (!set) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(set->end());
  for (auto it = set->begin(); it != set->end(); ++it) {
    std::string val = ConvertValue((*it), delimiter, for_cmd);
    if (val.empty()) {
      continue;
    }

    result += val;
    if (lastIt != it) {
      result += delimiter;
    }
  }

  return result;
}

std::string ConvertValue(common::ZSetValue* zset, const std::string& delimiter, bool for_cmd) {
  if (!zset) {
    return std::string();
  }

  std::string result;
  auto lastIt = std::prev(zset->end());
  for (auto it = zset->begin(); it != zset->end(); ++it) {
    auto v = *it;
    std::string key = ConvertValue((v.first), delimiter, for_cmd);
    std::string val = ConvertValue((v.second), delimiter, for_cmd);
    if (val.empty() || key.empty()) {
      continue;
    }

    result += key + " " + val;
    if (lastIt != it) {
      result += delimiter;
    }
  }
  return result;
}

std::string ConvertValue(common::HashValue* hash, const std::string& delimiter, bool for_cmd) {
  if (!hash) {
    return std::string();
  }

  std::string result;
  for (auto it = hash->begin(); it != hash->end(); ++it) {
    auto v = *it;
    std::string key = ConvertValue((v.first), delimiter, for_cmd);
    std::string val = ConvertValue((v.second), delimiter, for_cmd);
    if (val.empty() || key.empty()) {
      continue;
    }

    result += key + " " + val;
    if (std::next(it) != hash->end()) {
      result += delimiter;
    }
  }
  return result;
}

std::string ConvertValue(common::FundamentalValue* value, const std::string& delimiter, bool for_cmd) {
  UNUSED(delimiter);
  UNUSED(for_cmd);
  if (!value) {
    return std::string();
  }

  const common::Value::Type t = value->GetType();
  if (t == common::Value::TYPE_BOOLEAN) {
    bool res;
    if (!value->GetAsBoolean(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_INTEGER) {
    int res;
    if (!value->GetAsInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_UINTEGER) {
    unsigned int res;
    if (!value->GetAsUInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_LONG_INTEGER) {
    long res;
    if (!value->GetAsLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_ULONG_INTEGER) {
    unsigned long res;
    if (!value->GetAsULongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_LONG_LONG_INTEGER) {
    long long res;
    if (!value->GetAsLongLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_ULONG_LONG_INTEGER) {
    unsigned long long res;
    if (!value->GetAsULongLongInteger(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  } else if (t == common::Value::TYPE_DOUBLE) {
    double res;
    if (!value->GetAsDouble(&res)) {
      return std::string();
    }
    return common::ConvertToString(res);
  }
  return std::string();
}

std::string ConvertValue(common::StringValue* value, const std::string& delimiter, bool for_cmd) {
  UNUSED(delimiter);
  if (!value) {
    return std::string();
  }

  std::string res;
  if (!value->GetAsString(&res)) {
    return std::string();
  }

  if (!for_cmd) {
    return res;
  }

  if (detail::have_space(res)) {
    return "\"" + res + "\"";
  }

  return res;
}

std::string ConvertValue(common::ByteArrayValue* value, const std::string& delimiter, bool for_cmd) {
  UNUSED(delimiter);
  if (!value) {
    return std::string();
  }

  common::byte_array_t res;
  if (!value->GetAsByteArray(&res)) {
    return std::string();
  }

  if (!for_cmd) {
    return common::ConvertToString(res);
  }

  return detail::hex_string(res);
}

std::string ConvertToHumanReadable(common::Value* value, const std::string& delimiter) {
  return ConvertValue(value, delimiter, false);
}

}  // namespace core
}  // namespace fastonosql
