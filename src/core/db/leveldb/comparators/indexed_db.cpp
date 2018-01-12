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

#include "core/db/leveldb/comparators/indexed_db.h"

#include <memory>
#include <vector>

#include <leveldb/slice.h>

#include <common/macros.h>

#include "core/db/leveldb/comparators/indexed_db/indexed_db_leveldb_coding.h"

namespace fastonosql {
namespace core {
namespace leveldb {
namespace comparator {

int IndexedDB::Compare(const ::leveldb::Slice& a, const ::leveldb::Slice& b) const {
  common::StringPiece sa(a.data(), a.size());
  common::StringPiece sb(b.data(), b.size());
  return fastonosql::core::leveldb::comparator::Compare(sa, sb, false /*index_keys*/);
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
