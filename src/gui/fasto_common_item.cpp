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

#include "gui/text_converter.h"

#include <common/qt/convert2string.h>  // for ConvertToString

namespace fastonosql {
namespace gui {

FastoCommonItem::FastoCommonItem(const core::NDbKValue& key,
                                 const std::string& delimiter,
                                 bool read_only,
                                 TreeItem* parent,
                                 void* internalPointer)
    : TreeItem(parent, internalPointer), key_(key), delimiter_(delimiter), read_only_(read_only) {}

QString FastoCommonItem::key() const {
  QString qkey;
  const core::NKey key = key_.GetKey();
  const core::key_t raw_key = key.GetKey();
  common::ConvertFromString(raw_key.GetHumanReadable(), &qkey);
  return qkey;
}

core::value_t FastoCommonItem::coreValue() const {
  core::NValue nval = key_.GetValue();
  return nval.GetValue(delimiter_);
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
    core::value_t val = item->coreValue();
    std::string jstring;
    if (!string_to_json(val.GetData(), &jstring)) {
      return QString();
    }

    QString result;
    common::ConvertFromString(jstring, &result);
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
    QString result;
    core::value_t val = item->coreValue();
    common::ConvertFromString(val.GetData(), &result);
    return result;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toRaw(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString toXml(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    QString result;
    core::value_t val = item->coreValue();
    common::ConvertFromString(val.GetData(), &result);
    return result;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toXml(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString toHex(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_to_hex(val.GetData(), &out)) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toHex(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString fromHex(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_from_hex(val.GetData(), &out)) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromHex(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString toUnicode(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_to_unicode(val.GetData(), &out)) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += toUnicode(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString fromUnicode(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_from_unicode(val.GetData(), &out)) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromUnicode(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

QString fromSnappy(FastoCommonItem* item) {
  if (!item) {
    DNOTREACHED() << "Invalid input.";
    return QString();
  }

  if (!item->childrenCount()) {
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_from_snappy(val.GetData(), &out)) {
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
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_from_zlib(val.GetData(), &out)) {
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
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_from_lz4(val.GetData(), &out)) {
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
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_from_bzip2(val.GetData(), &out)) {
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
    core::value_t val = item->coreValue();
    std::string out;
    if (!string_from_msgpack(val.GetData(), &out)) {
      return QString();
    }

    QString qout;
    common::ConvertFromString(out, &qout);
    return qout;
  }

  QString value;
  for (size_t i = 0; i < item->childrenCount(); ++i) {
    value += fromHexMsgPack(dynamic_cast<FastoCommonItem*>(item->child(i)));  // +
  }

  return value;
}

}  // namespace gui
}  // namespace fastonosql
