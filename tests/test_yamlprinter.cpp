

#include <gtest/gtest.h>
#include "YamlPrinter.hpp"
#include "YamlParser.hpp"
#include <sstream>
#include <fstream>
#include <string>

using namespace yamlparser;

class YamlPrinterTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup for each test
  }

  void TearDown() override {
    // Cleanup after each test
  }
};

TEST_F(YamlPrinterTest, SimpleMapPrinting) {
  // Test printing of simple key-value map
  // Verifies that basic map structures are correctly formatted in YAML output

  // Setup: Create map with string and integer values
  YamlMap map;
  map["foo"] = YamlItem(YamlElement(std::string("bar")));
  map["num"] = YamlItem(YamlElement(42));

  // Action: Print map to string stream
  std::stringstream ss;
  YamlPrinter::print(map, ss);
  std::string output = ss.str();

  // Assertions: Check for expected key-value pairs in output
  EXPECT_NE(output.find("foo: bar"), std::string::npos);
  EXPECT_NE(output.find("num: 42"), std::string::npos);
}

TEST_F(YamlPrinterTest, SequencePrinting) {
  // Test printing of YAML sequences
  // Verifies that sequence structures are correctly formatted with dash notation

  // Setup: Create sequence with string elements
  YamlSeq seq;
  seq.push_back(YamlItem(YamlElement(std::string("a"))));
  seq.push_back(YamlItem(YamlElement(std::string("b"))));
  seq.push_back(YamlItem(YamlElement(std::string("c"))));

  // Action: Print sequence to string stream
  std::stringstream ss;
  YamlPrinter::print(seq, ss);
  std::string output = ss.str();

  // Assertions: Check for proper sequence formatting
  EXPECT_NE(output.find("- a"), std::string::npos);
  EXPECT_NE(output.find("- b"), std::string::npos);
  EXPECT_NE(output.find("- c"), std::string::npos);
}

TEST_F(YamlPrinterTest, NestedMapAndSequence) {
  // Test printing of nested map containing sequence
  // Verifies that complex nested structures are properly indented and formatted

  // Setup: Create nested structure with map containing sequence
  YamlMap map;
  YamlSeq seq;
  seq.push_back(YamlItem(YamlElement(1)));
  seq.push_back(YamlItem(YamlElement(2)));
  map["numbers"] = YamlItem(YamlElement(seq));
  map["name"]    = YamlItem(YamlElement(std::string("test")));

  // Action: Print nested structure to string stream
  std::stringstream ss;
  YamlPrinter::print(map, ss);
  std::string output = ss.str();

  // Assertions: Check for proper nested formatting
  EXPECT_NE(output.find("numbers:"), std::string::npos);
  EXPECT_NE(output.find("- 1"), std::string::npos);
  EXPECT_NE(output.find("- 2"), std::string::npos);
  EXPECT_NE(output.find("name: test"), std::string::npos);
}

TEST_F(YamlPrinterTest, NullAndEmptyValues) {
  // Test printing of null and empty values
  // Verifies that empty strings and null values are properly represented

  // Setup: Create map with empty and null values
  YamlMap map;
  map["empty"] = YamlItem(YamlElement(std::string("")));
  map["none"]  = YamlItem(YamlElement());

  // Action: Print map with null/empty values to string stream
  std::stringstream ss;
  YamlPrinter::print(map, ss);
  std::string output = ss.str();

  // Assertions: Check for proper null representation
  EXPECT_NE(output.find("empty: null"), std::string::npos);
  EXPECT_NE(output.find("none: null"), std::string::npos);
}

