CMAKE_MINIMUM_REQUIRED(VERSION 3.6.3) # OPENSSL_USE_STATIC_LIBS
# core
SET(HEADERS_CORE_INTERNAL
  ${CMAKE_SOURCE_DIR}/src/core/internal/connection.h
  ${CMAKE_SOURCE_DIR}/src/core/internal/db_connection.h
  ${CMAKE_SOURCE_DIR}/src/core/internal/cdb_connection.h
  ${CMAKE_SOURCE_DIR}/src/core/internal/cdb_connection_client.h
  ${CMAKE_SOURCE_DIR}/src/core/internal/command_handler.h
  ${CMAKE_SOURCE_DIR}/src/core/internal/commands_api.h
)
SET(SOURCES_CORE_INTERNAL
  ${CMAKE_SOURCE_DIR}/src/core/internal/connection.cpp
  ${CMAKE_SOURCE_DIR}/src/core/internal/db_connection.cpp
  ${CMAKE_SOURCE_DIR}/src/core/internal/cdb_connection.cpp
  ${CMAKE_SOURCE_DIR}/src/core/internal/cdb_connection_client.cpp
  ${CMAKE_SOURCE_DIR}/src/core/internal/command_handler.cpp
  ${CMAKE_SOURCE_DIR}/src/core/internal/commands_api.cpp
)

SET(HEADERS_CORE_DATABASE
  ${CMAKE_SOURCE_DIR}/src/core/database/idatabase_info.h
)
SET(SOURCES_CORE_DATABASE
  ${CMAKE_SOURCE_DIR}/src/core/database/idatabase_info.cpp
)

SET(HEADERS_CORE_SERVER
  ${CMAKE_SOURCE_DIR}/src/core/server/iserver_info.h
)
SET(SOURCES_CORE_SERVER
  ${CMAKE_SOURCE_DIR}/src/core/server/iserver_info.cpp
)

SET(HEADERS_CORE_CONFIG
  ${CMAKE_SOURCE_DIR}/src/core/config/config.h
)
SET(SOURCES_CORE_CONFIG
  ${CMAKE_SOURCE_DIR}/src/core/config/config.cpp
)

SET(HEADERS_CORE
  ${HEADERS_CORE_SERVER}
  ${HEADERS_CORE_DATABASE}
  ${HEADERS_CORE_CONFIG}
  ${HEADERS_CORE_TRANSLATOR}
  ${HEADERS_CORE_INTERNAL}

  ${CMAKE_SOURCE_DIR}/src/core/constant_commands_array.h
  ${CMAKE_SOURCE_DIR}/src/core/connection_commands_traits.h
  ${CMAKE_SOURCE_DIR}/src/core/display_strategy.h
  ${CMAKE_SOURCE_DIR}/src/core/connection_types.h
  ${CMAKE_SOURCE_DIR}/src/core/types.h
  ${CMAKE_SOURCE_DIR}/src/core/db_traits.h
  ${CMAKE_SOURCE_DIR}/src/core/db_key.h
  ${CMAKE_SOURCE_DIR}/src/core/db_ps_channel.h
  ${CMAKE_SOURCE_DIR}/src/core/icommand_translator.h
  ${CMAKE_SOURCE_DIR}/src/core/icommand_translator_base.h
  ${CMAKE_SOURCE_DIR}/src/core/command_info.h
  ${CMAKE_SOURCE_DIR}/src/core/module_info.h
  ${CMAKE_SOURCE_DIR}/src/core/command_holder.h
  ${CMAKE_SOURCE_DIR}/src/core/server_property_info.h
  ${CMAKE_SOURCE_DIR}/src/core/ssh_info.h
  ${CMAKE_SOURCE_DIR}/src/core/logger.h
  ${CMAKE_SOURCE_DIR}/src/core/value.h
  ${CMAKE_SOURCE_DIR}/src/core/global.h
)

SET(SOURCES_CORE
  ${SOURCES_CORE_DATABASE}
  ${SOURCES_CORE_SERVER}
  ${SOURCES_CORE_CONFIG}
  ${SOURCES_CORE_TRANSLATOR}
  ${SOURCES_CORE_INTERNAL}

  ${CMAKE_SOURCE_DIR}/src/core/constant_commands_array.cpp
  ${CMAKE_SOURCE_DIR}/src/core/connection_commands_traits.cpp
  ${CMAKE_SOURCE_DIR}/src/core/display_strategy.cpp
  ${CMAKE_SOURCE_DIR}/src/core/connection_types.cpp
  ${CMAKE_SOURCE_DIR}/src/core/types.cpp
  ${CMAKE_SOURCE_DIR}/src/core/db_traits.cpp
  ${CMAKE_SOURCE_DIR}/src/core/db_key.cpp
  ${CMAKE_SOURCE_DIR}/src/core/db_ps_channel.cpp
  ${CMAKE_SOURCE_DIR}/src/core/icommand_translator.cpp
  ${CMAKE_SOURCE_DIR}/src/core/icommand_translator_base.cpp
  ${CMAKE_SOURCE_DIR}/src/core/command_info.cpp
  ${CMAKE_SOURCE_DIR}/src/core/module_info.cpp
  ${CMAKE_SOURCE_DIR}/src/core/command_holder.cpp
  ${CMAKE_SOURCE_DIR}/src/core/server_property_info.cpp
  ${CMAKE_SOURCE_DIR}/src/core/ssh_info.cpp
  ${CMAKE_SOURCE_DIR}/src/core/logger.cpp
  ${CMAKE_SOURCE_DIR}/src/core/value.cpp
  ${CMAKE_SOURCE_DIR}/src/core/global.cpp
)

