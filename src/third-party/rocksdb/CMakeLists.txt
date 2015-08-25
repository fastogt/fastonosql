CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(rocksdb)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/src/include)
IF(OS_POSIX)
    ADD_DEFINITIONS(-DROCKSDB_PLATFORM_POSIX)
ELSEIF(OS_WINDOWS)
ELSE()
ENDIF(OS_POSIX)

SET(HEADERS_ROCKSDB
)

SET(SOURCES_ROCKSDB
    src/db/c.cc
    src/db/db_impl.cc
    src/db/db_impl_readonly.cc
    src/db/write_batch_base.cc
    src/db/write_batch.cc
    src/db/repair.cc
    src/db/column_family.cc
    src/db/merge_operator.cc
    src/db/version_set.cc
    src/db/flush_scheduler.cc
    src/db/db_iter.cc
    src/db/filename.cc
    src/db/log_writer.cc
    src/db/log_reader.cc
    src/db/dbformat.cc
    src/db/internal_stats.cc
    src/db/compaction.cc
    src/db/version_edit.cc
    src/db/internal_stats.cc
    src/db/table_cache.cc
    src/db/wal_manager.cc
    src/db/memtable_list.cc
    src/db/event_helpers.cc
    src/db/memtable.cc
    src/db/builder.cc
    src/db/flush_job.cc
    src/db/compaction_picker.cc
    src/db/compaction_job.cc
    src/db/write_thread.cc
    src/db/memtable_list.cc
    src/db/managed_iterator.cc
    src/db/forward_iterator.cc
    src/db/write_controller.cc
    src/db/db_filesnapshot.cc
    src/db/compacted_db_impl.cc
    src/db/compaction_picker.cc

    src/table/block_based_table_factory.cc
    src/table/cuckoo_table_factory.cc
    src/table/plain_table_factory.cc
    src/table/merger.cc
    src/table/iterator.cc

    src/util/options.cc
    src/util/status.cc
    src/util/coding.cc
    src/util/perf_level.cc
    src/util/perf_context.cc
    src/util/statistics.cc
    src/util/hash_skiplist_rep.cc
    src/util/hash_linklist_rep.cc
    src/util/bloom.cc
    src/util/cache.cc
    src/util/slice.cc
    src/util/options_helper.cc
    src/util/filter_policy.cc
    src/util/vectorrep.cc
    src/util/crc32c.cc
    src/util/thread_status_updater.cc
    src/util/auto_roll_logger.cc
    src/util/env.cc
    src/util/instrumented_mutex.cc
    src/util/file_reader_writer.cc
    src/util/db_info_dumper.cc
    src/util/file_util.cc
    src/util/sync_point.cc
    src/util/event_logger.cc
    src/util/arena.cc
    src/util/thread_status_util_debug.cc
    src/util/mutable_cf_options.cc
    src/util/comparator.cc
    src/util/log_buffer.cc
    src/util/thread_status_util.cc
    src/util/compaction_job_stats_impl.cc
    src/util/build_version.h
    src/util/iostats_context.cc
    src/util/logging.cc

    src/utilities/backupable/backupable_db.cc
    src/utilities/merge_operators/uint64add.cc
)

EXECUTE_PROCESS(COMMAND $ENV{COMSPEC} " /C date /T" OUTPUT_VARIABLE DATE)
EXECUTE_PROCESS(COMMAND $ENV{COMSPEC} " /C time /T" OUTPUT_VARIABLE TIME)
STRING(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" DATE ${DATE})
STRING(REGEX REPLACE "(..):(.....).*" " \\1:\\2" TIME ${TIME})
STRING(CONCAT GIT_DATE_TIME ${DATE} ${TIME})

EXECUTE_PROCESS(COMMAND $ENV{COMSPEC} " /C git rev-parse HEAD 2>nil" OUTPUT_VARIABLE GIT_SHA)
IF(NOT GIT_SHA)
    SET(GIT_SHA SHA)
ENDIF()
STRING(REGEX REPLACE "[^0-9a-f]+" "" GIT_SHA ${GIT_SHA})

SET(BUILD_VERSION_CC ${CMAKE_CURRENT_SOURCE_DIR}/src/util/build_version.cc)

ADD_CUSTOM_COMMAND(OUTPUT ${BUILD_VERSION_CC}
    COMMAND echo "#include \"build_version.h\"" > ${BUILD_VERSION_CC}
    COMMAND echo "const char* rocksdb_build_git_sha = \"rocksdb_build_git_sha:${GIT_SHA}\";" >> ${BUILD_VERSION_CC}
    COMMAND echo "const char* rocksdb_build_git_datetime = \"rocksdb_build_git_datetime:${GIT_DATE_TIME}\";" >> ${BUILD_VERSION_CC}
    COMMAND echo const char* rocksdb_build_compile_date = __DATE__\; >> ${BUILD_VERSION_CC}
)

ADD_CUSTOM_TARGET(GenerateBuildVersion DEPENDS ${BUILD_VERSION_CC})
IF(OS_POSIX)
    SET(SOURCES_ROCKSDB ${SOURCES_ROCKSDB} src/port/port_posix.cc)
ELSEIF(OS_WINDOWS)
    SET(SOURCES_ROCKSDB ${SOURCES_ROCKSDB} src/port/win/port_win.cc src/port/win/env_win.cc src/port/win/win_logger.cc)
    SET(PLATFORM_LIBRARIES Rpcrt4)
ENDIF(OS_POSIX)

ADD_LIBRARY(rocksdb STATIC ${HEADERS_ROCKSDB} ${SOURCES_ROCKSDB} ${BUILD_VERSION_CC})
TARGET_LINK_LIBRARIES(rocksdb ${PLATFORM_LIBRARIES})
ADD_DEPENDENCIES(rocksdb GenerateBuildVersion)
