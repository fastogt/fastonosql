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

#include "core/db/leveldb/comparators/indexed_db.h"

#include <memory>

#include <leveldb/slice.h>

#include <common/macros.h>

#include "core/db/leveldb/comparators/constants.h"
#include "core/db/leveldb/comparators/utils.h"

namespace fastonosql {
namespace core {
namespace leveldb {
namespace comparator {

namespace detail {

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

  static const size_t kMaxDatabaseIdBits = kMaxDatabaseIdSizeBytes * 8 - 1;        // 63
  static const size_t kMaxObjectStoreIdBits = kMaxObjectStoreIdSizeBytes * 8 - 1;  // 63
  static const size_t kMaxIndexIdBits = kMaxIndexIdSizeBytes * 8 - 1;              // 31

  static const int64_t kMaxDatabaseId = (1ULL << kMaxDatabaseIdBits) - 1;        // max signed int64_t
  static const int64_t kMaxObjectStoreId = (1ULL << kMaxObjectStoreIdBits) - 1;  // max signed int64_t
  static const int64_t kMaxIndexId = (1ULL << kMaxIndexIdBits) - 1;              // max signed int32_t

  static const int64_t kInvalidId = -1;

  KeyPrefix() : database_id_(INVALID_TYPE), object_store_id_(INVALID_TYPE), index_id_(INVALID_TYPE) {}
  explicit KeyPrefix(int64_t database_id);
  KeyPrefix(int64_t database_id, int64_t object_store_id);
  KeyPrefix(int64_t database_id, int64_t object_store_id, int64_t index_id);
  static KeyPrefix CreateWithSpecialIndex(int64_t database_id, int64_t object_store_id, int64_t index_id);

  static bool Decode(common::StringPiece* slice, KeyPrefix* result) {
    unsigned char first_byte;
    if (!DecodeByte(slice, &first_byte))
      return false;

    size_t database_id_bytes = ((first_byte >> 5) & 0x7) + 1;
    size_t object_store_id_bytes = ((first_byte >> 2) & 0x7) + 1;
    size_t index_id_bytes = (first_byte & 0x3) + 1;

    if (database_id_bytes + object_store_id_bytes + index_id_bytes > slice->size())
      return false;

    {
      common::StringPiece tmp(slice->begin(), database_id_bytes);
      if (!DecodeInt(&tmp, &result->database_id_))
        return false;
    }
    slice->remove_prefix(database_id_bytes);
    {
      common::StringPiece tmp(slice->begin(), object_store_id_bytes);
      if (!DecodeInt(&tmp, &result->object_store_id_))
        return false;
    }
    slice->remove_prefix(object_store_id_bytes);
    {
      common::StringPiece tmp(slice->begin(), index_id_bytes);
      if (!DecodeInt(&tmp, &result->index_id_))
        return false;
    }
    slice->remove_prefix(index_id_bytes);
    return true;
  }
  std::string Encode() const;
  static std::string EncodeEmpty();
  int Compare(const KeyPrefix& other) const {
    DCHECK(database_id_ != kInvalidId);
    DCHECK(object_store_id_ != kInvalidId);
    DCHECK(index_id_ != kInvalidId);

    if (database_id_ != other.database_id_)
      return CompareInts(database_id_, other.database_id_);
    if (object_store_id_ != other.object_store_id_)
      return CompareInts(object_store_id_, other.object_store_id_);
    if (index_id_ != other.index_id_)
      return CompareInts(index_id_, other.index_id_);
    return 0;
  }

  static bool IsValidDatabaseId(int64_t database_id);
  static bool IsValidObjectStoreId(int64_t index_id);
  static bool IsValidIndexId(int64_t index_id);
  static bool ValidIds(int64_t database_id, int64_t object_store_id, int64_t index_id) {
    return IsValidDatabaseId(database_id) && IsValidObjectStoreId(object_store_id) && IsValidIndexId(index_id);
  }
  static bool ValidIds(int64_t database_id, int64_t object_store_id) {
    return IsValidDatabaseId(database_id) && IsValidObjectStoreId(object_store_id);
  }

