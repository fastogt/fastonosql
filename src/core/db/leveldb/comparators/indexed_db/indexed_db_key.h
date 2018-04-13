// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <stddef.h>
#include <string>
#include <vector>

#include <common/string16.h>

#include "core/db/leveldb/comparators/indexed_db/WebIDBTypes.h"

/*
#include "base/logging.h"
#include "base/strings/string16.h"
#include "content/common/content_export.h"
#include "third_party/WebKit/public/platform/modules/indexeddb/WebIDBTypes.h"*/

namespace fastonosql {
namespace core {
namespace leveldb {
namespace comparator {
namespace indexed_db {

class IndexedDBKey {
 public:
  typedef std::vector<IndexedDBKey> KeyArray;
  IndexedDBKey();                               // Defaults to blink::WebIDBKeyTypeInvalid.
  explicit IndexedDBKey(blink::WebIDBKeyType);  // must be Null or Invalid
  explicit IndexedDBKey(const KeyArray& array);
  explicit IndexedDBKey(const std::string& binary);
  explicit IndexedDBKey(const common::string16& string);
  IndexedDBKey(double number,
               blink::WebIDBKeyType type);  // must be date or number
  IndexedDBKey(const IndexedDBKey& other);
  ~IndexedDBKey();
  IndexedDBKey& operator=(const IndexedDBKey& other);
  bool IsValid() const;
  bool IsLessThan(const IndexedDBKey& other) const;
  bool Equals(const IndexedDBKey& other) const;
  blink::WebIDBKeyType type() const { return type_; }
  const std::vector<IndexedDBKey>& array() const {
    DCHECK_EQ(type_, blink::kWebIDBKeyTypeArray);
    return array_;
  }
  const std::string& binary() const {
    DCHECK_EQ(type_, blink::kWebIDBKeyTypeBinary);
    return binary_;
  }
  const common::string16& string() const {
    DCHECK_EQ(type_, blink::kWebIDBKeyTypeString);
    return string_;
  }
  double date() const {
    DCHECK_EQ(type_, blink::kWebIDBKeyTypeDate);
    return number_;
  }
  double number() const {
    DCHECK_EQ(type_, blink::kWebIDBKeyTypeNumber);
    return number_;
  }
  size_t size_estimate() const { return size_estimate_; }

 private:
  int CompareTo(const IndexedDBKey& other) const;
  blink::WebIDBKeyType type_;
  std::vector<IndexedDBKey> array_;
  std::string binary_;
  common::string16 string_;
  double number_ = 0;
  size_t size_estimate_;
};
// An index id, and corresponding set of keys to insert.
using IndexedDBIndexKeys = std::pair<int64_t, std::vector<IndexedDBKey>>;

}  // namespace indexed_db
}  // namespace comparator
}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
