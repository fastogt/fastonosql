#include <gmock/gmock.h>

#include <common/file_system/file_system.h>

#ifdef BUILD_WITH_FORESTDB
#include "core/db/forestdb/db_connection.h"
#endif
#ifdef BUILD_WITH_LEVELDB
#include "core/db/leveldb/db_connection.h"
#endif
#ifdef BUILD_WITH_LMDB
#include "core/db/lmdb/db_connection.h"
#endif
#ifdef BUILD_WITH_ROCKSDB
#include "core/db/rocksdb/db_connection.h"
#endif
#ifdef BUILD_WITH_UNQLITE
#include "core/db/unqlite/db_connection.h"
#endif
#ifdef BUILD_WITH_UPSCALEDB
#include "core/db/upscaledb/db_connection.h"
#endif

using namespace fastonosql;

template <typename NConnection, typename Config, core::connectionTypes ContType>
void CheckSetGet(core::internal::CDBConnection<NConnection, Config, ContType>* db) {
  ASSERT_TRUE(db->IsConnected());
  core::NValue val(common::Value::CreateStringValue("test"));
  core::key_t key_str("test");
  core::NKey key(key_str);
  core::NDbKValue res1;
  common::Error err = db->Get(key, &res1);
  ASSERT_TRUE(err);

  core::NDbKValue kv(key, val);
  core::NDbKValue res;
  err = db->Set(kv, &res);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(kv == res);
  core::NDbKValue res2;
  err = db->Get(key, &res2);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(kv == res2);
  ASSERT_TRUE(db->IsConnected());

  core::NKeys want_delete = {key};
  core::NKeys deleted;
  err = db->Delete(want_delete, &deleted);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(want_delete == deleted);
  deleted = core::NKeys();
  err = db->Delete(want_delete, &deleted);
  ASSERT_TRUE(deleted.empty());  // must be
  ASSERT_TRUE(!err);
  core::NDbKValue res3;
  err = db->Get(key, &res3);
  ASSERT_TRUE(err);
  ASSERT_TRUE(db->IsConnected());
}

#ifdef BUILD_WITH_LEVELDB
TEST(Connection, leveldb) {
  core::leveldb::DBConnection db(nullptr);
  core::leveldb::Config lcfg;
  common::ErrnoError errn = common::file_system::remove_directory(lcfg.db_path, true);
  ASSERT_TRUE(!errn);

  lcfg.create_if_missing = false;
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(err);
  ASSERT_TRUE(!db.IsConnected());
  errn = common::file_system::create_directory(lcfg.db_path, false);
  ASSERT_TRUE(!errn);
  err = db.Connect(lcfg);
  ASSERT_TRUE(err);
  ASSERT_TRUE(!db.IsConnected());

  lcfg.create_if_missing = true;
  err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  errn = common::file_system::remove_directory(lcfg.db_path, true);
  ASSERT_TRUE(!errn);
}
#endif

#ifdef BUILD_WITH_ROCKSDB
TEST(Connection, rocksdb) {
  core::rocksdb::DBConnection db(nullptr);
  core::rocksdb::Config lcfg;
  common::ErrnoError errn = common::file_system::remove_directory(lcfg.db_path, true);
  ASSERT_TRUE(!errn);

  lcfg.create_if_missing = false;
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(err);
  ASSERT_TRUE(!db.IsConnected());
  errn = common::file_system::create_directory(lcfg.db_path, false);
  ASSERT_TRUE(!errn);
  err = db.Connect(lcfg);
  ASSERT_TRUE(err);
  ASSERT_TRUE(!db.IsConnected());

  lcfg.create_if_missing = true;
  err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  errn = common::file_system::remove_directory(lcfg.db_path, true);
  ASSERT_TRUE(!errn);
}
#endif

#ifdef BUILD_WITH_LMDB
TEST(Connection, lmdb) {
  core::lmdb::DBConnection db(nullptr);
  core::lmdb::Config lcfg;
  common::ErrnoError errn = common::file_system::remove_file(lcfg.db_path);
  ASSERT_TRUE(!errn);

  lcfg.SetReadOnlyDB(true);
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(err);
  ASSERT_TRUE(!db.IsConnected());
  /*errn = common::file_system::create_node(lcfg.db_path);
  ASSERT_TRUE(!errn);
  err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());*/

  lcfg.SetReadOnlyDB(false);
  err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  errn = common::file_system::remove_file(lcfg.db_path);
  ASSERT_TRUE(!errn);
}
#endif

#ifdef BUILD_WITH_UNQLITE
TEST(Connection, unqlite) {
  core::unqlite::DBConnection db(nullptr);
  core::unqlite::Config lcfg;
  lcfg.SetCreateIfMissingDB(true);  // workaround
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  common::ErrnoError errn = common::file_system::remove_file(lcfg.db_path);
  ASSERT_TRUE(!errn);
}
#endif

#ifdef BUILD_WITH_UPSCALEDB
TEST(Connection, upscaledb) {
  core::upscaledb::DBConnection db(nullptr);
  core::upscaledb::Config lcfg;
  lcfg.create_if_missing = true;  // workaround
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  common::ErrnoError errn = common::file_system::remove_file(lcfg.db_path);
  ASSERT_TRUE(!errn);
}
#endif

#ifdef BUILD_WITH_FORESTDB
TEST(Connection, forestdb) {
  core::forestdb::DBConnection db(nullptr);
  core::forestdb::Config lcfg;
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  common::ErrnoError errn = common::file_system::remove_file(lcfg.db_path);
  ASSERT_TRUE(!errn);
}
#endif
