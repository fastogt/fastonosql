CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(libssh2)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/include)

SET(HEADERS ${HEADERS} 
  include/libssh2.h
)
SET(SOURCES ${SOURCES} 
  src/global.c
  src/session.c
  src/openssl.c
  src/keepalive.c
  src/misc.c
  src/kex.c
  src/transport.c
  src/crypt.c
  src/packet.c
  src/hostkey.c
  src/comp.c
  src/channel.c
  src/mac.c
  src/userauth.c
)
	
SOURCE_GROUP("libssh2 files" FILES ${HEADERS} ${SOURCES})
ADD_LIBRARY(libssh2 OBJECT ${HEADERS} ${SOURCES})