  Type type() const {
    DCHECK(database_id_ != kInvalidId);
    DCHECK(object_store_id_ != kInvalidId);
    DCHECK(index_id_ != kInvalidId);

    if (!database_id_)
      return GLOBAL_METADATA;
    if (!object_store_id_)
      return DATABASE_METADATA;
    if (index_id_ == kObjectStoreDataIndexId)
      return OBJECT_STORE_DATA;
    if (index_id_ == kExistsEntryIndexId)
      return EXISTS_ENTRY;
    if (index_id_ == kBlobEntryIndexId)
      return BLOB_ENTRY;
    if (index_id_ >= kMinimumIndexId)
      return INDEX_DATA;

    NOTREACHED();
    return INVALID_TYPE;
  }

  int64_t database_id_;
  int64_t object_store_id_;
  int64_t index_id_;

 private:
  // Special constructor for CreateWithSpecialIndex()
  KeyPrefix(enum Type, int64_t database_id, int64_t object_store_id, int64_t index_id);

  static std::string EncodeInternal(int64_t database_id, int64_t object_store_id, int64_t index_id);
};

class DatabaseFreeListKey {
 public:
  DatabaseFreeListKey() : database_id_(-1) {}
  static bool Decode(common::StringPiece* slice, DatabaseFreeListKey* result) {
    KeyPrefix prefix;
    if (!KeyPrefix::Decode(slice, &prefix))
      return false;
    DCHECK(!prefix.database_id_);
    DCHECK(!prefix.object_store_id_);
    DCHECK(!prefix.index_id_);
    unsigned char type_byte = 0;
    if (!DecodeByte(slice, &type_byte))
      return false;
    DCHECK_EQ(type_byte, kDatabaseFreeListTypeByte);
    if (!DecodeVarInt(slice, &result->database_id_))
      return false;
    return true;
  }
  static std::string Encode(int64_t database_id);
  static std::string EncodeMaxKey();
  int64_t DatabaseId() const;
  int Compare(const DatabaseFreeListKey& other) const {
    DCHECK_GE(database_id_, 0);
    return CompareInts(database_id_, other.database_id_);
  }

 private:
  int64_t database_id_;
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

class DatabaseNameKey {
 public:
  static bool Decode(common::StringPiece* slice, DatabaseNameKey* result) {
    KeyPrefix prefix;
    if (!KeyPrefix::Decode(slice, &prefix))
      return false;
    DCHECK(!prefix.database_id_);
    DCHECK(!prefix.object_store_id_);
    DCHECK(!prefix.index_id_);
    unsigned char type_byte = 0;
    if (!DecodeByte(slice, &type_byte))
      return false;
    DCHECK_EQ(type_byte, kDatabaseNameTypeByte);
    if (!DecodeStringWithLength(slice, &result->origin_))
      return false;
    if (!DecodeStringWithLength(slice, &result->database_name_))
      return false;
    return true;
  }
  static std::string Encode(const std::string& origin_identifier, const common::string16& database_name);
  static std::string EncodeMinKeyForOrigin(const std::string& origin_identifier);
  static std::string EncodeStopKeyForOrigin(const std::string& origin_identifier);
  common::string16 origin() const { return origin_; }
  common::string16 database_name() const { return database_name_; }
  int Compare(const DatabaseNameKey& other) {
    if (int x = origin_.compare(other.origin_))
      return x;
    return database_name_.compare(other.database_name_);
  }

 private:
  common::string16 origin_;  // TODO(jsbell): Store encoded strings, or just
                             // pointers.
  common::string16 database_name_;
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

