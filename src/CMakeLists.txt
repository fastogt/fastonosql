SET(HEADERS_GLOBAL
  global/global.h
  global/types.h
)
SET(SOURCES_GLOBAL
  global/global.cpp
  global/types.cpp
)

SET(HEADERS_GUI_DIALOGS_TO_MOC
  gui/dialogs/about_dialog.h
  gui/dialogs/input_dialog.h
  gui/dialogs/info_server_dialog.h
  gui/dialogs/history_server_dialog.h
  gui/dialogs/property_server_dialog.h
  gui/dialogs/preferences_dialog.h
  gui/dialogs/connections_dialog.h
  gui/dialogs/connection_dialog.h
  gui/dialogs/connection_select_type_dialog.h
  gui/dialogs/cluster_dialog.h
  gui/dialogs/sentinel_dialog.h
  gui/dialogs/connection_diagnostic_dialog.h
  gui/dialogs/discovery_cluster_dialog.h
  gui/dialogs/discovery_sentinel_dialog.h
  gui/dialogs/encode_decode_dialog.h
  gui/dialogs/load_contentdb_dialog.h
  gui/dialogs/dbkey_dialog.h
  gui/dialogs/view_keys_dialog.h
  gui/dialogs/change_password_server_dialog.h
  gui/dialogs/discovery_connection.h
  gui/dialogs/discovery_sentinel_connection.h
  gui/dialogs/test_connection.h
)

SET(HEADERS_GUI_DIALOGS
  gui/dialogs/connection_listwidget_items.h
)

SET(SOURCES_GUI_DIALOGS
  gui/dialogs/about_dialog.cpp
  gui/dialogs/input_dialog.cpp
  gui/dialogs/preferences_dialog.cpp
  gui/dialogs/connections_dialog.cpp
  gui/dialogs/connection_dialog.cpp
  gui/dialogs/connection_select_type_dialog.cpp
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
  gui/dialogs/dbkey_dialog.cpp
  gui/dialogs/view_keys_dialog.cpp
  gui/dialogs/change_password_server_dialog.cpp
  gui/dialogs/discovery_connection.cpp
  gui/dialogs/discovery_sentinel_connection.cpp
  gui/dialogs/test_connection.cpp
)

SET(HEADERS_GUI_EDITOR_TO_MOC
  gui/editor/fasto_editor.h
  gui/editor/fasto_editor_shell.h
  gui/editor/fasto_editor_output.h
  gui/editor/fasto_hex_edit.h
)

SET(SOURCES_GUI_EDITOR
  gui/editor/fasto_editor.cpp
  gui/editor/fasto_editor_shell.cpp
  gui/editor/fasto_editor_output.cpp
  gui/editor/fasto_hex_edit.cpp
)

SET(HEADERS_GUI_EXPLORER_TO_MOC
  gui/explorer/explorer_tree_widget.h
  gui/explorer/explorer_tree_view.h
  gui/explorer/explorer_tree_model.h
  gui/explorer/explorer_tree_sort_filter_proxy_model.h
)

SET(HEADERS_GUI_EXPLORER
  gui/explorer/explorer_tree_item.h
)

SET(SOURCES_GUI_EXPLORER
  gui/explorer/explorer_tree_widget.cpp
  gui/explorer/explorer_tree_view.cpp
  gui/explorer/explorer_tree_model.cpp
  gui/explorer/explorer_tree_sort_filter_proxy_model.cpp
  gui/explorer/explorer_tree_item.cpp
)

SET(HEADERS_GUI_WIDGETS_TO_MOC
  gui/widgets/log_tab_widget.h
  gui/widgets/log_widget.h
  gui/widgets/commands_widget.h
  gui/widgets/query_widget.h
  gui/widgets/output_widget.h
  gui/widgets/main_widget.h
  gui/widgets/connection_base_widget.h
  gui/widgets/connection_local_widget.h
  gui/widgets/connection_remote_widget.h
  gui/widgets/host_port_widget.h
  gui/widgets/path_widget.h
  gui/widgets/ssh_widget.h
  gui/widgets/user_password_widget.h
  gui/widgets/list_type_widget.h
  gui/widgets/item_cell_delegate.h
)

SET(SOURCES_GUI_WIDGETS
  gui/widgets/log_tab_widget.cpp
  gui/widgets/log_widget.cpp
  gui/widgets/commands_widget.cpp
  gui/widgets/main_widget.cpp
  gui/widgets/query_widget.cpp
  gui/widgets/output_widget.cpp
  gui/widgets/connection_base_widget.cpp
  gui/widgets/connection_local_widget.cpp
  gui/widgets/connection_remote_widget.cpp
  gui/widgets/host_port_widget.cpp
  gui/widgets/path_widget.cpp
  gui/widgets/ssh_widget.cpp
  gui/widgets/user_password_widget.cpp
  gui/widgets/list_type_widget.cpp
  gui/widgets/item_cell_delegate.cpp
)

