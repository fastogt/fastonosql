CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(redis_cli)

ADD_CUSTOM_TARGET(redis_version
            COMMAND sh ./mkreleasehdr.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src
            COMMENT "Build redis_cli release.h")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/deps/hiredis ${CMAKE_CURRENT_LIST_DIR}/src ../sds)

SET(HEADERS_REDIS    
    src/anet.h
    src/crc64.h
    #src/release.h
    src/zmalloc.h
    src/config.h

    deps/hiredis/read.h
    deps/hiredis/fmacros.h
)

SET(SOURCES_REDIS
    src/anet.c
    src/crc64.c
    src/release.c
    src/zmalloc.c

    deps/hiredis/net.c
    deps/hiredis/read.c
    deps/hiredis/hiredis.c
)

ADD_LIBRARY(redis_cli OBJECT ${HEADERS_REDIS} ${SOURCES_REDIS})
ADD_DEPENDENCIES(redis_cli redis_version)