  ObjectStoreMetaDataKey() : object_store_id_(-1), meta_data_type_(0xFF) {}
  static bool Decode(common::StringPiece* slice, ObjectStoreMetaDataKey* result) {
    KeyPrefix prefix;
    if (!KeyPrefix::Decode(slice, &prefix))
      return false;
    DCHECK(prefix.database_id_);
    DCHECK(!prefix.object_store_id_);
    DCHECK(!prefix.index_id_);
    unsigned char type_byte = 0;
    if (!DecodeByte(slice, &type_byte))
      return false;
    DCHECK_EQ(type_byte, kObjectStoreMetaDataTypeByte);
    if (!DecodeVarInt(slice, &result->object_store_id_))
      return false;
    DCHECK(result->object_store_id_);
    if (!DecodeByte(slice, &result->meta_data_type_))
      return false;
    return true;
  }
  static std::string Encode(int64_t database_id, int64_t object_store_id, unsigned char meta_data_type);
  static std::string EncodeMaxKey(int64_t database_id);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id);
  int64_t ObjectStoreId() const;
  unsigned char MetaDataType() const;
  int Compare(const ObjectStoreMetaDataKey& other) {
    DCHECK_GE(object_store_id_, 0);
    if (int x = CompareInts(object_store_id_, other.object_store_id_))
      return x;
    return meta_data_type_ - other.meta_data_type_;
  }

 private:
  int64_t object_store_id_;
  unsigned char meta_data_type_;
};

class IndexMetaDataKey {
 public:
  enum MetaDataType { NAME = 0, UNIQUE = 1, KEY_PATH = 2, MULTI_ENTRY = 3 };

  IndexMetaDataKey() : object_store_id_(-1), index_id_(-1), meta_data_type_(0) {}
  static bool Decode(common::StringPiece* slice, IndexMetaDataKey* result) {
    KeyPrefix prefix;
    if (!KeyPrefix::Decode(slice, &prefix))
      return false;
    DCHECK(prefix.database_id_);
    DCHECK(!prefix.object_store_id_);
    DCHECK(!prefix.index_id_);
    unsigned char type_byte = 0;
    if (!DecodeByte(slice, &type_byte))
      return false;
    DCHECK_EQ(type_byte, kIndexMetaDataTypeByte);
    if (!DecodeVarInt(slice, &result->object_store_id_))
      return false;
    if (!DecodeVarInt(slice, &result->index_id_))
      return false;
    if (!DecodeByte(slice, &result->meta_data_type_))
      return false;
    return true;
  }
  static std::string Encode(int64_t database_id,
                            int64_t object_store_id,
                            int64_t index_id,
                            unsigned char meta_data_type);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id, int64_t index_id);
  int Compare(const IndexMetaDataKey& other) {
    DCHECK_GE(object_store_id_, 0);
    DCHECK_GE(index_id_, 0);

    if (int x = CompareInts(object_store_id_, other.object_store_id_))
      return x;
    if (int x = CompareInts(index_id_, other.index_id_))
      return x;
    return meta_data_type_ - other.meta_data_type_;
  }
  int64_t IndexId() const;
  unsigned char meta_data_type() const { return meta_data_type_; }

 private:
  int64_t object_store_id_;
  int64_t index_id_;
  unsigned char meta_data_type_;
};

class ObjectStoreFreeListKey {
 public:
  ObjectStoreFreeListKey() : object_store_id_(-1) {}
  static bool Decode(common::StringPiece* slice, ObjectStoreFreeListKey* result) {
    KeyPrefix prefix;
    if (!KeyPrefix::Decode(slice, &prefix))
      return false;
    DCHECK(prefix.database_id_);
    DCHECK(!prefix.object_store_id_);
    DCHECK(!prefix.index_id_);
    unsigned char type_byte = 0;
    if (!DecodeByte(slice, &type_byte))
      return false;
    DCHECK_EQ(type_byte, kObjectStoreFreeListTypeByte);
    if (!DecodeVarInt(slice, &result->object_store_id_))
      return false;
    return true;
  }
  static std::string Encode(int64_t database_id, int64_t object_store_id);
  static std::string EncodeMaxKey(int64_t database_id);
  int64_t ObjectStoreId() const;
  int Compare(const ObjectStoreFreeListKey& other) {
    // TODO(jsbell): It may seem strange that we're not comparing database id's,
    // but that comparison will have been made earlier.
    // We should probably make this more clear, though...
    DCHECK_GE(object_store_id_, 0);
    return CompareInts(object_store_id_, other.object_store_id_);
  }

