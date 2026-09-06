#include "YamlException.hpp"
#include "YamlParser.hpp"
#include "YamlPrinter.hpp"
#include "YamlTestHelpers.hpp"

#include <gtest/gtest.h>

#include <string>

using namespace yamlparser;
using yamlparser::test::expectSameYamlValue;

namespace {

YamlValue parseDocumentWithMergeAnchors(const std::string &documentContent) {
  const std::string anchors = R"(primary: &primary
  shared: from-primary
  primary_only: true
  setting: from-primary
fallback: &fallback
  shared: from-fallback
  fallback_only: true
  setting: from-fallback
)";
  return YamlParser().parseText(anchors + documentContent);
}

} // namespace

TEST(YamlMergeListTest, MergesFlowStyleAliasLists) {
  const YamlValue document = parseDocumentWithMergeAnchors(R"(service:
  <<: [*primary, *fallback] # this comment is outside the merge value
)");

  const YamlValue &serviceSettings = document.at("service");
  EXPECT_TRUE(serviceSettings.at("primary_only").asBoolean());
  EXPECT_TRUE(serviceSettings.at("fallback_only").asBoolean());
}

TEST(YamlMergeListTest, MergesBlockStyleAliasLists) {
  const YamlValue document = parseDocumentWithMergeAnchors(R"(service:
  <<:
    - *primary # the first mapping has precedence
    - *fallback
)");

  const YamlValue &serviceSettings = document.at("service");
  EXPECT_TRUE(serviceSettings.at("primary_only").asBoolean());
  EXPECT_TRUE(serviceSettings.at("fallback_only").asBoolean());
}

TEST(YamlMergeListTest, MergesAliasListsInsideFlowMappings) {
  const YamlValue document = parseDocumentWithMergeAnchors("service: {<<: [*primary, *fallback], local: true}\n");

  const YamlValue &serviceSettings = document.at("service");
  EXPECT_TRUE(serviceSettings.at("primary_only").asBoolean());
  EXPECT_TRUE(serviceSettings.at("fallback_only").asBoolean());
  EXPECT_TRUE(serviceSettings.at("local").asBoolean());
}

TEST(YamlMergeListTest, ExplicitKeysBeforeFlowMappingMergesTakePrecedence) {
  const YamlValue document = parseDocumentWithMergeAnchors("service: {setting: local, <<: [*primary, *fallback]}\n");

  EXPECT_EQ(document.at("service").at("setting").asString(), "local");
}

TEST(YamlMergeListTest, EarlierAliasesWinForDuplicateKeys) {
  const YamlValue document = parseDocumentWithMergeAnchors("service:\n  <<: [*primary, *fallback]\n");

  EXPECT_EQ(document.at("service").at("shared").asString(), "from-primary");
}

TEST(YamlMergeListTest, ExplicitKeysBeforeMergeTakePrecedence) {
  const YamlValue document = parseDocumentWithMergeAnchors(R"(service:
  setting: local
  <<: [*primary, *fallback]
)");

  EXPECT_EQ(document.at("service").at("setting").asString(), "local");
}

TEST(YamlMergeListTest, ExplicitKeysAfterMergeTakePrecedence) {
  const YamlValue document = parseDocumentWithMergeAnchors(R"(service:
  <<: [*primary, *fallback]
  setting: local
)");

  EXPECT_EQ(document.at("service").at("setting").asString(), "local");
}

TEST(YamlMergeListTest, RoundTripsResolvedMergeValues) {
  const YamlParser parser;
  const YamlValue  document = parseDocumentWithMergeAnchors(R"(service:
  <<: [*primary, *fallback]
  setting: local
)");

  const YamlValue reparsed = parser.parseText(YamlPrinter::toString(document));
  expectSameYamlValue(document, reparsed);
}

TEST(YamlMergeListTest, TreatsQuotedMergeSpellingsAsOrdinaryKeys) {
  const YamlValue document = YamlParser().parseText(R"(block_mapping:
  "<<": ordinary-block-value
flow_mapping: {"<<": ordinary-flow-value}
)");

  EXPECT_EQ(document.at("block_mapping").at("<<").asString(), "ordinary-block-value");
  EXPECT_EQ(document.at("flow_mapping").at("<<").asString(), "ordinary-flow-value");
}

TEST(YamlMergeListTest, ReportsMissingAliasInBlockMergeList) {
  try {
    YamlParser().parseText(R"(known: &known {value: 1}
combined:
  <<:
    - *known
    - *missing
)");
    FAIL() << "Expected a missing merge alias to throw";
  } catch (const KeyException &error) {
    EXPECT_NE(std::string(error.what()).find("*missing"), std::string::npos);
  }
}

TEST(YamlMergeListTest, ReportsNonMappingAliasInFlowStyleMergeList) {
  try {
    YamlParser().parseText(R"(mapping: &mapping {value: 1}
scalar: &scalar 42
combined:
  <<: [*mapping, *scalar]
)");
    FAIL() << "Expected a non-mapping merge alias to throw";
  } catch (const TypeException &error) {
    EXPECT_NE(std::string(error.what()).find("*scalar"), std::string::npos);
    EXPECT_NE(std::string(error.what()).find("must refer to a mapping"), std::string::npos);
  }
}

TEST(YamlMergeListTest, RejectsEmptyFlowStyleMergeListsAtTheClosingBracket) {
  try {
    YamlParser().parseText(R"(mapping: &mapping {value: 1}
combined:
  <<: []
)");
    FAIL() << "Expected an empty merge list to throw";
  } catch (const SyntaxException &error) {
    EXPECT_NE(std::string(error.what()).find("line 3, column 8"), std::string::npos);
    EXPECT_NE(std::string(error.what()).find("Merge alias list cannot be empty"), std::string::npos);
  }
}

TEST(YamlMergeListTest, RejectsTrailingCommasInFlowStyleMergeLists) {
  EXPECT_THROW(YamlParser().parseText(R"(mapping: &mapping {value: 1}
combined:
  <<: [*mapping,]
)"),
               SyntaxException);
}

TEST(YamlMergeListTest, RejectsNonAliasMergeListItems) {
  const YamlParser parser;
  EXPECT_THROW(parser.parseText(R"(mapping: &mapping {value: 1}
combined:
  <<: [*mapping, 42]
)"),
               SyntaxException);
  EXPECT_THROW(parser.parseText(R"(mapping: &mapping {value: 1}
combined:
  <<:
    - *mapping
    - 42
)"),
               SyntaxException);
}

TEST(YamlMergeListTest, DoesNotShareAnchorsBetweenParseOperations) {
  const YamlParser parser;
  parser.parseText("defaults: &defaults {value: 1}\n");

  EXPECT_THROW(parser.parseText("combined:\n  <<: [*defaults]\n"), KeyException);
}