# proxy
SET(HEADERS_PROXY_CONNECTION_SETTINGS
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/icluster_connection_settings.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/isentinel_connection_settings.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_local.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_remote.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_ssh.h
)
SET(SOURCES_PROXY_CONNECTION_SETTINGS
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/icluster_connection_settings.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/isentinel_connection_settings.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_local.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_remote.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings/iconnection_settings_ssh.cpp
)

SET(HEADERS_PROXY_DRIVER_TO_MOC
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver.h
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver_local.h
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver_remote.h
)
SET(HEADERS_PROXY_DRIVER
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/root_locker.h
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/first_child_update_root_locker.h
)
SET(SOURCES_PROXY_DRIVER
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver_local.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/idriver_remote.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/root_locker.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/driver/first_child_update_root_locker.cpp
)

SET(HEADERS_PROXY_SERVER_TO_MOC
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_base.h
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver.h
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_local.h
  ${CMAKE_SOURCE_DIR}/src/proxy/server/iserver_remote.h
)
SET(HEADERS_PROXY_SERVER
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

SET(HEADERS_PROXY_DATABASE
  ${CMAKE_SOURCE_DIR}/src/proxy/database/idatabase.h
)
SET(SOURCES_PROXY_DATABASE
  ${CMAKE_SOURCE_DIR}/src/proxy/database/idatabase.cpp
)

SET(HEADERS_PROXY_COMMAND_TO_MOC
  ${CMAKE_SOURCE_DIR}/src/proxy/command/command_logger.h
)
SET(HEADERS_PROXY_COMMAND
)
SET(SOURCES_PROXY_COMMAND
  ${CMAKE_SOURCE_DIR}/src/proxy/command/command_logger.cpp
)

#
SET(HEADERS_PROXY_TO_MOC
  ${HEADERS_PROXY_DRIVER_TO_MOC}
  ${HEADERS_PROXY_SERVER_TO_MOC}
  ${HEADERS_PROXY_COMMAND_TO_MOC}
)

SET(HEADERS_PROXY
  ${HEADERS_PROXY_DRIVER}
  ${HEADERS_PROXY_SERVER}
  ${HEADERS_PROXY_CLUSTER}
  ${HEADERS_PROXY_SENTINEL}
  ${HEADERS_PROXY_EVENTS}
  ${HEADERS_PROXY_DATABASE}
  ${HEADERS_PROXY_CONNECTION_SETTINGS}
  ${HEADERS_PROXY_COMMAND}

  ${CMAKE_SOURCE_DIR}/src/proxy/types.h
  ${CMAKE_SOURCE_DIR}/src/proxy/command/command.h
  ${CMAKE_SOURCE_DIR}/src/proxy/servers_manager.h
  ${CMAKE_SOURCE_DIR}/src/proxy/proxy_fwd.h
  ${CMAKE_SOURCE_DIR}/src/proxy/settings_manager.h
  ${CMAKE_SOURCE_DIR}/src/proxy/cluster_connection_settings_factory.h
  ${CMAKE_SOURCE_DIR}/src/proxy/sentinel_connection_settings_factory.h
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings_factory.h
)

SET(SOURCES_PROXY
  ${SOURCES_PROXY_DRIVER}
  ${SOURCES_PROXY_SERVER}
  ${SOURCES_PROXY_CLUSTER}
  ${SOURCES_PROXY_SENTINEL}
  ${SOURCES_PROXY_EVENTS}
  ${SOURCES_PROXY_DATABASE}
  ${SOURCES_PROXY_CONNECTION_SETTINGS}
  ${SOURCES_PROXY_COMMAND}

  ${CMAKE_SOURCE_DIR}/src/proxy/types.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/command/command.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/servers_manager.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/settings_manager.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/cluster_connection_settings_factory.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/sentinel_connection_settings_factory.cpp
  ${CMAKE_SOURCE_DIR}/src/proxy/connection_settings_factory.cpp
)

# gui, translation, global
SET(HEADERS_TRANSLATIONS
  ${CMAKE_SOURCE_DIR}/src/translations/global.h
)
SET(SOURCES_TRANSLATIONS
  ${CMAKE_SOURCE_DIR}/src/translations/global.cpp
)

SET(HEADERS_GUI_DIALOGS_TO_MOC
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
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/cluster_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/sentinel_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_diagnostic_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_cluster_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_sentinel_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/encode_decode_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/load_contentdb_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/dbkey_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/view_keys_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/pub_sub_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_connection.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_sentinel_connection.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/test_connection.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/stream_entry_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/trial_time_dialog.h
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/how_to_use_dialog.h
)

SET(HEADERS_GUI_DIALOGS
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_listwidget_items.h
)

SET(SOURCES_GUI_DIALOGS
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/eula_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/password_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/about_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/preferences_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connections_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_select_type_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/cluster_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/sentinel_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_diagnostic_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_cluster_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_sentinel_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/connection_listwidget_items.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/info_server_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/property_server_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/history_server_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/encode_decode_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/load_contentdb_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/dbkey_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/view_keys_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/pub_sub_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_connection.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/discovery_sentinel_connection.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/test_connection.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/stream_entry_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/trial_time_dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/dialogs/how_to_use_dialog.cpp
)

SET(HEADERS_GUI_EDITOR_TO_MOC
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor.h
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor_shell.h
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor_output.h
)

SET(SOURCES_GUI_EDITOR
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor_shell.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/editor/fasto_editor_output.cpp
)

SET(HEADERS_GUI_EXPLORER_TO_MOC
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_sort_filter_proxy_model.h
)

SET(HEADERS_GUI_EXPLORER
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_item.h
)

SET(SOURCES_GUI_EXPLORER
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_view.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_sort_filter_proxy_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/explorer/explorer_tree_item.cpp
)

SET(HEADERS_GUI_SHELL_TO_MOC
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_shell_widget.h
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_lexer.h
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_shell.h
)

SET(SOURCES_GUI_SHELL
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_shell_widget.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_lexer.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/shell/base_shell.cpp
)

SET(HEADERS_GUI_WIDGETS_TO_MOC
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

SET(HEADERS_GUI_TO_MOC
  ${HEADERS_GUI_DIALOGS_TO_MOC}
  ${HEADERS_GUI_EDITOR_TO_MOC}
  ${HEADERS_GUI_WIDGETS_TO_MOC}
  ${HEADERS_GUI_SHELL_TO_MOC}
  ${HEADERS_GUI_EXPLORER_TO_MOC}

  ${CMAKE_SOURCE_DIR}/src/gui/main_window.h
  ${CMAKE_SOURCE_DIR}/src/gui/statistic_sender.h
  ${CMAKE_SOURCE_DIR}/src/gui/update_checker.h
  ${CMAKE_SOURCE_DIR}/src/gui/property_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/main_tab_bar.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_text_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/keys_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/channels_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_tree_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_common_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_table_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/keys_table_view.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_scintilla.h
  ${CMAKE_SOURCE_DIR}/src/gui/hash_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/stream_table_model.h
  ${CMAKE_SOURCE_DIR}/src/gui/action_cell_delegate.h
)
SET(HEADERS_GUI
  ${HEADERS_GUI_EXPLORER}
  ${HEADERS_GUI_DIALOGS}

  ${CMAKE_SOURCE_DIR}/src/gui/gui_factory.h
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_common_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/property_table_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/shortcuts.h
  ${CMAKE_SOURCE_DIR}/src/gui/connection_widgets_factory.h
  ${CMAKE_SOURCE_DIR}/src/gui/key_value_table_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/key_table_item.h
  ${CMAKE_SOURCE_DIR}/src/gui/utils.h
)
SET(SOURCES_GUI
  ${SOURCES_GUI_DIALOGS}
  ${SOURCES_GUI_EDITOR}
  ${SOURCES_GUI_WIDGETS}
  ${SOURCES_GUI_SHELL}
  ${SOURCES_GUI_EXPLORER}

  ${CMAKE_SOURCE_DIR}/src/gui/main_window.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/statistic_sender.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/update_checker.cpp
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
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_common_item.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/shortcuts.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/fasto_scintilla.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/connection_widgets_factory.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/hash_table_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/stream_table_model.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/action_cell_delegate.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/key_value_table_item.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/utils.cpp
)

SET_DESKTOP_TARGET()

IF(OS_WINDOWS)
  SET(PLATFORM_HDRS)
  SET(PLATFORM_SRCS)
  SET(PLATFORM_LIBRARIES ws2_32 Rpcrt4 ntdll crypt32)
ELSEIF(OS_MACOSX)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  SET(PLATFORM_LIBRARIES ${COCOA_LIBRARY})
ELSEIF(OS_LINUX)
  SET(PLATFORM_HDRS)
  SET(PLATFORM_SRCS)
  SET(PLATFORM_LIBRARIES pthread dl)
ELSEIF(OS_FREEBSD)
  SET(PLATFORM_HDRS)
  SET(PLATFORM_SRCS)
  SET(PLATFORM_LIBRARIES util pthread)
ENDIF(OS_WINDOWS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR})

#redis or pika (compatible with redis protocol)
IF(BUILD_WITH_REDIS OR BUILD_WITH_PIKA)
  FIND_PACKAGE(OpenSSL REQUIRED)
  FIND_PACKAGE(Libssh2 REQUIRED CONFIG)
  ADD_SUBDIRECTORY(third-party/redis)

  SET(HEADERS_CORE_DB_REDIS_COMPATIBLE
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/cluster_infos.h
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/sentinel_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/database_info.h
  )
  SET(SOURCES_CORE_DB_REDIS_COMPATIBLE
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/cluster_infos.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/sentinel_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/redis_compatible/database_info.cpp
  )

  SET(HEADERS_PIKA_PROXY_DB_REDIS_COMPATIBLE_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/sentinel.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/cluster.h
  )
  SET(HEADERS_PROXY_DB_REDIS_COMPATIBLE
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/database.h
  )
  SET(SOURCES_PROXY_DB_REDIS_COMPATIBLE
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/sentinel.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/cluster.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis_compatible/database.cpp
  )

  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/redis/src third-party/redis/deps)
  SET(DB_LIBS ${DB_LIBS} hiredis Libssh2::libssh2 ${OPENSSL_LIBRARIES})
