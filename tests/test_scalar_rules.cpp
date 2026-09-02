#include "YamlParser.hpp"
#include "YamlPrinter.hpp"

#include <gtest/gtest.h>

#include <string>

using namespace yamlparser;

TEST(YamlScalarRulesTest, RecognizesEveryYamlCoreBooleanSpelling) {
  const YamlValue document = YamlParser().parseText("lower_true: true\n"
                                                    "title_true: True\n"
                                                    "upper_true: TRUE\n"
                                                    "lower_false: false\n"
                                                    "title_false: False\n"
                                                    "upper_false: FALSE\n"
                                                    "yes_value: yes\n"
                                                    "no_value: no\n"
                                                    "on_value: on\n"
                                                    "off_value: off\n");

  EXPECT_TRUE(document.at("lower_true").asBoolean());
  EXPECT_TRUE(document.at("title_true").asBoolean());
  EXPECT_TRUE(document.at("upper_true").asBoolean());
  EXPECT_FALSE(document.at("lower_false").asBoolean());
  EXPECT_FALSE(document.at("title_false").asBoolean());
  EXPECT_FALSE(document.at("upper_false").asBoolean());
  EXPECT_EQ(document.at("yes_value").asString(), "yes");
  EXPECT_EQ(document.at("no_value").asString(), "no");
  EXPECT_EQ(document.at("on_value").asString(), "on");
  EXPECT_EQ(document.at("off_value").asString(), "off");
}

TEST(YamlScalarRulesTest, DecodesCommonDoubleQuotedEscapes) {
  const YamlValue document = YamlParser().parseText("double: \"\\b\\t\\n\\f\\r\\\"\\/\\\\\"\n"
                                                    "single: 'can''t'\n");

  EXPECT_EQ(document.at("double").asString(), "\b\t\n\f\r\"/\\");
  EXPECT_EQ(document.at("single").asString(), "can't");
}

TEST(YamlScalarRulesTest, PreservesLiteralUtf8Text) {
  const YamlValue document = YamlParser().parseText(u8"currency: \"£\"\n"
                                                    "language: \"日本語\"\n"
                                                    "status: \"✓\"\n"
                                                    "emoji: \"😀\"\n");

  EXPECT_EQ(document.at("currency").asString(), u8"£");
  EXPECT_EQ(document.at("language").asString(), u8"日本語");
  EXPECT_EQ(document.at("status").asString(), u8"✓");
  EXPECT_EQ(document.at("emoji").asString(), u8"😀");
}

TEST(YamlScalarRulesTest, RejectsEscapesOutsideThePracticalSubset) {
  const YamlParser parser;
  const char      *unsupportedEscapes[] = {"0", "a", "v", "e", " ", "N", "_", "L", "P", "x41", "u00A3", "U0001F600"};

  for (const char *escape : unsupportedEscapes) {
    const std::string yaml = "value: \"\\" + std::string(escape) + "\"\n";
    EXPECT_THROW(parser.parseText(yaml), SyntaxException) << "escape: \\" << escape;
  }

  EXPECT_THROW(parser.parseText("value: \"\\\n"), SyntaxException);
  EXPECT_THROW(parser.parseText("value: \"unterminated\nnext: value\n"), SyntaxException);
}

TEST(YamlScalarRulesTest, RejectsMalformedOrMultilineQuotedStrings) {
  const YamlParser parser;
  EXPECT_THROW(parser.parseText("value: \"first\" trailing\n"), SyntaxException);
  EXPECT_THROW(parser.parseText("value: 'first' trailing\n"), SyntaxException);
  EXPECT_THROW(parser.parseText("value: \"first\n  second\"\n"), SyntaxException);
  EXPECT_THROW(parser.parseText("value: \"first\\\n  second\"\n"), SyntaxException);
}

TEST(YamlScalarRulesTest, PrinterRoundTripsSupportedStringsAndBooleans) {
  const YamlParser parser;

  const YamlValue escapedValue("\b\t\n\f\r\"/\\");
  EXPECT_EQ(parser.parseText(YamlPrinter::toString(escapedValue)).asString(), escapedValue.asString());

  const YamlValue unicodeValue(u8"Symbols: £ 日本語 ✓ 😀");
  EXPECT_EQ(parser.parseText(YamlPrinter::toString(unicodeValue)).asString(), unicodeValue.asString());

  const char *booleanLikeStrings[] = {"true", "True", "TRUE", "false", "False", "FALSE"};
  for (const char *text : booleanLikeStrings) {
    const YamlValue reparsed = parser.parseText(YamlPrinter::toString(YamlValue(text)));
    ASSERT_TRUE(reparsed.isString());
    EXPECT_EQ(reparsed.asString(), text);
  }
}

TEST(YamlScalarRulesTest, PrinterRejectsUnsupportedControlCharacters) {
  const char unsupportedControls[] = {'\0', '\a', '\v', '\x1B', '\x7F'};
  for (char character : unsupportedControls)
    EXPECT_THROW(YamlPrinter::toString(YamlValue(std::string(1U, character))), TypeException);

  std::string mixedControls("supported newline\nthen null");
  mixedControls.push_back('\0');
  EXPECT_THROW(YamlPrinter::toString(YamlValue(mixedControls)), TypeException);
}