SET(HEADERS_GUI_TO_MOC
  gui/main_window.h
  gui/statistic_sender.h
  gui/update_checker.h
  gui/property_table_model.h
  gui/main_tab_bar.h
  gui/fasto_text_view.h
  gui/keys_table_model.h
  gui/fasto_tree_view.h
  gui/fasto_common_model.h
  gui/fasto_table_view.h
  gui/fasto_scintilla.h
  gui/shell_widget.h
  gui/base_lexer.h
  gui/base_shell.h
)
SET(HEADERS_GUI
  gui/gui_factory.h
  gui/fasto_common_item.h
  gui/property_table_item.h
  gui/shortcuts.h
  gui/connection_widgets_factory.h
)
SET(SOURCES_GUI
  gui/main_window.cpp
  gui/statistic_sender.cpp
  gui/update_checker.cpp
  gui/gui_factory.cpp
  gui/property_table_item.cpp
  gui/property_table_model.cpp
  gui/main_tab_bar.cpp
  gui/fasto_text_view.cpp  
  gui/keys_table_model.cpp  
  gui/fasto_tree_view.cpp
  gui/fasto_common_model.cpp
  gui/fasto_table_view.cpp
  gui/fasto_common_item.cpp
  gui/shortcuts.cpp
  gui/fasto_scintilla.cpp
  gui/connection_widgets_factory.cpp
  gui/shell_widget.cpp
  gui/base_lexer.cpp
  gui/base_shell.cpp
)

SET(HEADERS_CORE_INTERNAL
  core/internal/connection.h
  core/internal/db_connection.h
  core/internal/cdb_connection.h
  core/internal/cdb_connection_client.h
  core/internal/command_handler.h
  core/internal/commands_api.h
)
SET(SOURCES_CORE_INTERNAL
  core/internal/connection.cpp
  core/internal/db_connection.cpp
  core/internal/cdb_connection.cpp
  core/internal/cdb_connection_client.cpp
  core/internal/command_handler.cpp
  core/internal/commands_api.cpp
)

SET(HEADERS_CORE_DATABASE
  core/database/idatabase_info.h
  core/database/idatabase.h
)
SET(SOURCES_CORE_DATABASE
  core/database/idatabase_info.cpp
  core/database/idatabase.cpp
)

SET(HEADERS_CORE_SERVER_TO_MOC
  core/server/iserver_base.h
  core/server/iserver.h
  core/server/iserver_local.h
  core/server/iserver_remote.h
)
SET(HEADERS_CORE_SERVER
  core/server/iserver_info.h
)
SET(SOURCES_CORE_SERVER
  core/server/iserver_base.cpp
  core/server/iserver_info.cpp
  core/server/iserver.cpp
  core/server/iserver_local.cpp
  core/server/iserver_remote.cpp
)

SET(HEADERS_CORE_CLUSTER
  core/cluster/icluster.h
)
SET(SOURCES_CORE_CLUSTER
  core/cluster/icluster.cpp
)

SET(HEADERS_CORE_SENTINEL
  core/sentinel/isentinel.h
)
SET(SOURCES_CORE_SENTINEL
  core/sentinel/isentinel.cpp
)

SET(HEADERS_CORE_DRIVER_TO_MOC
  core/driver/idriver.h
  core/driver/idriver_local.h
  core/driver/idriver_remote.h
)
SET(HEADERS_CORE_DRIVER
  core/driver/root_locker.h
  core/driver/first_child_update_root_locker.h
)
SET(SOURCES_CORE_DRIVER
  core/driver/idriver.cpp
  core/driver/idriver_local.cpp
  core/driver/idriver_remote.cpp
  core/driver/root_locker.cpp
  core/driver/first_child_update_root_locker.cpp
)

SET(HEADERS_CORE_CONNECTION_SETTINGS
  core/connection_settings/icluster_connection_settings.h
  core/connection_settings/isentinel_connection_settings.h
  core/connection_settings/iconnection_settings.h
  core/connection_settings/iconnection_settings_local.h
  core/connection_settings/iconnection_settings_remote.h
  core/connection_settings/iconnection_settings_ssh.h
)
SET(SOURCES_CORE_CONNECTION_SETTINGS
  core/connection_settings/icluster_connection_settings.cpp
  core/connection_settings/isentinel_connection_settings.cpp
  core/connection_settings/iconnection_settings.cpp
  core/connection_settings/iconnection_settings_local.cpp
  core/connection_settings/iconnection_settings_remote.cpp
  core/connection_settings/iconnection_settings_ssh.cpp
)

SET(HEADERS_CORE_COMMAND_TO_MOC
  core/command/command_logger.h
)
SET(HEADERS_CORE_COMMAND
  core/command/command.h
)
SET(SOURCES_CORE_COMMAND
  core/command/command.cpp
  core/command/command_logger.cpp
)

SET(HEADERS_CORE_CONFIG
  core/config/config.h
)
SET(SOURCES_CORE_CONFIG
  core/config/config.cpp
)

