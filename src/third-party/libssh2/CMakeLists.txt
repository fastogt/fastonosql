CMAKE_MINIMUM_REQUIRED(VERSION 3.3.0)
PROJECT(libssh2)

IF(OS_WINDOWS)
  SET(SSH2_CONFIG_IN libssh2_config_win.h.in)
ELSEIF(OS_MACOSX)
  SET(SSH2_CONFIG_IN libssh2_config_macos.h.in)
ELSEIF(OS_LINUX)
  SET(SSH2_CONFIG_IN libssh2_config_linux.h.in)
ELSEIF(OS_FREEBSD)
  SET(SSH2_CONFIG_IN libssh2_config_freebsd.h.in)
ELSEIF(OS_ANDROID)
  SET(SSH2_CONFIG_IN libssh2_config_android.h.in)
ELSE()
  MESSAGE(FATAL_ERROR "Not have libssh2_config.in: ${CMAKE_SYSTEM_NAME}")
ENDIF(OS_WINDOWS)

CONFIGURE_FILE(${SSH2_CONFIG_IN} ${CMAKE_CURRENT_LIST_DIR}/src/libssh2_config.h @ONLY IMMEDIATE)

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
