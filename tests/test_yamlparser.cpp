
#include <gtest/gtest.h>
#include "YamlParser.hpp"
#include "YamlException.hpp"
#include "YamlPrinter.hpp"
#include <string>
#include <fstream>

using namespace yamlparser;

class YamlParserTest : public ::testing::Test {
protected:
  YamlParser parser;

  void SetUp() override {
    // No special setup needed for YamlParser tests
  }

  void TearDown() override {
    // No cleanup needed for YamlParser tests
  }
};

TEST_F(YamlParserTest, ParseEmptyFile) {
  std::ofstream ofs("test_emptyfile.yaml");
  ofs.close();
  EXPECT_NO_THROW(parser.parse("test_emptyfile.yaml"));
  auto &root = parser.root();
  EXPECT_TRUE(root.empty());
  std::remove("test_emptyfile.yaml");
}

TEST_F(YamlParserTest, ParseCommentsOnlyFile) {
  std::string   yaml = "# This is a comment\n# Another comment\n   # Indented comment\n";
  std::ofstream ofs("test_commentsonly.yaml");
  ofs << yaml;
  ofs.close();
  EXPECT_NO_THROW(parser.parse("test_commentsonly.yaml"));
  auto &root = parser.root();
  EXPECT_TRUE(root.empty());
  std::remove("test_commentsonly.yaml");
}

TEST_F(YamlParserTest, ParseInvalidYamlSyntax) {
  std::string   yaml = "foo: [1, 2, 3\nbar: value"; // missing closing bracket
  std::ofstream ofs("test_invalidyaml.yaml");
  ofs << yaml;
  ofs.close();
  // Parser should throw SyntaxException for invalid YAML
  EXPECT_THROW(parser.parse("test_invalidyaml.yaml"), SyntaxException);
  std::remove("test_invalidyaml.yaml");
}

TEST_F(YamlParserTest, ParseDeeplyNestedStructure) {
  std::string   yaml = "a:\n  b:\n    c:\n      d:\n        e: value\n";
  std::ofstream ofs("test_deepnest.yaml");
  ofs << yaml;
  ofs.close();
  EXPECT_NO_THROW(parser.parse("test_deepnest.yaml"));
  auto &root = parser.root();
  ASSERT_TRUE(root.find("a") != root.end());
  const auto &b = root.at("a").value.asMap();
  ASSERT_TRUE(b.find("b") != b.end());
  const auto &c = b.at("b").value.asMap();
  ASSERT_TRUE(c.find("c") != c.end());
  const auto &d = c.at("c").value.asMap();
  ASSERT_TRUE(d.find("d") != d.end());
  const auto &e = d.at("d").value.asMap();
  ASSERT_TRUE(e.find("e") != e.end());
  EXPECT_EQ(e.at("e").value.asString(), "value");
  std::remove("test_deepnest.yaml");
}

TEST_F(YamlParserTest, ParseVeryLargeFile) {
  std::ofstream ofs("test_large.yaml");
  for (int i = 0; i < 10000; ++i) {
    ofs << "key" << i << ": value" << i << "\n";
  }
  ofs.close();
  EXPECT_NO_THROW(parser.parse("test_large.yaml"));
  auto &root = parser.root();
  ASSERT_EQ(root.size(), 10000);
  EXPECT_EQ(root.at("key9999").value.asString(), "value9999");
  std::remove("test_large.yaml");
}

TEST_F(YamlParserTest, ParseFileWithNoReadPermission) {
  std::string   yaml = "foo: bar\n";
  std::ofstream ofs("test_noperm.yaml");
  ofs << yaml;
  ofs.close();
  chmod("test_noperm.yaml", 0); // Remove all permissions
  EXPECT_THROW(parser.parse("test_noperm.yaml"), FileException);
  chmod("test_noperm.yaml", 0644); // Restore permissions for cleanup
  std::remove("test_noperm.yaml");
}

TEST_F(YamlParserTest, ParseCorruptedInput) {
  std::ofstream ofs("test_corrupt.yaml", std::ios::binary);
  char          data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  ofs.write(data, 8);
  ofs.close();
  EXPECT_THROW(parser.parse("test_corrupt.yaml"), std::exception);
  std::remove("test_corrupt.yaml");
}

TEST_F(YamlParserTest, ParseDuplicateKeys) {
  std::string   yaml = "foo: 1\nfoo: 2\n";
  std::ofstream ofs("test_dupekey.yaml");
  ofs << yaml;
  ofs.close();
  // Parser should throw SyntaxException for duplicate mapping keys
  EXPECT_THROW(parser.parse("test_dupekey.yaml"), SyntaxException);
  std::remove("test_dupekey.yaml");
}