SET(HEADERS_CORE_EVENTS
  core/events/events.h
  core/events/events_info.h
)
SET(SOURCES_CORE_EVENTS
  core/events/events_info.cpp
)

SET(HEADERS_CORE_TO_MOC
  ${HEADERS_CORE_DRIVER_TO_MOC}
  ${HEADERS_CORE_SERVER_TO_MOC}
  ${HEADERS_CORE_COMMAND_TO_MOC}
)
SET(HEADERS_CORE
  ${HEADERS_CORE_SERVER}
  ${HEADERS_CORE_CLUSTER}
  ${HEADERS_CORE_SENTINEL}
  ${HEADERS_CORE_DATABASE}
  ${HEADERS_CORE_CONNECTION_SETTINGS}
  ${HEADERS_CORE_COMMAND}
  ${HEADERS_CORE_CONFIG}
  ${HEADERS_CORE_TRANSLATOR}
  ${HEADERS_CORE_EVENTS}
  ${HEADERS_CORE_INTERNAL}

  core/servers_manager.h
  core/connection_types.h
  core/core_fwd.h
  core/settings_manager.h
  core/types.h
  core/db_traits.h
  core/db_key.h
  core/icommand_translator.h
  core/command_info.h
  core/command_holder.h
  core/server_property_info.h
  core/ssh_info.h

  core/cluster_connection_settings_factory.h
  core/sentinel_connection_settings_factory.h
  core/sentinel_connection_settings_factory.h
)

SET(SOURCES_CORE
  ${SOURCES_CORE_DATABASE}
  ${SOURCES_CORE_SERVER}
  ${SOURCES_CORE_SENTINEL}
  ${SOURCES_CORE_CLUSTER}
  ${SOURCES_CORE_DRIVER}
  ${SOURCES_CORE_CONNECTION_SETTINGS}
  ${SOURCES_CORE_COMMAND}
  ${SOURCES_CORE_CONFIG}
  ${SOURCES_CORE_TRANSLATOR}
  ${SOURCES_CORE_EVENTS}
  ${SOURCES_CORE_INTERNAL}

  core/connection_types.cpp
  core/settings_manager.cpp
  core/servers_manager.cpp
  core/types.cpp
  core/db_traits.cpp
  core/db_key.cpp
  core/icommand_translator.cpp
  core/command_info.cpp
  core/command_holder.cpp
  core/server_property_info.cpp
  core/ssh_info.cpp

  core/cluster_connection_settings_factory.cpp
  core/sentinel_connection_settings_factory.cpp
  core/connection_settings_factory.cpp
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
  SET(PLATFORM_LIBRARIES pthread)
ELSEIF(OS_FREEBSD)
  SET(PLATFORM_HDRS)
  SET(PLATFORM_SRCS)
  SET(PLATFORM_LIBRARIES util pthread)
ENDIF(OS_WINDOWS)

#ADD_SUBDIRECTORY(common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR})

#redis
IF(BUILD_WITH_REDIS)
  FIND_PACKAGE(OpenSSL REQUIRED)
  FIND_PACKAGE(Libssh2 REQUIRED CONFIG)
  ADD_SUBDIRECTORY(third-party/redis)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/redis/src third-party/redis/deps)

  SET(HEADERS_REDIS_GUI_TO_MOC gui/db/redis/connection_widget.h gui/db/redis/lexer.h)
  SET(HEADERS_REDIS_GUI)
  SET(SOURCES_REDIS_GUI gui/db/redis/connection_widget.cpp gui/db/redis/lexer.cpp)

  SET(HEADERS_REDIS_TO_MOC
    core/db/redis/sentinel.h
    core/db/redis/cluster.h
    core/db/redis/server.h
    core/db/redis/driver.h
  )
  SET(HEADERS_REDIS
    core/db/redis/config.h
    core/db/redis/database.h
    core/db/redis/command.h
    core/db/redis/command_translator.h
    core/db/redis/server_info.h
    core/db/redis/connection_settings.h
    core/db/redis/db_connection.h
    core/db/redis/internal/commands_api.h

    core/db/redis/sentinel_info.h
    core/db/redis/sentinel_settings.h
    core/db/redis/cluster_infos.h
    core/db/redis/cluster_settings.h
  )
  SET(SOURCES_REDIS
    core/db/redis/config.cpp
    core/db/redis/database.cpp
    core/db/redis/command.cpp
    core/db/redis/command_translator.cpp
    core/db/redis/server_info.cpp
    core/db/redis/connection_settings.cpp
    core/db/redis/db_connection.cpp
    core/db/redis/internal/commands_api.cpp
    core/db/redis/server.cpp
    core/db/redis/driver.cpp

    core/db/redis/sentinel_info.cpp
    core/db/redis/sentinel.cpp
    core/db/redis/sentinel_settings.cpp
    core/db/redis/cluster_infos.cpp
    core/db/redis/cluster.cpp
    core/db/redis/cluster_settings.cpp
  )
  SET(DB_LIBS ${DB_LIBS} hiredis Libssh2::libssh2 ${OPENSSL_LIBRARIES})
