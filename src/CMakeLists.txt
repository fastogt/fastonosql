SET(HEADERS_GLOBAL
  global/global.h
  global/types.h
)
SET(SOURCES_GLOBAL
  global/global.cpp
  global/types.cpp
)

SET(HEADERS_GUI_TO_MOC
  gui/main_window.h
  gui/statistic_sender.h
  gui/update_checker.h
  gui/widgets/log_tab_widget.h
  gui/widgets/log_widget.h
  gui/widgets/commands_widget.h
  gui/property_table_model.h
  gui/explorer/explorer_tree_view.h
  gui/explorer/explorer_tree_model.h
  gui/dialogs/about_dialog.h
  gui/dialogs/input_dialog.h
  gui/dialogs/info_server_dialog.h
  gui/dialogs/history_server_dialog.h
  gui/dialogs/property_server_dialog.h
  gui/dialogs/preferences_dialog.h
  gui/dialogs/connections_dialog.h
  gui/dialogs/connection_dialog.h
  gui/dialogs/cluster_dialog.h
  gui/dialogs/sentinel_dialog.h
  gui/dialogs/connection_diagnostic_dialog.h
  gui/dialogs/discovery_cluster_dialog.h
  gui/dialogs/discovery_sentinel_dialog.h
  gui/dialogs/encode_decode_dialog.h
  gui/dialogs/load_contentdb_dialog.h
  gui/dialogs/create_dbkey_dialog.h
  gui/dialogs/view_keys_dialog.h
  gui/dialogs/change_password_server_dialog.h
  gui/widgets/main_widget.h
  gui/main_tab_bar.h
  gui/fasto_editor.h
  gui/fasto_editor_shell.h
  gui/fasto_editor_output.h
  gui/fasto_hex_edit.h
  gui/fasto_text_view.h
  gui/widgets/query_widget.h
  gui/widgets/output_widget.h
  gui/keys_table_model.h
  gui/fasto_tree_view.h
  gui/fasto_common_model.h
  gui/fasto_table_view.h
)
SET(HEADERS_GUI
  gui/dialogs/connection_listwidget_items.h
  gui/gui_factory.h
  gui/fasto_common_item.h
  gui/shortcuts.h
)
SET(SOURCES_GUI
  gui/main_window.cpp
  gui/statistic_sender.cpp
  gui/update_checker.cpp
  gui/gui_factory.cpp
  gui/widgets/log_tab_widget.cpp
  gui/widgets/log_widget.cpp
  gui/widgets/commands_widget.cpp
  gui/property_table_model.cpp
  gui/dialogs/about_dialog.cpp
  gui/dialogs/input_dialog.cpp
  gui/dialogs/preferences_dialog.cpp
  gui/dialogs/connections_dialog.cpp
  gui/dialogs/connection_dialog.cpp
  gui/dialogs/cluster_dialog.cpp
  gui/dialogs/sentinel_dialog.cpp
  gui/dialogs/connection_diagnostic_dialog.cpp
  gui/dialogs/discovery_cluster_dialog.cpp
  gui/dialogs/discovery_sentinel_dialog.cpp
  gui/dialogs/connection_listwidget_items.cpp
  gui/dialogs/info_server_dialog.cpp
  gui/dialogs/property_server_dialog.cpp
  gui/dialogs/history_server_dialog.cpp
  gui/dialogs/encode_decode_dialog.cpp
  gui/dialogs/load_contentdb_dialog.cpp
  gui/dialogs/create_dbkey_dialog.cpp
  gui/dialogs/view_keys_dialog.cpp
  gui/dialogs/change_password_server_dialog.cpp
  gui/widgets/main_widget.cpp
  gui/main_tab_bar.cpp
  gui/fasto_editor.cpp
  gui/fasto_editor_shell.cpp
  gui/fasto_editor_output.cpp
  gui/fasto_hex_edit.cpp
  gui/fasto_text_view.cpp
  gui/widgets/query_widget.cpp
  gui/keys_table_model.cpp
  gui/widgets/output_widget.cpp
  gui/explorer/explorer_tree_view.cpp
  gui/explorer/explorer_tree_model.cpp
  gui/fasto_tree_view.cpp
  gui/fasto_common_model.cpp
  gui/fasto_table_view.cpp
  gui/fasto_common_item.cpp
  gui/shortcuts.cpp
)