ENDIF(BUILD_WITH_REDIS OR BUILD_WITH_PIKA)

#redis
IF(BUILD_WITH_REDIS)
  #core redis
  SET(HEADERS_CORE_DB_REDIS ${HEADERS_CORE_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/core/db/redis/internal/modules.h
    ${CMAKE_SOURCE_DIR}/src/core/db/redis/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/redis/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/redis/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/redis/db_connection.h
  )
  SET(SOURCES_CORE_DB_REDIS ${SOURCES_CORE_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/core/db/redis/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/redis/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/redis/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/redis/db_connection.cpp
  )

  # proxy redis
  SET(HEADERS_REDIS_PROXY_TO_MOC ${HEADERS_PIKA_PROXY_DB_REDIS_COMPATIBLE_TO_MOC}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/driver.h
  )
  SET(HEADERS_PROXY_DB_REDIS ${HEADERS_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/sentinel_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/cluster_settings.h
  )
  SET(SOURCES_PROXY_DB_REDIS ${SOURCES_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/command.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/sentinel_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/cluster_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/redis/driver.cpp
  )

  #gui redis
  SET(HEADERS_REDIS_GUI_TO_MOC
    gui/db/redis/connection_widget.h
    gui/db/redis/lexer.h
    gui/db/redis/shell_widget.h
  )
  SET(HEADERS_REDIS_GUI)
  SET(SOURCES_REDIS_GUI
    gui/db/redis/connection_widget.cpp
    gui/db/redis/lexer.cpp
    gui/db/redis/shell_widget.cpp
  )

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_REDIS})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_REDIS})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_REDIS_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_REDIS})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_REDIS})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_REDIS_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_REDIS_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_REDIS_GUI})
ENDIF(BUILD_WITH_REDIS)