ENDIF(BUILD_WITH_REDIS)

#memcached
IF(BUILD_WITH_MEMCACHED)
  FIND_PACKAGE(Sasl2 QUIET)
  SET(USE_SASL2 ${SASL2_FOUND})
  ADD_SUBDIRECTORY(third-party/memcached)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/memcached/src)

  SET(HEADERS_MEMCACHED_GUI_TO_MOC gui/db/memcached/connection_widget.h gui/db/memcached/lexer.h)
  SET(HEADERS_MEMCACHED_GUI)
  SET(SOURCES_MEMCACHED_GUI gui/db/memcached/connection_widget.cpp gui/db/memcached/lexer.cpp)

  SET(HEADERS_MEMCACHED_TO_MOC
    core/db/memcached/server.h
    core/db/memcached/driver.h
  )
  SET(HEADERS_MEMCACHED
    core/db/memcached/config.h
    core/db/memcached/database.h
    core/db/memcached/command.h
    core/db/memcached/command_translator.h
    core/db/memcached/server_info.h
    core/db/memcached/connection_settings.h
    core/db/memcached/db_connection.h
    core/db/memcached/internal/commands_api.h
  )
  SET(SOURCES_MEMCACHED
    core/db/memcached/config.cpp
    core/db/memcached/database.cpp
    core/db/memcached/command.cpp
    core/db/memcached/command_translator.cpp
    core/db/memcached/server_info.cpp
    core/db/memcached/connection_settings.cpp
    core/db/memcached/db_connection.cpp
    core/db/memcached/internal/commands_api.cpp
    core/db/memcached/server.cpp
    core/db/memcached/driver.cpp
  )
  SET(DB_LIBS ${DB_LIBS} memcached)
ENDIF(BUILD_WITH_MEMCACHED)

#ssdb
IF(BUILD_WITH_SSDB)
  ADD_SUBDIRECTORY(third-party/ssdb)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/ssdb/src)

  SET(HEADERS_SSDB_GUI_TO_MOC gui/db/ssdb/connection_widget.h gui/db/ssdb/lexer.h)
  SET(HEADERS_SSDB_GUI)
  SET(SOURCES_SSDB_GUI gui/db/ssdb/connection_widget.cpp gui/db/ssdb/lexer.cpp)

  SET(HEADERS_SSDB_TO_MOC
    core/db/ssdb/server.h
    core/db/ssdb/driver.h
  )
  SET(HEADERS_SSDB
    core/db/ssdb/config.h
    core/db/ssdb/database.h
    core/db/ssdb/command.h
    core/db/ssdb/command_translator.h
    core/db/ssdb/server_info.h
    core/db/ssdb/connection_settings.h
    core/db/ssdb/db_connection.h
    core/db/ssdb/internal/commands_api.h
  )
  SET(SOURCES_SSDB
    core/db/ssdb/config.cpp
    core/db/ssdb/database.cpp
    core/db/ssdb/command.cpp
    core/db/ssdb/command_translator.cpp
    core/db/ssdb/server_info.cpp
    core/db/ssdb/connection_settings.cpp
    core/db/ssdb/db_connection.cpp
    core/db/ssdb/internal/commands_api.cpp
    core/db/ssdb/server.cpp
    core/db/ssdb/driver.cpp
  )
  SET(DB_LIBS ${DB_LIBS} ssdb)
ENDIF(BUILD_WITH_SSDB)

#leveldb
IF(BUILD_WITH_LEVELDB)
  ADD_SUBDIRECTORY(third-party/leveldb)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/leveldb/src/include)

  SET(HEADERS_LEVELDB_GUI_TO_MOC gui/db/leveldb/connection_widget.h gui/db/leveldb/lexer.h)
  SET(HEADERS_LEVELDB_GUI)
  SET(SOURCES_LEVELDB_GUI gui/db/leveldb/connection_widget.cpp gui/db/leveldb/lexer.cpp)

  SET(HEADERS_LEVELDB_TO_MOC
    core/db/leveldb/server.h
    core/db/leveldb/driver.h
  )
  SET(HEADERS_LEVELDB
    core/db/leveldb/config.h
    core/db/leveldb/database.h
    core/db/leveldb/command.h
    core/db/leveldb/command_translator.h
    core/db/leveldb/server_info.h
    core/db/leveldb/connection_settings.h
    core/db/leveldb/db_connection.h
    core/db/leveldb/internal/commands_api.h
  )
  SET(SOURCES_LEVELDB
    core/db/leveldb/config.cpp
    core/db/leveldb/database.cpp
    core/db/leveldb/command.cpp
    core/db/leveldb/command_translator.cpp
    core/db/leveldb/server_info.cpp
    core/db/leveldb/connection_settings.cpp
    core/db/leveldb/db_connection.cpp
    core/db/leveldb/internal/commands_api.cpp
    core/db/leveldb/server.cpp
    core/db/leveldb/driver.cpp
  )
  SET(DB_LIBS ${DB_LIBS} leveldb)
