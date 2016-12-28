#include <gmock/gmock.h>

#include <common/file_system.h>

#include "core/db/leveldb/db_connection.h"
#include "core/db/rocksdb/db_connection.h"
#include "core/db/lmdb/db_connection.h"
#include "core/db/unqlite/db_connection.h"
#include "core/db/upscaledb/db_connection.h"

using namespace fastonosql;

template <typename NConnection, typename Config, core::connectionTypes ContType>
void CheckSetGet(core::internal::CDBConnection<NConnection, Config, ContType>* db) {
  ASSERT_TRUE(db->IsConnected());
  core::NValue val(common::Value::createStringValue("test"));
  core::NKey key("test");
  core::NDbKValue kv(key, val);
  core::NDbKValue res;
  common::Error err = db->Set(kv, &res);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(kv == res);
  core::NDbKValue res2;
  err = db->Get(key, &res2);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(kv == res2);
  ASSERT_TRUE(db->IsConnected());
}

TEST(Connection, leveldb) {
  core::leveldb::DBConnection db(nullptr);
  core::leveldb::Config lcfg;
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(err && err->isError());
  ASSERT_TRUE(!db.IsConnected());
  err = common::file_system::create_directory(lcfg.dbname, false);
  ASSERT_TRUE(!err);
  err = db.Connect(lcfg);
  ASSERT_TRUE(err && err->isError());
  ASSERT_TRUE(!db.IsConnected());

  lcfg.create_if_missing = true;
  err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  err = common::file_system::remove_directory(lcfg.dbname, true);
  ASSERT_TRUE(!err);
}

TEST(Connection, rocksdb) {
  core::rocksdb::DBConnection db(nullptr);
  core::rocksdb::Config lcfg;
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(err && err->isError());
  ASSERT_TRUE(!db.IsConnected());
  err = common::file_system::create_directory(lcfg.dbname, false);
  ASSERT_TRUE(!err);
  err = db.Connect(lcfg);
  ASSERT_TRUE(err && err->isError());
  ASSERT_TRUE(!db.IsConnected());

  lcfg.create_if_missing = true;
  err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  err = common::file_system::remove_directory(lcfg.dbname, true);
  ASSERT_TRUE(!err);
}

TEST(Connection, lmdb) {
  core::lmdb::DBConnection db(nullptr);
  core::lmdb::Config lcfg;
  common::Error err = db.Connect(lcfg);
  ASSERT_TRUE(err && err->isError());
  ASSERT_TRUE(!db.IsConnected());
  err = common::file_system::create_directory(lcfg.dbname, false);
  ASSERT_TRUE(!err);
  err = db.Connect(lcfg);
  ASSERT_TRUE(err && err->isError());
  ASSERT_TRUE(!db.IsConnected());

  lcfg.SetReadOnlyDB(false);
  err = db.Connect(lcfg);
  ASSERT_TRUE(!err);
  ASSERT_TRUE(db.IsConnected());

  CheckSetGet(&db);

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());

  err = common::file_system::remove_directory(lcfg.dbname, true);
  ASSERT_TRUE(!err);
}

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

  err = common::file_system::remove_file(lcfg.dbname);
  ASSERT_TRUE(!err);
}

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

  err = common::file_system::remove_file(lcfg.dbname);
  ASSERT_TRUE(!err);
}
