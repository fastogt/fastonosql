CMAKE_MINIMUM_REQUIRED(VERSION 3.6.3) # OPENSSL_USE_STATIC_LIBS

IF(POLICY CMP0071)
  CMAKE_POLICY(SET CMP0071 NEW)
ENDIF(POLICY CMP0071)
SET(CMAKE_AUTOMOC ON)

# proxy
SET(HEADERS_PROXY_CONNECTION_SETTINGS
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/settings_fwd.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/connection_settings_path.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_local.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_remote.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_ssh.h
)
SET(SOURCES_PROXY_CONNECTION_SETTINGS
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/connection_settings_path.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_local.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_remote.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_ssh.cpp
)

IF(PRO_VERSION)
  SET(HEADERS_PROXY_CONNECTION_SETTINGS ${HEADERS_PROXY_CONNECTION_SETTINGS}
    ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/isentinel_connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/icluster_connection_settings.h
  )
  SET(SOURCES_PROXY_CONNECTION_SETTINGS ${SOURCES_PROXY_CONNECTION_SETTINGS}
    ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/isentinel_connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/icluster_connection_settings.cpp
  )
ENDIF(PRO_VERSION)

SET(HEADERS_PROXY_DRIVER
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/root_locker.h
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/first_child_update_root_locker.h

  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver.h
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver_local.h
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver_remote.h
)
SET(SOURCES_PROXY_DRIVER
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver_local.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver_remote.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/root_locker.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/first_child_update_root_locker.cpp
)

SET(HEADERS_PROXY_SERVER
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_base.h
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver.h
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_local.h
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_remote.h
)
SET(SOURCES_PROXY_SERVER
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_base.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_local.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_remote.cpp
)

SET(HEADERS_PROXY_EVENTS
  ${CMAKE_SOURCE_DIR}/src/proxy/events/events.h
  ${CMAKE_SOURCE_DIR}/src/proxy/events/events_info.h
)
SET(SOURCES_PROXY_EVENTS
  ${CMAKE_SOURCE_DIR}/src/proxy/events/events_info.cpp
)

IF(PRO_VERSION)
  SET(HEADERS_PROXY_CLUSTER
    ${CMAKE_SOURCE_DIR}/src/proxy/cluster/icluster.h
  )
  SET(SOURCES_PROXY_CLUSTER
    ${CMAKE_SOURCE_DIR}/src/proxy/cluster/icluster.cpp
  )
  SET(HEADERS_PROXY_SENTINEL
    ${CMAKE_SOURCE_DIR}/src/proxy/sentinel/isentinel.h
  )
  SET(SOURCES_PROXY_SENTINEL
    ${CMAKE_SOURCE_DIR}/src/proxy/sentinel/isentinel.cpp
  )
ENDIF(PRO_VERSION)

SET(HEADERS_PROXY_DATABASE
  ${CMAKE_SOURCE_DIR}/src/proxy/database/idatabase.h
)
SET(SOURCES_PROXY_DATABASE
  ${CMAKE_SOURCE_DIR}/src/proxy/database/idatabase.cpp
)

SET(HEADERS_PROXY_COMMAND
  ${CMAKE_SOURCE_DIR}/src/proxy/command/command_logger.h
)
SET(SOURCES_PROXY_COMMAND
  ${CMAKE_SOURCE_DIR}/src/proxy/command/command_logger.cpp
)

#
SET(HEADERS_PROXY
  ${HEADERS_PROXY_DRIVER}
  ${HEADERS_PROXY_SERVER}
  ${HEADERS_PROXY_SENTINEL}
  ${HEADERS_PROXY_EVENTS}
  ${HEADERS_PROXY_DATABASE}
  ${HEADERS_PROXY_CONNECTION_SETTINGS}
  ${HEADERS_PROXY_COMMAND}

  ${CMAKE_SOURCE_DIR}/src/proxy/server_config.h
  ${CMAKE_SOURCE_DIR}/src/proxy/types.h
  ${CMAKE_SOURCE_DIR}/src/proxy/command/command.h
  ${CMAKE_SOURCE_DIR}/src/proxy/servers_manager.h
  ${CMAKE_SOURCE_DIR}/src/proxy/proxy_fwd.h
  ${CMAKE_SOURCE_DIR}/src/proxy/settings_manager.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings_factory.h
)

SET(SOURCES_PROXY
  ${SOURCES_PROXY_DRIVER}
  ${SOURCES_PROXY_SERVER}
  ${SOURCES_PROXY_SENTINEL}
  ${SOURCES_PROXY_EVENTS}
  ${SOURCES_PROXY_DATABASE}
  ${SOURCES_PROXY_CONNECTION_SETTINGS}
  ${SOURCES_PROXY_COMMAND}

  ${CMAKE_SOURCE_DIR}/src/proxy/server_config.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/types.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/command/command.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/servers_manager.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/settings_manager.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings_factory.cpp
)

