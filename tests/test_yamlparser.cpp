#include "YamlParser.hpp"

#include <gtest/gtest.h>

#include <fstream>

using namespace yamlparser;

TEST(YamlParserTest, ParsesFilesAndTextIntoTheSameDocumentShape) {
  const char *filename = "parser_file_test.yaml";
  {
    std::ofstream output(filename);
    output << "name: example\nenabled: true\nports: [80, 443]\n";
  }

  const YamlValue document = YamlParser().parseFile(filename);
  std::remove(filename);

  EXPECT_EQ(document.at("name").asString(), "example");
  EXPECT_TRUE(document.at("enabled").asBoolean());
  EXPECT_EQ(document.at("ports").at(1U).asInteger(), 443);
}

TEST(YamlParserTest, ReturnsAnEmptyMappingForAnEmptyDocument) {
  const YamlValue document = YamlParser().parseText("\n # comment only\n");
  EXPECT_TRUE(document.isMapping());
  EXPECT_TRUE(document.asMapping().empty());
}

TEST(YamlParserTest, ParsesMappingAndSequenceRoots) {
  const YamlParser parser;
  const YamlValue  mapping  = parser.parseText("name: value\n");
  const YamlValue  sequence = parser.parseText("- one\n- two\n");

  EXPECT_EQ(mapping.at("name").asString(), "value");
  EXPECT_EQ(sequence.at(1U).asString(), "two");
}

TEST(YamlParserTest, ParsesNestedMappingsAndSequencesRecursively) {
  const YamlValue document = YamlParser().parseText("application:\n"
                                                    "  servers:\n"
                                                    "    - name: primary\n"
                                                    "      ports:\n"
                                                    "        - 8080\n"
                                                    "        - 8443\n"
                                                    "    - name: backup\n"
                                                    "      ports: [9080]\n");

  const YamlValue &servers = document.at("application").at("servers");
  EXPECT_EQ(servers.at(0U).at("name").asString(), "primary");
  EXPECT_EQ(servers.at(0U).at("ports").at(1U).asInteger(), 8443);
  EXPECT_EQ(servers.at(1U).at("ports").at(0U).asInteger(), 9080);
}

TEST(YamlParserTest, ParsesNestedBlockSequencesAsSequences) {
  const YamlValue document = YamlParser().parseText("matrix:\n"
                                                    "  - - 1\n"
                                                    "    - 2\n"
                                                    "  - - 3\n"
                                                    "    - 4\n");

  const YamlValue &matrix = document.at("matrix");
  ASSERT_TRUE(matrix.at(0U).isSequence());
  EXPECT_EQ(matrix.at(0U).at(1U).asInteger(), 2);
  EXPECT_EQ(matrix.at(1U).at(0U).asInteger(), 3);
}

TEST(YamlParserTest, ResolvesAnchorsAliasesAndMergeKeysPerDocument) {
  const YamlParser parser;
  const YamlValue  document = parser.parseText("defaults: &defaults\n"
                                                "  timeout: 30\n"
                                                "  retries: 3\n"
                                                "service:\n"
                                                "  <<: *defaults # comment after merge\n"
                                                "  timeout: 60\n"
                                                "copy: *defaults\n");

  EXPECT_EQ(document.at("service").at("timeout").asInteger(), 60);
  EXPECT_EQ(document.at("service").at("retries").asInteger(), 3);
  EXPECT_EQ(document.at("copy").at("timeout").asInteger(), 30);
  EXPECT_THROW(parser.parseText("value: *defaults\n"), KeyException);
}

TEST(YamlParserTest, ParsesScalarAnchors) {
  const YamlValue document = YamlParser().parseText("original: &answer 42\ncopy: *answer\n");
  EXPECT_EQ(document.at("original").asInteger(), 42);
  EXPECT_EQ(document.at("copy").asInteger(), 42);
}

TEST(YamlParserTest, ParsesNullAndQuotedEmptyStringAsDifferentTypes) {
  const YamlValue document = YamlParser().parseText("implicit:\nexplicit: null\nempty: ''\n");
  EXPECT_TRUE(document.at("implicit").isNull());
  EXPECT_TRUE(document.at("explicit").isNull());
  EXPECT_TRUE(document.at("empty").isString());
  EXPECT_TRUE(document.at("empty").asString().empty());
}

TEST(YamlParserTest, DecodesCommonEscapesAndPreservesUtf8) {
  const YamlValue document = YamlParser().parseText(u8"text: \"tab\\tline\\npound £\"\n");
  EXPECT_EQ(document.at("text").asString(), u8"tab\tline\npound £");
}

TEST(YamlParserTest, ReportsFileAndAliasErrors) {
  const YamlParser parser;
  EXPECT_THROW(parser.parseFile("file_that_does_not_exist.yaml"), FileException);
  EXPECT_THROW(parser.parseText("value: *missing\n"), KeyException);
}