SET(HEADERS_CORE_TO_MOC
  core/command_logger.h
  core/idriver.h
  core/iserver.h
  core/servers_manager.h
)
SET(HEADERS_CORE
  core/connection_types.h
  core/connection_confg.h
  core/core_fwd.h
  core/icluster.h
  core/isentinel.h
  core/idatabase.h
  core/settings_manager.h
  core/connection_settings.h
  core/sentinel_connection_settings.h
  core/cluster_connection_settings.h
  core/events/events.h
  core/events/events_info.h
  core/types.h
  core/command_info.h
  core/command_holder.h
  core/command_handler.h
  core/command_key.h
  core/db_key.h
  core/connection.h
  core/server_property_info.h
  core/ssh_info.h
)
SET(SOURCES_CORE
  core/connection_confg.cpp
  core/events/events_info.cpp
  core/command_logger.cpp
  core/connection_types.cpp
  core/settings_manager.cpp
  core/connection_settings.cpp
  core/sentinel_connection_settings.cpp
  core/cluster_connection_settings.cpp
  core/idriver.cpp
  core/iserver.cpp
  core/icluster.cpp
  core/isentinel.cpp
  core/idatabase.cpp
  core/servers_manager.cpp
  core/types.cpp
  core/command_info.cpp
  core/command_holder.cpp
  core/command_handler.cpp
  core/command_key.cpp
  core/db_key.cpp
  core/connection.cpp
  core/server_property_info.cpp
  core/ssh_info.cpp
)

SET(HEADERS_SHELL_TO_MOC
  shell/shell_widget.h
  shell/base_lexer.h
  shell/base_shell.h
)
SET(HEADERS_SHELL
)
SET(SOURCES_SHELL
  shell/shell_widget.cpp
  shell/base_lexer.cpp
  shell/base_shell.cpp
)

SET(HEADERS_TRANSLATIONS
  translations/global.h
)
SET(SOURCES_TRANSLATIONS
  translations/global.cpp
)

SET_DESKTOP_TARGET()

IF(OS_WINDOWS)
  SET(PLATFORM_HDRS)
  SET(PLATFORM_SRCS)
  SET(PLATFORM_LIBRARIES ws2_32 Rpcrt4 ntdll)
ELSEIF(OS_MACOSX)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  SET(PLATFORM_LIBRARIES ${COCOA_LIBRARY})
ELSEIF(OS_LINUX)
  SET(PLATFORM_HDRS)
  SET(PLATFORM_SRCS)
  SET(PLATFORM_LIBRARIES dl)
ELSEIF(OS_FREEBSD)
  SET(PLATFORM_HDRS)
  SET(PLATFORM_SRCS)
  SET(PLATFORM_LIBRARIES util pthread)
ENDIF(OS_WINDOWS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR} ${OPENSSL_INCLUDE_DIR})

ADD_SUBDIRECTORY(fasto)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/fasto)

