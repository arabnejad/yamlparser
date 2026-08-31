#include "YamlParser.hpp"
#include "YamlPrinter.hpp"

#include <gtest/gtest.h>

using namespace yamlparser;

namespace {
void expectSameValue(const YamlValue &expected, const YamlValue &actual) {
  ASSERT_EQ(expected.type(), actual.type());
  switch (expected.type()) {
  case YamlValue::Type::Null:
    return;
  case YamlValue::Type::String:
    EXPECT_EQ(expected.asString(), actual.asString());
    return;
  case YamlValue::Type::Double:
    EXPECT_DOUBLE_EQ(expected.asDouble(), actual.asDouble());
    return;
  case YamlValue::Type::Integer:
    EXPECT_EQ(expected.asInteger(), actual.asInteger());
    return;
  case YamlValue::Type::Boolean:
    EXPECT_EQ(expected.asBoolean(), actual.asBoolean());
    return;
  case YamlValue::Type::Sequence: {
    ASSERT_EQ(expected.asSequence().size(), actual.asSequence().size());
    for (std::size_t index = 0; index < expected.asSequence().size(); ++index)
      expectSameValue(expected.at(index), actual.at(index));
    return;
  }
  case YamlValue::Type::Mapping: {
    ASSERT_EQ(expected.asMapping().size(), actual.asMapping().size());
    for (const auto &entry : expected.asMapping())
      expectSameValue(entry.second, actual.at(entry.first));
    return;
  }
  }
}
} // namespace

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
  expectSameValue(original, reparsed);
}

TEST(YamlPrinterTest, RoundTripsRootScalarsAndContainers) {
  const YamlParser parser;

  expectSameValue(YamlValue("true"), parser.parseText(YamlPrinter::toString(YamlValue("true"))));
  expectSameValue(YamlValue(YamlSequence()), parser.parseText(YamlPrinter::toString(YamlValue(YamlSequence()))));
  expectSameValue(YamlValue(YamlMapping()), parser.parseText(YamlPrinter::toString(YamlValue(YamlMapping()))));
}
