CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(libmemcached)

IF(OS_WINDOWS)
  SET(MEM_CONFIG_IN src/mem_config_win.h.in)
ELSEIF(OS_MACOSX)
  SET(MEM_CONFIG_IN src/mem_config_macos.h.in)
ELSEIF(OS_LINUX)
  SET(MEM_CONFIG_IN src/mem_config_linux.h.in)
ELSEIF(OS_FREEBSD)
  SET(MEM_CONFIG_IN src/mem_config_freebsd.h.in)
ELSEIF(OS_ANDROID)
  SET(MEM_CONFIG_IN src/mem_config_android.h.in)
ELSE()
  MESSAGE(FATAL_ERROR "Not have mem_config.in: ${CMAKE_SYSTEM_NAME}")
ENDIF(OS_WINDOWS)

CONFIGURE_FILE(${MEM_CONFIG_IN} ${CMAKE_CURRENT_LIST_DIR}/src/mem_config.h @ONLY IMMEDIATE)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src)

FIND_PACKAGE(Sasl2 QUIET)
IF(SASL2_FOUND)
  ADD_DEFINITIONS(-DHAVE_LIBSASL)
  INCLUDE_DIRECTORIES(${SASL2_INCLUDE_DIR})
ENDIF(SASL2_FOUND)

SET(HEADERS_MEMCACHED
  ${CMAKE_CURRENT_LIST_DIR}/src/mem_config.h
)

SET(SOURCES_MEMCACHED
  src/libmemcached/connect.cc
  src/libmemcached/sasl.cc
  src/libmemcached/version.cc
  src/libmemcached/server.cc
  src/libmemcached/get.cc
  src/libmemcached/strerror.cc
  src/libmemcached/server_list.cc
  src/libmemcached/hosts.cc
  src/libmemcached/delete.cc
  src/libmemcached/memcached.cc
  src/libmemcached/error.cc
  src/libmemcached/instance.cc
  src/libmemcached/io.cc
  src/libmemcached/array.c
  src/libmemcached/storage.cc
  src/libmemcached/response.cc
  src/libmemcached/result.cc
  src/libmemcached/behavior.cc
  src/libmemcached/initialize_query.cc
  src/libmemcached/allocators.cc
  src/libmemcached/quit.cc
  src/libmemcached/do.cc
  src/libmemcached/fetch.cc
  src/libmemcached/string.cc
  src/libmemcached/hash.cc
  src/libmemcached/key.cc
  src/libmemcached/options.cc
  src/libmemcached/flush_buffers.cc
  src/libmemcached/virtual_bucket.c
  src/libmemcached/purge.cc
  src/libmemcached/byteorder.cc
  src/libmemcached/flag.cc
  src/libmemcached/udp.cc
  src/libmemcached/namespace.cc
  src/libmemcached/callback.cc
  src/libmemcached/auto.cc
  src/libmemcached/flush.cc
  src/libmemcached/stats.cc
  src/libmemcached/dump.cc

  src/libmemcached/csl/context.cc
  src/libmemcached/csl/scanner.cc
  src/libmemcached/csl/parser.cc

  src/libhashkit/algorithm.cc
  src/libhashkit/digest.cc
  src/libhashkit/hashkit.cc
  src/libhashkit/encrypt.cc
  src/libhashkit/aes.cc
  src/libhashkit/rijndael.cc
  src/libhashkit/string.cc
  src/libhashkit/jenkins.cc
  src/libhashkit/function.cc
  src/libhashkit/str_algorithm.cc
  src/libhashkit/has.cc
  src/libhashkit/fnv_64.cc
  src/libhashkit/fnv_32.cc
  src/libhashkit/one_at_a_time.cc
  src/libhashkit/crc32.cc
  src/libhashkit/md5.cc
  src/libhashkit/hsieh.cc
  src/libhashkit/murmur.cc
  src/libhashkit/murmur3.cc
  src/libhashkit/murmur3_api.cc

  src/libmemcachedutil/ping.cc
)

IF(OS_WINDOWS)
  SET(SOURCES_MEMCACHED ${SOURCES_MEMCACHED}
    src/libmemcached/poll.cc
  )
ENDIF(OS_WINDOWS)

ADD_LIBRARY(memcached STATIC ${HEADERS_MEMCACHED} ${SOURCES_MEMCACHED})
IF(SASL2_FOUND)
  TARGET_LINK_LIBRARIES(memcached sasl2)
ENDIF(SASL2_FOUND)