#redis
IF(BUILD_WITH_REDIS)
  SET(HEADERS_SHELL_TO_MOC ${HEADERS_SHELL_TO_MOC}
    shell/redis_lexer.h
  )
  SET(HEADERS_SHELL ${HEADERS_SHELL}
  )
  SET(SOURCES_SHELL ${SOURCES_SHELL}
    shell/redis_lexer.cpp
  )

  ADD_SUBDIRECTORY(third-party/libssh2)
  ADD_SUBDIRECTORY(third-party/redis)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/redis/src third-party/redis/deps third-party/sds)

  SET(HEADERS_REDIS_TO_MOC
    core/redis/sentinel.h
    core/redis/cluster.h
    core/redis/server.h
    core/redis/driver.h
  )
  SET(HEADERS_REDIS
    core/redis/config.h
    core/redis/database.h
    core/redis/command.h
    core/redis/server_info.h
    core/redis/connection_settings.h
    core/redis/db_connection.h

    core/redis/sentinel_info.h
    core/redis/sentinel_settings.h
    core/redis/cluster_infos.h
    core/redis/cluster_settings.h
  )
  SET(SOURCES_REDIS
    core/redis/config.cpp
    core/redis/database.cpp
    core/redis/command.cpp
    core/redis/server_info.cpp
    core/redis/connection_settings.cpp
    core/redis/db_connection.cpp
    core/redis/server.cpp
    core/redis/driver.cpp

    core/redis/sentinel_info.cpp
    core/redis/sentinel.cpp
    core/redis/sentinel_settings.cpp
    core/redis/cluster_infos.cpp
    core/redis/cluster.cpp
    core/redis/cluster_settings.cpp
  )
  SET(OBJECT_LIBS ${OBJECT_LIBS} $<TARGET_OBJECTS:hiredis> $<TARGET_OBJECTS:libssh2>)
ENDIF(BUILD_WITH_REDIS)

#memcached
IF(BUILD_WITH_MEMCACHED)
  SET(HEADERS_SHELL_TO_MOC ${HEADERS_SHELL_TO_MOC}
    shell/memcached_lexer.h
  )
  SET(HEADERS_SHELL ${HEADERS_SHELL}
  )
  SET(SOURCES_SHELL ${SOURCES_SHELL}
    shell/memcached_lexer.cpp
  )
  FIND_PACKAGE(Sasl2 QUIET)
  IF(SASL2_FOUND)
    ADD_DEFINITIONS(-DHAVE_LIBSASL)
    INCLUDE_DIRECTORIES(${SASL2_INCLUDE_DIR})
    SET(ALL_LIBS ${ALL_LIBS} ${SASL2_LIBRARIES})
  ENDIF(SASL2_FOUND)

  ADD_SUBDIRECTORY(third-party/memcached)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/memcached/src)

  SET(HEADERS_MEMCACHED_TO_MOC
    core/memcached/server.h
    core/memcached/driver.h
  )
  SET(HEADERS_MEMCACHED
    core/memcached/config.h
    core/memcached/database.h
    core/memcached/command.h
    core/memcached/server_info.h
    core/memcached/connection_settings.h
    core/memcached/db_connection.h
  )
  SET(SOURCES_MEMCACHED
    core/memcached/config.cpp
    core/memcached/database.cpp
    core/memcached/command.cpp
    core/memcached/server_info.cpp
    core/memcached/connection_settings.cpp
    core/memcached/db_connection.cpp
    core/memcached/server.cpp
    core/memcached/driver.cpp
  )
  SET(OBJECT_LIBS ${OBJECT_LIBS} $<TARGET_OBJECTS:libmemcached>)
ENDIF(BUILD_WITH_MEMCACHED)

#ssdb
IF(BUILD_WITH_SSDB)
  SET(HEADERS_SHELL_TO_MOC ${HEADERS_SHELL_TO_MOC}
    shell/ssdb_lexer.h
  )
  SET(HEADERS_SHELL ${HEADERS_SHELL}
  )
  SET(SOURCES_SHELL ${SOURCES_SHELL}
    shell/ssdb_lexer.cpp
  )

  ADD_SUBDIRECTORY(third-party/ssdb)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/ssdb/src)

  SET(HEADERS_SSDB_TO_MOC
    core/ssdb/server.h
    core/ssdb/driver.h
  )
  SET(HEADERS_SSDB
    core/ssdb/config.h
    core/ssdb/database.h
    core/ssdb/command.h
    core/ssdb/server_info.h
    core/ssdb/connection_settings.h
    core/ssdb/db_connection.h
  )
  SET(SOURCES_SSDB
    core/ssdb/config.cpp
    core/ssdb/database.cpp
    core/ssdb/command.cpp
    core/ssdb/server_info.cpp
    core/ssdb/connection_settings.cpp
    core/ssdb/db_connection.cpp
    core/ssdb/server.cpp
    core/ssdb/driver.cpp
  )
  SET(OBJECT_LIBS ${OBJECT_LIBS} $<TARGET_OBJECTS:ssdb>)
