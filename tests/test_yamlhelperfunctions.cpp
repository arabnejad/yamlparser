
#include "YamlHelperFunctions.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <map>
#include <string>
#include "YamlParser.hpp"
#include "YamlElement.hpp"

using namespace yamlparser;

class YamlHelperFunctionsTest : public ::testing::Test {
protected:
  void SetUp() override {
    // No special setup needed for helper function tests
  }

  void TearDown() override {
    // No cleanup needed for helper function tests
  }
};

TEST_F(YamlHelperFunctionsTest, TrimFunctionRemovesWhitespace) {
  // Test basic string trimming functionality
  // Verifies that trim function correctly removes leading and trailing whitespace

  // Test trimming spaces and tabs
  EXPECT_EQ(trim("  abc  "), "abc");
  EXPECT_EQ(trim("\tabc\t"), "abc");

  // Test strings that don't need trimming
  EXPECT_EQ(trim("abc"), "abc");

  // Test edge cases
  EXPECT_EQ(trim("   "), "");
  EXPECT_EQ(trim(""), "");
}

TEST_F(YamlHelperFunctionsTest, MultilineLiteralDetection) {
  // Test multiline literal detection
  // Verifies that isMultilineLiteral correctly identifies YAML block scalar indicators

  // Test valid multiline indicators
  EXPECT_TRUE(isMultilineLiteral("|")); // Literal block scalar
  EXPECT_TRUE(isMultilineLiteral(">")); // Folded block scalar

  // Test invalid cases
  EXPECT_FALSE(isMultilineLiteral("abc"));
  EXPECT_FALSE(isMultilineLiteral(""));
}

TEST_F(YamlHelperFunctionsTest, AnchorDetection) {
  // Test anchor detection
  // Verifies that isAnchor correctly identifies YAML anchor syntax (&)

  // Test valid anchor
  EXPECT_TRUE(isAnchor("&foo"));

  // Test invalid cases
  EXPECT_FALSE(isAnchor("foo"));
  EXPECT_FALSE(isAnchor(""));
}

TEST_F(YamlHelperFunctionsTest, AliasDetection) {
  // Test alias detection
  // Verifies that isAlias correctly identifies YAML alias syntax (*)

  // Test valid alias
  EXPECT_TRUE(isAlias("*foo"));

  // Test invalid cases
  EXPECT_FALSE(isAlias("foo"));
  EXPECT_FALSE(isAlias(""));
}

TEST_F(YamlHelperFunctionsTest, InlineSequenceDetection) {
  // Test inline sequence detection
  // Verifies that isInlineSeq correctly identifies YAML flow sequence syntax

  // Test valid inline sequence
  EXPECT_TRUE(isInlineSeq("[a, b, c]"));

  // Test invalid cases
  EXPECT_FALSE(isInlineSeq("a, b, c")); // Missing brackets
  EXPECT_FALSE(isInlineSeq("[]"));      // Empty sequence
  EXPECT_FALSE(isInlineSeq("[abc"));    // Unclosed bracket

  // Test whitespace-only content (covers line 67)
  EXPECT_FALSE(isInlineSeq("[   ]"));   // Only spaces between brackets
  EXPECT_FALSE(isInlineSeq("[\t\n ]")); // Only whitespace characters
}

TEST_F(YamlHelperFunctionsTest, MergeKeyDetection) {
  // Test merge key detection
  // Verifies that isMergeKey correctly identifies YAML merge key syntax (<<)

  // Test valid merge key
  EXPECT_TRUE(isMergeKey("<<", "*foo"));

  // Test invalid cases
  EXPECT_FALSE(isMergeKey("foo", "*foo")); // Wrong key
  EXPECT_FALSE(isMergeKey("<<", "foo"));   // Wrong value format
}

TEST_F(YamlHelperFunctionsTest, MultilineLiteralParsing) {
  // Test multiline literal parsing
  // Verifies that parseMultilineLiteral correctly processes block scalar content

  // Setup: Create test lines with multiline literal (pipe style - preserves newlines)
  std::vector<std::string> lines = {"key: |", "  line1", "  line2", "other: value"};
  size_t                   idx   = 0;

  // Action: Parse multiline literal with indent of 1
  auto result = parseMultilineLiteral(lines, idx, 1, '|');

  // Assertion: Should combine lines with newlines
  EXPECT_EQ(result.value.asString(), "line1\nline2\n");

  // Test folded block scalar (covers lines 115-117)
  std::vector<std::string> foldedLines = {"key: >", "  line1", "  line2", "other: value"};
  size_t                   foldedIdx   = 0;

  // Action: Parse folded multiline literal (> style - folds newlines to spaces)
  auto foldedResult = parseMultilineLiteral(foldedLines, foldedIdx, 1, '>');

  // Assertion: Should combine lines with spaces (folded style)
  EXPECT_EQ(foldedResult.value.asString(), "line1 line2");
}