IF(PRO_VERSION)
  SET(HEADERS_PROXY ${HEADERS_PROXY}
    ${HEADERS_PROXY_CLUSTER}
    ${CMAKE_SOURCE_DIR}/src/proxy/cluster_connection_settings_factory.h
    ${CMAKE_SOURCE_DIR}/src/proxy/sentinel_connection_settings_factory.h

    ${CMAKE_SOURCE_DIR}/src/proxy/user_info.h
  )

  SET(SOURCES_PROXY ${SOURCES_PROXY}
    ${SOURCES_PROXY_CLUSTER}
    ${CMAKE_SOURCE_DIR}/src/proxy/cluster_connection_settings_factory.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/sentinel_connection_settings_factory.cpp

    ${CMAKE_SOURCE_DIR}/src/proxy/user_info.cpp
  )
ENDIF(PRO_VERSION)

# gui, translation, global
SET(HEADERS_TRANSLATIONS
  ${CMAKE_SOURCE_DIR}/src/translations/global.h
)
SET(SOURCES_TRANSLATIONS
  ${CMAKE_SOURCE_DIR}/src/translations/global.cpp
)

SET(HEADERS_GUI_WORKERS
  ${CMAKE_SOURCE_DIR}/src/gui/workers/test_connection.h
  ${CMAKE_SOURCE_DIR}/src/gui/workers/update_checker.h
  ${CMAKE_SOURCE_DIR}/src/gui/workers/statistic_sender.h
)

SET(SOURCES_GUI_WORKERS
  ${CMAKE_SOURCE_DIR}/src/gui/workers/test_connection.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/workers/update_checker.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/workers/statistic_sender.cpp
)

IF(PRO_VERSION)
  SET(HEADERS_GUI_WORKERS ${HEADERS_GUI_WORKERS}
    ${CMAKE_SOURCE_DIR}/src/gui/workers/discovery_cluster_connection.h
    ${CMAKE_SOURCE_DIR}/src/gui/workers/discovery_sentinel_connection.h
  )

  SET(SOURCES_GUI_WORKERS ${SOURCES_GUI_WORKERS}
    ${CMAKE_SOURCE_DIR}/src/gui/workers/discovery_cluster_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/workers/discovery_sentinel_connection.cpp
  )
ENDIF(PRO_VERSION)

SET(HEADERS_GUI_DIALOGS
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/eula_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/about_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/password_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/info_server_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/history_server_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/property_server_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/preferences_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connections_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_select_type_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_diagnostic_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/encode_decode_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/load_contentdb_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/dbkey_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/view_keys_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/pub_sub_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/stream_entry_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/how_to_use_dialog.h
)

SET(SOURCES_GUI_DIALOGS
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/eula_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/password_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/about_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/preferences_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connections_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_select_type_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_diagnostic_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/info_server_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/property_server_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/history_server_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/encode_decode_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/load_contentdb_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/dbkey_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/view_keys_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/pub_sub_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/stream_entry_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/how_to_use_dialog.cpp
)

IF(PRO_VERSION)
  SET(HEADERS_GUI_DIALOGS ${HEADERS_GUI_DIALOGS}
    ${CMAKE_SOURCE_DIR}/src/gui/dialogs/cluster_dialog.h
    ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_cluster_dialog.h

    ${CMAKE_SOURCE_DIR}/src/gui/dialogs/sentinel_dialog.h
    ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_sentinel_dialog.h
  )

  SET(SOURCES_GUI_DIALOGS ${SOURCES_GUI_DIALOGS}
    ${CMAKE_SOURCE_DIR}/src/gui/dialogs/cluster_dialog.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_cluster_dialog.cpp

    ${CMAKE_SOURCE_DIR}/src/gui/dialogs/sentinel_dialog.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_sentinel_dialog.cpp
  )
ENDIF(PRO_VERSION)

SET(HEADERS_GUI_EDITOR
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor_shell.h
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor_model_output.h
)

SET(SOURCES_GUI_EDITOR
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor_shell.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor_model_output.cpp
)

SET(HEADERS_GUI_EXPLORER
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_item.h

  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_sort_filter_proxy_model.h
)

SET(SOURCES_GUI_EXPLORER
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_view.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_sort_filter_proxy_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_item.cpp
)

SET(HEADERS_GUI_SHELL
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_shell_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_lexer.h
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_shell.h
)

SET(SOURCES_GUI_SHELL
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_shell_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_lexer.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_shell.cpp
)

SET(HEADERS_GUI_WIDGETS
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/fasto_scintilla.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/fasto_editor.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/fasto_viewer.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/key_edit_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/save_key_edit_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/log_tab_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/log_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/commands_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/query_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/output_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/main_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/connection_base_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/connection_local_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/connection_remote_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/host_port_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/path_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/ssh_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/user_password_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/list_type_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/type_delegate.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/stream_type_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/hash_type_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/how_to_use_widget.h
)

SET(SOURCES_GUI_WIDGETS
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/fasto_scintilla.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/fasto_editor.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/fasto_viewer.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/key_edit_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/save_key_edit_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/log_tab_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/log_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/commands_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/main_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/query_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/output_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/connection_base_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/connection_local_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/connection_remote_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/host_port_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/path_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/ssh_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/user_password_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/list_type_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/type_delegate.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/stream_type_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/hash_type_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/widgets/how_to_use_widget.cpp
)