ENDIF(BUILD_WITH_SSDB)

#leveldb
IF(BUILD_WITH_LEVELDB)
  SET(HEADERS_SHELL_TO_MOC ${HEADERS_SHELL_TO_MOC}
    shell/leveldb_lexer.h
  )
  SET(HEADERS_SHELL ${HEADERS_SHELL}
  )
  SET(SOURCES_SHELL ${SOURCES_SHELL}
    shell/leveldb_lexer.cpp
  )

  ADD_SUBDIRECTORY(third-party/leveldb)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/leveldb/src/include)

  SET(HEADERS_LEVELDB_TO_MOC
    core/leveldb/server.h
    core/leveldb/driver.h
  )
  SET(HEADERS_LEVELDB
    core/leveldb/config.h
    core/leveldb/database.h
    core/leveldb/command.h
    core/leveldb/server_info.h
    core/leveldb/connection_settings.h
    core/leveldb/db_connection.h
  )
  SET(SOURCES_LEVELDB
    core/leveldb/config.cpp
    core/leveldb/database.cpp
    core/leveldb/command.cpp
    core/leveldb/server_info.cpp
    core/leveldb/connection_settings.cpp
    core/leveldb/db_connection.cpp
    core/leveldb/server.cpp
    core/leveldb/driver.cpp
  )
  SET(OBJECT_LIBS ${OBJECT_LIBS} $<TARGET_OBJECTS:leveldb>)
ENDIF(BUILD_WITH_LEVELDB)

#rocksdb
IF(BUILD_WITH_ROCKSDB)
  SET(HEADERS_SHELL_TO_MOC ${HEADERS_SHELL_TO_MOC}
    shell/rocksdb_lexer.h
  )
  SET(HEADERS_SHELL ${HEADERS_SHELL}
  )
  SET(SOURCES_SHELL ${SOURCES_SHELL}
    shell/rocksdb_lexer.cpp
  )

  ADD_SUBDIRECTORY(third-party/rocksdb)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/rocksdb/src/include)

  SET(HEADERS_ROCKSDB_TO_MOC
    core/rocksdb/server.h
    core/rocksdb/driver.h
  )
  SET(HEADERS_ROCKSDB
    core/rocksdb/config.h
    core/rocksdb/database.h
    core/rocksdb/command.h
    core/rocksdb/server_info.h
    core/rocksdb/connection_settings.h
    core/rocksdb/db_connection.h
  )
  SET(SOURCES_ROCKSDB
    core/rocksdb/config.cpp
    core/rocksdb/database.cpp
    core/rocksdb/command.cpp
    core/rocksdb/server_info.cpp
    core/rocksdb/connection_settings.cpp
    core/rocksdb/db_connection.cpp
    core/rocksdb/server.cpp
    core/rocksdb/driver.cpp
  )
  SET(OBJECT_LIBS ${OBJECT_LIBS} $<TARGET_OBJECTS:rocksdb>)
ENDIF(BUILD_WITH_ROCKSDB)