ENDIF(BUILD_WITH_LEVELDB)

#rocksdb
IF(BUILD_WITH_ROCKSDB)
  FIND_LIBRARY(ROCKSDB_LIBRARY rocksdb)
  #ADD_SUBDIRECTORY(third-party/rocksdb)
  #SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/rocksdb/src/include)

  SET(HEADERS_ROCKSDB_GUI_TO_MOC gui/db/rocksdb/connection_widget.h gui/db/rocksdb/lexer.h)
  SET(HEADERS_ROCKSDB_GUI)
  SET(SOURCES_ROCKSDB_GUI gui/db/rocksdb/connection_widget.cpp gui/db/rocksdb/lexer.cpp)

  SET(HEADERS_ROCKSDB_TO_MOC
    core/db/rocksdb/server.h
    core/db/rocksdb/driver.h
  )
  SET(HEADERS_ROCKSDB
    core/db/rocksdb/config.h
    core/db/rocksdb/database.h
    core/db/rocksdb/command.h
    core/db/rocksdb/command_translator.h
    core/db/rocksdb/server_info.h
    core/db/rocksdb/connection_settings.h
    core/db/rocksdb/db_connection.h
    core/db/rocksdb/internal/commands_api.h
  )
  SET(SOURCES_ROCKSDB
    core/db/rocksdb/config.cpp
    core/db/rocksdb/database.cpp
    core/db/rocksdb/command.cpp
    core/db/rocksdb/command_translator.cpp
    core/db/rocksdb/server_info.cpp
    core/db/rocksdb/connection_settings.cpp
    core/db/rocksdb/db_connection.cpp
    core/db/rocksdb/internal/commands_api.cpp
    core/db/rocksdb/server.cpp
    core/db/rocksdb/driver.cpp
  )

  SET(DB_LIBS ${DB_LIBS} ${ROCKSDB_LIBRARY} ${BZIP2_LIBRARIES})
ENDIF(BUILD_WITH_ROCKSDB)

#unqlite
IF(BUILD_WITH_UNQLITE)
  ADD_SUBDIRECTORY(third-party/unqlite)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/unqlite/src)

  SET(HEADERS_UNQLITE_GUI_TO_MOC gui/db/unqlite/connection_widget.h gui/db/unqlite/lexer.h)
  SET(HEADERS_UNQLITE_GUI)
  SET(SOURCES_UNQLITE_GUI gui/db/unqlite/connection_widget.cpp gui/db/unqlite/lexer.cpp)

  SET(HEADERS_UNQLITE_TO_MOC
    core/db/unqlite/server.h
    core/db/unqlite/driver.h
  )
  SET(HEADERS_UNQLITE
    core/db/unqlite/config.h
    core/db/unqlite/database.h
    core/db/unqlite/command.h
    core/db/unqlite/command_translator.h
    core/db/unqlite/server_info.h
    core/db/unqlite/connection_settings.h
    core/db/unqlite/db_connection.h
    core/db/unqlite/internal/commands_api.h
  )
  SET(SOURCES_UNQLITE
    core/db/unqlite/config.cpp
    core/db/unqlite/database.cpp
    core/db/unqlite/command.cpp
    core/db/unqlite/command_translator.cpp
    core/db/unqlite/server_info.cpp
    core/db/unqlite/connection_settings.cpp
    core/db/unqlite/db_connection.cpp
    core/db/unqlite/server.cpp
    core/db/unqlite/driver.cpp
    core/db/unqlite/internal/commands_api.cpp
  )
  SET(DB_LIBS ${DB_LIBS} unqlite)
ENDIF(BUILD_WITH_UNQLITE)

#lmdb
IF(BUILD_WITH_LMDB)
  ADD_SUBDIRECTORY(third-party/lmdb)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/lmdb/src/libraries/liblmdb)

  SET(HEADERS_LMDB_GUI_TO_MOC gui/db/lmdb/connection_widget.h gui/db/lmdb/lexer.h)
  SET(HEADERS_LMDB_GUI)
  SET(SOURCES_LMDB_GUI gui/db/lmdb/connection_widget.cpp gui/db/lmdb/lexer.cpp)

  SET(HEADERS_LMDB_TO_MOC
    core/db/lmdb/server.h
    core/db/lmdb/driver.h
  )
  SET(HEADERS_LMDB
    core/db/lmdb/config.h
    core/db/lmdb/database.h
    core/db/lmdb/command.h
    core/db/lmdb/command_translator.h
    core/db/lmdb/server_info.h
    core/db/lmdb/connection_settings.h
    core/db/lmdb/db_connection.h
    core/db/lmdb/internal/commands_api.h
  )
  SET(SOURCES_LMDB
    core/db/lmdb/config.cpp
    core/db/lmdb/database.cpp
    core/db/lmdb/command.cpp
    core/db/lmdb/command_translator.cpp
    core/db/lmdb/server_info.cpp
    core/db/lmdb/connection_settings.cpp
    core/db/lmdb/db_connection.cpp
    core/db/lmdb/internal/commands_api.cpp
    core/db/lmdb/server.cpp
    core/db/lmdb/driver.cpp
  )
  SET(DB_LIBS ${DB_LIBS} lmdb)