TEST_F(YamlHelperFunctionsTest, AnchorParsing) {
  // Test anchor parsing functionality
  // Verifies that parseAnchor correctly handles both sequence and map anchors

  // Setup: Create a parser instance and anchors map
  yamlparser::YamlParser          parser;
  std::map<std::string, YamlItem> anchors;

  // Test anchor with map content (covers line 143)
  std::vector<std::string> mapLines = {"key: &anchor", "  subkey1: value1", "  subkey2: value2", "next: value"};
  size_t                   mapIdx   = 0;

  // Action: Parse anchor that points to a map
  auto mapResult = parseAnchor("&anchor", mapLines, mapIdx, anchors, parser);

  // Assertions: Should create a map and store it in anchors
  ASSERT_TRUE(mapResult.value.isMap());
  ASSERT_TRUE(anchors.find("anchor") != anchors.end());
  auto mapData = mapResult.value.asMap();
  EXPECT_TRUE(mapData.find("subkey1") != mapData.end());
  EXPECT_EQ(mapData["subkey1"].value.asString(), "value1");
}

TEST_F(YamlHelperFunctionsTest, AliasParsing) {
  // Test alias parsing and resolution
  // Verifies that parseAlias correctly resolves anchor references

  // Setup: Create anchors map with test data
  std::map<std::string, YamlItem> anchors;
  anchors["foo"] = YamlItem(YamlElement(std::string("bar")));

  // Action: Parse valid alias
  auto result = parseAlias("*foo", anchors);

  // Assertion: Should resolve to anchored value
  EXPECT_EQ(result.value.asString(), "bar");

  // Action: Parse invalid alias
  EXPECT_THROW(parseAlias("*baz", anchors), KeyException);
}

TEST_F(YamlHelperFunctionsTest, InlineSequenceParsing) {
  // Test inline sequence parsing
  // Verifies that parseInlineSeq correctly parses flow sequence syntax

  // Action: Parse inline sequence with integers
  auto result = parseInlineSeq("[1, 2, 3]");

  // Assertions: Should create sequence with correct elements
  ASSERT_TRUE(result.value.isSeq());
  auto seq = result.value.asSeq();
  ASSERT_EQ(seq.size(), 3);
  EXPECT_EQ(seq[0].value.asInt(), 1);
  EXPECT_EQ(seq[1].value.asInt(), 2);
  EXPECT_EQ(seq[2].value.asInt(), 3);

  // Test sequence with quoted strings (covers line 184 - single quote handling)
  auto result2 = parseInlineSeq("['hello', 'world', 'test']");
  ASSERT_TRUE(result2.value.isSeq());
  auto seq2 = result2.value.asSeq();
  ASSERT_EQ(seq2.size(), 3);
  EXPECT_EQ(seq2[0].value.asString(), "hello");
  EXPECT_EQ(seq2[1].value.asString(), "world");
  EXPECT_EQ(seq2[2].value.asString(), "test");

  // Test sequence with double quotes (covers line 186 - double quote handling)
  auto result3 = parseInlineSeq("[\"hello\", \"world\", \"test\"]");
  ASSERT_TRUE(result3.value.isSeq());
  auto seq3 = result3.value.asSeq();
  ASSERT_EQ(seq3.size(), 3);
  EXPECT_EQ(seq3[0].value.asString(), "hello");
  EXPECT_EQ(seq3[1].value.asString(), "world");
  EXPECT_EQ(seq3[2].value.asString(), "test");
}

TEST_F(YamlHelperFunctionsTest, MergeKeyParsing) {
  // Test merge key parsing and application
  // Verifies that parseMergeKey correctly merges anchor content into map

  // Setup: Create anchors and target map
  std::map<std::string, YamlItem> anchors;
  std::map<std::string, YamlItem> map;
  YamlMap                         m;
  m["foo"]       = YamlItem(YamlElement(std::string("bar")));
  anchors["baz"] = YamlItem(YamlElement(m));

  // Action: Parse merge key and apply to map
  parseMergeKey("*baz", map, anchors);

  // Assertions: Map should contain merged content
  ASSERT_TRUE(map.find("foo") != map.end());
  EXPECT_EQ(map["foo"].value.asString(), "bar");
}

TEST_F(YamlHelperFunctionsTest, TrimWhitespaceOnlyStrings) {
  // This test checks that trim returns expected results for whitespace-only strings.
  EXPECT_EQ(trim("   \t  \n  "), "\n");
  EXPECT_EQ(trim("\n\n\t\t"), "\n\n");
  EXPECT_EQ(trim("\t\t\n\n"), "\n\n");
}