#unqlite
IF(BUILD_WITH_UNQLITE)
  SET(HEADERS_SHELL_TO_MOC ${HEADERS_SHELL_TO_MOC}
    shell/unqlite_lexer.h
  )
  SET(HEADERS_SHELL ${HEADERS_SHELL}
  )
  SET(SOURCES_SHELL ${SOURCES_SHELL}
    shell/unqlite_lexer.cpp
  )

  ADD_SUBDIRECTORY(third-party/unqlite)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/unqlite/src)

  SET(HEADERS_UNQLITE_TO_MOC
    core/unqlite/server.h
    core/unqlite/driver.h
  )
  SET(HEADERS_UNQLITE
    core/unqlite/config.h
    core/unqlite/database.h
    core/unqlite/command.h
    core/unqlite/server_info.h
    core/unqlite/connection_settings.h
    core/unqlite/db_connection.h
  )
  SET(SOURCES_UNQLITE
    core/unqlite/config.cpp
    core/unqlite/database.cpp
    core/unqlite/command.cpp
    core/unqlite/server_info.cpp
    core/unqlite/connection_settings.cpp
    core/unqlite/db_connection.cpp
    core/unqlite/server.cpp
    core/unqlite/driver.cpp
  )
  SET(OBJECT_LIBS ${OBJECT_LIBS} $<TARGET_OBJECTS:unqlite>)
ENDIF(BUILD_WITH_UNQLITE)

#lmdb
IF(BUILD_WITH_LMDB)
  SET(HEADERS_SHELL_TO_MOC ${HEADERS_SHELL_TO_MOC}
    shell/lmdb_lexer.h
  )
  SET(HEADERS_SHELL ${HEADERS_SHELL}
  )
  SET(SOURCES_SHELL ${SOURCES_SHELL}
    shell/lmdb_lexer.cpp
  )

  ADD_SUBDIRECTORY(third-party/lmdb)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/lmdb/src/libraries/liblmdb)

  SET(HEADERS_LMDB_TO_MOC
    core/lmdb/server.h
    core/lmdb/driver.h
  )
  SET(HEADERS_LMDB
    core/lmdb/config.h
    core/lmdb/database.h
    core/lmdb/command.h
    core/lmdb/server_info.h
    core/lmdb/connection_settings.h
    core/lmdb/db_connection.h
  )
  SET(SOURCES_LMDB
    core/lmdb/config.cpp
    core/lmdb/database.cpp
    core/lmdb/command.cpp
    core/lmdb/server_info.cpp
    core/lmdb/connection_settings.cpp
    core/lmdb/db_connection.cpp
    core/lmdb/server.cpp
    core/lmdb/driver.cpp
  )
  SET(OBJECT_LIBS ${OBJECT_LIBS} $<TARGET_OBJECTS:lmdb>)
ENDIF(BUILD_WITH_LMDB)

SET(SOURCES_SDS
  third-party/sds/sds.c
)

# User specific
SET(USER_SPECIFIC_PUBLIC_ID anon_id)
SET(USER_SPECIFIC_PUBLIC_LOGIN anon@fastogt.com)
SET(USER_SPECIFIC_PUBLIC_PASSWORD anon)
SET(USER_SPECIFIC_ID ${USER_SPECIFIC_PUBLIC_ID} CACHE STRING "Login for ${PROJECT_NAME_TITLE}")
SET(USER_SPECIFIC_LOGIN ${USER_SPECIFIC_PUBLIC_LOGIN} CACHE STRING "Login for ${PROJECT_NAME_TITLE}")
SET(USER_SPECIFIC_PASSWORD ${USER_SPECIFIC_PUBLIC_PASSWORD} CACHE STRING "Default password for ${USER_SPECIFIC_LOGIN}")

ADD_DEFINITIONS(
  -DUSER_SPECIFIC_LOGIN="${USER_SPECIFIC_LOGIN}"
  -DUSER_SPECIFIC_PASSWORD="${USER_SPECIFIC_PASSWORD}"
  -DUSER_SPECIFIC_ID="${USER_SPECIFIC_ID}"
)

IF("${USER_SPECIFIC_PUBLIC_LOGIN}" STREQUAL "${USER_SPECIFIC_LOGIN}")
  SET(IS_PUBLIC_LOGIN ON)
  ADD_DEFINITIONS(-DIS_PUBLIC_LOGIN)
ENDIF("${USER_SPECIFIC_PUBLIC_LOGIN}" STREQUAL "${USER_SPECIFIC_LOGIN}")