#pika
IF(BUILD_WITH_PIKA)
  #core pika
  SET(HEADERS_CORE_DB_PIKA ${HEADERS_CORE_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/core/db/pika/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/pika/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/pika/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/pika/db_connection.h
  )
  SET(SOURCES_CORE_DB_PIKA ${SOURCES_CORE_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/core/db/pika/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/pika/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/pika/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/pika/db_connection.cpp
  )

  # proxy pika
  SET(HEADERS_PIKA_PROXY_TO_MOC ${HEADERS_PIKA_PROXY_DB_REDIS_COMPATIBLE_TO_MOC}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/driver.h
  )
  SET(HEADERS_PROXY_DB_PIKA ${HEADERS_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/command.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/sentinel_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/cluster_settings.h
  )
  SET(SOURCES_PROXY_DB_PIKA ${SOURCES_PROXY_DB_REDIS_COMPATIBLE}
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/command.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/sentinel_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/cluster_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/pika/driver.cpp
  )

  #gui pika
  SET(HEADERS_PIKA_GUI_TO_MOC
    gui/db/pika/connection_widget.h
    gui/db/pika/lexer.h
  )
  SET(HEADERS_PIKA_GUI)
  SET(SOURCES_PIKA_GUI
    gui/db/pika/connection_widget.cpp
    gui/db/pika/lexer.cpp
  )

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_PIKA})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_PIKA})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_PIKA_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_PIKA})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_PIKA})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_PIKA_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_PIKA_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_PIKA_GUI})
ENDIF(BUILD_WITH_PIKA)

#memcached
IF(BUILD_WITH_MEMCACHED)
  FIND_PACKAGE(Sasl2 QUIET)
  SET(USE_SASL2 ${SASL2_FOUND})
  IF(SASL2_FOUND)
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${SASL2_INCLUDE_DIR})
  ENDIF(SASL2_FOUND)
  FIND_LIBRARY(MEMCACHED_LIBRARY memcached)
  FIND_LIBRARY(MEMCACHED_UTIL_LIBRARY memcachedutil)
  FIND_PATH(MEMCACHED_INCLUDE_DIR NAMES libmemcached/memcached.h PATH_SUFFIXES include)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${MEMCACHED_INCLUDE_DIR} third-party/memcached/src)

  #core
  SET(HEADERS_CORE_DB_MEMCACHED
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/database_info.h
  )
  SET(SOURCES_CORE_DB_MEMCACHED
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/memcached/database_info.cpp
  )

  #proxy
  SET(HEADERS_MEMCACHED_PROXY_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/driver.h
  )
  SET(HEADERS_PROXY_DB_MEMCACHED
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/command.h
  )
  SET(SOURCES_PROXY_DB_MEMCACHED
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/memcached/command.cpp
  )

  #gui
  SET(HEADERS_MEMCACHED_GUI_TO_MOC gui/db/memcached/connection_widget.h gui/db/memcached/lexer.h)
  SET(HEADERS_MEMCACHED_GUI)
  SET(SOURCES_MEMCACHED_GUI gui/db/memcached/connection_widget.cpp gui/db/memcached/lexer.cpp)

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_MEMCACHED})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_MEMCACHED})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_MEMCACHED_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_MEMCACHED})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_MEMCACHED})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_MEMCACHED_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_MEMCACHED_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_MEMCACHED_GUI})
  SET(DB_LIBS ${DB_LIBS} ${MEMCACHED_LIBRARY} ${MEMCACHED_UTIL_LIBRARY})
  IF(USE_SASL2)
    SET(DB_LIBS ${DB_LIBS} ${SASL2_LIBRARIES})
  ENDIF(USE_SASL2)
ENDIF(BUILD_WITH_MEMCACHED)

