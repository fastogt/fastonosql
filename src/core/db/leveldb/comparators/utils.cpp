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

#include "core/db/leveldb/comparators/utils.h"

#include <string.h>

#include <common/macros.h>

#include <common/sys_byteorder.h>

#include "core/db/leveldb/comparators/constants.h"

namespace fastonosql {
namespace core {
namespace leveldb {
namespace comparator {

namespace {
int CompareTypes(WebIDBKeyType a, WebIDBKeyType b) {
  return b - a;
}

WebIDBKeyType KeyTypeByteToKeyType(unsigned char type) {
  switch (type) {
    case kIndexedDBKeyNullTypeByte:
      return kWebIDBKeyTypeInvalid;
    case kIndexedDBKeyArrayTypeByte:
      return kWebIDBKeyTypeArray;
    case kIndexedDBKeyBinaryTypeByte:
      return kWebIDBKeyTypeBinary;
    case kIndexedDBKeyStringTypeByte:
      return kWebIDBKeyTypeString;
    case kIndexedDBKeyDateTypeByte:
      return kWebIDBKeyTypeDate;
    case kIndexedDBKeyNumberTypeByte:
      return kWebIDBKeyTypeNumber;
    case kIndexedDBKeyMinKeyTypeByte:
      return kWebIDBKeyTypeMin;
  }

  NOTREACHED();
  return kWebIDBKeyTypeInvalid;
}

}  // namespace

int CompareSizes(size_t a, size_t b) {
  if (a > b)
    return 1;
  if (b > a)
    return -1;
  return 0;
}

int CompareInts(int64_t a, int64_t b) {
#ifndef NDEBUG
  // Exercised by unit tests in debug only.
  DCHECK_GE(a, 0);
  DCHECK_GE(b, 0);
#endif
  int64_t diff = a - b;
  if (diff < 0)
    return -1;
  if (diff > 0)
    return 1;
  return 0;
}

bool DecodeByte(common::StringPiece* slice, unsigned char* value) {
  if (slice->empty())
    return false;

  *value = (*slice)[0];
  slice->remove_prefix(1);
  return true;
}

bool DecodeVarInt(common::StringPiece* slice, int64_t* value) {
  if (slice->empty())
    return false;

  common::StringPiece::const_iterator it = slice->begin();
  int shift = 0;
  int64_t ret = 0;
  do {
    if (it == slice->end())
      return false;

    unsigned char c = *it;
    ret |= static_cast<int64_t>(c & 0x7f) << shift;
    shift += 7;
  } while (*it++ & 0x80);
  *value = ret;
  slice->remove_prefix(it - slice->begin());
  return true;
}

bool DecodeInt(common::StringPiece* slice, int64_t* value) {
  if (slice->empty())
    return false;

  common::StringPiece::const_iterator it = slice->begin();
  int shift = 0;
  int64_t ret = 0;
  while (it != slice->end()) {
    unsigned char c = *it++;
    ret |= static_cast<int64_t>(c) << shift;
    shift += 8;
  }
  *value = ret;
  slice->remove_prefix(it - slice->begin());
  return true;
}

bool DecodeString(common::StringPiece* slice, common::string16* value) {
  if (slice->empty()) {
    value->clear();
    return true;
  }

  // Backing store is UTF-16BE, convert to host endianness.
  DCHECK(!(slice->size() % sizeof(common::char16)));
  size_t length = slice->size() / sizeof(common::char16);
  common::string16 decoded;
  decoded.reserve(length);
  const common::char16* encoded = reinterpret_cast<const common::char16*>(slice->begin());
  for (unsigned i = 0; i < length; ++i) {
    decoded.push_back(common::NetToHost16(*encoded++));
  }

  *value = decoded;
  slice->remove_prefix(length * sizeof(common::char16));
  return true;
}

bool DecodeStringWithLength(common::StringPiece* slice, common::string16* value) {
  if (slice->empty())
    return false;

  int64_t length = 0;
  if (!DecodeVarInt(slice, &length) || length < 0)
    return false;
  size_t bytes = length * sizeof(common::char16);
  if (slice->size() < bytes)
    return false;

  common::StringPiece subpiece(slice->begin(), bytes);
  slice->remove_prefix(bytes);
  if (!DecodeString(&subpiece, value))
    return false;

  return true;
}

int CompareEncodedBinary(common::StringPiece* slice1, common::StringPiece* slice2, bool* ok) {
  int64_t len1, len2;
  if (!DecodeVarInt(slice1, &len1) || !DecodeVarInt(slice2, &len2)) {
    *ok = false;
    return 0;
  }
  DCHECK_GE(len1, 0);
  DCHECK_GE(len2, 0);
  if (len1 < 0 || len2 < 0) {
    *ok = false;
    return 0;
  }
  size_t size1 = len1;
  size_t size2 = len2;

  DCHECK_GE(slice1->size(), size1);
  DCHECK_GE(slice2->size(), size2);
  if (slice1->size() < size1 || slice2->size() < size2) {
    *ok = false;
    return 0;
  }

  // Extract the binary data, and advance the passed slices.
  common::StringPiece binary1(slice1->begin(), size1);
  common::StringPiece binary2(slice2->begin(), size2);
  slice1->remove_prefix(size1);
  slice2->remove_prefix(size2);

  *ok = true;
  // This is the same as a memcmp()
  return binary1.compare(binary2);
}

int CompareEncodedStringsWithLength(common::StringPiece* slice1, common::StringPiece* slice2, bool* ok) {
  int64_t len1, len2;
  if (!DecodeVarInt(slice1, &len1) || !DecodeVarInt(slice2, &len2)) {
    *ok = false;
    return 0;
  }
  DCHECK_GE(len1, 0);
  DCHECK_GE(len2, 0);
  if (len1 < 0 || len2 < 0) {
    *ok = false;
    return 0;
  }
  DCHECK_GE(slice1->size(), len1 * sizeof(common::char16));
  DCHECK_GE(slice2->size(), len2 * sizeof(common::char16));
  if (slice1->size() < len1 * sizeof(common::char16) || slice2->size() < len2 * sizeof(common::char16)) {
    *ok = false;
    return 0;
  }

  // Extract the string data, and advance the passed slices.
  common::StringPiece string1(slice1->begin(), len1 * sizeof(common::char16));
  common::StringPiece string2(slice2->begin(), len2 * sizeof(common::char16));
  slice1->remove_prefix(len1 * sizeof(common::char16));
  slice2->remove_prefix(len2 * sizeof(common::char16));

  *ok = true;
  // Strings are UTF-16BE encoded, so a simple memcmp is sufficient.
  return string1.compare(string2);
}

bool DecodeDouble(common::StringPiece* slice, double* value) {
  if (slice->size() < sizeof(*value))
    return false;

  memcpy(value, slice->begin(), sizeof(*value));
  slice->remove_prefix(sizeof(*value));
  return true;
}

int CompareEncodedIDBKeys(common::StringPiece* slice_a, common::StringPiece* slice_b, bool* ok) {
  DCHECK(!slice_a->empty());
  DCHECK(!slice_b->empty());
  *ok = true;
  unsigned char type_a = (*slice_a)[0];
  unsigned char type_b = (*slice_b)[0];
  slice_a->remove_prefix(1);
  slice_b->remove_prefix(1);

  if (int x = CompareTypes(KeyTypeByteToKeyType(type_a), KeyTypeByteToKeyType(type_b)))
    return x;

  switch (type_a) {
    case kIndexedDBKeyNullTypeByte:
    case kIndexedDBKeyMinKeyTypeByte:
      // Null type or max type; no payload to compare.
      return 0;
    case kIndexedDBKeyArrayTypeByte: {
      int64_t length_a, length_b;
      if (!DecodeVarInt(slice_a, &length_a) || !DecodeVarInt(slice_b, &length_b)) {
        *ok = false;
        return 0;
      }
      for (int64_t i = 0; i < length_a && i < length_b; ++i) {
        int result = CompareEncodedIDBKeys(slice_a, slice_b, ok);
        if (!*ok || result)
          return result;
      }
      return length_a - length_b;
    }
    case kIndexedDBKeyBinaryTypeByte:
      return CompareEncodedBinary(slice_a, slice_b, ok);
    case kIndexedDBKeyStringTypeByte:
      return CompareEncodedStringsWithLength(slice_a, slice_b, ok);
    case kIndexedDBKeyDateTypeByte:
    case kIndexedDBKeyNumberTypeByte: {
      double d, e;
      if (!DecodeDouble(slice_a, &d) || !DecodeDouble(slice_b, &e)) {
        *ok = false;
        return 0;
      }
      if (d < e)
        return -1;
      if (d > e)
        return 1;
      return 0;
    }
  }

  NOTREACHED();
  return 0;
}

}  // namespace comparator
}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