SET(HEADERS ${HEADERS} ${HEADERS_GLOBAL} ${HEADERS_GUI} ${HEADERS_CORE} ${HEADERS_SHELL}
  ${HEADERS_REDIS} ${HEADERS_MEMCACHED} ${HEADERS_SSDB}
  ${HEADERS_LEVELDB} ${HEADERS_ROCKSDB} ${HEADERS_UNQLITE} ${HEADERS_LMDB}
  ${HEADERS_TRANSLATIONS})
SET(SOURCES ${SOURCES} ${SOURCES_GLOBAL} ${SOURCES_GUI} ${SOURCES_CORE} ${SOURCES_SHELL}
  ${SOURCES_REDIS} ${SOURCES_MEMCACHED} ${SOURCES_SSDB}
  ${SOURCES_LEVELDB} ${SOURCES_ROCKSDB} ${SOURCES_UNQLITE} ${SOURCES_LMDB}
  ${SOURCES_TRANSLATIONS} ${SOURCES_SDS})
SET(HEADERS_TOMOC ${HEADERS_GUI_TO_MOC} ${HEADERS_CORE_TO_MOC} ${HEADERS_SHELL_TO_MOC}
  ${HEADERS_REDIS_TO_MOC} ${HEADERS_MEMCACHED_TO_MOC} ${HEADERS_SSDB_TO_MOC}
  ${HEADERS_LEVELDB_TO_MOC} ${HEADERS_ROCKSDB_TO_MOC} ${HEADERS_UNQLITE_TO_MOC} ${HEADERS_LMDB_TO_MOC})

QT5_WRAP_CPP(MOC_FILES ${HEADERS_TOMOC})
CONFIGURE_FILE("resources/gui.qrc.in" "${CMAKE_CURRENT_LIST_DIR}/resources/gui.qrc" @ONLY IMMEDIATE)
CONFIGURE_FILE("resources/${PROJECT_NAME_LOWERCASE}.qrc.in" "${CMAKE_CURRENT_LIST_DIR}/resources/branding.qrc" @ONLY IMMEDIATE)
SET(RESOURCES ${CMAKE_CURRENT_LIST_DIR}/resources/gui.qrc ${CMAKE_CURRENT_LIST_DIR}/resources/branding.qrc)
QT5_ADD_RESOURCES(RES_SRC ${RESOURCES})
SET(INCLUDE_DIRS ${INCLUDE_DIRS} fasto/third-party/qscintilla/Qt4Qt5)
INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

SOURCE_GROUP("Header Files" FILES ${HEADERS} ${HEADERS_TOMOC})
SOURCE_GROUP("Source Files" FILES ${SOURCES})
SOURCE_GROUP("Generated" FILES ${MOC_FILES})
SOURCE_GROUP("Platform Files" FILES ${PLATFORM_HDRS} ${PLATFORM_SRCS})
SOURCE_GROUP("Resources" FILES ${RES_SRC} ${RESOURCE_OS})

SET(TRANSLATIONS
  translations/${PROJECT_NAME_LOWERCASE}/${PROJECT_NAME_LOWERCASE}_ru.ts
  translations/${PROJECT_NAME_LOWERCASE}/${PROJECT_NAME_LOWERCASE}_fr.ts
)

FIND_PACKAGE(Qt5LinguistTools)

IF(GENERATE_TRANSLATIONS)
  QT5_CREATE_TRANSLATION(QM_FILES ${HEADERS} ${SOURCES} ${TRANSLATIONS} OPTIONS -no-obsolete)
ELSE(GENERATE_TRANSLATIONS)
  QT5_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS})
ENDIF(GENERATE_TRANSLATIONS)

SET(ALL_SOURCES ${ALL_SOURCES} ${HEADERS} ${HEADERS_TOMOC} ${SOURCES} ${MOC_FILES} ${PLATFORM_HDRS} ${PLATFORM_SRCS})
SET(ALL_LIBS ${ALL_LIBS} fasto ${QT_LIBRARIES} ${PLATFORM_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARY})

