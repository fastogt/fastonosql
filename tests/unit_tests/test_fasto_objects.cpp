#include <gtest/gtest.h>

#include "core/global.h"

using namespace fastonosql::core;

TEST(FastoObject, LifeTime) {
  FastoObjectIPtr obj = FastoObject::CreateRoot(MAKE_COMMAND_BUFFER("root"));
  obj.reset();
  FastoObject* ptr = obj.get();
  ASSERT_TRUE(ptr == NULL);
}

TEST(FastoObject, LifeTimeScope) {
  common::StringValue* obj = common::Value::CreateStringValue("Sasha");
  {
    FastoObjectIPtr root = FastoObject::CreateRoot(MAKE_COMMAND_BUFFER("root"));
    FastoObject* ptr = new FastoObject(root.get(), obj, "/n");
    root->AddChildren(ptr);
  }
}
