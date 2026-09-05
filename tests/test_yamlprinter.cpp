#include "YamlTestHelpers.hpp"
#include "YamlParser.hpp"
#include "YamlPrinter.hpp"

#include <gtest/gtest.h>

using namespace yamlparser;
using yamlparser::test::expectSameYamlValue;

TEST(YamlPrinterTest, ProducesTwoSpaceBlockIndentation) {
  const YamlValue document(YamlMapping{{
      "application",
      YamlValue(YamlMapping{{"ports", YamlValue(YamlSequence{YamlValue(80), YamlValue(443)})}}),
  }});

  EXPECT_EQ(YamlPrinter::toString(document), "application:\n  ports:\n    - 80\n    - 443\n");
}

TEST(YamlPrinterTest, DistinguishesNullEmptyStringAndEmptyContainers) {
  const YamlValue document(YamlMapping{
      {"empty_mapping", YamlValue(YamlMapping())},
      {"empty_sequence", YamlValue(YamlSequence())},
      {"empty_string", YamlValue("")},
      {"null", YamlValue()},
  });

  const std::string output = YamlPrinter::toString(document);
  EXPECT_NE(output.find("empty_mapping: {}"), std::string::npos);
  EXPECT_NE(output.find("empty_sequence: []"), std::string::npos);
  EXPECT_NE(output.find("empty_string: ''"), std::string::npos);
  EXPECT_NE(output.find("'null': null"), std::string::npos);
}

TEST(YamlPrinterTest, RoundTripsEverySupportedValueWithoutChangingTypesOrValues) {
  const YamlValue original(YamlMapping{
      {"boolean", YamlValue(true)},
      {"double", YamlValue(1.23456789012345)},
      {"empty_mapping", YamlValue(YamlMapping())},
      {"empty_sequence", YamlValue(YamlSequence())},
      {"empty_string", YamlValue("")},
      {"integer", YamlValue(42)},
      {"multiline", YamlValue("first\nsecond\tcolumn")},
      {"null", YamlValue()},
      {"numeric_string", YamlValue("012345")},
      {"reserved_string", YamlValue("true")},
      {"sequence", YamlValue(YamlSequence{YamlValue("one"), YamlValue(YamlMapping{{"two", YamlValue(2)}})})},
  });

  const std::string printed  = YamlPrinter::toString(original);
  const YamlValue   reparsed = YamlParser().parseText(printed);
  expectSameYamlValue(original, reparsed);
}

TEST(YamlPrinterTest, RoundTripsRootScalarsAndContainers) {
  const YamlParser parser;

  expectSameYamlValue(YamlValue("true"), parser.parseText(YamlPrinter::toString(YamlValue("true"))));
  expectSameYamlValue(YamlValue(YamlSequence()), parser.parseText(YamlPrinter::toString(YamlValue(YamlSequence()))));
  expectSameYamlValue(YamlValue(YamlMapping()), parser.parseText(YamlPrinter::toString(YamlValue(YamlMapping()))));
}
