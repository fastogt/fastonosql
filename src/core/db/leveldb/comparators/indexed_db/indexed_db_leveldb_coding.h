// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <common/string16.h>
#include <common/string_piece.h>

#include "core/db/leveldb/comparators/indexed_db/indexed_db_key.h"

/*#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "content/common/indexed_db/indexed_db_key.h"
#include "content/common/indexed_db/indexed_db_key_path.h"*/

namespace fastonosql {
namespace core {
namespace leveldb {
namespace comparator {
namespace indexed_db {

extern const unsigned char kMinimumIndexId;
std::string MaxIDBKey();
std::string MinIDBKey();
// DatabaseId, BlobKey
typedef std::pair<int64_t, int64_t> BlobJournalEntryType;
typedef std::vector<BlobJournalEntryType> BlobJournalType;
void EncodeByte(unsigned char value, std::string* into);
void EncodeBool(bool value, std::string* into);
void EncodeInt(int64_t value, std::string* into);
void EncodeVarInt(int64_t value, std::string* into);
void EncodeString(const common::string16& value, std::string* into);
void EncodeStringWithLength(const common::string16& value, std::string* into);
void EncodeBinary(const std::string& value, std::string* into);
void EncodeDouble(double value, std::string* into);
void EncodeIDBKey(const IndexedDBKey& value, std::string* into);
// void EncodeIDBKeyPath(const IndexedDBKeyPath& value, std::string* into);
void EncodeBlobJournal(const BlobJournalType& journal, std::string* into);
bool DecodeByte(common::StringPiece* slice, unsigned char* value) WARN_UNUSED_RESULT;
bool DecodeBool(common::StringPiece* slice, bool* value) WARN_UNUSED_RESULT;
bool DecodeInt(common::StringPiece* slice, int64_t* value) WARN_UNUSED_RESULT;
bool DecodeVarInt(common::StringPiece* slice, int64_t* value) WARN_UNUSED_RESULT;
bool DecodeString(common::StringPiece* slice, common::string16* value) WARN_UNUSED_RESULT;
bool DecodeStringWithLength(common::StringPiece* slice, common::string16* value) WARN_UNUSED_RESULT;
bool DecodeBinary(common::StringPiece* slice, std::string* value) WARN_UNUSED_RESULT;
bool DecodeDouble(common::StringPiece* slice, double* value) WARN_UNUSED_RESULT;
bool DecodeIDBKey(common::StringPiece* slice, std::unique_ptr<IndexedDBKey>* value) WARN_UNUSED_RESULT;
// bool DecodeIDBKeyPath(common::StringPiece* slice, IndexedDBKeyPath* value) WARN_UNUSED_RESULT;
bool DecodeBlobJournal(common::StringPiece* slice, BlobJournalType* journal) WARN_UNUSED_RESULT;
int CompareEncodedStringsWithLength(common::StringPiece* slice1, common::StringPiece* slice2, bool* ok);
bool ExtractEncodedIDBKey(common::StringPiece* slice, std::string* result) WARN_UNUSED_RESULT;
int CompareEncodedIDBKeys(common::StringPiece* slice1, common::StringPiece* slice2, bool* ok);
int Compare(const common::StringPiece& a, const common::StringPiece& b, bool index_keys);
int CompareKeys(const common::StringPiece& a, const common::StringPiece& b);
int CompareIndexKeys(const common::StringPiece& a, const common::StringPiece& b);
class KeyPrefix {
 public:
  // These are serialized to disk; any new items must be appended, and none can
  // be deleted.
  enum Type {
    GLOBAL_METADATA = 0,
    DATABASE_METADATA = 1,
    OBJECT_STORE_DATA = 2,
    EXISTS_ENTRY = 3,
    INDEX_DATA = 4,
    INVALID_TYPE = 5,
    BLOB_ENTRY = 6
  };
  static const size_t kMaxDatabaseIdSizeBits = 3;
  static const size_t kMaxObjectStoreIdSizeBits = 3;
  static const size_t kMaxIndexIdSizeBits = 2;
  static const size_t kMaxDatabaseIdSizeBytes = 1ULL << kMaxDatabaseIdSizeBits;        // 8
  static const size_t kMaxObjectStoreIdSizeBytes = 1ULL << kMaxObjectStoreIdSizeBits;  // 8
  static const size_t kMaxIndexIdSizeBytes = 1ULL << kMaxIndexIdSizeBits;              // 4
  static const size_t kMaxDatabaseIdBits = kMaxDatabaseIdSizeBytes * 8 - 1;            // 63
  static const size_t kMaxObjectStoreIdBits = kMaxObjectStoreIdSizeBytes * 8 - 1;      // 63
  static const size_t kMaxIndexIdBits = kMaxIndexIdSizeBytes * 8 - 1;                  // 31
  static const int64_t kMaxDatabaseId = (1ULL << kMaxDatabaseIdBits) - 1;              // max signed int64_t
  static const int64_t kMaxObjectStoreId = (1ULL << kMaxObjectStoreIdBits) - 1;        // max signed int64_t
  static const int64_t kMaxIndexId = (1ULL << kMaxIndexIdBits) - 1;                    // max signed int32_t
  static const int64_t kInvalidId = -1;
  KeyPrefix();
  explicit KeyPrefix(int64_t database_id);
  KeyPrefix(int64_t database_id, int64_t object_store_id);
  KeyPrefix(int64_t database_id, int64_t object_store_id, int64_t index_id);
  static KeyPrefix CreateWithSpecialIndex(int64_t database_id, int64_t object_store_id, int64_t index_id);
  static bool Decode(common::StringPiece* slice, KeyPrefix* result);
  std::string Encode() const;
  static std::string EncodeEmpty();
  int Compare(const KeyPrefix& other) const;
  static bool IsValidDatabaseId(int64_t database_id);
  static bool IsValidObjectStoreId(int64_t index_id);
  static bool IsValidIndexId(int64_t index_id);
  static bool ValidIds(int64_t database_id, int64_t object_store_id, int64_t index_id) {
    return IsValidDatabaseId(database_id) && IsValidObjectStoreId(object_store_id) && IsValidIndexId(index_id);
  }
  static bool ValidIds(int64_t database_id, int64_t object_store_id) {
    return IsValidDatabaseId(database_id) && IsValidObjectStoreId(object_store_id);
  }
  Type type() const;
  int64_t database_id_;
  int64_t object_store_id_;
  int64_t index_id_;