SET(HEADERS_GUI
  ${HEADERS_GUI_WORKERS}
  ${HEADERS_GUI_DIALOGS}
  ${HEADERS_GUI_EDITOR}
  ${HEADERS_GUI_WIDGETS}
  ${HEADERS_GUI_SHELL}
  ${HEADERS_GUI_EXPLORER}

  ${CMAKE_SOURCE_DIR}/src/gui/gui_factory.h
  ${CMAKE_SOURCE_DIR}/src/gui/channel_table_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_common_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/property_table_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/shortcuts.h
  ${CMAKE_SOURCE_DIR}/src/gui/connection_widgets_factory.h
  ${CMAKE_SOURCE_DIR}/src/gui/key_value_table_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/key_table_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/utils.h
  ${CMAKE_SOURCE_DIR}/src/gui/text_converter.h
  ${CMAKE_SOURCE_DIR}/src/gui/key_info.h
  ${CMAKE_SOURCE_DIR}/src/gui/connection_listwidget_items.h
  ${CMAKE_SOURCE_DIR}/src/gui/main_window.h
  ${CMAKE_SOURCE_DIR}/src/gui/property_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/main_tab_bar.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_text_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/keys_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/channels_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_tree_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_common_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_table_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/keys_table_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/hash_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/stream_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/action_cell_delegate.h
)
SET(SOURCES_GUI
  ${SOURCES_GUI_WORKERS}
  ${SOURCES_GUI_DIALOGS}
  ${SOURCES_GUI_EDITOR}
  ${SOURCES_GUI_WIDGETS}
  ${SOURCES_GUI_SHELL}
  ${SOURCES_GUI_EXPLORER}

  ${CMAKE_SOURCE_DIR}/src/gui/connection_listwidget_items.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/main_window.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/gui_factory.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/property_table_item.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/property_table_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/main_tab_bar.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_text_view.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/keys_table_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/key_table_item.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/channels_table_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_tree_view.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_common_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_table_view.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/keys_table_view.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/channel_table_item.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_common_item.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/shortcuts.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/connection_widgets_factory.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/hash_table_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/stream_table_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/action_cell_delegate.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/key_value_table_item.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/utils.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/text_converter.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/key_info.cpp
)

SET_DESKTOP_TARGET()

IF(OS_WINDOWS)
  SET(PLATFORM_HDRS)
  SET(PLATFORM_SRCS)
  SET(PLATFORM_LIBRARIES ws2_32)
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
  SET(PLATFORM_LIBRARIES util)
ENDIF(OS_WINDOWS)
SET(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR})

#redis or pika (compatible with redis protocol)
IF(BUILD_WITH_REDIS OR BUILD_WITH_PIKA)
  FIND_PACKAGE(OpenSSL REQUIRED)
  FIND_PACKAGE(Libssh2 REQUIRED CONFIG)
  FIND_PACKAGE(Hiredis REQUIRED)

  SET(HEADERS_PROXY_DB_REDIS_COMPATIBLE
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/database.h
  )
  SET(SOURCES_PROXY_DB_REDIS_COMPATIBLE
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/database.cpp
  )

IF(PRO_VERSION)
  SET(HEADERS_PROXY_DB_REDIS_COMPATIBLE ${HEADERS_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/cluster.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/sentinel.h
  )
  SET(SOURCES_PROXY_DB_REDIS_COMPATIBLE ${SOURCES_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/cluster.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/sentinel.cpp
  )
ENDIF(PRO_VERSION)

  SET(DB_LIBS ${DB_LIBS} ${HIREDIS_LIBRARIES} Libssh2::libssh2 ${OPENSSL_LIBRARIES})
  IF(OS_WINDOWS)
    SET(DB_LIBS ${DB_LIBS} crypt32)
  ENDIF(OS_WINDOWS)
ENDIF(BUILD_WITH_REDIS OR BUILD_WITH_PIKA)

#redis
IF(BUILD_WITH_REDIS)
  # proxy redis
  SET(HEADERS_PROXY_DB_REDIS ${HEADERS_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/driver.h
  )
  SET(SOURCES_PROXY_DB_REDIS ${SOURCES_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/command.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/driver.cpp
  )

  #gui redis
  SET(HEADERS_REDIS_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/redis/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/redis/lexer.h
  )
  SET(SOURCES_REDIS_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/redis/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/redis/lexer.cpp
  )

IF(PRO_VERSION)
  SET(HEADERS_REDIS_GUI ${HEADERS_REDIS_GUI}
    ${CMAKE_SOURCE_DIR}/src/gui/db/redis/shell_widget.h
  )
  SET(SOURCES_REDIS_GUI ${SOURCES_REDIS_GUI}
    ${CMAKE_SOURCE_DIR}/src/gui/db/redis/shell_widget.cpp
  )
  SET(HEADERS_PROXY_DB_REDIS ${HEADERS_PROXY_DB_REDIS}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/cluster_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/sentinel_settings.h
  )
  SET(SOURCES_PROXY_DB_REDIS ${SOURCES_PROXY_DB_REDIS}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/cluster_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/sentinel_settings.cpp
  )
ENDIF(PRO_VERSION)

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_REDIS})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_REDIS})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_REDIS_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_REDIS_GUI})
ENDIF(BUILD_WITH_REDIS)