 private:
  int64_t object_store_id_;
};

class IndexFreeListKey {
 public:
  IndexFreeListKey() : object_store_id_(-1), index_id_(-1) {}
  static bool Decode(common::StringPiece* slice, IndexFreeListKey* result) {
    KeyPrefix prefix;
    if (!KeyPrefix::Decode(slice, &prefix))
      return false;
    DCHECK(prefix.database_id_);
    DCHECK(!prefix.object_store_id_);
    DCHECK(!prefix.index_id_);
    unsigned char type_byte = 0;
    if (!DecodeByte(slice, &type_byte))
      return false;
    DCHECK_EQ(type_byte, kIndexFreeListTypeByte);
    if (!DecodeVarInt(slice, &result->object_store_id_))
      return false;
    if (!DecodeVarInt(slice, &result->index_id_))
      return false;
    return true;
  }

  static std::string Encode(int64_t database_id, int64_t object_store_id, int64_t index_id);
  static std::string EncodeMaxKey(int64_t database_id, int64_t object_store_id);
  int Compare(const IndexFreeListKey& other) {
    DCHECK_GE(object_store_id_, 0);
    DCHECK_GE(index_id_, 0);
    if (int x = CompareInts(object_store_id_, other.object_store_id_))
      return x;
    return CompareInts(index_id_, other.index_id_);
  }
  int64_t ObjectStoreId() const;
  int64_t IndexId() const;

 private:
  int64_t object_store_id_;
  int64_t index_id_;
};

class IndexNamesKey {
 public:
  IndexNamesKey() : object_store_id_(-1) {}
  // TODO(jsbell): We never use this to look up index ids, because a mapping
  // is kept at a higher level.
  static bool Decode(common::StringPiece* slice, IndexNamesKey* result) {
    KeyPrefix prefix;
    if (!KeyPrefix::Decode(slice, &prefix))
      return false;
    DCHECK(prefix.database_id_);
    DCHECK(!prefix.object_store_id_);
    DCHECK(!prefix.index_id_);
    unsigned char type_byte = 0;
    if (!DecodeByte(slice, &type_byte))
      return false;
    DCHECK_EQ(type_byte, kIndexNamesKeyTypeByte);
    if (!DecodeVarInt(slice, &result->object_store_id_))
      return false;
    if (!DecodeStringWithLength(slice, &result->index_name_))
      return false;
    return true;
  }
  static std::string Encode(int64_t database_id, int64_t object_store_id, const common::string16& index_name);
  int Compare(const IndexNamesKey& other) {
    DCHECK_GE(object_store_id_, 0);
    if (int x = CompareInts(object_store_id_, other.object_store_id_))
      return x;
    return index_name_.compare(other.index_name_);
  }
  common::string16 index_name() const { return index_name_; }

 private:
  int64_t object_store_id_;
  common::string16 index_name_;
};

class ObjectStoreNamesKey {
 public:
  // TODO(jsbell): We never use this to look up object store ids,
  // because a mapping is kept in the IndexedDBDatabase. Can the
  // mapping become unreliable?  Can we remove this?
  static bool Decode(common::StringPiece* slice, ObjectStoreNamesKey* result) {
    KeyPrefix prefix;
    if (!KeyPrefix::Decode(slice, &prefix))
      return false;
    DCHECK(prefix.database_id_);
    DCHECK(!prefix.object_store_id_);
    DCHECK(!prefix.index_id_);
    unsigned char type_byte = 0;
    if (!DecodeByte(slice, &type_byte))
      return false;
    DCHECK_EQ(type_byte, kObjectStoreNamesTypeByte);
    if (!DecodeStringWithLength(slice, &result->object_store_name_))
      return false;
    return true;
  }
  static std::string Encode(int64_t database_id, const common::string16& object_store_name);
  int Compare(const ObjectStoreNamesKey& other) { return object_store_name_.compare(other.object_store_name_); }
  common::string16 object_store_name() const { return object_store_name_; }