#ssdb
IF(BUILD_WITH_SSDB)
  ADD_SUBDIRECTORY(third-party/ssdb)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/ssdb/src)

  #core
  SET(HEADERS_CORE_DB_SSDB
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/database_info.h
  )
  SET(SOURCES_CORE_DB_SSDB
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/ssdb/database_info.cpp
  )

  #proxy
  SET(HEADERS_SSDB_PROXY_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/driver.h
  )
  SET(HEADERS_PROXY_DB_SSDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/command.h
  )
  SET(SOURCES_PROXY_DB_SSDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/ssdb/command.cpp
  )

  #gui
  SET(HEADERS_SSDB_GUI_TO_MOC gui/db/ssdb/connection_widget.h gui/db/ssdb/lexer.h)
  SET(HEADERS_SSDB_GUI)
  SET(SOURCES_SSDB_GUI gui/db/ssdb/connection_widget.cpp gui/db/ssdb/lexer.cpp)

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_SSDB})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_SSDB})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_SSDB_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_SSDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_SSDB})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_SSDB_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_SSDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_SSDB_GUI})
  SET(DB_LIBS ${DB_LIBS} ssdb)
ENDIF(BUILD_WITH_SSDB)

#leveldb
IF(BUILD_WITH_LEVELDB)
  FIND_LIBRARY(LEVELDB_LIBRARY leveldb)
  FIND_PATH(LEVELDB_INCLUDE_DIR NAMES leveldb/db.h PATH_SUFFIXES include)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${LEVELDB_INCLUDE_DIR})

  #core
  SET(HEADERS_CORE_DB_LEVELDB
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/database_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/comparators/indexed_db.h
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/comparators/indexed_db/indexed_db_leveldb_coding.h
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/comparators/indexed_db/indexed_db_key.h
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/comparators/indexed_db/WebIDBTypes.h
  )
  SET(SOURCES_CORE_DB_LEVELDB
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/database_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/comparators/indexed_db.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/comparators/indexed_db/indexed_db_leveldb_coding.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/leveldb/comparators/indexed_db/indexed_db_key.cpp
  )

  #proxy
  SET(HEADERS_LEVELDB_PROXY_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/driver.h
  )
  SET(HEADERS_PROXY_DB_LEVELDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/command.h
  )
  SET(SOURCES_PROXY_DB_LEVELDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/leveldb/command.cpp
  )

  #gui
  SET(HEADERS_LEVELDB_GUI_TO_MOC gui/db/leveldb/connection_widget.h gui/db/leveldb/lexer.h)
  SET(HEADERS_LEVELDB_GUI)
  SET(SOURCES_LEVELDB_GUI gui/db/leveldb/connection_widget.cpp gui/db/leveldb/lexer.cpp)

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_LEVELDB})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_LEVELDB})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_LEVELDB_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_LEVELDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_LEVELDB})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_LEVELDB_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_LEVELDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_LEVELDB_GUI})
  SET(DB_LIBS ${DB_LIBS} ${LEVELDB_LIBRARY})
  IF(SNAPPY_FOUND)
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${SNAPPY_INCLUDE_DIRS})
    SET(DB_LIBS ${DB_LIBS} ${SNAPPY_LIBRARIES})
  ENDIF(SNAPPY_FOUND)
ENDIF(BUILD_WITH_LEVELDB)

#rocksdb
IF(BUILD_WITH_ROCKSDB)
  FIND_LIBRARY(ROCKSDB_LIBRARY rocksdb)
  FIND_PATH(ROCKSDB_INCLUDE_DIR NAMES rocksdb/db.h PATH_SUFFIXES include)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${ROCKSDB_INCLUDE_DIR})

  #core
  SET(HEADERS_CORE_DB_ROCKSDB
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/database_info.h
  )
  SET(SOURCES_CORE_DB_ROCKSDB
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/rocksdb/database_info.cpp
  )

  #proxy
  SET(HEADERS_ROCKSDB_PROXY_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/driver.h
  )
  SET(HEADERS_PROXY_DB_ROCKSDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/command.h
  )
  SET(SOURCES_PROXY_DB_ROCKSDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/rocksdb/command.cpp
  )

  #gui
  SET(HEADERS_ROCKSDB_GUI_TO_MOC gui/db/rocksdb/connection_widget.h gui/db/rocksdb/lexer.h)
  SET(HEADERS_ROCKSDB_GUI)
  SET(SOURCES_ROCKSDB_GUI gui/db/rocksdb/connection_widget.cpp gui/db/rocksdb/lexer.cpp)

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_ROCKSDB})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_ROCKSDB})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_ROCKSDB_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_ROCKSDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_ROCKSDB})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_ROCKSDB_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_ROCKSDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_ROCKSDB_GUI})
  SET(DB_LIBS ${DB_LIBS} ${ROCKSDB_LIBRARY})
  IF(SNAPPY_FOUND)
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${SNAPPY_INCLUDE_DIRS})
    SET(DB_LIBS ${DB_LIBS} ${SNAPPY_LIBRARIES})
  ENDIF(SNAPPY_FOUND)
  IF(ZLIB_FOUND)
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
    SET(DB_LIBS ${DB_LIBS} ${ZLIB_LIBRARIES})
  ENDIF(ZLIB_FOUND)
  IF(LZ4_FOUND)
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${LZ4_INCLUDE_DIRS})
    SET(DB_LIBS ${DB_LIBS} ${LZ4_LIBRARIES})
  ENDIF(LZ4_FOUND)
  IF(BZIP2_FOUND)
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${BZIP2_INCLUDE_DIRS})
    SET(DB_LIBS ${DB_LIBS} ${BZIP2_LIBRARIES})
  ENDIF(BZIP2_FOUND)
ENDIF(BUILD_WITH_ROCKSDB)

