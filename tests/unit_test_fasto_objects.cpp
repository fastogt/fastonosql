#include <gtest/gtest.h>

#include "global/global.h"

using namespace fastonosql;

TEST(FastoObject, LifeTime)
{
    FastoObjectIPtr obj = FastoObject::createRoot("root");
    obj.reset();
    FastoObject *ptr = obj.get();
    ASSERT_TRUE(ptr == NULL);
}

TEST(FastoObject, LifeTimeScope)
{
    common::StringValue* obj = common::Value::createStringValue("Sasha");
    {
        FastoObjectIPtr root = FastoObject::createRoot("root");
        FastoObject* ptr = new FastoObject(root.get(), obj, "/n", ":");
        root->addChildren(ptr);
    }
}