TEST_F(YamlParserTest, ParseInvalidAnchorOrAlias) {
  std::string   yaml = "foo: *unknown\n";
  std::ofstream ofs("test_badanchor.yaml");
  ofs << yaml;
  ofs.close();
  EXPECT_NO_THROW(parser.parse("test_badanchor.yaml"));
  auto &root = parser.root();
  ASSERT_TRUE(root.find("foo") != root.end());
  // Should be empty or NONE for unresolved alias
  EXPECT_TRUE(root.at("foo").value.type == YamlElement::ElementType::NONE || root.at("foo").value.asString() == "");
  std::remove("test_badanchor.yaml");
}

TEST_F(YamlParserTest, FileNotFound) {
  // Test file not found exception handling
  // Verifies that YamlParser throws FileException when attempting to parse a non-existent file
  // Setup: Use non-existent file path

  // Action & Assertion: Parsing non-existent file should throw FileException
  EXPECT_THROW(parser.parse("nonexistent_file.yaml"), FileException);
}

TEST_F(YamlParserTest, SkipEmptyLines) {
  // Test parsing file with only empty lines
  // Verifies that YamlParser correctly handles YAML files containing only empty lines
  // and whitespace, resulting in an empty root map

  // Setup: Create YAML file with only empty lines and whitespace
  std::string   yaml = "\n\n   \t  \n\n";
  std::ofstream ofs("test_empty.yaml");
  ofs << yaml;
  ofs.close();

  // Action: Parse the empty file
  EXPECT_NO_THROW(parser.parse("test_empty.yaml"));
  auto &root = parser.root();

  // Assertion: Root should be empty
  EXPECT_TRUE(root.empty());

  // Cleanup
  std::remove("test_empty.yaml");
}

TEST_F(YamlParserTest, SequenceAtMappingLevel) {
  // Test sequence at mapping level triggering parser break
  // Verifies that when a sequence appears at the same indentation level as a mapping,
  // the parser correctly stops processing the mapping

  // Setup: Create YAML with mapping followed by sequence at same level
  std::string   yaml = "foo: bar\n- baz\n";
  std::ofstream ofs("test_seq_at_map.yaml");
  ofs << yaml;
  ofs.close();

  // Action: Parse the YAML
  EXPECT_NO_THROW(parser.parse("test_seq_at_map.yaml"));
  auto &root = parser.root();

  // Assertions: Should only parse the mapping, sequence triggers break
  ASSERT_TRUE(root.find("foo") != root.end());
  EXPECT_TRUE(root.at("foo").value.isString());
  EXPECT_EQ(root.at("foo").value.asString(), "bar");
  // Sequence should not be parsed as a mapping key
  EXPECT_TRUE(root.find("baz") == root.end());

  // Cleanup
  std::remove("test_seq_at_map.yaml");
}

TEST_F(YamlParserTest, EmptyValueFollowedBySequence) {
  // Test mapping key with empty value followed by sequence
  // Verifies that a mapping key with no immediate value can have a sequence
  // as its value on subsequent indented lines

  // Setup: Create YAML with mapping key with empty value, followed by sequence
  std::string   yaml = "foo:\n  - bar\n  - baz\n";
  std::ofstream ofs("test_map_seq.yaml");
  ofs << yaml;
  ofs.close();

  // Action: Parse the YAML
  EXPECT_NO_THROW(parser.parse("test_map_seq.yaml"));
  auto &root = parser.root();

  // Assertions: foo should map to a sequence with two elements
  ASSERT_TRUE(root.find("foo") != root.end());
  EXPECT_TRUE(root.at("foo").value.isSeq());
  auto &seq = root.at("foo").value.asSeq();
  ASSERT_EQ(seq.size(), 2);
  EXPECT_TRUE(seq[0].value.isString());
  EXPECT_TRUE(seq[1].value.isString());
  EXPECT_EQ(seq[0].value.asString(), "bar");
  EXPECT_EQ(seq[1].value.asString(), "baz");

  // Cleanup
  std::remove("test_map_seq.yaml");
}

TEST_F(YamlParserTest, MultilineStringValue) {
  // Test parsing multiline string values in mappings
  // Verifies that the parser correctly handles block scalar indicators (like |)
  // for multiline string values

  // Setup: Create YAML with folded block scalar (multiline string)
  std::string   yaml = "foo: |\n  This is line one.\n  This is line two.\n  This is line three.\n";
  std::ofstream ofs("test_multiline.yaml");
  ofs << yaml;
  ofs.close();

  // Action: Parse the YAML
  EXPECT_NO_THROW(parser.parse("test_multiline.yaml"));
  auto &root = parser.root();

  // Assertions: foo should contain all three lines
  ASSERT_TRUE(root.find("foo") != root.end());
  EXPECT_TRUE(root.at("foo").value.isString());
  std::string val = root.at("foo").value.asString();
  EXPECT_NE(val.find("This is line one."), std::string::npos);
  EXPECT_NE(val.find("This is line two."), std::string::npos);
  EXPECT_NE(val.find("This is line three."), std::string::npos);

  // Cleanup
  std::remove("test_multiline.yaml");
}