#pika
IF(BUILD_WITH_PIKA)
  # proxy pika
  SET(HEADERS_PROXY_DB_PIKA ${HEADERS_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/driver.h
  )
  SET(SOURCES_PROXY_DB_PIKA ${SOURCES_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/command.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/driver.cpp
  )

IF(PRO_VERSION)
  SET(HEADERS_PROXY_DB_PIKA ${HEADERS_PROXY_DB_PIKA}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/cluster_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/sentinel_settings.h
  )
  SET(SOURCES_PROXY_DB_PIKA ${SOURCES_PROXY_DB_PIKA}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/cluster_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/sentinel_settings.cpp
  )
ENDIF(PRO_VERSION)

  #gui pika
  SET(HEADERS_PIKA_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/pika/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/pika/lexer.h
  )
  SET(SOURCES_PIKA_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/pika/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/pika/lexer.cpp
  )

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_PIKA})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_PIKA})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_PIKA_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_PIKA_GUI})
ENDIF(BUILD_WITH_PIKA)

#memcached
IF(BUILD_WITH_MEMCACHED)
  FIND_PACKAGE(Sasl2 QUIET)
  SET(USE_SASL2 ${SASL2_FOUND})
  FIND_LIBRARY(MEMCACHED_LIBRARY memcached)
  FIND_LIBRARY(MEMCACHED_UTIL_LIBRARY memcachedutil)

  #proxy
  SET(HEADERS_PROXY_DB_MEMCACHED
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/driver.h
  )
  SET(SOURCES_PROXY_DB_MEMCACHED
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/command.cpp
  )

  #gui
  SET(HEADERS_MEMCACHED_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/memcached/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/memcached/lexer.h
  )
  SET(SOURCES_MEMCACHED_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/memcached/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/memcached/lexer.cpp
  )

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_MEMCACHED})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_MEMCACHED})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_MEMCACHED_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_MEMCACHED_GUI})

  SET(DB_LIBS ${DB_LIBS} ${MEMCACHED_LIBRARY} ${MEMCACHED_UTIL_LIBRARY})
  IF(USE_SASL2)
    SET(DB_LIBS ${DB_LIBS} ${SASL2_LIBRARIES})
  ENDIF(USE_SASL2)
ENDIF(BUILD_WITH_MEMCACHED)

#ssdb
IF(BUILD_WITH_SSDB)
  #proxy
  SET(HEADERS_PROXY_DB_SSDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/driver.h
  )
  SET(SOURCES_PROXY_DB_SSDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/command.cpp
  )

  #gui
  SET(HEADERS_SSDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/ssdb/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/ssdb/lexer.h
  )
  SET(SOURCES_SSDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/ssdb/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/ssdb/lexer.cpp
  )

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_SSDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_SSDB})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_SSDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_SSDB_GUI})
ENDIF(BUILD_WITH_SSDB)

#leveldb
IF(BUILD_WITH_LEVELDB)
  FIND_PACKAGE(leveldb REQUIRED)

  #proxy
  SET(HEADERS_PROXY_DB_LEVELDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/driver.h
  )
  SET(SOURCES_PROXY_DB_LEVELDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/command.cpp
  )

  #gui
  SET(HEADERS_LEVELDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/leveldb/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/leveldb/lexer.h
  )
  SET(SOURCES_LEVELDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/leveldb/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/leveldb/lexer.cpp
  )

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_LEVELDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_LEVELDB})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_LEVELDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_LEVELDB_GUI})

  SET(DB_LIBS ${DB_LIBS} leveldb::leveldb)
  IF(SNAPPY_FOUND)
    SET(DB_LIBS ${DB_LIBS} ${SNAPPY_LIBRARIES})
  ENDIF(SNAPPY_FOUND)
ENDIF(BUILD_WITH_LEVELDB)

#rocksdb
IF(BUILD_WITH_ROCKSDB)
  FIND_PACKAGE(RocksDB REQUIRED)

  #proxy
  SET(HEADERS_PROXY_DB_ROCKSDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/driver.h
  )
  SET(SOURCES_PROXY_DB_ROCKSDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/command.cpp
  )

  #gui
  SET(HEADERS_ROCKSDB_GUI ${CMAKE_SOURCE_DIR}/src/gui/db/rocksdb/connection_widget.h ${CMAKE_SOURCE_DIR}/src/gui/db/rocksdb/lexer.h)
  SET(SOURCES_ROCKSDB_GUI ${CMAKE_SOURCE_DIR}/src/gui/db/rocksdb/connection_widget.cpp ${CMAKE_SOURCE_DIR}/src/gui/db/rocksdb/lexer.cpp)

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_ROCKSDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_ROCKSDB})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_ROCKSDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_ROCKSDB_GUI})

  SET(DB_LIBS ${DB_LIBS} RocksDB::rocksdb)
  IF(SNAPPY_FOUND)
    SET(DB_LIBS ${DB_LIBS} ${SNAPPY_LIBRARIES})
  ENDIF(SNAPPY_FOUND)
  IF(ZLIB_FOUND)
    SET(DB_LIBS ${DB_LIBS} ${ZLIB_LIBRARIES})
  ENDIF(ZLIB_FOUND)
  IF(LZ4_FOUND)
    SET(DB_LIBS ${DB_LIBS} ${LZ4_LIBRARIES})
  ENDIF(LZ4_FOUND)
  IF(BZIP2_FOUND)
    SET(DB_LIBS ${DB_LIBS} ${BZIP2_LIBRARIES})
  ENDIF(BZIP2_FOUND)
  IF(OS_WINDOWS)
    SET(DB_LIBS ${DB_LIBS} shlwapi Rpcrt4)
  ENDIF(OS_WINDOWS)