TEST_F(YamlPrinterTest, ComplexStructurePrinting) {
  // Test printing of complex nested structure with multiple data types
  // Verifies that YAML structures with mixed types are properly formatted

  // Setup: Create complex nested structure with various data types
  YamlMap root;
  root["null"] = YamlItem(YamlElement());

  YamlSeq bool_seq;
  bool_seq.push_back(YamlItem(YamlElement(true)));
  bool_seq.push_back(YamlItem(YamlElement(false)));
  root["booleans"] = YamlItem(YamlElement(bool_seq));

  root["string"] = YamlItem(YamlElement(std::string("012345")));

  YamlSeq int_seq;
  int_seq.push_back(YamlItem(YamlElement(1)));
  int_seq.push_back(YamlItem(YamlElement(2)));
  int_seq.push_back(YamlItem(YamlElement(3)));
  int_seq.push_back(YamlItem(YamlElement(4)));
  root["integers"] = YamlItem(YamlElement(int_seq));

  YamlSeq float_seq;
  float_seq.push_back(YamlItem(YamlElement(1.2)));
  float_seq.push_back(YamlItem(YamlElement(3.4)));
  float_seq.push_back(YamlItem(YamlElement(5.6)));
  root["floats"] = YamlItem(YamlElement(float_seq));

  // Action: Print complex structure to string stream
  std::stringstream ss;
  YamlPrinter::print(root, ss);
  std::string output = ss.str();

  // Assertions: Check for all expected elements in proper format
  EXPECT_NE(output.find("null: null"), std::string::npos);
  EXPECT_NE(output.find("booleans:"), std::string::npos);
  EXPECT_NE(output.find("- true"), std::string::npos);
  EXPECT_NE(output.find("- false"), std::string::npos);
  EXPECT_NE(output.find("string: 012345"), std::string::npos);
  EXPECT_NE(output.find("integers:"), std::string::npos);
  EXPECT_NE(output.find("- 1"), std::string::npos);
  EXPECT_NE(output.find("- 2"), std::string::npos);
  EXPECT_NE(output.find("- 3"), std::string::npos);
  EXPECT_NE(output.find("- 4"), std::string::npos);
  EXPECT_NE(output.find("floats:"), std::string::npos);
  EXPECT_NE(output.find("- 1.2"), std::string::npos);
  EXPECT_NE(output.find("- 3.4"), std::string::npos);
  EXPECT_NE(output.find("- 5.6"), std::string::npos);
}

// --- Additional YamlPrinter edge/round-trip tests ---

TEST_F(YamlPrinterTest, PrintEmptyMapAndSequence) {
  YamlMap           emptyMap;
  std::stringstream ssMap;
  YamlPrinter::print(emptyMap, ssMap);
  std::string mapOut = ssMap.str();
  EXPECT_TRUE(mapOut.empty() || mapOut.find_first_not_of(" \t\n\r") == std::string::npos);

  YamlSeq           emptySeq;
  std::stringstream ssSeq;
  YamlPrinter::print(emptySeq, ssSeq);
  std::string seqOut = ssSeq.str();
  EXPECT_TRUE(seqOut.empty() || seqOut.find_first_not_of(" \t\n\r") == std::string::npos);
}

TEST_F(YamlPrinterTest, PrintNoneTypeElement) {
  YamlItem          noneItem((YamlElement()));
  std::stringstream ss;
  YamlPrinter::print(noneItem, ss);
  std::string out = ss.str();
  EXPECT_NE(out.find("null"), std::string::npos);
}

TEST_F(YamlPrinterTest, RoundTripParsePrintParse) {
  std::string   yaml = "foo: 1\nbar:\n  - a\n  - b\nempty: null\n";
  YamlParser    parser1;
  std::string   fname = "test_roundtrip.yaml";
  std::ofstream ofs(fname);
  ofs << yaml;
  ofs.close();
  parser1.parse(fname);
  std::stringstream ss;
  YamlPrinter::print(parser1.root(), ss);
  std::string   printed = ss.str();
  YamlParser    parser2;
  std::ofstream ofs2("test_roundtrip2.yaml");
  ofs2 << printed;
  ofs2.close();
  parser2.parse("test_roundtrip2.yaml");
  auto &root1 = parser1.root();
  auto &root2 = parser2.root();
  EXPECT_EQ(root1.size(), root2.size());
  for (const auto &kv : root1) {
    EXPECT_TRUE(root2.find(kv.first) != root2.end());
    EXPECT_EQ(kv.second.value.type, root2.at(kv.first).value.type);
  }
  std::remove(fname.c_str());
  std::remove("test_roundtrip2.yaml");
}

TEST_F(YamlPrinterTest, PrintDeeplyNestedStructure) {
  YamlMap root;
  YamlMap level1;
  YamlMap level2;
  YamlSeq seq;
  seq.push_back(YamlItem(YamlElement(std::string("deep"))));
  seq.push_back(YamlItem(YamlElement(42)));
  level2["seq"]    = YamlItem(YamlElement(seq));
  level1["level2"] = YamlItem(YamlElement(level2));
  root["level1"]   = YamlItem(YamlElement(level1));
  std::stringstream ss;
  YamlPrinter::print(root, ss);
  std::string out = ss.str();
  EXPECT_NE(out.find("level1:"), std::string::npos);
  EXPECT_NE(out.find("level2:"), std::string::npos);
  EXPECT_NE(out.find("- deep"), std::string::npos);
  EXPECT_NE(out.find("- 42"), std::string::npos);
}