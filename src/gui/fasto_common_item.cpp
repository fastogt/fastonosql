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

#include "gui/fasto_common_item.h"

#include <common/qt/convert2string.h>                      // for ConvertToString
#include <common/text_decoders/compress_bzip2_edcoder.h>   // for CompressEDcoder
#include <common/text_decoders/compress_lz4_edcoder.h>     // for CompressEDcoder
#include <common/text_decoders/compress_snappy_edcoder.h>  // for CompressEDcoder
#include <common/text_decoders/compress_zlib_edcoder.h>    // for CompressEDcoder
#include <common/text_decoders/hex_edcoder.h>              // for HexEDcoder
#include <common/text_decoders/msgpack_edcoder.h>          // for MsgPackEDcoder

#include <json-c/json_tokener.h>

#define CSV_SEPARATOR ","

namespace fastonosql {
namespace gui {

FastoCommonItem::FastoCommonItem(const core::NDbKValue& key,
                                 const std::string& delimiter,
                                 bool isReadOnly,
                                 TreeItem* parent,
                                 void* internalPointer)
    : TreeItem(parent, internalPointer), key_(key), delimiter_(delimiter), read_only_(isReadOnly) {}

QString FastoCommonItem::key() const {
  QString qkey;
  const core::NKey key = key_.GetKey();
  const core::key_t raw_key = key.GetKey();
  common::ConvertFromString(raw_key.GetHumanReadable(), &qkey);
  return qkey;
}

QString FastoCommonItem::value() const {
  std::string valstr = basicStringValue();
  QString qvalstr;
  common::ConvertFromString(valstr, &qvalstr);
  return qvalstr;
}

std::string FastoCommonItem::basicStringValue() const {
  core::NValue nval = key_.GetValue();
  core::value_t value_str = nval.GetValue(delimiter_);
  return value_str.GetHumanReadable();
}

void FastoCommonItem::setValue(core::NValue val) {
  key_.SetValue(val);
}

core::NValue FastoCommonItem::nvalue() const {
  return key_.GetValue();
}

core::NDbKValue FastoCommonItem::dbv() const {
  return key_;
}

const char* FastoCommonItem::delimiter() const {
  return delimiter_.c_str();
}

common::Value::Type FastoCommonItem::type() const {
  return key_.GetType();
  QString qvalstr;
}

bool FastoCommonItem::isReadOnly() const {
  return read_only_;
}

QString toJson(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    std::string json = common::ConvertToString(item->value());
    json_object* obj = json_tokener_parse(json.c_str());
    if (!obj) {
      return QString();
    }

    std::string jstring = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY);
    QString result;
    common::ConvertFromString(jstring, &result);
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
    DNOTREACHED() << "Invalid input.";
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

QString toCsv(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    QString val = item->value();
    return val.replace(item->delimiter(), ",");
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toCsv(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
    if (i != item->childrenCount() - 1) {
      value += CSV_SEPARATOR;
    }
  }

  return value;
}

QString fromSnappy(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    QString val = item->value();
    std::string sval = common::ConvertToString(val);
    std::string out;
    common::CompressSnappyEDcoder enc;
    common::Error err = enc.Decode(sval, &out);
    if (err) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromSnappy(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString fromGzip(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    QString val = item->value();
    std::string sval = common::ConvertToString(val);
    std::string out;
    common::CompressZlibEDcoder enc;
    common::Error err = enc.Decode(sval, &out);
    if (err) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromGzip(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString fromLZ4(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    QString val = item->value();
    std::string sval = common::ConvertToString(val);
    std::string out;
    common::CompressLZ4EDcoder enc;
    common::Error err = enc.Decode(sval, &out);
    if (err) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromLZ4(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString fromBZip2(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    QString val = item->value();
    std::string sval = common::ConvertToString(val);
    std::string out;
    common::CompressBZip2EDcoder enc;
    common::Error err = enc.Decode(sval, &out);
    if (err) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromBZip2(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString fromHexMsgPack(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    std::string sval = common::ConvertToString(item->value());

    common::HexEDcoder hex;
    std::string hexstr;
    common::Error err = hex.Decode(sval, &hexstr);
    if (err) {
      return QString();
    }

    common::MsgPackEDcoder msg;
    std::string upack;
    err = msg.Decode(hexstr, &upack);
    if (err) {
      return QString();
    }

    QString qupack;
    common::ConvertFromString(upack, &qupack);
    return qupack;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromHexMsgPack(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

}  // namespace gui
}  // namespace fastonosql