 private:
  // Special constructor for CreateWithSpecialIndex()
  KeyPrefix(enum Type, int64_t database_id, int64_t object_store_id, int64_t index_id);
  static std::string EncodeInternal(int64_t database_id, int64_t object_store_id, int64_t index_id);
};
class SchemaVersionKey {
 public:
  static std::string Encode();
};
class MaxDatabaseIdKey {
 public:
  static std::string Encode();
};
class DataVersionKey {
 public:
  static std::string Encode();
};
class BlobJournalKey {
 public:
  static std::string Encode();
};
class LiveBlobJournalKey {
 public:
  static std::string Encode();
};
class EarliestSweepKey {
 public:
  static std::string Encode();
};
class DatabaseFreeListKey {
 public:
  DatabaseFreeListKey();
  static bool Decode(common::StringPiece* slice, DatabaseFreeListKey* result);
  static std::string Encode(int64_t database_id);
  static std::string EncodeMaxKey();
  int64_t DatabaseId() const;
  int Compare(const DatabaseFreeListKey& other) const;

 private:
  int64_t database_id_;
};
class DatabaseNameKey {
 public:
  static bool Decode(common::StringPiece* slice, DatabaseNameKey* result);
  static std::string Encode(const std::string& origin_identifier, const common::string16& database_name);
  static std::string EncodeMinKeyForOrigin(const std::string& origin_identifier);
  static std::string EncodeStopKeyForOrigin(const std::string& origin_identifier);
  common::string16 origin() const { return origin_; }
  common::string16 database_name() const { return database_name_; }
  int Compare(const DatabaseNameKey& other);