ENDIF(BUILD_WITH_ROCKSDB)

#unqlite
IF(BUILD_WITH_UNQLITE)
  FIND_LIBRARY(UNQLITE_LIBRARY unqlite)

  #proxy
  SET(HEADERS_PROXY_DB_UNQLITE
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/driver.h
  )
  SET(SOURCES_PROXY_DB_UNQLITE
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/command.cpp
  )

  #gui
  SET(HEADERS_UNQLITE_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/unqlite/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/unqlite/lexer.h
  )
  SET(SOURCES_UNQLITE_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/unqlite/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/unqlite/lexer.cpp
  )

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_UNQLITE})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_UNQLITE})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_UNQLITE_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_UNQLITE_GUI})

  SET(DB_LIBS ${DB_LIBS} ${UNQLITE_LIBRARY})
ENDIF(BUILD_WITH_UNQLITE)

#lmdb
IF(BUILD_WITH_LMDB)
  FIND_LIBRARY(LMDB_LIBRARY lmdb)

  #proxy
  SET(HEADERS_PROXY_DB_LMDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/driver.h
  )
  SET(SOURCES_PROXY_DB_LMDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/command.cpp
  )

  #gui
  SET(HEADERS_LMDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/lmdb/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/lmdb/lexer.h
  )
  SET(SOURCES_LMDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/lmdb/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/lmdb/lexer.cpp
  )

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_LMDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_LMDB})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_LMDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_LMDB_GUI})

  SET(DB_LIBS ${DB_LIBS} ${LMDB_LIBRARY})
  IF(OS_WINDOWS)
    SET(DB_LIBS ${DB_LIBS} ntdll)
  ENDIF(OS_WINDOWS)
ENDIF(BUILD_WITH_LMDB)

#upscaledb
IF(BUILD_WITH_UPSCALEDB)
  SET(Boost_USE_MULTITHREADED ON)
  SET(Boost_USE_STATIC_LIBS ON)
  FIND_PACKAGE(Boost COMPONENTS thread system REQUIRED)
  FIND_LIBRARY(UPSCALEDB_LIBRARY upscaledb)

  #proxy
  SET(HEADERS_PROXY_DB_UPSCALEDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/driver.h
  )
  SET(SOURCES_PROXY_DB_UPSCALEDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/command.cpp
  )

  #gui
  SET(HEADERS_UPSCALEDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/upscaledb/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/upscaledb/lexer.h
  )
  SET(SOURCES_UPSCALEDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/upscaledb/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/upscaledb/lexer.cpp
  )

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_UPSCALEDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_UPSCALEDB})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_UPSCALEDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_UPSCALEDB_GUI})

  SET(DB_LIBS ${DB_LIBS} ${UPSCALEDB_LIBRARY} ${Boost_LIBRARIES})
  IF(ZLIB_FOUND)
    SET(DB_LIBS ${DB_LIBS} ${ZLIB_LIBRARIES})
  ENDIF(ZLIB_FOUND)
ENDIF(BUILD_WITH_UPSCALEDB)

#forestdb
IF(BUILD_WITH_FORESTDB)
  FIND_LIBRARY(FORESTDB_LIBRARY forestdb)

  #proxy
  SET(HEADERS_PROXY_DB_FORESTDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/driver.h
  )
  SET(SOURCES_PROXY_DB_FORESTDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/command.cpp
  )

  #gui
  SET(HEADERS_FORESTDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/forestdb/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/forestdb/lexer.h
  )
  SET(SOURCES_FORESTDB_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/forestdb/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/forestdb/lexer.cpp
  )

  #update common variables
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_FORESTDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_FORESTDB})

  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_FORESTDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_FORESTDB_GUI})

  SET(DB_LIBS ${DB_LIBS} ${FORESTDB_LIBRARY})
  IF(SNAPPY_FOUND)
    SET(DB_LIBS ${DB_LIBS} ${SNAPPY_LIBRARIES})
  ENDIF(SNAPPY_FOUND)
ENDIF(BUILD_WITH_FORESTDB)

# APPLICATION_DEFINES
SET(APPLICATION_DEFINES)
SET(APPLICATION_HEADERS)
SET(APPLICATION_SOURCES)

