#include <gtest/gtest.h>

#include "common/convert2string.h"
#include "common/utf_string_conversions.h"

TEST(string, convertTo) {
  int val = 11;
  const std::string stext = "11";
  const std::string convText = common::ConvertToString(val);
  ASSERT_EQ(stext, convText);

  long lval = 11;
  const std::string ltext = "11";
  const std::string lconvText = common::ConvertToString(lval);
  ASSERT_EQ(ltext, lconvText);
}

TEST(string16, convertTo) {
  const common::string16 text = common::UTF8ToUTF16("text");
  const std::string stext = "text";
  const common::string16 convText = common::ConvertToString16(stext);
  ASSERT_EQ(text, convText);
  const std::string fstext = common::ConvertFromString16<std::string>(convText);
  ASSERT_EQ(stext, fstext);

  const common::string16 text2 = common::UTF8ToUTF16("Привет");
  const std::string stext2 = "Привет";
  const common::string16 convText2 = common::ConvertToString16(stext2);
  ASSERT_EQ(text2, convText2);
  const std::string fstext2 = common::ConvertFromString16<std::string>(convText2);
  ASSERT_EQ(stext2, fstext2);
}
