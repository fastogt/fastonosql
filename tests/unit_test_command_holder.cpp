#include <gtest/gtest.h>

#include <common/sprintf.h>

#include "core/command/command_holder.h"
#include "core/command/command_handler.h"

#define SET "SET"
#define GET "GET"
#define GET2 "GET2"
#define CONFIG "CONFIG"
#define GET_CONFIG GET " " CONFIG
#define GET_CONFIG_INVALID GET " " CONFIG "E"

using namespace fastonosql;

core::CommandHandler* ghand = NULL;

common::Error test(core::CommandHandler* handler, int argc, const char** argv, FastoObject* out) {
  UNUSED(argc);
  UNUSED(argv);

  CHECK(handler == ghand);
  CHECK(out == NULL);

  return common::Error();
}

const std::vector<core::CommandHolder> cmds = {core::CommandHolder(SET,
                                                                   "<key> <value>",
                                                                   "Set the value of a key.",
                                                                   UNDEFINED_SINCE,
                                                                   UNDEFINED_EXAMPLE_STR,
                                                                   2,
                                                                   0,
                                                                   &test),
                                               core::CommandHolder(GET_CONFIG,
                                                                   "<key>",
                                                                   "Set the value of a key.",
                                                                   UNDEFINED_SINCE,
                                                                   UNDEFINED_EXAMPLE_STR,
                                                                   1,
                                                                   0,
                                                                   &test),
                                               core::CommandHolder(GET2,
                                                                   "<key>",
                                                                   "Set the value of a key.",
                                                                   UNDEFINED_SINCE,
                                                                   UNDEFINED_EXAMPLE_STR,
                                                                   1,
                                                                   0,
                                                                   &test)};

TEST(CommandHolder, execute) {
  core::CommandHandler hand(cmds);
  ghand = &hand;
  const char* cmd_valid_set[] = {SET, "alex", "palec"};
  common::Error err = hand.execute(SIZEOFMASS(cmd_valid_set), cmd_valid_set, NULL);
  ASSERT_TRUE(!err);

  const char* cmd_invalid_set[] = {SET, "alex"};
  err = hand.execute(SIZEOFMASS(cmd_invalid_set), cmd_invalid_set, NULL);
  ASSERT_TRUE(err && err->isError());

  const char* cmd_not_exists[] = {GET, "alex"};
  err = hand.execute(SIZEOFMASS(cmd_not_exists), cmd_not_exists, NULL);
  ASSERT_TRUE(err && err->isError());

  const char* cmd_get_config[] = {GET, CONFIG, "alex"};
  err = hand.execute(SIZEOFMASS(cmd_get_config), cmd_get_config, NULL);
  ASSERT_TRUE(!err);

  const char* cmd_get_config_invalid[] = {GET_CONFIG_INVALID, "alex"};
  err = hand.execute(SIZEOFMASS(cmd_get_config_invalid), cmd_get_config_invalid, NULL);
  ASSERT_TRUE(err && err->isError());

  const char* cmd_get2[] = {GET2, "alex"};
  err = hand.execute(SIZEOFMASS(cmd_get2), cmd_get2, NULL);
  ASSERT_TRUE(!err);

  const char* cmd_get_config_many_args[] = {GET, CONFIG, "last", "alex"};
  err = hand.execute(SIZEOFMASS(cmd_get_config_many_args), cmd_get_config_many_args, NULL);
  ASSERT_TRUE(err && err->isError());
}
