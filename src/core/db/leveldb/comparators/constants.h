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

#pragma once

namespace fastonosql {
namespace core {
namespace leveldb {
namespace comparator {

static const unsigned char kMaxSimpleGlobalMetaDataTypeByte = 5;  // Insert before this and increment.
static const unsigned char kDatabaseFreeListTypeByte = 100;
static const unsigned char kDatabaseNameTypeByte = 201;

static const unsigned char kObjectStoreMetaDataTypeByte = 50;
static const unsigned char kIndexMetaDataTypeByte = 100;
static const unsigned char kObjectStoreFreeListTypeByte = 150;
static const unsigned char kIndexFreeListTypeByte = 151;
static const unsigned char kObjectStoreNamesTypeByte = 200;
static const unsigned char kIndexNamesKeyTypeByte = 201;

static const unsigned char kObjectStoreDataIndexId = 1;
static const unsigned char kExistsEntryIndexId = 2;
static const unsigned char kBlobEntryIndexId = 3;
static const unsigned char kMinimumIndexId = 30;

static const unsigned char kIndexedDBKeyNullTypeByte = 0;
static const unsigned char kIndexedDBKeyStringTypeByte = 1;
static const unsigned char kIndexedDBKeyDateTypeByte = 2;
static const unsigned char kIndexedDBKeyNumberTypeByte = 3;
static const unsigned char kIndexedDBKeyArrayTypeByte = 4;
static const unsigned char kIndexedDBKeyMinKeyTypeByte = 5;
static const unsigned char kIndexedDBKeyBinaryTypeByte = 6;

enum WebIDBKeyType {
  kWebIDBKeyTypeInvalid = 0,
  kWebIDBKeyTypeArray,
  kWebIDBKeyTypeBinary,
  kWebIDBKeyTypeString,
  kWebIDBKeyTypeDate,
  kWebIDBKeyTypeNumber,
  kWebIDBKeyTypeNull,
  kWebIDBKeyTypeMin,
};

}  // namespace comparator
}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