#unqlite
IF(BUILD_WITH_UNQLITE)
  FIND_LIBRARY(UNQLITE_LIBRARY unqlite)
  FIND_PATH(UNQLITE_INCLUDE_DIR NAMES unqlite.h PATH_SUFFIXES include)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${UNQLITE_INCLUDE_DIR})

  #core
  SET(HEADERS_CORE_DB_UNQLITE
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/database_info.h
  )
  SET(SOURCES_CORE_DB_UNQLITE
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/unqlite/database_info.cpp
  )

  #proxy
  SET(HEADERS_UNQLITE_PROXY_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/driver.h
  )
  SET(HEADERS_PROXY_DB_UNQLITE
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/command.h
  )
  SET(SOURCES_PROXY_DB_UNQLITE
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/unqlite/command.cpp
  )

  #gui
  SET(HEADERS_UNQLITE_GUI_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/gui/db/unqlite/connection_widget.h
    ${CMAKE_SOURCE_DIR}/src/gui/db/unqlite/lexer.h
  )
  SET(HEADERS_UNQLITE_GUI)
  SET(SOURCES_UNQLITE_GUI
    ${CMAKE_SOURCE_DIR}/src/gui/db/unqlite/connection_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/gui/db/unqlite/lexer.cpp
  )

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_UNQLITE})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_UNQLITE})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_UNQLITE_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_UNQLITE})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_UNQLITE})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_UNQLITE_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_UNQLITE_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_UNQLITE_GUI})
  SET(DB_LIBS ${DB_LIBS} ${UNQLITE_LIBRARY})
ENDIF(BUILD_WITH_UNQLITE)

#lmdb
IF(BUILD_WITH_LMDB)
  FIND_LIBRARY(LMDB_LIBRARY lmdb)
  FIND_PATH(LMDB_INCLUDE_DIR NAMES lmdb.h PATH_SUFFIXES include)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${LMDB_INCLUDE_DIR})

  #core
  SET(HEADERS_CORE_DB_LMDB
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/database_info.h)
  SET(SOURCES_CORE_DB_LMDB
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/lmdb/database_info.cpp
  )

  #proxy
  SET(HEADERS_LMDB_PROXY_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/driver.h
  )
  SET(HEADERS_PROXY_DB_LMDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/command.h
  )
  SET(SOURCES_PROXY_DB_LMDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/lmdb/command.cpp
  )

  #gui
  SET(HEADERS_LMDB_GUI_TO_MOC gui/db/lmdb/connection_widget.h gui/db/lmdb/lexer.h)
  SET(HEADERS_LMDB_GUI)
  SET(SOURCES_LMDB_GUI gui/db/lmdb/connection_widget.cpp gui/db/lmdb/lexer.cpp)

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_LMDB})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_LMDB})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_LMDB_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_LMDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_LMDB})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_LMDB_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_LMDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_LMDB_GUI})
  SET(DB_LIBS ${DB_LIBS} ${LMDB_LIBRARY})
ENDIF(BUILD_WITH_LMDB)

#upscaledb
IF(BUILD_WITH_UPSCALEDB)
  SET(Boost_USE_MULTITHREADED ON)
  SET(Boost_USE_STATIC_LIBS ON)
  FIND_PACKAGE(Boost COMPONENTS thread system REQUIRED)
  FIND_LIBRARY(UPSCALEDB_LIBRARY upscaledb)
  FIND_PATH(UPSCALEDB_INCLUDE_DIR NAMES ups/upscaledb.h PATH_SUFFIXES include)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${UPSCALEDB_INCLUDE_DIR})

  #core
  SET(HEADERS_CORE_DB_UPSCALEDB
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/database_info.h
  )
  SET(SOURCES_CORE_DB_UPSCALEDB
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/upscaledb/database_info.cpp
  )

  #proxy
  SET(HEADERS_UPSCALEDB_PROXY_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/driver.h
  )
  SET(HEADERS_PROXY_DB_UPSCALEDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/command.h
  )
  SET(SOURCES_PROXY_DB_UPSCALEDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/upscaledb/command.cpp
  )

  #gui
  SET(HEADERS_UPSCALEDB_GUI_TO_MOC gui/db/upscaledb/connection_widget.h gui/db/upscaledb/lexer.h)
  SET(HEADERS_UPSCALEDB_GUI)
  SET(SOURCES_UPSCALEDB_GUI gui/db/upscaledb/connection_widget.cpp gui/db/upscaledb/lexer.cpp)

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_UPSCALEDB})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_UPSCALEDB})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_UPSCALEDB_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_UPSCALEDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_UPSCALEDB})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_UPSCALEDB_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_UPSCALEDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_UPSCALEDB_GUI})
  SET(DB_LIBS ${DB_LIBS} ${UPSCALEDB_LIBRARY} ${Boost_LIBRARIES})
  IF(ZLIB_FOUND)
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
    SET(DB_LIBS ${DB_LIBS} ${ZLIB_LIBRARIES})
  ENDIF(ZLIB_FOUND)
ENDIF(BUILD_WITH_UPSCALEDB)