 private:
  common::string16 origin_;  // TODO(jsbell): Store encoded strings, or just
                             // pointers.
  common::string16 database_name_;
};
class DatabaseMetaDataKey {
 public:
  enum MetaDataType {
    ORIGIN_NAME = 0,
    DATABASE_NAME = 1,
    USER_STRING_VERSION = 2,  // Obsolete
    MAX_OBJECT_STORE_ID = 3,
    USER_VERSION = 4,
    BLOB_KEY_GENERATOR_CURRENT_NUMBER = 5,
    MAX_SIMPLE_METADATA_TYPE = 6
  };
  static const int64_t kAllBlobsKey;
  static const int64_t kBlobKeyGeneratorInitialNumber;
  // All keys <= 0 are invalid.  This one's just a convenient example.
  static const int64_t kInvalidBlobKey;
  static bool IsValidBlobKey(int64_t blob_key);
  static std::string Encode(int64_t database_id, MetaDataType type);
};
class ObjectStoreMetaDataKey {
 public:
  enum MetaDataType {
    NAME = 0,
    KEY_PATH = 1,
    AUTO_INCREMENT = 2,
    EVICTABLE = 3,
    LAST_VERSION = 4,
    MAX_INDEX_ID = 5,
    HAS_KEY_PATH = 6,
    KEY_GENERATOR_CURRENT_NUMBER = 7
  };
  // From the IndexedDB specification.
  static const int64_t kKeyGeneratorInitialNumber;
  ObjectStoreMetaDataKey();
  static bool Decode(common::StringPiece* slice, ObjectStoreMetaDataKey* result);
  static std::string Encode(int64_t database_id, int64_t object_store_id, unsigned char meta_data_type);
  static std::string EncodeMaxKey(int64_t database_id);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id);
  int64_t ObjectStoreId() const;
  unsigned char MetaDataType() const;
  int Compare(const ObjectStoreMetaDataKey& other);

 private:
  int64_t object_store_id_;
  unsigned char meta_data_type_;
};
class IndexMetaDataKey {
 public:
  enum MetaDataType { NAME = 0, UNIQUE = 1, KEY_PATH = 2, MULTI_ENTRY = 3 };
  IndexMetaDataKey();
  static bool Decode(common::StringPiece* slice, IndexMetaDataKey* result);
  static std::string Encode(int64_t database_id,
                            int64_t object_store_id,
                            int64_t index_id,
                            unsigned char meta_data_type);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id, int64_t index_id);
  int Compare(const IndexMetaDataKey& other);
  int64_t IndexId() const;
  unsigned char meta_data_type() const { return meta_data_type_; }

 private:
  int64_t object_store_id_;
  int64_t index_id_;
  unsigned char meta_data_type_;
};
class ObjectStoreFreeListKey {
 public:
  ObjectStoreFreeListKey();
  static bool Decode(common::StringPiece* slice, ObjectStoreFreeListKey* result);
  static std::string Encode(int64_t database_id, int64_t object_store_id);
  static std::string EncodeMaxKey(int64_t database_id);
  int64_t ObjectStoreId() const;
  int Compare(const ObjectStoreFreeListKey& other);

 private:
  int64_t object_store_id_;
};
class IndexFreeListKey {
 public:
  IndexFreeListKey();
  static bool Decode(common::StringPiece* slice, IndexFreeListKey* result);
  static std::string Encode(int64_t database_id, int64_t object_store_id, int64_t index_id);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id);
  int Compare(const IndexFreeListKey& other);
  int64_t ObjectStoreId() const;
  int64_t IndexId() const;

 private:
  int64_t object_store_id_;
  int64_t index_id_;
};
class ObjectStoreNamesKey {
 public:
  // TODO(jsbell): We never use this to look up object store ids,
  // because a mapping is kept in the IndexedDBDatabase. Can the
  // mapping become unreliable?  Can we remove this?
  static bool Decode(common::StringPiece* slice, ObjectStoreNamesKey* result);
  static std::string Encode(int64_t database_id, const common::string16& object_store_name);
  int Compare(const ObjectStoreNamesKey& other);
  common::string16 object_store_name() const { return object_store_name_; }

 private:
  // TODO(jsbell): Store the encoded string, or just pointers to it.
  common::string16 object_store_name_;
};
class IndexNamesKey {
 public:
  IndexNamesKey();
  // TODO(jsbell): We never use this to look up index ids, because a mapping
  // is kept at a higher level.
  static bool Decode(common::StringPiece* slice, IndexNamesKey* result);
  static std::string Encode(int64_t database_id, int64_t object_store_id, const common::string16& index_name);
  int Compare(const IndexNamesKey& other);
  common::string16 index_name() const { return index_name_; }

