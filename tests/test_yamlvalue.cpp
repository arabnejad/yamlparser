#include "YamlValue.hpp"

#include <gtest/gtest.h>

using namespace yamlparser;

TEST(YamlValueTest, StoresEverySupportedType) {
  const YamlValue nullValue;
  const YamlValue stringValue("text");
  const YamlValue integerValue(42);
  const YamlValue doubleValue(3.5);
  const YamlValue booleanValue(true);
  const YamlValue sequenceValue(YamlSequence{YamlValue(1), YamlValue("two")});
  const YamlValue mappingValue(YamlMapping{{"key", YamlValue("value")}});

  EXPECT_TRUE(nullValue.isNull());
  EXPECT_EQ(stringValue.asString(), "text");
  EXPECT_EQ(integerValue.asInteger(), 42);
  EXPECT_DOUBLE_EQ(doubleValue.asDouble(), 3.5);
  EXPECT_TRUE(booleanValue.asBoolean());
  EXPECT_EQ(sequenceValue.at(1U).asString(), "two");
  EXPECT_EQ(mappingValue.at("key").asString(), "value");
}

TEST(YamlValueTest, ReportsTypesWithoutExposingStorage) {
  EXPECT_EQ(YamlValue().type(), YamlValue::Type::Null);
  EXPECT_EQ(YamlValue("text").type(), YamlValue::Type::String);
  EXPECT_EQ(YamlValue(1).type(), YamlValue::Type::Integer);
  EXPECT_EQ(YamlValue(1.0).type(), YamlValue::Type::Double);
  EXPECT_EQ(YamlValue(false).type(), YamlValue::Type::Boolean);
  EXPECT_EQ(YamlValue(YamlSequence()).type(), YamlValue::Type::Sequence);
  EXPECT_EQ(YamlValue(YamlMapping()).type(), YamlValue::Type::Mapping);
}

TEST(YamlValueTest, CopiesNestedValuesDeeply) {
  const YamlMapping  innerMapping{{"answer", YamlValue(42)}};
  const YamlSequence outerSequence{YamlValue(innerMapping)};
  const YamlValue    original(YamlMapping{{"outer", YamlValue(outerSequence)}});
  const YamlValue    copy = original;

  EXPECT_EQ(copy.at("outer").at(0U).at("answer").asInteger(), 42);
}

TEST(YamlValueTest, ThrowsClearErrorsForInvalidAccess) {
  const YamlValue integerValue(42);
  const YamlValue sequenceValue(YamlSequence{YamlValue("first")});
  const YamlValue mappingValue(YamlMapping{{"present", YamlValue(true)}});

  EXPECT_THROW(integerValue.asString(), TypeException);
  EXPECT_THROW(integerValue.asSequence(), TypeException);
  EXPECT_THROW(sequenceValue.at(1U), IndexException);
  EXPECT_THROW(mappingValue.at("missing"), KeyException);
}

TEST(YamlValueTest, RemainsValidAfterMoves) {
  YamlValue source(YamlMapping{{"key", YamlValue("value")}});
  YamlValue destination(std::move(source));

  EXPECT_EQ(destination.at("key").asString(), "value");
  EXPECT_TRUE(source.isNull());
}