IF(PRO_VERSION)
  SET(BUILD_STRATEGY "community" CACHE STRING "Strategy build for ${PROJECT_NAME_TITLE} project")
    # community, public, private
  MESSAGE(STATUS "BUILD_STRATEGY: ${BUILD_STRATEGY}")
  IF (BUILD_STRATEGY STREQUAL "community")
    SET(APPLICATION_DEFINES ${APPLICATION_DEFINES}
      -DBUILD_STRATEGY=0
    )
    SET(APPLICATION_HEADERS ${APPLICATION_HEADERS} ${CMAKE_SOURCE_DIR}/src/app/online_verify_user.h)
    SET(APPLICATION_SOURCES ${APPLICATION_SOURCES} ${CMAKE_SOURCE_DIR}/src/app/online_verify_user.cpp)
  ELSEIF (BUILD_STRATEGY STREQUAL "public")
    IF("${USER_LOGIN}" STREQUAL "")
      MESSAGE(FATAL_ERROR "For individual builds USER_LOGIN vaiable must have value.")
    ENDIF("${USER_LOGIN}" STREQUAL "")
    MESSAGE(STATUS "USER_LOGIN: ${USER_LOGIN}")

    SET(APPLICATION_DEFINES ${APPLICATION_DEFINES}
      -DBUILD_STRATEGY=1
      -DUSER_LOGIN="${USER_LOGIN}"
    )
    SET(APPLICATION_HEADERS ${APPLICATION_HEADERS} ${CMAKE_SOURCE_DIR}/src/app/online_verify_user.h)
    SET(APPLICATION_SOURCES ${APPLICATION_SOURCES} ${CMAKE_SOURCE_DIR}/src/app/online_verify_user.cpp)
  ELSEIF (BUILD_STRATEGY STREQUAL "private")
    IF("${USER_LOGIN}" STREQUAL "")
      MESSAGE(FATAL_ERROR "For individual builds USER_LOGIN vaiable must have value.")
    ENDIF("${USER_LOGIN}" STREQUAL "")
    IF("${USER_PASSWORD}" STREQUAL "")
      MESSAGE(FATAL_ERROR "For individual builds USER_PASSWORD vaiable must have value.")
    ENDIF("${USER_PASSWORD}" STREQUAL "")
    MESSAGE(STATUS "USER_LOGIN: ${USER_LOGIN}")
    MESSAGE(STATUS "USER_PASSWORD: ${USER_PASSWORD}")

    SET(APPLICATION_DEFINES ${APPLICATION_DEFINES}
      -DBUILD_STRATEGY=2
      -DUSER_LOGIN="${USER_LOGIN}"
      -DUSER_PASSWORD="${USER_PASSWORD}"
    )
    SET(APPLICATION_HEADERS ${APPLICATION_HEADERS} ${CMAKE_SOURCE_DIR}/src/offline_verify_user.h)
    SET(APPLICATION_SOURCES ${APPLICATION_SOURCES} ${CMAKE_SOURCE_DIR}/src/offline_verify_user.cpp)
  ELSE()
    MESSAGE(FATAL_ERROR "Not supported strategy: ${BUILD_STRATEGY}")
  ENDIF(BUILD_STRATEGY STREQUAL "community")

  SET(HEADERS_CREDS
    ${CMAKE_SOURCE_DIR}/src/app/credentials_dialog.h
    ${CMAKE_SOURCE_DIR}/src/app/iverify_user.h
  )

  SET(SOURCES_CREDS
    ${CMAKE_SOURCE_DIR}/src/app/credentials_dialog.cpp
    ${CMAKE_SOURCE_DIR}/src/app/iverify_user.cpp
  )
ENDIF(PRO_VERSION)

SET(HEADERS
  ${HEADERS_CREDS}
  ${HEADERS_GUI}
  ${HEADERS_PROXY}
  ${HEADERS_TRANSLATIONS}
)
SET(SOURCES
  ${SOURCES_CREDS}
  ${SOURCES_GUI}
  ${SOURCES_PROXY}
  ${SOURCES_TRANSLATIONS}
)

SET(GUI_QRC ${CMAKE_CURRENT_LIST_DIR}/resources/gui.qrc)
SET(BRANDING_QRC ${CMAKE_CURRENT_LIST_DIR}/resources/branding.qrc)
CONFIGURE_FILE("resources/gui.qrc.in" ${GUI_QRC} @ONLY IMMEDIATE)
CONFIGURE_FILE("resources/${PROJECT_NAME_LOWERCASE}.qrc.in" ${BRANDING_QRC} @ONLY IMMEDIATE)
SET(RESOURCES ${GUI_QRC} ${BRANDING_QRC})
QT5_ADD_RESOURCES(RES_SRC ${RESOURCES})

SOURCE_GROUP("Header Files" FILES ${HEADERS} ${HEADERS_TO_MOC})
SOURCE_GROUP("Source Files" FILES ${SOURCES})
SOURCE_GROUP("Platform Files" FILES ${PLATFORM_HDRS} ${PLATFORM_SRCS})
SOURCE_GROUP("Resources" FILES ${RES_SRC} ${RESOURCE_OS})

SET(TRANSLATIONS
  ${CMAKE_SOURCE_DIR}/src/translations/${PROJECT_NAME_LOWERCASE}/${PROJECT_NAME_LOWERCASE}_ru.ts
  ${CMAKE_SOURCE_DIR}/src/translations/${PROJECT_NAME_LOWERCASE}/${PROJECT_NAME_LOWERCASE}_fr.ts
)

FIND_PACKAGE(Common REQUIRED)
FIND_PACKAGE(JSON-C REQUIRED)
FIND_PACKAGE(FastoNoSQL REQUIRED)

