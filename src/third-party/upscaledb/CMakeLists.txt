CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(upscaledb CXX)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src/src ${CMAKE_CURRENT_LIST_DIR}/src/include ${CMAKE_CURRENT_LIST_DIR}/src)

SET(HEADERS_UPSCALEDB

)

SET(SOURCES_UPSCALEDB
  src/src/1globals/globals.cc
  src/src/1globals/callbacks.cc

  src/src/1base/error.cc
  src/src/1base/util.cc

  src/src/1errorinducer/errorinducer.cc

  src/src/1mem/mem.cc

  src/src/1os/os.cc

  src/src/2page/page.cc

  src/src/2compressor/compressor_factory.cc

  src/3rdparty/liblzf/lzf_d.c
  src/3rdparty/liblzf/lzf_c.c
  src/3rdparty/for/for.c

  src/src/3page_manager/page_manager.cc
  src/src/3page_manager/freelist.cc

  src/src/3btree/btree_stats.cc
  src/src/3btree/btree_cursor.cc
  src/src/3btree/btree_index.cc
  src/src/3btree/btree_visit.cc
  src/src/3btree/btree_find.cc
  src/src/3btree/btree_erase.cc
  src/src/3btree/btree_update.cc
  src/src/3btree/btree_insert.cc
  src/src/3btree/btree_check.cc

  src/src/3blob_manager/blob_manager_disk.cc
  src/src/3blob_manager/blob_manager_inmem.cc

  src/src/3journal/journal.cc

  src/src/3changeset/changeset.cc

  src/src/4txn/txn_local.cc
  src/src/4txn/txn_cursor.cc

  src/src/4uqi/parser.cc
  src/src/4uqi/plugins.cc
  src/src/4uqi/scanvisitorfactory.cc
  src/src/4uqi/uqi.cc

  src/src/4db/db_local.cc
  src/src/4db/db.cc

  src/src/4env/env_local.cc
  src/src/4env/env.cc

  src/src/4cursor/cursor_local.cc
  src/src/4cursor/cursor.cc

  src/src/5upscaledb/upscaledb.cc
)

IF(OS_POSIX)
  SET(SOURCES_UPSCALEDB ${SOURCES_UPSCALEDB} src/src/1os/os_posix.cc)
ELSEIF(OS_WINDOWS)
  SET(SOURCES_UPSCALEDB ${SOURCES_UPSCALEDB} src/src/1os/os_posix.cc)
ENDIF(OS_POSIX)

ADD_LIBRARY(upscaledb STATIC ${HEADERS_UPSCALEDB} ${SOURCES_UPSCALEDB})