TEST_F(YamlParserTest, NestedMapping) {
  // Test nested mapping parsing
  // Verifies that the parser correctly handles nested mappings where a mapping
  // value is itself another mapping

  // Setup: Create YAML with nested mapping structure
  std::string   yaml = "parent:\n  child1: value1\n  child2: value2\n";
  std::ofstream ofs("test_nested_map.yaml");
  ofs << yaml;
  ofs.close();

  // Action: Parse the YAML
  EXPECT_NO_THROW(parser.parse("test_nested_map.yaml"));
  auto &root = parser.root();

  // Assertions: parent should contain nested mapping with two children
  ASSERT_TRUE(root.find("parent") != root.end());
  EXPECT_TRUE(root.at("parent").value.isMap());
  auto &nested = root.at("parent").value.asMap();
  ASSERT_TRUE(nested.find("child1") != nested.end());
  ASSERT_TRUE(nested.find("child2") != nested.end());
  EXPECT_TRUE(nested.at("child1").value.isString());
  EXPECT_TRUE(nested.at("child2").value.isString());
  EXPECT_EQ(nested.at("child1").value.asString(), "value1");
  EXPECT_EQ(nested.at("child2").value.asString(), "value2");

  // Cleanup
  std::remove("test_nested_map.yaml");
}

TEST_F(YamlParserTest, AnchorSequence) {
  // Test anchor assignment to sequence values
  // Verifies that the parser correctly handles YAML anchors assigned to sequence values

  // Setup: Create YAML with anchor assigned to a sequence
  std::string   yaml = "seq_anchor: &myseq\n  - item1\n  - item2\n";
  std::ofstream ofs("test_anchor_seq.yaml");
  ofs << yaml;
  ofs.close();

  // Action: Parse the YAML
  EXPECT_NO_THROW(parser.parse("test_anchor_seq.yaml"));
  auto &root = parser.root();

  // Assertions: seq_anchor should contain sequence with anchor
  ASSERT_TRUE(root.find("seq_anchor") != root.end());
  EXPECT_TRUE(root.at("seq_anchor").value.isSeq());
  auto &seq = root.at("seq_anchor").value.asSeq();
  ASSERT_EQ(seq.size(), 2);
  EXPECT_TRUE(seq[0].value.isString());
  EXPECT_TRUE(seq[1].value.isString());
  EXPECT_EQ(seq[0].value.asString(), "item1");
  EXPECT_EQ(seq[1].value.asString(), "item2");

  // Cleanup
  std::remove("test_anchor_seq.yaml");
}

TEST_F(YamlParserTest, ExplicitNullValue) {
  // Test explicit null mapping value followed by another key
  // Verifies that a mapping key with no value (implicit null) is correctly parsed,
  // followed by parsing the next key-value pair

  // Setup: Create YAML with null value followed by another key
  std::string   yaml = "foo:\nbar: value\n";
  std::ofstream ofs("test_explicit_null_followed.yaml");
  ofs << yaml;
  ofs.close();

  // Action: Parse the YAML
  EXPECT_NO_THROW(parser.parse("test_explicit_null_followed.yaml"));
  auto &root = parser.root();

  // Assertions: foo should have empty string value, bar should have "value"
  ASSERT_TRUE(root.find("foo") != root.end());
  EXPECT_TRUE(root.at("foo").value.isString());
  EXPECT_EQ(root.at("foo").value.asString(), "");
  ASSERT_TRUE(root.find("bar") != root.end());
  EXPECT_EQ(root.at("bar").value.asString(), "value");

  // Cleanup
  std::remove("test_explicit_null_followed.yaml");
}

TEST_F(YamlParserTest, InlineArrayInNestedSequence) {
  // Test that inline arrays in nested sequences are parsed as sequences, not strings
  std::string   yaml = "matrix:\n  - [1, 2, 3]\n  - [4, 5, 6]\n  - [7, 8, 9]\n";
  std::ofstream ofs("test_inline_matrix.yaml");
  ofs << yaml;
  ofs.close();

  EXPECT_NO_THROW(parser.parse("test_inline_matrix.yaml"));
  auto &root = parser.root();
  ASSERT_TRUE(root.find("matrix") != root.end());
  EXPECT_TRUE(root.at("matrix").value.isSeq());
  auto &matrix = root.at("matrix").value.asSeq();
  ASSERT_EQ(matrix.size(), 3);
  for (size_t i = 0; i < matrix.size(); ++i) {
    EXPECT_TRUE(matrix[i].value.isSeq());
    auto &row = matrix[i].value.asSeq();
    ASSERT_EQ(row.size(), 3);
    for (size_t j = 0; j < row.size(); ++j) {
      EXPECT_TRUE(row[j].value.isInt());
    }
  }
  // Cleanup
  std::remove("test_inline_matrix.yaml");
}