SET(EXE_SOURCES main.cpp ${RES_SRC} ${ICON_FILE} ${RESOURCE_OS} ${QM_FILES})

IF(MINGW OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCXX)
  ADD_APP_LIBRARY_OBJECT(${PROJECT_CORE_LIBRARY} "${ALL_SOURCES}" "${ALL_LIBS}" "${OBJECT_LIBS}")
  IF(OS_ANDROID)
    ADD_APP_SHARED_LIBRARY(${PROJECT_NAME} "${EXE_SOURCES}" "${PROJECT_CORE_LIBRARY}")
  ELSE()
    ADD_APP_EXECUTABLE(${PROJECT_NAME} "${EXE_SOURCES}" "${PROJECT_CORE_LIBRARY}")
  ENDIF(OS_ANDROID)
ELSE()
  MESSAGE(FATAL_ERROR "NOT SUPPORTED COMPILER!!!")
ENDIF(MINGW OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCXX)

IF(OS_WINDOWS)
  SET_SOURCE_FILES_PROPERTIES(${RESOURCE_OS} PROPERTIES LANGUAGE RC)
ELSEIF(OS_MACOSX)
  SET_SOURCE_FILES_PROPERTIES(${RESOURCE_OS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  SET_SOURCE_FILES_PROPERTIES(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
ENDIF(OS_WINDOWS)

GET_FILENAME_COMPONENT(ICON_FILE_NAME ${ICON_FILE} NAME)
IF(OS_MACOSX)
  SET(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_NAME})
  SET(MACOSX_BUNDLE_INFO_STRING "${PROJECT_VERSION},${PROJECT_COPYRIGHT}" )
  SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${SHORT_VERSION}" )
  SET(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
  SET(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
  SET(MACOSX_BUNDLE_COPYRIGHT ${PROJECT_COPYRIGHT})
  SET(MACOSX_BUNDLE_ICON_FILE ${ICON_FILE_NAME})
ENDIF(OS_MACOSX)

IF (CHECK_STYLE)
  SET(CHECK_SOURCES ${HEADERS} ${HEADERS_TOMOC} ${SOURCES} ${PLATFORM_HDRS} ${PLATFORM_SRCS})
  REGISTER_CHECK_STYLE_TARGET(check_style "${CHECK_SOURCES}")
  REGISTER_CHECK_INCLUDES_TARGET(${PROJECT_CORE_LIBRARY})
ENDIF(CHECK_STYLE)

# Start to install

VersionConf(${PROJECT_NAME} ${RESOURCE_OS_IN} ${RESOURCE_OS} ${ICON_FILE_NAME})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT APPLICATIONS)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION . COMPONENT LICENSE)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/COPYRIGHT DESTINATION . COMPONENT LICENSE)
INSTALL(FILES ${PROJECT_CHANGELOG_FILE} DESTINATION . COMPONENT LICENSE RENAME CHANGELOG OPTIONAL)
INSTALL_QT(${PROJECT_NAME} ${LIB_INSTALL_DESTINATION})

IF(OS_WINDOWS)
  DEPLOY_QT_PLUGIN(Qt5::QWindowsIntegrationPlugin ${LIB_INSTALL_DESTINATION}/platforms)
  DEPLOY_QT_PLUGIN(Qt5::QGifPlugin ${LIB_INSTALL_DESTINATION}/imageformats)

  #find runtime zlib
  SET(SHARED_ZLIB_NAMES zlib1.dll z.dll zlib.dll zdll.dll zlibd.dll zlibd1.dll)
  FIND_RUNTIME_LIBRARY(SHARED_ZLIB_LIBRARY SHARED_ZLIB_NAMES)
  INSTALL(FILES ${SHARED_ZLIB_LIBRARY} DESTINATION ${LIB_INSTALL_DESTINATION} COMPONENT RUNTIME)
  IF(SASL2_FOUND)
    #find runtime msys
    SET(SHARED_MSYS_NAMES msys-2.0.dll)
    FIND_RUNTIME_LIBRARY(SHARED_MSYS_LIBRARY SHARED_MSYS_NAMES)
    INSTALL(FILES ${SHARED_MSYS_LIBRARY} DESTINATION ${LIB_INSTALL_DESTINATION} COMPONENT RUNTIME)

    #find runtime sasl
    SET(SHARED_SASL_NAMES libsasl2-3.dll)
    FIND_RUNTIME_LIBRARY(SHARED_SASL_LIBRARY SHARED_SASL_NAMES)
    INSTALL(FILES ${SHARED_SASL_LIBRARY} DESTINATION ${LIB_INSTALL_DESTINATION} COMPONENT RUNTIME)
  ENDIF(SASL2_FOUND)
ELSEIF(OS_MACOSX)
  SET(BUNDLE_NAME ${MACOSX_BUNDLE_BUNDLE_NAME}.app)
  DEPLOY_QT_PLUGIN(Qt5::QCocoaIntegrationPlugin ${BUNDLE_NAME}/Contents/plugins/platforms)
  DEPLOY_QT_PLUGIN(Qt5::QGifPlugin ${BUNDLE_NAME}/Contents/plugins/imageformats)
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME_LOWERCASE}/qt.conf DESTINATION ${BUNDLE_NAME}/Contents/Resources COMPONENT RESOURCES)
ELSEIF(OS_LINUX OR OS_FREEBSD)
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME_LOWERCASE}/linux/start.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}.sh" @ONLY IMMEDIATE)
  INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}.sh DESTINATION ${TARGET_INSTALL_DESTINATION})
  INSTALL(FILES ${RESOURCE_OS} DESTINATION share/applications COMPONENT RESOURCES)
  INSTALL(FILES ${ICON_FILE} DESTINATION share/icons COMPONENT RESOURCES)
  DEPLOY_QT_PLUGIN(Qt5::QXcbIntegrationPlugin ${LIB_INSTALL_DESTINATION}/platforms)
  DEPLOY_QT_PLUGIN(Qt5::QGifPlugin ${LIB_INSTALL_DESTINATION}/imageformats)
  #INSTALL(DIRECTORY ${QT_LIBS_DIR}/fonts DESTINATION ${LIB_INSTALL_DESTINATION})
