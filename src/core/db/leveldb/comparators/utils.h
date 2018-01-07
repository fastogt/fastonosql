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

#include <common/string_piece.h>

namespace fastonosql {
namespace core {
namespace leveldb {
namespace comparator {

int CompareSizes(size_t a, size_t b);

int CompareInts(int64_t a, int64_t b);

bool DecodeByte(common::StringPiece* slice, unsigned char* value);

bool DecodeVarInt(common::StringPiece* slice, int64_t* value);

bool DecodeInt(common::StringPiece* slice, int64_t* value);

bool DecodeString(common::StringPiece* slice, common::string16* value);

bool DecodeStringWithLength(common::StringPiece* slice, common::string16* value);

int CompareEncodedBinary(common::StringPiece* slice1, common::StringPiece* slice2, bool* ok);

int CompareEncodedStringsWithLength(common::StringPiece* slice1, common::StringPiece* slice2, bool* ok);

bool DecodeDouble(common::StringPiece* slice, double* value);

int CompareEncodedIDBKeys(common::StringPiece* slice_a, common::StringPiece* slice_b, bool* ok);

}  // namespace comparator
}  // namespace leveldb
}  // namespace core
}  // namespace fastonosql
