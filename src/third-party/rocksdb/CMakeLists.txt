CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(rocksdb)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/src/include)
IF(OS_POSIX)
    ADD_DEFINITIONS(-DROCKSDB_PLATFORM_POSIX)
ELSEIF(OS_WINDOWS)
ELSE()
ENDIF(OS_POSIX)

IF(CMAKE_CROSSCOMPILING)
    ADD_DEFINITIONS(-DIOS_CROSS_COMPILE -DROCKSDB_LITE)
ENDIF(CMAKE_CROSSCOMPILING)

SET(HEADERS_ROCKSDB
)

SET(SOURCES_ROCKSDB
    src/db/db_impl.cc
    src/db/write_batch_base.cc
    src/db/write_batch.cc
    src/db/column_family.cc
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
    src/db/managed_iterator.cc
    src/db/forward_iterator.cc
    src/db/write_controller.cc
    src/db/slice.cc
    src/db/db_filesnapshot.cc
    src/db/table_properties_collector.cc
    src/db/file_indexer.cc
    src/db/version_builder.cc
    src/db/transaction_log_impl.cc
    src/db/memtable_allocator.cc
    src/db/merge_helper.cc

    src/table/block_based_table_factory.cc
    src/table/block_based_table_reader.cc
    src/table/block_prefix_index.cc
    src/table/block_based_table_builder.cc
    src/table/block_based_filter_block.cc
    src/table/plain_table_factory.cc
    src/table/plain_table_reader.cc
    src/table/plain_table_builder.cc
    src/table/plain_table_key_coding.cc
    src/table/plain_table_index.cc
    src/table/merger.cc
    src/table/iterator.cc
    src/table/format.cc
    src/table/table_properties.cc
    src/table/meta_blocks.cc
    src/table/block.cc
    src/table/block_hash_index.cc
    src/table/block_builder.cc
    src/table/two_level_iterator.cc
    src/table/get_context.cc
    src/table/flush_block_policy.cc
    src/table/bloom_block.cc
    src/table/full_filter_block.cc

    src/util/options.cc
    src/util/status.cc
    src/util/hash_linklist_rep.cc
    src/util/bloom.cc
    src/util/cache.cc
    src/util/slice.cc
    src/util/options_helper.cc
    src/util/filter_policy.cc
    src/util/crc32c.cc
    src/util/comparator.cc
    src/util/auto_roll_logger.cc
    src/util/env.cc
    src/util/instrumented_mutex.cc
    src/util/db_info_dumper.cc
    src/util/sync_point.cc
    src/util/event_logger.cc
    src/util/arena.cc
    src/util/perf_context.cc
    src/util/mutable_cf_options.cc
    src/util/log_buffer.cc
    src/util/thread_status_util.cc
    src/util/compaction_job_stats_impl.cc
    src/util/file_reader_writer.cc
    src/util/file_util.cc
    src/util/coding.cc
    src/util/perf_level.cc
    src/util/thread_status_updater.cc
    src/util/thread_status_util_debug.cc
    src/util/logging.cc
    src/util/hash.cc
    src/util/histogram.cc
    src/util/murmurhash.cc
    src/util/xxhash.cc
    src/util/rate_limiter.cc
    src/util/dynamic_bloom.cc
    src/util/thread_local.cc
    src/util/skiplistrep.cc
    src/util/iostats_context.cc
)

SET(BUILD_VERSION_CC ${CMAKE_CURRENT_SOURCE_DIR}/src/util/build_version.cc)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/build_version.cc.in" ${BUILD_VERSION_CC} @ONLY IMMEDIATE)

IF(OS_POSIX)
    SET(SOURCES_ROCKSDB ${SOURCES_ROCKSDB} src/port/port_posix.cc src/util/env_posix.cc)
ELSEIF(OS_WINDOWS)
    SET(SOURCES_ROCKSDB ${SOURCES_ROCKSDB} src/port/win/port_win.cc src/port/win/env_win.cc src/port/win/win_logger.cc)
    #SET(PLATFORM_LIBRARIES Rpcrt4)
ENDIF(OS_POSIX)

ADD_LIBRARY(rocksdb OBJECT ${HEADERS_ROCKSDB} ${SOURCES_ROCKSDB} ${BUILD_VERSION_CC})
#TARGET_LINK_LIBRARIES(rocksdb ${PLATFORM_LIBRARIES})
