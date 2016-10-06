#include <gtest/gtest.h>

extern "C" {
#include "third-party/sds/sds.h"
}

#include "third-party/json-c/json-c/json.h"

#include <string.h>

TEST(sds, sdssplitargslong) {
  const std::string json = R"({
                           "array": [
                             1,
                             2,
                             3
                           ],
                           "boolean": true,
                           "null": null,
                           "number": 123,
                           "object": {
                               "a": "b",
                               "c": "d",
                               "e": "f"
                             },
                           "string": "Hello World"
                         })";
  const std::string json2 = R"({
                           "array": [
                             1,
                             2,
                             3
                           ],
                           "boolean": true,
                           "null": null,
                           "number": 123,
                           "string": "Hello World"
                         })";
  json_object* obj = json_tokener_parse(json.c_str());
  ASSERT_TRUE(obj);
  json_object_put(obj);
  obj = json_tokener_parse(json2.c_str());
  ASSERT_TRUE(obj);
  json_object_put(obj);
  const std::string line = "-d \n -c -h 127.0.0.1 -p \t -j " + json + " -j2 " + json2;
  int argc = 0;
  sds* argv = sdssplitargslong(line.c_str(), &argc);
  ASSERT_TRUE(argv && argc == 11);
  ASSERT_STREQ(argv[0], "-d");
  ASSERT_STREQ(argv[1], "\n");
  ASSERT_STREQ(argv[2], "-c");
  ASSERT_STREQ(argv[3], "-h");
  ASSERT_STREQ(argv[4], "127.0.0.1");
  ASSERT_STREQ(argv[5], "-p");
  ASSERT_STREQ(argv[6], "\t");
  ASSERT_STREQ(argv[7], "-j");
  const char* jafter = argv[8];
  ASSERT_STREQ(jafter, json.c_str());
  ASSERT_STREQ(argv[9], "-j2");
  const char* jafter2 = argv[10];
  ASSERT_STREQ(jafter2, json2.c_str());
  sdsfreesplitres(argv, argc);
}