 private:
  int64_t object_store_id_;
  common::string16 index_name_;
};
class ObjectStoreDataKey {
 public:
  static const int64_t kSpecialIndexNumber;
  ObjectStoreDataKey();
  ~ObjectStoreDataKey();
  static bool Decode(common::StringPiece* slice, ObjectStoreDataKey* result);
  static std::string Encode(int64_t database_id, int64_t object_store_id, const std::string& encoded_user_key);
  static std::string Encode(int64_t database_id, int64_t object_store_id, const IndexedDBKey& user_key);
  std::unique_ptr<IndexedDBKey> user_key() const;

 private:
  std::string encoded_user_key_;
};
class ExistsEntryKey {
 public:
  ExistsEntryKey();
  ~ExistsEntryKey();
  static bool Decode(common::StringPiece* slice, ExistsEntryKey* result);
  static std::string Encode(int64_t database_id, int64_t object_store_id, const std::string& encoded_key);
  static std::string Encode(int64_t database_id, int64_t object_store_id, const IndexedDBKey& user_key);
  std::unique_ptr<IndexedDBKey> user_key() const;

 private:
  static const int64_t kSpecialIndexNumber;
  std::string encoded_user_key_;
  DISALLOW_COPY_AND_ASSIGN(ExistsEntryKey);
};
class BlobEntryKey {
 public:
  BlobEntryKey() : database_id_(0), object_store_id_(0) {}
  static bool Decode(common::StringPiece* slice, BlobEntryKey* result);
  static bool FromObjectStoreDataKey(common::StringPiece* slice, BlobEntryKey* result);
  static std::string ReencodeToObjectStoreDataKey(common::StringPiece* slice);
  static std::string EncodeMinKeyForObjectStore(int64_t database_id, int64_t object_store_id);
  static std::string EncodeStopKeyForObjectStore(int64_t database_id, int64_t object_store_id);
  static std::string Encode(int64_t database_id, int64_t object_store_id, const IndexedDBKey& user_key);
  std::string Encode() const;
  int64_t database_id() const { return database_id_; }
  int64_t object_store_id() const { return object_store_id_; }

 private:
  static const int64_t kSpecialIndexNumber;
  static std::string Encode(int64_t database_id, int64_t object_store_id, const std::string& encoded_user_key);
  int64_t database_id_;
  int64_t object_store_id_;
  // This is the user's ObjectStoreDataKey, not the BlobEntryKey itself.
  std::string encoded_user_key_;
};
class IndexDataKey {
 public:
  IndexDataKey();
  IndexDataKey(IndexDataKey&& other);
  ~IndexDataKey();
  static bool Decode(common::StringPiece* slice, IndexDataKey* result);
  static std::string Encode(int64_t database_id,
                            int64_t object_store_id,
                            int64_t index_id,
                            const std::string& encoded_user_key,
                            const std::string& encoded_primary_key,
                            int64_t sequence_number);
  static std::string Encode(int64_t database_id,
                            int64_t object_store_id,
                            int64_t index_id,
                            const IndexedDBKey& user_key);
  static std::string Encode(int64_t database_id,
                            int64_t object_store_id,
                            int64_t index_id,
                            const IndexedDBKey& user_key,
                            const IndexedDBKey& user_primary_key);
  static std::string EncodeMinKey(int64_t database_id, int64_t object_store_id, int64_t index_id);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id, int64_t index_id);
  int64_t DatabaseId() const;
  int64_t ObjectStoreId() const;
  int64_t IndexId() const;
  std::unique_ptr<IndexedDBKey> user_key() const;
  std::unique_ptr<IndexedDBKey> primary_key() const;
  std::string Encode() const;

 private:
  int64_t database_id_;
  int64_t object_store_id_;
  int64_t index_id_;
  std::string encoded_user_key_;
  std::string encoded_primary_key_;
  int64_t sequence_number_;
  DISALLOW_COPY_AND_ASSIGN(IndexDataKey);
};

}  // namespace indexed_db
}  // namespace comparator
}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