IF(PRO_VERSION)
  SET(FASTONOSQL_CORE_LIBRARY ${FASTONOSQL_CORE_PRO_LIBRARY})
ELSE()
  SET(FASTONOSQL_CORE_LIBRARY ${FASTONOSQL_CORE_LIBRARY})
ENDIF(PRO_VERSION)

MESSAGE("FASTONOSQL_CORE_LIBRARY: ${FASTONOSQL_CORE_LIBRARY}")

# core engine
SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${COMMON_INCLUDE_DIRS} ${JSONC_INCLUDE_DIRS} ${FASTONOSQL_INCLUDE_DIRS})

# all
IF(SNAPPY_FOUND)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${SNAPPY_INCLUDE_DIRS})
  SET(COMPRESS_LIBRARIES ${COMPRESS_LIBRARIES} ${SNAPPY_LIBRARIES})
ENDIF(SNAPPY_FOUND)
IF(ZLIB_FOUND)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
  SET(COMPRESS_LIBRARIES ${COMPRESS_LIBRARIES} ${ZLIB_LIBRARIES})
ENDIF(ZLIB_FOUND)
IF(LZ4_FOUND)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${LZ4_INCLUDE_DIRS})
  SET(COMPRESS_LIBRARIES ${COMPRESS_LIBRARIES} ${LZ4_LIBRARIES})
ENDIF(LZ4_FOUND)
IF(BZIP2_FOUND)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${BZIP2_INCLUDE_DIRS})
  SET(COMPRESS_LIBRARIES ${COMPRESS_LIBRARIES} ${BZIP2_LIBRARIES})
ENDIF(BZIP2_FOUND)

FIND_LIBRARY(QSCINTILLA2_LIBRARY qscintilla2)

SET(ALL_SOURCES
  ${HEADERS} ${SOURCES}
  ${PLATFORM_HDRS} ${PLATFORM_SRCS}
)
SET(ALL_LIBS
  ${FASTONOSQL_CORE_LIBRARY}
  ${COMMON_BASE_LIBRARY} ${COMMON_QT_LIBRARY}
  ${JSONC_LIBRARIES} ${QSCINTILLA2_LIBRARY}
  ${QT_LIBRARIES} ${COMPRESS_LIBRARIES}
  ${DB_LIBS} ${PLATFORM_LIBRARIES}
)

FIND_PACKAGE(Qt5LinguistTools REQUIRED)

IF(GENERATE_TRANSLATIONS)
  QT5_CREATE_TRANSLATION(QM_FILES ${ALL_SOURCES} ${TRANSLATIONS} OPTIONS -no-obsolete)
ELSE(GENERATE_TRANSLATIONS)
  QT5_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS})
ENDIF(GENERATE_TRANSLATIONS)

SET(EXE_SOURCES app/main.cpp ${APPLICATION_HEADERS} ${APPLICATION_SOURCES} ${RES_SRC} ${ICON_FILE} ${RESOURCE_OS} ${QM_FILES})

IF(MINGW OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCXX)
  IF(OS_ANDROID)
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${DESKTOP_TARGET} ${ALL_SOURCES} ${EXE_SOURCES})
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ALL_LIBS})
  ELSE()
    ADD_EXECUTABLE(${PROJECT_NAME} ${DESKTOP_TARGET} ${ALL_SOURCES} ${EXE_SOURCES})
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ALL_LIBS})
  ENDIF(OS_ANDROID)
  TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE ${APPLICATION_DEFINES})
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
  SET(MACOSX_BUNDLE_BUNDLE_NAME ${BUNDLE_BASE_NAME})
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER ${MACOSX_BUNDLE_BUNDLE_NAME})
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

INSTALL(FILES "${CMAKE_SOURCE_DIR}/LICENSE" DESTINATION . COMPONENT LICENSE RENAME LICENSE OPTIONAL)
INSTALL(FILES "${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME_LOWERCASE}/COPYRIGHT" DESTINATION . COMPONENT LICENSE RENAME COPYRIGHT OPTIONAL)
INSTALL(FILES ${PROJECT_CHANGELOG_FILE} DESTINATION . COMPONENT LICENSE RENAME CHANGELOG OPTIONAL)
INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME_LOWERCASE}/resources" DESTINATION
  ${SHARE_INSTALL_DESTINATION} COMPONENT RESOURCES)
INSTALL_QT(${PROJECT_NAME} ${LIB_INSTALL_DESTINATION})

