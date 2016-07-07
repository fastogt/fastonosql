CMAKE_MINIMUM_REQUIRED(VERSION 3.3.0)
PROJECT(leveldb)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/src/include)
IF(OS_LINUX)
  ADD_DEFINITIONS(-DLEVELDB_PLATFORM_POSIX -DLEVELDB_ATOMIC_PRESENT)
ELSEIF(OS_WINDOWS)
  ADD_DEFINITIONS(-D_REENTRANT -DLEVELDB_PLATFORM_WINDOWS -DLEVELDB_ATOMIC_PRESENT)
ELSEIF(OS_MACOSX)
  ADD_DEFINITIONS(-D_REENTRANT -DLEVELDB_PLATFORM_POSIX)
ELSEIF(OS_FREEBSD)
  ADD_DEFINITIONS(-D_REENTRANT -DLEVELDB_PLATFORM_POSIX -DLEVELDB_ATOMIC_PRESENT)
ELSEIF(OS_ANDROID)
  ADD_DEFINITIONS(-D_REENTRANT -DLEVELDB_PLATFORM_POSIX)
ELSE()
  ADD_DEFINITIONS(-DLEVELDB_PLATFORM_POSIX -DLEVELDB_ATOMIC_PRESENT)
ENDIF(OS_LINUX)

SET(HEADERS_LEVELDB
)

SET(SOURCES_LEVELDB
  src/db/builder.cc
  src/db/c.cc
  src/db/dbformat.cc
  src/db/db_impl.cc
  src/db/db_iter.cc
  src/db/dumpfile.cc
  src/db/filename.cc
  src/db/log_reader.cc
  src/db/log_writer.cc
  src/db/memtable.cc
  src/db/repair.cc
  src/db/table_cache.cc
  src/db/version_edit.cc
  src/db/version_set.cc
  src/db/write_batch.cc

  src/table/block_builder.cc
  src/table/block.cc
  src/table/filter_block.cc
  src/table/format.cc
  src/table/iterator.cc
  src/table/merger.cc
  src/table/table_builder.cc
  src/table/table.cc
  src/table/two_level_iterator.cc

  src/util/arena.cc
  src/util/bloom.cc
  src/util/cache.cc
  src/util/coding.cc
  src/util/comparator.cc
  src/util/crc32c.cc
  src/util/env.cc
  src/util/filter_policy.cc
  src/util/hash.cc
  src/util/histogram.cc
  src/util/logging.cc
  src/util/options.cc
  src/util/status.cc
)

IF(OS_POSIX)
  SET(SOURCES_LEVELDB ${SOURCES_LEVELDB} src/port/port_posix.cc src/util/env_posix.cc)
ELSEIF(OS_WINDOWS)
  SET(SOURCES_LEVELDB ${SOURCES_LEVELDB} src/port/port_win.cc src/util/env_win.cc src/port/mman.c)
ENDIF(OS_POSIX)

ADD_LIBRARY(leveldb OBJECT ${HEADERS_LEVELDB} ${SOURCES_LEVELDB})