 private:
  // TODO(jsbell): Store the encoded string, or just pointers to it.
  common::string16 object_store_name_;
};

template <typename KeyType>
int Compare(const common::StringPiece& a, const common::StringPiece& b, bool only_compare_index_keys, bool* ok) {
  KeyType key_a;
  KeyType key_b;

  common::StringPiece slice_a(a);
  if (!KeyType::Decode(&slice_a, &key_a)) {
    *ok = false;
    return 0;
  }
  common::StringPiece slice_b(b);
  if (!KeyType::Decode(&slice_b, &key_b)) {
    *ok = false;
    return 0;
  }

  *ok = true;
  return key_a.Compare(key_b);
}

class IndexedDBKey {
 public:
  typedef std::vector<IndexedDBKey> KeyArray;

  IndexedDBKey();                        // Defaults to blink::WebIDBKeyTypeInvalid.
  explicit IndexedDBKey(WebIDBKeyType);  // must be Null or Invalid
  explicit IndexedDBKey(const KeyArray& array);
  explicit IndexedDBKey(const std::string& binary);
  explicit IndexedDBKey(const common::string16& string);
  IndexedDBKey(double number,
               WebIDBKeyType type);  // must be date or number
  IndexedDBKey(const IndexedDBKey& other);
  ~IndexedDBKey();
  IndexedDBKey& operator=(const IndexedDBKey& other);

  bool IsValid() const;

  bool IsLessThan(const IndexedDBKey& other) const;
  bool Equals(const IndexedDBKey& other) const;

  WebIDBKeyType type() const { return type_; }
  const std::vector<IndexedDBKey>& array() const {
    DCHECK_EQ(type_, kWebIDBKeyTypeArray);
    return array_;
  }
  const std::string& binary() const {
    DCHECK_EQ(type_, kWebIDBKeyTypeBinary);
    return binary_;
  }
  const common::string16& string() const {
    DCHECK_EQ(type_, kWebIDBKeyTypeString);
    return string_;
  }
  double date() const {
    DCHECK_EQ(type_, kWebIDBKeyTypeDate);
    return number_;
  }
  double number() const {
    DCHECK_EQ(type_, kWebIDBKeyTypeNumber);
    return number_;
  }

  size_t size_estimate() const { return size_estimate_; }

 private:
  int CompareTo(const IndexedDBKey& other) const;

  WebIDBKeyType type_;
  std::vector<IndexedDBKey> array_;
  std::string binary_;
  common::string16 string_;
  double number_ = 0;

  size_t size_estimate_;
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

 private:
  int64_t database_id_;
  int64_t object_store_id_;
  int64_t index_id_;
  std::string encoded_user_key_;
  std::string encoded_primary_key_;
  int64_t sequence_number_;