IF(OS_WINDOWS)
  DEPLOY_QT_PLUGIN(Qt5::QWindowsIntegrationPlugin ${LIB_INSTALL_DESTINATION}/platforms)
  DEPLOY_QT_PLUGIN(Qt5::QGifPlugin ${LIB_INSTALL_DESTINATION}/imageformats)

  #find runtime zlib
  SET(SHARED_ZLIB_NAMES zlib1.dll z.dll zlib.dll zdll.dll zlibd.dll zlibd1.dll)
  FIND_RUNTIME_LIBRARY(SHARED_ZLIB_LIBRARY SHARED_ZLIB_NAMES)
  INSTALL(FILES ${SHARED_ZLIB_LIBRARY} DESTINATION ${LIB_INSTALL_DESTINATION} COMPONENT RUNTIME)

  IF(LZ4_FOUND)
    #find runtime lz4
    SET(SHARED_LZ4_NAMES liblz4.dll)
    FIND_RUNTIME_LIBRARY(SHARED_LZ4_LIBRARY SHARED_LZ4_NAMES)
    INSTALL(FILES ${SHARED_LZ4_LIBRARY} DESTINATION ${LIB_INSTALL_DESTINATION} COMPONENT RUNTIME)
  ENDIF(LZ4_FOUND)
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
  DEPLOY_QT_PLUGIN(Qt5::QCocoaIntegrationPlugin ${BUNDLE_NAME}/Contents/plugins/platforms)
  DEPLOY_QT_PLUGIN(Qt5::QGifPlugin ${BUNDLE_NAME}/Contents/plugins/imageformats)
  FIXUP_LIBS_IN_BUNDLE(${BUNDLE_NAME} ${QT_LIBS_DIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME_LOWERCASE}/qt.conf DESTINATION ${BUNDLE_NAME}/Contents/Resources COMPONENT RESOURCES)
ELSEIF(OS_LINUX OR OS_FREEBSD)
  SET(START_SCRIPT_GENERATED_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}")
  CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME_LOWERCASE}/linux/start.sh.in" ${START_SCRIPT_GENERATED_PATH} @ONLY IMMEDIATE)
  INSTALL(PROGRAMS ${START_SCRIPT_GENERATED_PATH} DESTINATION ${TARGET_INSTALL_DESTINATION})
  INSTALL(FILES ${RESOURCE_OS} DESTINATION ${SHARE_INSTALL_DESTINATION}/applications COMPONENT RESOURCES)
  INSTALL(FILES ${ICON_FILE} DESTINATION ${SHARE_INSTALL_DESTINATION}/icons COMPONENT RESOURCES)
  DEPLOY_QT_PLUGIN(Qt5::QXcbIntegrationPlugin ${LIB_INSTALL_DESTINATION}/platforms)
  DEPLOY_QT_PLUGIN(Qt5::QGifPlugin ${LIB_INSTALL_DESTINATION}/imageformats)

  IF(LZ4_FOUND)
    GET_FILENAME_COMPONENT(LZ4_WITHOUT_SYMLINK ${LZ4_LIBRARIES} REALPATH)
    GET_FILENAME_COMPONENT(LZ4_LIB_NAME ${LZ4_WITHOUT_SYMLINK} NAME)
    STRING(REGEX REPLACE "[^so]+$" ".1" LZ4_LNNAME ${LZ4_LIB_NAME})
    INSTALL(FILES ${LZ4_WITHOUT_SYMLINK} DESTINATION ${LIB_INSTALL_DESTINATION} RENAME ${LZ4_LNNAME} COMPONENT RUNTIME NAMELINK_SKIP) #liblz4.so.1
  ENDIF(LZ4_FOUND)
  IF(USE_SASL2)
    GET_FILENAME_COMPONENT(SASL2_WITHOUT_SYMLINK ${SASL2_LIBRARIES} REALPATH)
    GET_FILENAME_COMPONENT(SASL2_LIB_NAME ${SASL2_WITHOUT_SYMLINK} NAME)
    STRING(REGEX REPLACE "[^so]+$" ".2" SASL2_LNNAME ${SASL2_LIB_NAME})
    INSTALL(FILES ${SASL2_WITHOUT_SYMLINK} DESTINATION ${LIB_INSTALL_DESTINATION} RENAME ${SASL2_LNNAME} COMPONENT RUNTIME NAMELINK_SKIP) #libsasl2.so.2
  ENDIF(USE_SASL2)
  #INSTALL(DIRECTORY ${QT_LIBS_DIR}/fonts DESTINATION ${LIB_INSTALL_DESTINATION})
ENDIF(OS_WINDOWS)

IF(OS_MACOSX)
  INSTALL(FILES ${QM_FILES} DESTINATION ${BUNDLE_NAME}/Contents/MacOS/translations COMPONENT TRANSLATIONS)
ELSE(OS_MACOSX)
  INSTALL(FILES ${QM_FILES} DESTINATION ${TARGET_INSTALL_DESTINATION}/translations COMPONENT TRANSLATIONS)
ENDIF(OS_MACOSX)

INSTALL_RUNTIME_LIBRARIES()

IF(DEVELOPER_CHECK_STYLE)
  SET(CHECK_SOURCES ${HEADERS} ${HEADERS} ${HEADERS_TO_MOC} ${SOURCES} ${PLATFORM_HDRS} ${PLATFORM_SRCS})
  REGISTER_CHECK_STYLE_TARGET(check_style "${CHECK_SOURCES}")
  REGISTER_CHECK_INCLUDES_TARGET(${PROJECT_NAME})
ENDIF(DEVELOPER_CHECK_STYLE)

IF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release")
  STRIP_TARGET(${PROJECT_NAME})
  SIGN_TARGET(${EXECUTABLE_NAME})
ENDIF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release")

IF(DEVELOPER_ENABLE_TESTS)
  FIND_PACKAGE(GTest REQUIRED)
ENDIF(DEVELOPER_ENABLE_TESTS)