#forestdb
IF(BUILD_WITH_FORESTDB)
  FIND_LIBRARY(FORESTDB_LIBRARY forestdb)
  FIND_PATH(FORESTDB_INCLUDE_DIR NAMES libforestdb/forestdb.h PATH_SUFFIXES include)
  SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${FORESTDB_INCLUDE_DIR})

  #core
  SET(HEADERS_CORE_DB_FORESTDB
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/internal/commands_api.h

    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/config.h
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/command_translator.h
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/server_info.h
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/db_connection.h
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/database_info.h)
  SET(SOURCES_CORE_DB_FORESTDB
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/internal/commands_api.cpp

    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/command_translator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/db_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/core/db/forestdb/database_info.cpp
  )

  #proxy
  SET(HEADERS_FORESTDB_PROXY_TO_MOC
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/server.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/driver.h
  )
  SET(HEADERS_PROXY_DB_FORESTDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/connection_settings.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/database.h
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/command.h
  )
  SET(SOURCES_PROXY_DB_FORESTDB
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/connection_settings.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/database.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/server.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/proxy/db/forestdb/command.cpp
  )

  #gui
  SET(HEADERS_FORESTDB_GUI_TO_MOC gui/db/forestdb/connection_widget.h gui/db/forestdb/lexer.h)
  SET(HEADERS_FORESTDB_GUI)
  SET(SOURCES_FORESTDB_GUI gui/db/forestdb/connection_widget.cpp gui/db/forestdb/lexer.cpp)

  #update common variables
  SET(HEADERS_CORE ${HEADERS_CORE} ${HEADERS_CORE_DB_FORESTDB})
  SET(SOURCES_CORE ${SOURCES_CORE} ${SOURCES_CORE_DB_FORESTDB})

  SET(HEADERS_PROXY_TO_MOC ${HEADERS_PROXY_TO_MOC} ${HEADERS_FORESTDB_PROXY_TO_MOC})
  SET(HEADERS_PROXY ${HEADERS_PROXY} ${HEADERS_PROXY_DB_FORESTDB})
  SET(SOURCES_PROXY ${SOURCES_PROXY} ${SOURCES_PROXY_DB_FORESTDB})

  SET(HEADERS_GUI_TO_MOC ${HEADERS_GUI_TO_MOC} ${HEADERS_FORESTDB_GUI_TO_MOC})
  SET(HEADERS_GUI ${HEADERS_GUI} ${HEADERS_FORESTDB_GUI})
  SET(SOURCES_GUI ${SOURCES_GUI} ${SOURCES_FORESTDB_GUI})
  SET(DB_LIBS ${DB_LIBS} ${FORESTDB_LIBRARY})
  IF(SNAPPY_FOUND)
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${SNAPPY_INCLUDE_DIRS})
    SET(DB_LIBS ${DB_LIBS} ${SNAPPY_LIBRARIES})
  ENDIF(SNAPPY_FOUND)
ENDIF(BUILD_WITH_FORESTDB)

SET(SOURCES_SDS
  third-party/sds/sds.c
)

IF (IS_PUBLIC_BUILD)
  ADD_DEFINITIONS(-DIS_PUBLIC_BUILD)
ELSE()
  IF("${USER_SPECIFIC_LOGIN}" STREQUAL "")
    MESSAGE(FATAL_ERROR "For individual builds USER_SPECIFIC_LOGIN vaiable must have value.")
  ENDIF()
  MESSAGE(STATUS "USER_SPECIFIC_LOGIN: ${USER_SPECIFIC_LOGIN}")

  ADD_DEFINITIONS(-DUSER_SPECIFIC_LOGIN="${USER_SPECIFIC_LOGIN}")
ENDIF(IS_PUBLIC_BUILD)

SET(HEADERS_TOMOC
  ${HEADERS_GUI_TO_MOC}
  ${HEADERS_PROXY_TO_MOC}
)
SET(HEADERS
  ${HEADERS_GUI}
  ${HEADERS_PROXY}
  ${HEADERS_TRANSLATIONS}
  ${CMAKE_SOURCE_DIR}/src/server/server_config.h
)
SET(SOURCES
  ${SOURCES_GUI}
  ${SOURCES_PROXY}
  ${SOURCES_TRANSLATIONS}
  ${CMAKE_SOURCE_DIR}/src/server/server_config.cpp
)

QT5_WRAP_CPP(MOC_FILES ${HEADERS_TOMOC})
SET(GUI_QRC ${CMAKE_CURRENT_LIST_DIR}/resources/gui.qrc)
SET(BRANDING_QRC ${CMAKE_CURRENT_LIST_DIR}/resources/branding.qrc)
CONFIGURE_FILE("resources/gui.qrc.in" ${GUI_QRC} @ONLY IMMEDIATE)
CONFIGURE_FILE("resources/${PROJECT_NAME_LOWERCASE}.qrc.in" ${BRANDING_QRC} @ONLY IMMEDIATE)
SET(RESOURCES ${GUI_QRC} ${BRANDING_QRC})
QT5_ADD_RESOURCES(RES_SRC ${RESOURCES})

SOURCE_GROUP("Header Files" FILES ${HEADERS} ${HEADERS_TOMOC})
SOURCE_GROUP("Source Files" FILES ${SOURCES})
SOURCE_GROUP("Generated" FILES ${MOC_FILES})
SOURCE_GROUP("Platform Files" FILES ${PLATFORM_HDRS} ${PLATFORM_SRCS})
SOURCE_GROUP("Resources" FILES ${RES_SRC} ${RESOURCE_OS})

SET(TRANSLATIONS
  ${CMAKE_SOURCE_DIR}/src/translations/${PROJECT_NAME_LOWERCASE}/${PROJECT_NAME_LOWERCASE}_ru.ts
  ${CMAKE_SOURCE_DIR}/src/translations/${PROJECT_NAME_LOWERCASE}/${PROJECT_NAME_LOWERCASE}_fr.ts
)

FIND_PACKAGE(Common REQUIRED)
FIND_PACKAGE(JSON-C REQUIRED)