ENDIF(BUILD_WITH_LMDB)

#upscaledb
IF(BUILD_WITH_UPSCALEDB)
  SET(Boost_USE_MULTITHREADED ON)
  SET(Boost_USE_STATIC_LIBS ON)
  FIND_PACKAGE(Boost COMPONENTS thread system REQUIRED)
  FIND_LIBRARY(UPSCALEDB_LIBRARY upscaledb)
  #ADD_SUBDIRECTORY(third-party/upscaledb)
  #SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/upscaledb/src/include)

  SET(HEADERS_UPSCALEDB_GUI_TO_MOC gui/db/upscaledb/connection_widget.h gui/db/upscaledb/lexer.h)
  SET(HEADERS_UPSCALEDB_GUI)
  SET(SOURCES_UPSCALEDB_GUI gui/db/upscaledb/connection_widget.cpp gui/db/upscaledb/lexer.cpp)

  SET(HEADERS_UPSCALEDB_TO_MOC
    core/db/upscaledb/server.h
    core/db/upscaledb/driver.h
  )
  SET(HEADERS_UPSCALEDB
    core/db/upscaledb/config.h
    core/db/upscaledb/database.h
    core/db/upscaledb/command.h
    core/db/upscaledb/command_translator.h
    core/db/upscaledb/server_info.h
    core/db/upscaledb/connection_settings.h
    core/db/upscaledb/db_connection.h
    core/db/upscaledb/internal/commands_api.h
  )
  SET(SOURCES_UPSCALEDB
    core/db/upscaledb/config.cpp
    core/db/upscaledb/database.cpp
    core/db/upscaledb/command.cpp
    core/db/upscaledb/command_translator.cpp
    core/db/upscaledb/server_info.cpp
    core/db/upscaledb/connection_settings.cpp
    core/db/upscaledb/db_connection.cpp
    core/db/upscaledb/internal/commands_api.cpp
    core/db/upscaledb/server.cpp
    core/db/upscaledb/driver.cpp
  )
  SET(DB_LIBS ${DB_LIBS} ${UPSCALEDB_LIBRARY} ${Boost_LIBRARIES})
ENDIF(BUILD_WITH_UPSCALEDB)

SET(SOURCES_SDS
  third-party/sds/sds.c
)

#ADD_SUBDIRECTORY(third-party/qscintilla/Qt4Qt5)
ADD_SUBDIRECTORY(third-party/json-c)

IF (IS_PUBLIC_BUILD)
  ADD_DEFINITIONS(-DIS_PUBLIC_BUILD)
ELSE()
  MESSAGE(STATUS "USER_SPECIFIC_ID: ${USER_SPECIFIC_ID}")
  MESSAGE(STATUS "USER_SPECIFIC_LOGIN: ${USER_SPECIFIC_LOGIN}")
  MESSAGE(STATUS "USER_SPECIFIC_PASSWORD: ${USER_SPECIFIC_PASSWORD}")

  ADD_DEFINITIONS(
    -DUSER_SPECIFIC_LOGIN="${USER_SPECIFIC_LOGIN}"
    -DUSER_SPECIFIC_PASSWORD="${USER_SPECIFIC_PASSWORD}"
    -DUSER_SPECIFIC_ID="${USER_SPECIFIC_ID}"
  )
ENDIF(IS_PUBLIC_BUILD)

