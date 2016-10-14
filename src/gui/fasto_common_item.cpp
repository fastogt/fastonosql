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

#include "gui/fasto_common_item.h"

#include <memory>    // for __shared_ptr
#include <stddef.h>  // for size_t
#include <string>    // for string

#include <common/convert2string.h>                  // for ConvertFromString
#include <common/error.h>                           // for Error
#include <common/qt/convert2string.h>               // for ConvertToString
#include <common/text_decoders/compress_edcoder.h>  // for CompressEDcoder
#include <common/text_decoders/hex_edcoder.h>       // for HexEDcoder
#include <common/text_decoders/msgpack_edcoder.h>   // for MsgPackEDcoder

#include <common/qt/gui/base/tree_item.h>  // for TreeItem

#include "third-party/json-c/json-c/json_tokener.h"

#include "global/global.h"  // for ConvertToString

namespace fastonosql {
namespace gui {

FastoCommonItem::FastoCommonItem(const core::NDbKValue& key,
                                 const std::string& delimiter,
                                 bool isReadOnly,
                                 TreeItem* parent,
                                 void* internalPointer)
    : TreeItem(parent, internalPointer), key_(key), delimiter_(delimiter), read_only_(isReadOnly) {}

QString FastoCommonItem::key() const {
  return common::ConvertFromString<QString>(key_.keyString());
}

QString FastoCommonItem::value() const {
  core::NValue nval = key_.value();
  common::Value* val = nval.get();
  std::string valstr = common::ConvertToString(val, delimiter_);
  return common::ConvertFromString<QString>(valstr);
}

void FastoCommonItem::setValue(core::NValue val) {
  key_.setValue(val);
}

common::Value::Type FastoCommonItem::type() const {
  return key_.type();
}

bool FastoCommonItem::isReadOnly() const {
  return read_only_;
}

QString toJson(FastoCommonItem* item) {
  if (!item) {
    return QString();
  }

  if (!item->childrenCount()) {
    std::string json = common::ConvertToString(item->value());
    json_object* obj = json_tokener_parse(json.c_str());
    if (!obj) {
      return QString();
    }

    const char* jstring = json_object_get_string(obj);
    QString result = common::ConvertFromString<QString>(jstring);
    json_object_put(obj);
    return result;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toJson(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString toRaw(FastoCommonItem* item) {
  if (!item) {
    return QString();
  }

  if (!item->childrenCount()) {
    return item->value();
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toRaw(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString toHex(FastoCommonItem* item) {
  if (!item) {
    return QString();
  }

  if (!item->childrenCount()) {
    QString val = item->value();
    std::string sval = common::ConvertToString(val);

    std::string hexstr;
    common::HexEDcoder hex;
    common::Error err = hex.encode(sval, &hexstr);
    if (err && err->isError()) {
      return QString();
    }

    return common::ConvertFromString<QString>(hexstr);
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toHex(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString toCsv(FastoCommonItem* item, const QString& delimiter) {
  if (!item) {
    return QString();
  }

  if (!item->childrenCount()) {
    return item->value().replace(delimiter, ",");
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toCsv(dynamic_cast<FastoCommonItem*>(item->child(i)), delimiter);  // +
    if (i != item->childrenCount() - 1) {
      value += ",";
    }
  }

  return value;
}

QString fromGzip(FastoCommonItem* item) {
  if (!item) {
    return QString();
  }

  if (!item->childrenCount()) {
    QString val = item->value();
    std::string sval = common::ConvertToString(val);
    std::string out;
    common::CompressEDcoder enc;
    common::Error err = enc.decode(sval, &out);
    if (err && err->isError()) {
      return QString();
    }

    return common::ConvertFromString<QString>(out);
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromGzip(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString fromHexMsgPack(FastoCommonItem* item) {
  if (!item) {
    return QString();
  }

  if (!item->childrenCount()) {
    std::string sval = common::ConvertToString(item->value());

    common::HexEDcoder hex;
    std::string hexstr;
    common::Error err = hex.decode(sval, &hexstr);
    if (err && err->isError()) {
      return QString();
    }

    common::MsgPackEDcoder msg;
    std::string upack;
    err = msg.decode(hexstr, &upack);
    if (err && err->isError()) {
      return QString();
    }
    return common::ConvertFromString<QString>(upack);
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromHexMsgPack(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

}  // namespace gui
}  // namespace fastonosql