# modules
SET(PROJECT_CORE_LIBRARY ${PROJECT_NAME_LOWERCASE}_core)
SET(PROJECT_CORE_ENGINE_LIBRARY ${PROJECT_NAME_LOWERCASE}_core_engine)

# core engine
SET(INCLUDE_DIRS ${INCLUDE_DIRS} third-party/sds ${COMMON_INCLUDE_DIR} ${JSONC_INCLUDE_DIRS})
ADD_LIBRARY(${PROJECT_CORE_ENGINE_LIBRARY} STATIC ${HEADERS_CORE} ${SOURCES_CORE} ${SOURCES_SDS})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_CORE_ENGINE_LIBRARY} PRIVATE ${INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_CORE_ENGINE_LIBRARY} ${DB_LIBS} ${JSONC_LIBRARIES})

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

SET(ALL_SOURCES ${ALL_SOURCES} ${HEADERS} ${HEADERS_TOMOC} ${SOURCES} ${MOC_FILES} ${PLATFORM_HDRS} ${PLATFORM_SRCS})
SET(ALL_LIBS ${ALL_LIBS} ${PROJECT_CORE_ENGINE_LIBRARY} ${COMMON_LIBRARIES} ${JSONC_LIBRARIES} qscintilla2
  ${QT_LIBRARIES} ${COMPRESS_LIBRARIES} ${PLATFORM_LIBRARIES})

FIND_PACKAGE(Qt5LinguistTools REQUIRED)

IF(GENERATE_TRANSLATIONS)
  QT5_CREATE_TRANSLATION(QM_FILES ${ALL_SOURCES} ${TRANSLATIONS} OPTIONS -no-obsolete)
ELSE(GENERATE_TRANSLATIONS)
  QT5_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS})
ENDIF(GENERATE_TRANSLATIONS)

SET(EXE_SOURCES main.cpp ${RES_SRC} ${ICON_FILE} ${RESOURCE_OS} ${QM_FILES})

IF(MINGW OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCXX)
  ADD_LIBRARY(${PROJECT_CORE_LIBRARY} STATIC ${ALL_SOURCES})
  TARGET_LINK_LIBRARIES(${PROJECT_CORE_LIBRARY} ${OPENSSL_LIBRARIES} ${ALL_LIBS})
  TARGET_INCLUDE_DIRECTORIES(${PROJECT_CORE_LIBRARY} PRIVATE ${INCLUDE_DIRS})
  IF(OS_ANDROID)
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${DESKTOP_TARGET} ${EXE_SOURCES})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PROJECT_CORE_LIBRARY})
  ELSE()
    ADD_EXECUTABLE(${PROJECT_NAME} ${DESKTOP_TARGET} ${EXE_SOURCES})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PROJECT_CORE_LIBRARY})
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

INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION . COMPONENT LICENSE RENAME LICENSE OPTIONAL)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/COPYRIGHT DESTINATION . COMPONENT LICENSE RENAME COPYRIGHT OPTIONAL)
INSTALL(FILES ${PROJECT_CHANGELOG_FILE} DESTINATION . COMPONENT LICENSE RENAME CHANGELOG OPTIONAL)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME_LOWERCASE}/resources DESTINATION
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

IF (DEVELOPER_CHECK_STYLE)
  SET(CHECK_SOURCES ${HEADERS} ${HEADERS} ${HEADERS_TOMOC} ${SOURCES} ${PLATFORM_HDRS} ${PLATFORM_SRCS} ${HEADERS_CORE} ${SOURCES_CORE})
  REGISTER_CHECK_STYLE_TARGET(check_style "${CHECK_SOURCES}")
  REGISTER_CHECK_INCLUDES_TARGET(${PROJECT_CORE_LIBRARY})
  REGISTER_CHECK_INCLUDES_TARGET(${PROJECT_CORE_ENGINE_LIBRARY})
ENDIF(DEVELOPER_CHECK_STYLE)

IF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release")
  STRIP_TARGET(${PROJECT_NAME})
ENDIF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release")

IF(DEVELOPER_ENABLE_TESTS)
  FIND_PACKAGE(GTest REQUIRED)
  ADD_DEFINITIONS(-DPROJECT_TEST_SOURCES_DIR="${CMAKE_SOURCE_DIR}/tests")

  ADD_EXECUTABLE(unit_tests
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_fasto_objects.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_parsinng_command_line.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_tests/test_command_holder.cpp
  )

  TARGET_LINK_LIBRARIES(unit_tests gtest gtest_main ${PROJECT_CORE_ENGINE_LIBRARY} ${COMMON_LIBRARIES} ${JSONC_LIBRARIES} ${PLATFORM_LIBRARIES})
  ADD_TEST_TARGET(unit_tests)
  SET_PROPERTY(TARGET unit_tests PROPERTY FOLDER "Unit tests")

  #Mock tests
  ADD_EXECUTABLE(mock_tests
    ${CMAKE_SOURCE_DIR}/tests/mock_tests/test_connections.cpp
  )
  TARGET_LINK_LIBRARIES(mock_tests gmock gmock_main ${PROJECT_CORE_ENGINE_LIBRARY} ${COMMON_LIBRARIES} ${PLATFORM_LIBRARIES})
  ADD_TEST_TARGET(mock_tests)
  SET_PROPERTY(TARGET mock_tests PROPERTY FOLDER "Mock tests")
ENDIF(DEVELOPER_ENABLE_TESTS)