SET(HEADERS ${HEADERS} ${HEADERS_GLOBAL}
  ${HEADERS_GUI} ${HEADERS_GUI_EXPLORER} ${HEADERS_GUI_DIALOGS}
  ${HEADERS_CORE} ${HEADERS_SHELL}

  ${HEADERS_REDIS} ${HEADERS_MEMCACHED}
  ${HEADERS_SSDB} ${HEADERS_LEVELDB}
  ${HEADERS_ROCKSDB} ${HEADERS_UNQLITE}
  ${HEADERS_LMDB} ${HEADERS_UPSCALEDB}

  ${HEADERS_REDIS_GUI} ${HEADERS_MEMCACHED_GUI}
  ${HEADERS_SSDB_GUI} ${HEADERS_LEVELDB_GUI}
  ${HEADERS_ROCKSDB_GUI} ${HEADERS_UNQLITE_GUI}
  ${HEADERS_LMDB_GUI} ${HEADERS_UPSCALEDB_GUI}

  ${HEADERS_TRANSLATIONS}
)
SET(SOURCES ${SOURCES} ${SOURCES_GLOBAL}
  ${SOURCES_GUI} ${SOURCES_GUI_DIALOGS} ${SOURCES_GUI_EDITOR}
  ${SOURCES_GUI_WIDGETS} ${SOURCES_GUI_EXPLORER}
  ${SOURCES_CORE} ${SOURCES_SHELL}

  ${SOURCES_REDIS} ${SOURCES_MEMCACHED}
  ${SOURCES_SSDB} ${SOURCES_LEVELDB}
  ${SOURCES_ROCKSDB} ${SOURCES_UNQLITE}
  ${SOURCES_LMDB} ${SOURCES_UPSCALEDB}

  ${SOURCES_REDIS_GUI} ${SOURCES_MEMCACHED_GUI}
  ${SOURCES_SSDB_GUI} ${SOURCES_LEVELDB_GUI}
  ${SOURCES_ROCKSDB_GUI} ${SOURCES_UNQLITE_GUI}
  ${SOURCES_LMDB_GUI} ${SOURCES_UPSCALEDB_GUI}

  ${SOURCES_TRANSLATIONS}
)
SET(HEADERS_TOMOC ${HEADERS_GUI_TO_MOC} ${HEADERS_GUI_DIALOGS_TO_MOC} ${HEADERS_GUI_EDITOR_TO_MOC}
  ${HEADERS_GUI_WIDGETS_TO_MOC} ${HEADERS_GUI_EXPLORER_TO_MOC}
  ${HEADERS_CORE_TO_MOC} ${HEADERS_SHELL_TO_MOC}

  ${HEADERS_REDIS_TO_MOC} ${HEADERS_MEMCACHED_TO_MOC}
  ${HEADERS_SSDB_TO_MOC} ${HEADERS_LEVELDB_TO_MOC}
  ${HEADERS_ROCKSDB_TO_MOC} ${HEADERS_UNQLITE_TO_MOC}
  ${HEADERS_LMDB_TO_MOC} ${HEADERS_UPSCALEDB_TO_MOC}

  ${HEADERS_REDIS_GUI_TO_MOC} ${HEADERS_MEMCACHED_GUI_TO_MOC}
  ${HEADERS_SSDB_GUI_TO_MOC} ${HEADERS_LEVELDB_GUI_TO_MOC}
  ${HEADERS_ROCKSDB_GUI_TO_MOC} ${HEADERS_UNQLITE_GUI_TO_MOC}
  ${HEADERS_LMDB_GUI_TO_MOC} ${HEADERS_UPSCALEDB_GUI_TO_MOC}
)

QT5_WRAP_CPP(MOC_FILES ${HEADERS_TOMOC})
CONFIGURE_FILE("resources/gui.qrc.in" "${CMAKE_CURRENT_LIST_DIR}/resources/gui.qrc" @ONLY IMMEDIATE)
CONFIGURE_FILE("resources/${PROJECT_NAME_LOWERCASE}.qrc.in" "${CMAKE_CURRENT_LIST_DIR}/resources/branding.qrc" @ONLY IMMEDIATE)
SET(RESOURCES ${CMAKE_CURRENT_LIST_DIR}/resources/gui.qrc ${CMAKE_CURRENT_LIST_DIR}/resources/branding.qrc)
QT5_ADD_RESOURCES(RES_SRC ${RESOURCES})
SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/sds)
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

SET(ALL_SOURCES ${ALL_SOURCES} ${HEADERS} ${HEADERS_TOMOC} ${SOURCES} ${MOC_FILES} ${PLATFORM_HDRS} ${PLATFORM_SRCS} ${SOURCES_SDS})
SET(ALL_LIBS ${ALL_LIBS} ${DB_LIBS} common common_qt json-c qscintilla2 ${QT_LIBRARIES} ${ZLIB_LIBRARY} ${PLATFORM_LIBRARIES})

FIND_PACKAGE(Qt5LinguistTools REQUIRED)

IF(GENERATE_TRANSLATIONS)
  QT5_CREATE_TRANSLATION(QM_FILES ${ALL_SOURCES} ${TRANSLATIONS} OPTIONS -no-obsolete)
ELSE(GENERATE_TRANSLATIONS)
  QT5_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS})
ENDIF(GENERATE_TRANSLATIONS)

SET(EXE_SOURCES main.cpp ${RES_SRC} ${ICON_FILE} ${RESOURCE_OS} ${QM_FILES})

IF(MINGW OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCXX)
  ADD_APP_STATIC_LIBRARY(${PROJECT_CORE_LIBRARY} "${ALL_SOURCES}" "${ALL_LIBS}")
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

# Start to install

VersionConf(${PROJECT_NAME} ${RESOURCE_OS_IN} ${RESOURCE_OS} ${ICON_FILE_NAME})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT APPLICATIONS)

INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION . COMPONENT LICENSE RENAME LICENSE OPTIONAL)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/COPYRIGHT DESTINATION . COMPONENT LICENSE RENAME COPYRIGHT OPTIONAL)
INSTALL(FILES ${PROJECT_CHANGELOG_FILE} DESTINATION . COMPONENT LICENSE RENAME CHANGELOG OPTIONAL)
INSTALL_QT(${PROJECT_NAME} ${LIB_INSTALL_DESTINATION})

IF(OS_WINDOWS)
  DEPLOY_QT_PLUGIN(Qt5::QWindowsIntegrationPlugin ${LIB_INSTALL_DESTINATION}/platforms)
  DEPLOY_QT_PLUGIN(Qt5::QGifPlugin ${LIB_INSTALL_DESTINATION}/imageformats)

  #find runtime zlib
  SET(SHARED_ZLIB_NAMES zlib1.dll z.dll zlib.dll zdll.dll zlibd.dll zlibd1.dll)
  FIND_RUNTIME_LIBRARY(SHARED_ZLIB_LIBRARY SHARED_ZLIB_NAMES)
  INSTALL(FILES ${SHARED_ZLIB_LIBRARY} DESTINATION ${LIB_INSTALL_DESTINATION} COMPONENT RUNTIME)

  IF(USE_SASL2)
    #find runtime msys
    SET(SHARED_MSYS_NAMES msys-2.0.dll)
    FIND_RUNTIME_LIBRARY(SHARED_MSYS_LIBRARY SHARED_MSYS_NAMES)
    INSTALL(FILES ${SHARED_MSYS_LIBRARY} DESTINATION ${LIB_INSTALL_DESTINATION} COMPONENT RUNTIME)

    #find runtime sasl
    SET(SHARED_SASL_NAMES libsasl2-3.dll)
    FIND_RUNTIME_LIBRARY(SHARED_SASL_LIBRARY SHARED_SASL_NAMES)
    INSTALL(FILES ${SHARED_SASL_LIBRARY} DESTINATION ${LIB_INSTALL_DESTINATION} COMPONENT RUNTIME)
  ENDIF(USE_SASL2)
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
  IF(USE_SASL2)
    GET_FILENAME_COMPONENT(SASL2_WITHOUT_SYMLINK ${SASL2_LIBRARIES} REALPATH)
    GET_FILENAME_COMPONENT(SASL2_LIB_NAME ${SASL2_WITHOUT_SYMLINK} NAME)
    STRING(REGEX REPLACE "[^so]+$" ".2" lnname ${SASL2_LIB_NAME})
    INSTALL(FILES ${SASL2_WITHOUT_SYMLINK} DESTINATION ${LIB_INSTALL_DESTINATION} RENAME ${lnname} COMPONENT RUNTIME NAMELINK_SKIP) #libsasl2.so.2
  ENDIF(USE_SASL2)
  #INSTALL(DIRECTORY ${QT_LIBS_DIR}/fonts DESTINATION ${LIB_INSTALL_DESTINATION})
ENDIF(OS_WINDOWS)

IF(OS_MACOSX)
  INSTALL(FILES ${QM_FILES} DESTINATION ${BUNDLE_NAME}/Contents/MacOS/translations COMPONENT TRANSLATIONS)
ELSE(OS_MACOSX)
  INSTALL(FILES ${QM_FILES} DESTINATION ${TARGET_INSTALL_DESTINATION}/translations COMPONENT TRANSLATIONS)
ENDIF(OS_MACOSX)

INSTALL_RUNTIME_LIBRARIES()
TARGET_BUNDLEFIX(${PROJECT_NAME} ${QT_LIBS_DIR})

IF (DEVELOPER_CHECK_STYLE)
  SET(CHECK_SOURCES ${HEADERS} ${HEADERS_TOMOC} ${SOURCES} ${PLATFORM_HDRS} ${PLATFORM_SRCS})
  REGISTER_CHECK_STYLE_TARGET(check_style "${CHECK_SOURCES}")
  REGISTER_CHECK_INCLUDES_TARGET(${PROJECT_CORE_LIBRARY})
ENDIF(DEVELOPER_CHECK_STYLE)

IF(DEVELOPER_ENABLE_TESTS)
########## PREPARE GTEST LIBRARY ##########
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/tests/googletest/googletest googletest)
  INCLUDE_DIRECTORIES(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
########## PREPARE GTEST LIBRARY ##########

  ADD_EXECUTABLE(unit_tests
    ${CMAKE_SOURCE_DIR}/tests/unit_test_fasto_objects.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_parsinng_command_line.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_command_holder.cpp
    global/global.cpp
    ${SOURCES_SDS}
  )

  TARGET_LINK_LIBRARIES(unit_tests gtest gtest_main ${PROJECT_CORE_LIBRARY} common)

  ADD_TEST_TARGET(unit_tests)
  SET_PROPERTY(TARGET unit_tests PROPERTY FOLDER "Unit tests")
ENDIF(DEVELOPER_ENABLE_TESTS)
