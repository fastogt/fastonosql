#include <gmock/gmock.h>

#include <common/file_system.h>

#include "core/db/lmdb/db_connection.h"

using namespace fastonosql::core;

TEST(Connection, lmdb) {
  lmdb::DBConnection db(nullptr);
  lmdb::Config lcfg;
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

  err = db.Disconnect();
  ASSERT_TRUE(!err);
  ASSERT_TRUE(!db.IsConnected());
}
