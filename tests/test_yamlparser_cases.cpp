#include "YamlParser.hpp"

#include <gtest/gtest.h>

using namespace yamlparser;

namespace {
YamlValue parseFixture(const std::string &filename) {
  return YamlParser().parseFile("test_cases/" + filename);
}
} // namespace

TEST(YamlParserFixtureTest, ParsesNestedTypes) {
  const YamlValue  document = parseFixture("01_nested_types.yaml");
  const YamlValue &config   = document.at("config");

  EXPECT_EQ(config.at("server").at("ports").at(2U).asInteger(), 8082);
  EXPECT_TRUE(config.at("server").at("enabled").asBoolean());
  EXPECT_DOUBLE_EQ(config.at("server").at("timeout").asDouble(), 30.5);
  EXPECT_EQ(config.at("databases").at(1U).at("settings").at("max_connections").asInteger(), 50);
  EXPECT_DOUBLE_EQ(config.at("databases").at(1U).at("settings").at("timeout").asDouble(), 5.0);
}

TEST(YamlParserFixtureTest, ParsesBlockScalarStyles) {
  const YamlValue  document    = parseFixture("02_multiline_formats.yaml");
  const YamlValue &description = document.at("description");

  EXPECT_NE(description.at("folded").asString().find("text that should be folded"), std::string::npos);
  EXPECT_NE(description.at("literal").asString().find("keep its\nexact formatting"), std::string::npos);
  EXPECT_EQ(description.at("literal_strip").asString().back(), 's');
  EXPECT_EQ(description.at("folded_keep").asString().back(), '\n');
  EXPECT_NE(document.at("documentation").asString().find("- /api/v1/users"), std::string::npos);
}

TEST(YamlParserFixtureTest, ParsesNumbersAndKeepsUnsupportedNumberFormatsAsStrings) {
  const YamlValue numbers = parseFixture("03_dates_and_numbers.yaml").at("numbers");

  EXPECT_EQ(numbers.at("integers").at("decimal").asInteger(), 12345);
  EXPECT_EQ(numbers.at("integers").at("hexadecimal").asString(), "0xFF");
  EXPECT_NEAR(numbers.at("floats").at("scientific").asDouble(), 1.23e-4, 1e-12);
  EXPECT_EQ(numbers.at("floats").at("infinity").asString(), ".inf");
}

TEST(YamlParserFixtureTest, ParsesNestedAnchorsAndMerges) {
  const YamlValue document = parseFixture("04_anchors_and_merging.yaml");

  EXPECT_EQ(document.at("service2").at("retries").asInteger(), 3);
  EXPECT_EQ(document.at("service2").at("timeout").asInteger(), 60);
  EXPECT_EQ(document.at("service2").at("logging").at("level").asString(), "DEBUG");
  EXPECT_EQ(document.at("production").at("database").at("host").asString(), "prod-db.example.com");
}

TEST(YamlParserFixtureTest, ParsesEverySequenceForm) {
  const YamlValue document = parseFixture("05_sequence_variations.yaml");

  EXPECT_EQ(document.at("simple_sequence").at(2U).asString(), "item3");
  EXPECT_EQ(document.at("flow_sequence").at(1U).asString(), "item2");
  EXPECT_EQ(document.at("nested_sequence").at(1U).at(0U).asString(), "nested3");
  EXPECT_EQ(document.at("sequence_of_mappings").at(0U).at("roles").at(0U).asString(), "admin");
  EXPECT_EQ(document.at("complex_nesting").at(1U).at("data").at(0U).at("values").at(2U).asInteger(), 9);
}

TEST(YamlParserFixtureTest, ParsesStringFormsAndEscapes) {
  const YamlValue document = parseFixture("06_string_formats.yaml");

  EXPECT_EQ(document.at("strings").at("single_quoted").asString(), "This is a single-quoted string");
  EXPECT_NE(document.at("strings").at("special_chars").at("escapes").asString().find('\t'), std::string::npos);
  EXPECT_NE(document.at("strings").at("special_chars").at("escapes").asString().find('&'), std::string::npos);
  EXPECT_EQ(document.at("paths").at("url").asString(), "https://example.com");
  EXPECT_TRUE(document.at("special_values").at("null_explicit").isNull());
  EXPECT_TRUE(document.at("special_values").at("null_implicit").isNull());
}

TEST(YamlParserFixtureTest, ParsesDeepMappingsAndComments) {
  const YamlValue comments = parseFixture("10_common_features.yaml").at("comments");
  const YamlValue document = parseFixture("07_comments_and_docs.yaml");

  EXPECT_EQ(comments.at("inline_comment").asString(), "value");
  EXPECT_EQ(document.at("database").at("settings").at("retry").at("attempts").asInteger(), 3);
  EXPECT_EQ(document.at("cache").at("settings").at("algorithm").at("params").at("chunks").asInteger(), 16);
}

TEST(YamlParserFixtureTest, ParsesMappingPatterns) {
  const YamlValue document = parseFixture("08_mapping_patterns.yaml");

  EXPECT_EQ(document.at("nested_mapping").at("level1").at("level2").at("level3").asString(), "value3");
  EXPECT_EQ(document.at("mapping_in_sequence").at(1U).at("value").asInteger(), 200);
}

TEST(YamlParserFixtureTest, DistinguishesBasicTypes) {
  const YamlValue document = parseFixture("09_basic_types.yaml");

  EXPECT_TRUE(document.at("booleans").at("true_values").at(0U).asBoolean());
  EXPECT_TRUE(document.at("booleans").at("true_values").at(1U).asBoolean());
  EXPECT_TRUE(document.at("booleans").at("true_values").at(2U).asBoolean());
  EXPECT_FALSE(document.at("booleans").at("false_values").at(0U).asBoolean());
  EXPECT_FALSE(document.at("booleans").at("false_values").at(1U).asBoolean());
  EXPECT_FALSE(document.at("booleans").at("false_values").at(2U).asBoolean());
  EXPECT_TRUE(document.at("null_values").at("explicit_null").isNull());
  EXPECT_EQ(document.at("dates").at("simple_date").asString(), "2025-07-26");
}

TEST(YamlParserFixtureTest, PreservesLiteralUtf8Text) {
  const YamlValue  document     = parseFixture("11_utf8_text.yaml");
  const YamlValue &utf8Examples = document.at("utf8_examples");

  EXPECT_EQ(utf8Examples.at("currency").asString(), u8"£");
  EXPECT_EQ(utf8Examples.at("language").asString(), u8"日本語");
  EXPECT_EQ(utf8Examples.at("status").asString(), u8"✓");
  EXPECT_EQ(utf8Examples.at("emoji").asString(), u8"😀");
}

TEST(YamlParserFixtureTest, ParsesNestedFlowCollections) {
  const YamlValue document = parseFixture("12_flow_collections.yaml");

  EXPECT_EQ(document.at("service").at("ports").at(1U).asInteger(), 8443);
  EXPECT_EQ(document.at("service").at("logging").at("level").asString(), "info");
  EXPECT_EQ(document.at("workers").at(0U).at("name").asString(), "primary");
  EXPECT_FALSE(document.at("workers").at(1U).at("enabled").asBoolean());
}