  DISALLOW_COPY_AND_ASSIGN(IndexDataKey);
};

template <typename KeyType>
int CompareSuffix(common::StringPiece* a, common::StringPiece* b, bool only_compare_index_keys, bool* ok) {
  NOTREACHED();
  return 0;
}

template <>
int CompareSuffix<ExistsEntryKey>(common::StringPiece* slice_a,
                                  common::StringPiece* slice_b,
                                  bool only_compare_index_keys,
                                  bool* ok) {
  DCHECK(!slice_a->empty());
  DCHECK(!slice_b->empty());
  return CompareEncodedIDBKeys(slice_a, slice_b, ok);
}

template <>
int CompareSuffix<ObjectStoreDataKey>(common::StringPiece* slice_a,
                                      common::StringPiece* slice_b,
                                      bool only_compare_index_keys,
                                      bool* ok) {
  return CompareEncodedIDBKeys(slice_a, slice_b, ok);
}

template <>
int CompareSuffix<BlobEntryKey>(common::StringPiece* slice_a,
                                common::StringPiece* slice_b,
                                bool only_compare_index_keys,
                                bool* ok) {
  return CompareEncodedIDBKeys(slice_a, slice_b, ok);
}

template <>
int CompareSuffix<IndexDataKey>(common::StringPiece* slice_a,
                                common::StringPiece* slice_b,
                                bool only_compare_index_keys,
                                bool* ok) {
  // index key
  int result = CompareEncodedIDBKeys(slice_a, slice_b, ok);
  if (!*ok || result)
    return result;
  if (only_compare_index_keys)
    return 0;

  // sequence number [optional]
  int64_t sequence_number_a = -1;
  int64_t sequence_number_b = -1;
  if (!slice_a->empty() && !DecodeVarInt(slice_a, &sequence_number_a))
    return 0;
  if (!slice_b->empty() && !DecodeVarInt(slice_b, &sequence_number_b))
    return 0;

  if (slice_a->empty() || slice_b->empty())
    return CompareSizes(slice_a->size(), slice_b->size());

  // primary key [optional]
  result = CompareEncodedIDBKeys(slice_a, slice_b, ok);
  if (!*ok || result)
    return result;

  return CompareInts(sequence_number_a, sequence_number_b);
}

int CompareImpl(const common::StringPiece& a, const common::StringPiece& b, bool only_compare_index_keys, bool* ok) {
  common::StringPiece slice_a(a);
  common::StringPiece slice_b(b);
  KeyPrefix prefix_a;
  KeyPrefix prefix_b;
  bool ok_a = KeyPrefix::Decode(&slice_a, &prefix_a);
  bool ok_b = KeyPrefix::Decode(&slice_b, &prefix_b);
  DCHECK(ok_a);
  DCHECK(ok_b);
  if (!ok_a || !ok_b) {
    *ok = false;
    return 0;
  }

  *ok = true;
  if (int x = prefix_a.Compare(prefix_b))
    return x;

  switch (prefix_a.type()) {
    case KeyPrefix::GLOBAL_METADATA: {
      DCHECK(!slice_a.empty());
      DCHECK(!slice_b.empty());

      unsigned char type_byte_a;
      if (!DecodeByte(&slice_a, &type_byte_a)) {
        *ok = false;
        return 0;
      }

      unsigned char type_byte_b;
      if (!DecodeByte(&slice_b, &type_byte_b)) {
        *ok = false;
        return 0;
      }

      if (int x = type_byte_a - type_byte_b)
        return x;
      if (type_byte_a < kMaxSimpleGlobalMetaDataTypeByte)
        return 0;

      // Compare<> is used (which re-decodes the prefix) rather than an
      // specialized CompareSuffix<> because metadata is relatively uncommon
      // in the database.

      if (type_byte_a == kDatabaseFreeListTypeByte) {
        // TODO(jsbell): No need to pass only_compare_index_keys through here.
        return Compare<DatabaseFreeListKey>(a, b, only_compare_index_keys, ok);
      }
      if (type_byte_a == kDatabaseNameTypeByte) {
        return Compare<DatabaseNameKey>(a, b, /*only_compare_index_keys*/ false, ok);
      }
      break;
    }

    case KeyPrefix::DATABASE_METADATA: {
      DCHECK(!slice_a.empty());
      DCHECK(!slice_b.empty());

      unsigned char type_byte_a;
      if (!DecodeByte(&slice_a, &type_byte_a)) {
        *ok = false;
        return 0;
      }

      unsigned char type_byte_b;
      if (!DecodeByte(&slice_b, &type_byte_b)) {
        *ok = false;
        return 0;
      }

      if (int x = type_byte_a - type_byte_b)
        return x;
      if (type_byte_a < DatabaseMetaDataKey::MAX_SIMPLE_METADATA_TYPE)
        return 0;

      // Compare<> is used (which re-decodes the prefix) rather than an
      // specialized CompareSuffix<> because metadata is relatively uncommon
      // in the database.

      if (type_byte_a == kObjectStoreMetaDataTypeByte) {
        // TODO(jsbell): No need to pass only_compare_index_keys through here.
        return Compare<ObjectStoreMetaDataKey>(a, b, only_compare_index_keys, ok);
      }
      if (type_byte_a == kIndexMetaDataTypeByte) {
        return Compare<IndexMetaDataKey>(a, b, /*only_compare_index_keys*/ false, ok);
      }
      if (type_byte_a == kObjectStoreFreeListTypeByte) {
        return Compare<ObjectStoreFreeListKey>(a, b, only_compare_index_keys, ok);
      }
      if (type_byte_a == kIndexFreeListTypeByte) {
        return Compare<IndexFreeListKey>(a, b, /*only_compare_index_keys*/ false, ok);
      }
      if (type_byte_a == kObjectStoreNamesTypeByte) {
        // TODO(jsbell): No need to pass only_compare_index_keys through here.
        return Compare<ObjectStoreNamesKey>(a, b, only_compare_index_keys, ok);
      }
      if (type_byte_a == kIndexNamesKeyTypeByte) {
        return Compare<IndexNamesKey>(a, b, /*only_compare_index_keys*/ false, ok);
      }
      break;
    }

    case KeyPrefix::OBJECT_STORE_DATA: {
      // Provide a stable ordering for invalid data.
      if (slice_a.empty() || slice_b.empty())
        return CompareSizes(slice_a.size(), slice_b.size());

      return CompareSuffix<ObjectStoreDataKey>(&slice_a, &slice_b, /*only_compare_index_keys*/ false, ok);
    }

    case KeyPrefix::EXISTS_ENTRY: {
      // Provide a stable ordering for invalid data.
      if (slice_a.empty() || slice_b.empty())
        return CompareSizes(slice_a.size(), slice_b.size());

      return CompareSuffix<ExistsEntryKey>(&slice_a, &slice_b, /*only_compare_index_keys*/ false, ok);
    }

    case KeyPrefix::BLOB_ENTRY: {
      // Provide a stable ordering for invalid data.
      if (slice_a.empty() || slice_b.empty())
        return CompareSizes(slice_a.size(), slice_b.size());

      return CompareSuffix<BlobEntryKey>(&slice_a, &slice_b, /*only_compare_index_keys*/ false, ok);
    }

    case KeyPrefix::INDEX_DATA: {
      // Provide a stable ordering for invalid data.
      if (slice_a.empty() || slice_b.empty())
        return CompareSizes(slice_a.size(), slice_b.size());

      return CompareSuffix<IndexDataKey>(&slice_a, &slice_b, only_compare_index_keys, ok);
    }

    case KeyPrefix::INVALID_TYPE:
      break;
  }

  NOTREACHED();
  *ok = false;
  return 0;
}

int Compare(const common::StringPiece& a, const common::StringPiece& b, bool only_compare_index_keys) {
  bool ok;
  int result = CompareImpl(a, b, only_compare_index_keys, &ok);
  DCHECK(ok);
  if (!ok) {
    return 0;
  }
  return result;
}

}  // namespace detail

int IndexedDB::Compare(const ::leveldb::Slice& a, const ::leveldb::Slice& b) const {
  common::StringPiece sa(a.data(), a.size());
  common::StringPiece sb(b.data(), b.size());
  return detail::Compare(sa, sb, false /*index_keys*/);
}

const char* IndexedDB::Name() const {
  return "idb_cmp1";
}

void IndexedDB::FindShortestSeparator(std::string* start, const ::leveldb::Slice& limit) const {
  UNUSED(start);
  UNUSED(limit);
}

void IndexedDB::FindShortSuccessor(std::string* key) const {
  UNUSED(key);
}

}  // namespace comparator
}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
