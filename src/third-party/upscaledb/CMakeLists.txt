CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(upscaledb CXX)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src/src ${CMAKE_CURRENT_LIST_DIR}/src/include ${CMAKE_CURRENT_LIST_DIR}/src)

SET(HEADERS_UPSCALEDB
)

SET(SOURCES_UPSCALEDB
  #src/src/4env/env_local.cc
  src/src/5upscaledb/upscaledb.cc
  src/src/1base/error.cc
  src/src/2compressor/compressor_factory.cc
  src/src/1globals/globals.cc
  src/src/4env/env.cc
  src/src/1mem/mem.cc
  src/src/4db/db_local.cc
  src/src/4cursor/cursor_local.cc
  src/src/4txn/txn_local.cc
  src/src/3page_manager/page_manager.cc
  src/src/3btree/btree_stats.cc
)

ADD_LIBRARY(upscaledb STATIC ${HEADERS_UPSCALEDB} ${SOURCES_UPSCALEDB})

