#include "YamlException.hpp"
#include "YamlParser.hpp"
#include "YamlPrinter.hpp"
#include "YamlTestHelpers.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace yamlparser;
using yamlparser::test::expectSameYamlValue;

TEST(YamlFlowCollectionsTest, ParsesMappingsAndSequencesNestedInsideEachOther) {
  const YamlValue document = YamlParser().parseText(
      R"(service: {host: localhost, port: 8080, secure: TRUE, values: [1, {name: first}, [2, 3]], url: https://example.com}
workers: [{name: primary, enabled: true}, {name: backup, enabled: false}]
)");

  const YamlValue &service = document.at("service");
  EXPECT_EQ(service.at("host").asString(), "localhost");
  EXPECT_EQ(service.at("port").asInteger(), 8080);
  EXPECT_TRUE(service.at("secure").asBoolean());
  EXPECT_EQ(service.at("url").asString(), "https://example.com");
  EXPECT_EQ(service.at("values").at(1U).at("name").asString(), "first");
  EXPECT_EQ(service.at("values").at(2U).at(1U).asInteger(), 3);

  EXPECT_EQ(document.at("workers").at(0U).at("name").asString(), "primary");
  EXPECT_FALSE(document.at("workers").at(1U).at("enabled").asBoolean());
}

TEST(YamlFlowCollectionsTest, ParsesFlowCollectionDocumentRootsAndQuotedDelimiters) {
  const YamlParser parser;
  const YamlValue  mapping = parser.parseText(
      R"yaml({host: localhost, labels: ["comma, stays", 'colon: stays', 'it''s quoted'], escaped: "line\nnext, \"quoted\""}
)yaml");
  const YamlValue sequence           = parser.parseText("[{name: first}, {name: second}]\n");
  const YamlValue compactCollections = parser.parseText("{values:[1, 2], nested:{enabled: true}}\n");

  EXPECT_EQ(mapping.at("labels").at(0U).asString(), "comma, stays");
  EXPECT_EQ(mapping.at("labels").at(1U).asString(), "colon: stays");
  EXPECT_EQ(mapping.at("labels").at(2U).asString(), "it's quoted");
  EXPECT_EQ(mapping.at("escaped").asString(), "line\nnext, \"quoted\"");
  EXPECT_EQ(sequence.at(1U).at("name").asString(), "second");
  EXPECT_EQ(compactCollections.at("values").at(1U).asInteger(), 2);
  EXPECT_TRUE(compactCollections.at("nested").at("enabled").asBoolean());
}

TEST(YamlFlowCollectionsTest, SupportsAnchorsAliasesMergesAndEmptyValues) {
  const YamlValue document = YamlParser().parseText(R"(defaults: &defaults {host: localhost, port: 80}
service: {<<: *defaults, port: 8080, description:}
copies: [&item {name: first}, *item]
)");

  EXPECT_EQ(document.at("service").at("host").asString(), "localhost");
  EXPECT_EQ(document.at("service").at("port").asInteger(), 8080);
  EXPECT_TRUE(document.at("service").at("description").isNull());
  EXPECT_EQ(document.at("copies").at(1U).at("name").asString(), "first");
}

TEST(YamlFlowCollectionsTest, PrinterUsesReadableBlockStyleAndRoundTripsFlowInput) {
  const YamlParser  parser;
  const YamlValue   original = parser.parseText("{values: [1, {name: first}], enabled: true}\n");
  const std::string printed  = YamlPrinter::toString(original);
  const YamlValue   reparsed = parser.parseText(printed);

  EXPECT_NE(printed.find("values:\n"), std::string::npos);
  expectSameYamlValue(original, reparsed);
}

TEST(YamlFlowCollectionsTest, RejectsMalformedFlowCollections) {
  struct MalformedFlowCase {
    std::string yaml;
    std::string expectedMessage;
  };

  const std::vector<MalformedFlowCase> malformedCases{
      {"[1,, 2]\n", "empty item"},
      {"[1, 2\n", "Unmatched '['"},
      {"[1, 2]]\n", "Unexpected ']'"},
      {"[1 [2]]\n", "Expected ',' or ']'"},
      {"[1,]\n", "empty item"},
      {"{host localhost}\n", "Expected ':' after flow mapping key"},
      {"{host: localhost port: 80}\n", "Expected ',' or '}'"},
      {"{host: localhost,, port: 80}\n", "empty entry"},
      {"{host: localhost,}\n", "empty entry"},
      {"{host: first, host: second}\n", "Duplicate mapping key"},
      {"{host: localhost} trailing\n", "Unexpected content after flow collection"},
      {"{? [one, two]: value}\n", "Complex mapping keys"},
      {"{value: !custom text}\n", "Tags are not supported"},
      {R"yaml({value: "\q"})yaml", "Unsupported escape sequence"},
  };

  const YamlParser parser;
  for (const MalformedFlowCase &malformedCase : malformedCases) {
    try {
      parser.parseText(malformedCase.yaml);
      ADD_FAILURE() << "Expected invalid YAML to fail: " << malformedCase.yaml;
    } catch (const SyntaxException &error) {
      EXPECT_NE(std::string(error.what()).find(malformedCase.expectedMessage), std::string::npos)
          << "YAML: " << malformedCase.yaml << "\nError: " << error.what();
    }
  }
}

TEST(YamlFlowCollectionsTest, ReportsLineAndColumnForFlowSyntaxErrors) {
  try {
    YamlParser().parseText("config: {host: localhost,, port: 80}\n");
    FAIL() << "Expected malformed flow mapping to throw";
  } catch (const SyntaxException &error) {
    const std::string message = error.what();
    EXPECT_NE(message.find("line 1, column 26"), std::string::npos);
    EXPECT_NE(message.find("empty entry"), std::string::npos);
  }
}

TEST(YamlFlowCollectionsTest, ReportsQuotedEscapeAtItsExactSourceColumn) {
  try {
    YamlParser().parseText(R"yaml(config: {value: "\q"}
)yaml");
    FAIL() << "Expected unsupported quoted escape to throw";
  } catch (const SyntaxException &error) {
    const std::string message = error.what();
    EXPECT_NE(message.find("line 1, column 18"), std::string::npos);
    EXPECT_NE(message.find("Unsupported escape sequence"), std::string::npos);
  }
}
