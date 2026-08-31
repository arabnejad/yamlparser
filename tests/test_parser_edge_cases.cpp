#include "YamlParser.hpp"

#include <gtest/gtest.h>

using namespace yamlparser;

TEST(YamlParserEdgeCasesTest, HandlesBlankLinesBeforeNestedContent) {
  const YamlValue document = YamlParser().parseText("parent:\n\n  # comment\n  child: value\n");
  EXPECT_EQ(document.at("parent").at("child").asString(), "value");
}

TEST(YamlParserEdgeCasesTest, KeepsHashCharactersThatAreNotComments) {
  const YamlValue document = YamlParser().parseText("fragment: value#part\n"
                                                    "quoted: \"value # part\" # actual comment\n"
                                                    "plain: value # actual comment\n");

  EXPECT_EQ(document.at("fragment").asString(), "value#part");
  EXPECT_EQ(document.at("quoted").asString(), "value # part");
  EXPECT_EQ(document.at("plain").asString(), "value");
}

TEST(YamlParserEdgeCasesTest, ParsesSingleLineMappingsInsideSequences) {
  const YamlValue document = YamlParser().parseText("items:\n  - name: first\n  - name: second\n");
  const auto     &items    = document.at("items").asSequence();

  ASSERT_EQ(items.size(), 2U);
  EXPECT_EQ(items[0].at("name").asString(), "first");
  EXPECT_EQ(items[1].at("name").asString(), "second");
}

TEST(YamlParserEdgeCasesTest, AppliesAllValueRulesToFirstMappingEntryInSequence) {
  const YamlValue document = YamlParser().parseText("items:\n"
                                                    "  - roles: [admin, user]\n"
                                                    "    active: true\n");

  const YamlValue &item = document.at("items").at(0U);
  EXPECT_EQ(item.at("roles").at(1U).asString(), "user");
  EXPECT_TRUE(item.at("active").asBoolean());
}

TEST(YamlParserEdgeCasesTest, RejectsMalformedStructures) {
  const YamlParser parser;
  EXPECT_THROW(parser.parseText("key: [1, 2\n"), SyntaxException);
  EXPECT_THROW(parser.parseText("key: 1\nkey: 2\n"), SyntaxException);
  EXPECT_THROW(parser.parseText("key: 1\n  unexpected: 2\n"), SyntaxException);
  EXPECT_THROW(parser.parseText("items: [1,,2]\n"), SyntaxException);
  EXPECT_THROW(parser.parseText("key: value\nmissing colon\n"), SyntaxException);
}

TEST(YamlParserEdgeCasesTest, SupportsDocumentMarkersAndEmptyContainers) {
  const YamlParser parser;
  EXPECT_TRUE(parser.parseText("---\n{}\n...\n").asMapping().empty());
  EXPECT_TRUE(parser.parseText("[]\n").asSequence().empty());
}