TEST_F(YamlHelperFunctionsTest, MultilineLiteralDetectionWithMalformedInput) {
  // This test checks that isMultilineLiteral handles malformed or permissive indicators.
  EXPECT_TRUE(isMultilineLiteral("|>"));  // Permissive implementation
  EXPECT_TRUE(isMultilineLiteral("|  ")); // Permissive implementation
  EXPECT_TRUE(isMultilineLiteral("|-"));  // Permissive implementation
  EXPECT_FALSE(isMultilineLiteral(" "));
}

TEST_F(YamlHelperFunctionsTest, AnchorDetectionWithMalformedInput) {
  // This test checks that isAnchor returns true for permissive/malformed anchor strings.
  EXPECT_TRUE(isAnchor("&"));     // Permissive implementation
  EXPECT_TRUE(isAnchor("&&foo")); // Permissive implementation
  EXPECT_TRUE(isAnchor("& foo")); // Permissive implementation
}

TEST_F(YamlHelperFunctionsTest, AliasDetectionWithMalformedInput) {
  // This test checks that isAlias returns true for permissive/malformed alias strings.
  EXPECT_TRUE(isAlias("*"));     // Permissive implementation
  EXPECT_TRUE(isAlias("**foo")); // Permissive implementation
  EXPECT_TRUE(isAlias("* foo")); // Permissive implementation
}

TEST_F(YamlHelperFunctionsTest, InlineSeqDetectionWithMalformedInput) {
  // This test checks that isInlineSeq returns expected results for malformed/permissive input.
  EXPECT_FALSE(isInlineSeq("["));     // Only opening bracket
  EXPECT_FALSE(isInlineSeq("]"));     // Only closing bracket
  EXPECT_TRUE(isInlineSeq("[,]"));    // Permissive implementation
  EXPECT_TRUE(isInlineSeq("[a,,b]")); // Permissive implementation
}

TEST_F(YamlHelperFunctionsTest, MergeKeyDetectionWithMalformedInput) {
  // This test checks that isMergeKey returns expected results for malformed/permissive input.
  EXPECT_FALSE(isMergeKey("<", "*foo")); // Single less-than
  EXPECT_FALSE(isMergeKey("<<", ""));    // Empty value
  EXPECT_TRUE(isMergeKey("<<", "*"));    // Permissive implementation
}

TEST_F(YamlHelperFunctionsTest, ParseMultilineLiteralWithEmptyOrWhitespaceLines) {
  // This test checks that parseMultilineLiteral handles empty/whitespace-only lines correctly.
  std::vector<std::string> lines1  = {"key: |", "   ", "   ", "other: value"};
  size_t                   idx1    = 0;
  auto                     result1 = parseMultilineLiteral(lines1, idx1, 1, '|');
  EXPECT_EQ(result1.value.asString(), "");

  std::vector<std::string> lines2  = {"key: |", "other: value"};
  size_t                   idx2    = 0;
  auto                     result2 = parseMultilineLiteral(lines2, idx2, 1, '|');
  EXPECT_EQ(result2.value.asString(), "");
}

TEST_F(YamlHelperFunctionsTest, ParseMultilineLiteralWithMissingBlockIndicator) {
  // This test checks that parseMultilineLiteral does not crash if block indicator is missing.
  std::vector<std::string> lines = {"key:", "  line1", "  line2"};
  size_t                   idx   = 0;
  // Should not crash, returns joined lines
  auto result = parseMultilineLiteral(lines, idx, 1, ' ');
  EXPECT_EQ(result.value.asString(), "line1 line2");
}

TEST_F(YamlHelperFunctionsTest, ParseAnchorWithMissingOrUnknownAnchor) {
  // This test checks that parseAnchor throws for malformed or missing anchor names.
  yamlparser::YamlParser          parser;
  std::map<std::string, YamlItem> anchors;
  std::vector<std::string>        lines = {"key: &", "  value"};
  size_t                          idx   = 0;
  // Should throw due to malformed anchor
  EXPECT_THROW(parseAnchor("&", lines, idx, anchors, parser), std::exception);
}

TEST_F(YamlHelperFunctionsTest, ParseAliasWithUnknownAnchor) {
  // This test checks that parseAlias returns NONE/empty for unknown anchor.
  std::map<std::string, YamlItem> anchors;

  EXPECT_THROW(parseAlias("*unknown", anchors), KeyException);
}

TEST_F(YamlHelperFunctionsTest, ParseInlineSeqWithEmptyOrMalformedInput) {
  // This test checks that parseInlineSeq returns correct results or throws for malformed input.
  auto result1 = parseInlineSeq("[]");
  EXPECT_TRUE(result1.value.isSeq());
  EXPECT_EQ(result1.value.asSeq().size(), 0);

  EXPECT_THROW(parseInlineSeq("["), std::exception);

  auto result3 = parseInlineSeq("[1, 'a', true]"); // Mixed types
  EXPECT_TRUE(result3.value.isSeq());
  EXPECT_EQ(result3.value.asSeq().size(), 3);
}