ENDIF(OS_WINDOWS)

IF(OS_MACOSX)
  INSTALL(FILES ${QM_FILES} DESTINATION ${BUNDLE_NAME}/Contents/MacOS/translations COMPONENT TRANSLATIONS)
ELSE(OS_MACOSX)
  INSTALL(FILES ${QM_FILES} DESTINATION ${TARGET_INSTALL_DESTINATION}/translations COMPONENT TRANSLATIONS)
ENDIF(OS_MACOSX)

INSTALL_RUNTIME_LIBRARIES()
TARGET_BUNDLEFIX(${PROJECT_NAME} ${QT_LIBS_DIR})

IF(DEVELOPER_ENABLE_TESTS)
########## PREPARE GTEST LIBRARY ##########
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/tests/googletest/googletest googletest)
  INCLUDE_DIRECTORIES(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
########## PREPARE GTEST LIBRARY ##########

  ADD_EXECUTABLE(unit_tests
    ${CMAKE_SOURCE_DIR}/tests/unit_test_fasto_objects.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_common_net.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_common_strings.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_parsinng_command_line.cpp
    global/global.cpp
    ${SOURCES_SDS}
  )

  TARGET_LINK_LIBRARIES(unit_tests gtest gtest_main fasto)

  ADD_TEST(NAME unit_tests COMMAND tests)
  SET_PROPERTY(TARGET unit_tests PROPERTY FOLDER "Unit tests")
ENDIF(DEVELOPER_ENABLE_TESTS)
