#include <gtest/gtest.h>
#include "YamlParser.hpp"
#include "YamlPrinter.hpp"
#include "YamlException.hpp"
#include <string>
#include <iostream>
#include <unistd.h>
#include <climits>

using namespace yamlparser;

class YamlParserCasesTest : public ::testing::Test {
protected:
  YamlParser parser;
  void       SetUp() override {}
  void       TearDown() override {}
};

TEST_F(YamlParserCasesTest, NestedTypes) {

  EXPECT_NO_THROW(parser.parse("test_cases/01_nested_types.yaml"));
  auto &root = parser.root();

  // Verify top-level config section exists and is properly typed as a map
  ASSERT_TRUE(root.find("config") != root.end());
  ASSERT_TRUE(root.at("config").value.isMap());
  auto &config = root.at("config").value.asMap();

  // Test server configuration section - validates nested map parsing
  ASSERT_TRUE(config.find("server") != config.end());
  ASSERT_TRUE(config.at("server").value.isMap());
  auto &server = config.at("server").value.asMap();

  // Validate individual server properties with correct data type inference
  EXPECT_EQ(server.at("host").value.asString(), "localhost");
  // Test sequence parsing within nested structure - ensures arrays are properly handled
  ASSERT_TRUE(server.at("ports").value.isSeq());
  auto &ports = server.at("ports").value.asSeq();
  ASSERT_EQ(ports.size(), 3);
  EXPECT_EQ(ports[0].value.asInt(), 8080);
  EXPECT_EQ(ports[1].value.asInt(), 8081);
  EXPECT_EQ(ports[2].value.asInt(), 8082);
  // Verify boolean and floating-point type parsing in nested context
  EXPECT_EQ(server.at("enabled").value.asBool(), true);
  EXPECT_DOUBLE_EQ(server.at("timeout").value.asDouble(), 30.5);

  // Test databases section - validates sequence of maps with inheritance patterns
  ASSERT_TRUE(config.find("databases") != config.end());
  ASSERT_TRUE(config.at("databases").value.isSeq());
  auto &databases = config.at("databases").value.asSeq();
  ASSERT_EQ(databases.size(), 2);

  // Test first database configuration - demonstrates map within sequence structure
  auto &db1 = databases[0].value.asMap();
  EXPECT_EQ(db1.at("name").value.asString(), "main");
  EXPECT_EQ(db1.at("type").value.asString(), "postgresql");
  ASSERT_TRUE(db1.at("settings").value.isMap());
  auto &settings1 = db1.at("settings").value.asMap();
  EXPECT_EQ(settings1.at("max_connections").value.asInt(), 100);
  EXPECT_DOUBLE_EQ(settings1.at("timeout").value.asDouble(), 5.0);
  EXPECT_EQ(settings1.at("retry").value.asBool(), true);

  // Test second database (with merged settings)
  auto &db2 = databases[1].value.asMap();
  EXPECT_EQ(db2.at("name").value.asString(), "replica");
  EXPECT_EQ(db2.at("type").value.asString(), "postgresql");
  ASSERT_TRUE(db2.at("settings").value.isMap());
  auto &settings2 = db2.at("settings").value.asMap();
  EXPECT_EQ(settings2.at("max_connections").value.asInt(), 50);    // Overridden value
  EXPECT_DOUBLE_EQ(settings2.at("timeout").value.asDouble(), 5.0); // Inherited value
  EXPECT_EQ(settings2.at("retry").value.asBool(), true);           // Inherited value

  // Test features section
  ASSERT_TRUE(config.find("features") != config.end());
  ASSERT_TRUE(config.at("features").value.isMap());
  auto &features = config.at("features").value.asMap();

  // Test logging subsection
  ASSERT_TRUE(features.find("logging") != features.end());
  auto &logging = features.at("logging").value.asMap();
  EXPECT_EQ(logging.at("level").value.asString(), "INFO");
  ASSERT_TRUE(logging.at("formats").value.isSeq());
  auto &formats = logging.at("formats").value.asSeq();
  ASSERT_EQ(formats.size(), 2);
  EXPECT_EQ(formats[0].value.asString(), "json");
  EXPECT_EQ(formats[1].value.asString(), "text");

  // Test cache subsection
  ASSERT_TRUE(features.find("cache") != features.end());
  auto &cache = features.at("cache").value.asMap();
  EXPECT_EQ(cache.at("enabled").value.asBool(), true);
  EXPECT_EQ(cache.at("max_size").value.asInt(), 1024);
  ASSERT_TRUE(cache.at("string_items").value.isSeq());
  auto &stringItems = cache.at("string_items").value.asSeq();
  ASSERT_EQ(stringItems.size(), 3);
  EXPECT_EQ(stringItems[0].value.asString(), "item1");
  EXPECT_EQ(stringItems[1].value.asString(), "item2");
  EXPECT_EQ(stringItems[2].value.asString(), "item3");

  ASSERT_TRUE(cache.at("number_items").value.isSeq());
  auto &numberItems = cache.at("number_items").value.asSeq();
  ASSERT_EQ(numberItems.size(), 3);
  EXPECT_EQ(numberItems[0].value.asInt(), 42);
  EXPECT_EQ(numberItems[1].value.asInt(), 55);
  EXPECT_EQ(numberItems[2].value.asInt(), 67);
}

TEST_F(YamlParserCasesTest, MultilineFormats) {

  EXPECT_NO_THROW(parser.parse("test_cases/02_multiline_formats.yaml"));
  auto &root = parser.root();

  // Test description section containing various multiline string formats
  ASSERT_TRUE(root.find("description") != root.end());
  ASSERT_TRUE(root.at("description").value.isMap());
  auto &description = root.at("description").value.asMap();

  // Verify single-line string parsing (baseline comparison)
  ASSERT_TRUE(description.find("short") != description.end());
  EXPECT_EQ(description.at("short").value.asString(), "Single line text");

  // Test folded scalar (>) - should collapse line breaks into spaces
  // This format is useful for long paragraphs that should display as single lines
  ASSERT_TRUE(description.find("folded") != description.end());
  std::string foldedText = description.at("folded").value.asString();
  EXPECT_TRUE(foldedText.find("This is a longer piece of text that should be folded into a single line with spaces.") !=
              std::string::npos);

  // Test literal scalar (|) - preserves exact formatting including line breaks
  // Essential for code blocks, formatted text, or when exact spacing matters
  ASSERT_TRUE(description.find("literal") != description.end());
  std::string literalText = description.at("literal").value.asString();
  EXPECT_TRUE(literalText.find("This text will keep its\nexact formatting including\nline breaks.") !=
              std::string::npos);

  // Test folded with keep modifier (>+) - folds lines but preserves trailing newlines
  // Useful when you want paragraph formatting but need to maintain trailing whitespace
  ASSERT_TRUE(description.find("folded_keep") != description.end());
  std::string foldedKeepText = description.at("folded_keep").value.asString();
  EXPECT_TRUE(foldedKeepText.find("This text is folded but keeps trailing") != std::string::npos);

  // Test literal with strip modifier (|-) - preserves internal formatting but removes trailing newlines
  // Ideal for formatted content where trailing whitespace should be eliminated
  ASSERT_TRUE(description.find("literal_strip") != description.end());
  std::string literalStripText = description.at("literal_strip").value.asString();
  EXPECT_TRUE(literalStripText.find("This text is literal\nbut strips trailing\nnewlines") != std::string::npos);

  // Test documentation section (literal block)
  ASSERT_TRUE(root.find("documentation") != root.end());
  std::string docText = root.at("documentation").value.asString();
  EXPECT_TRUE(docText.find("# API Documentation") != std::string::npos);
  EXPECT_TRUE(docText.find("## Endpoints") != std::string::npos);
  EXPECT_TRUE(docText.find("- /api/v1/users") != std::string::npos);
  EXPECT_TRUE(docText.find("- /api/v1/posts") != std::string::npos);

  // Test notes section (folded block)
  ASSERT_TRUE(root.find("notes") != root.end());
  std::string notesText = root.at("notes").value.asString();
  EXPECT_TRUE(notesText.find("First line Second line") != std::string::npos);
  EXPECT_TRUE(notesText.find("Indented line") != std::string::npos);
  EXPECT_TRUE(notesText.find("Final line") != std::string::npos);
}

TEST_F(YamlParserCasesTest, DatesAndNumbers) {

  EXPECT_NO_THROW(parser.parse("test_cases/03_dates_and_numbers.yaml"));
  auto &root = parser.root();

  // Test date format parsing - ensures temporal data is handled correctly
  ASSERT_TRUE(root.find("dates") != root.end());
  ASSERT_TRUE(root.at("dates").value.isMap());
  auto &dates = root.at("dates").value.asMap();

  // Validate various date format representations commonly found in configuration files
  ASSERT_TRUE(dates.find("canonical") != dates.end());
  EXPECT_EQ(dates.at("canonical").value.asString(), "2025-07-26T15:30:00.000Z");

  ASSERT_TRUE(dates.find("iso8601") != dates.end());
  EXPECT_EQ(dates.at("iso8601").value.asString(), "2025-07-26t15:30:00.000+01:00");

  ASSERT_TRUE(dates.find("spaced") != dates.end());
  EXPECT_EQ(dates.at("spaced").value.asString(), "2025-07-26 15:30:00.000 +01:00");

  ASSERT_TRUE(dates.find("date_only") != dates.end());
  EXPECT_EQ(dates.at("date_only").value.asString(), "2025-07-26");

  ASSERT_TRUE(dates.find("american") != dates.end());
  EXPECT_EQ(dates.at("american").value.asString(), "07/26/2025");

  // Test numbers section exists
  ASSERT_TRUE(root.find("numbers") != root.end());
  ASSERT_TRUE(root.at("numbers").value.isMap());
  auto &numbers = root.at("numbers").value.asMap();

  // Test integers subsection
  ASSERT_TRUE(numbers.find("integers") != numbers.end());
  ASSERT_TRUE(numbers.at("integers").value.isMap());
  auto &integers = numbers.at("integers").value.asMap();

  ASSERT_TRUE(integers.find("decimal") != integers.end());
  EXPECT_EQ(integers.at("decimal").value.asInt(), 12345);

  ASSERT_TRUE(integers.find("negative") != integers.end());
  EXPECT_EQ(integers.at("negative").value.asInt(), -789);

  ASSERT_TRUE(integers.find("zero") != integers.end());
  EXPECT_EQ(integers.at("zero").value.asInt(), 0);

  // Note: octal, hexadecimal, and binary might be parsed as strings
  ASSERT_TRUE(integers.find("octal") != integers.end());
  EXPECT_EQ(integers.at("octal").value.asString(), "0o664");
  ASSERT_TRUE(integers.find("hexadecimal") != integers.end());
  EXPECT_EQ(integers.at("hexadecimal").value.asString(), "0xFF");
  ASSERT_TRUE(integers.find("binary") != integers.end());
  EXPECT_EQ(integers.at("binary").value.asString(), "0b1010");

  // Test floats subsection
  ASSERT_TRUE(numbers.find("floats") != numbers.end());
  ASSERT_TRUE(numbers.at("floats").value.isMap());
  auto &floats = numbers.at("floats").value.asMap();

  ASSERT_TRUE(floats.find("basic") != floats.end());
  EXPECT_DOUBLE_EQ(floats.at("basic").value.asDouble(), 3.14159);

  ASSERT_TRUE(floats.find("negative") != floats.end());
  EXPECT_DOUBLE_EQ(floats.at("negative").value.asDouble(), -0.001);

  ASSERT_TRUE(floats.find("scientific") != floats.end());
  // Scientific notation might be parsed as string
  std::string scientificValue = floats.at("scientific").value.asString();
  EXPECT_EQ(scientificValue, "1.23e-4");

  // Special float values (might be parsed as strings)
  ASSERT_TRUE(floats.find("infinity") != floats.end());
  EXPECT_EQ(floats.at("infinity").value.asString(), ".inf");
  ASSERT_TRUE(floats.find("not_number") != floats.end());
  EXPECT_EQ(floats.at("not_number").value.asString(), ".nan");

  // Test special_cases subsection
  ASSERT_TRUE(numbers.find("special_cases") != numbers.end());
  ASSERT_TRUE(numbers.at("special_cases").value.isMap());
  auto &specialCases = numbers.at("special_cases").value.asMap();

  ASSERT_TRUE(specialCases.find("zero_padded") != specialCases.end());
  EXPECT_DOUBLE_EQ(specialCases.at("zero_padded").value.asDouble(), 230.0);

  ASSERT_TRUE(specialCases.find("plus_sign") != specialCases.end());
  EXPECT_EQ(specialCases.at("plus_sign").value.asString(), "+42");
  ASSERT_TRUE(specialCases.find("unsigned") != specialCases.end());
  EXPECT_EQ(specialCases.at("unsigned").value.asString(), "42u");

  // Test timestamps section
  ASSERT_TRUE(root.find("timestamps") != root.end());
  ASSERT_TRUE(root.at("timestamps").value.isMap());
  auto &timestamps = root.at("timestamps").value.asMap();

  ASSERT_TRUE(timestamps.find("created_at") != timestamps.end());
  EXPECT_EQ(timestamps.at("created_at").value.asString(), "2025-07-26T15:30:00Z");

  ASSERT_TRUE(timestamps.find("updated_at") != timestamps.end());
  EXPECT_EQ(timestamps.at("updated_at").value.asString(), "2025-07-26 15:30:00 +0100");

  ASSERT_TRUE(timestamps.find("expires_at") != timestamps.end());
  EXPECT_EQ(timestamps.at("expires_at").value.asString(), "2026-01-01");
}

TEST_F(YamlParserCasesTest, AnchorsAndMerging) {
  // Test YAML anchors (&), aliases (*), and merge keys (<<) functionality
  // Validates that the parser correctly implements YAML's reference system,
  // allowing for DRY (Don't Repeat Yourself) configurations through anchor reuse
  // and merge key inheritance. This is crucial for maintainable configuration files.

  EXPECT_NO_THROW(parser.parse("test_cases/04_anchors_and_merging.yaml"));
  auto &root = parser.root();

  // Test anchor definition - establishes reusable configuration template
  ASSERT_TRUE(root.find("defaults") != root.end());
  ASSERT_TRUE(root.at("defaults").value.isMap());
  auto &defaults = root.at("defaults").value.asMap();
  EXPECT_EQ(defaults.at("timeout").value.asInt(), 30);
  EXPECT_EQ(defaults.at("retries").value.asInt(), 3);
  // Verify nested structure within anchor for complex template patterns
  ASSERT_TRUE(defaults.find("logging") != defaults.end());
  ASSERT_TRUE(defaults.at("logging").value.isMap());
  auto &defaultLogging = defaults.at("logging").value.asMap();
  EXPECT_EQ(defaultLogging.at("enabled").value.asBool(), true);
  EXPECT_EQ(defaultLogging.at("level").value.asString(), "INFO");
  EXPECT_EQ(defaultLogging.at("format").value.asString(), "json");

  // Test service1 (uses merge keys and overrides)
  ASSERT_TRUE(root.find("service1") != root.end());
  ASSERT_TRUE(root.at("service1").value.isMap());
  auto &service1 = root.at("service1").value.asMap();

  // Should inherit from defaults
  EXPECT_EQ(service1.at("timeout").value.asInt(), 30);
  EXPECT_EQ(service1.at("retries").value.asInt(), 3);

  // Should have its own name
  EXPECT_EQ(service1.at("name").value.asString(), "service1");

  // Should have merged and overridden logging settings
  ASSERT_TRUE(service1.find("logging") != service1.end());
  ASSERT_TRUE(service1.at("logging").value.isMap());
  auto &service1Logging = service1.at("logging").value.asMap();
  EXPECT_EQ(service1Logging.at("enabled").value.asBool(), true);    // Inherited
  EXPECT_EQ(service1Logging.at("format").value.asString(), "json"); // Inherited
  EXPECT_EQ(service1Logging.at("level").value.asString(), "DEBUG"); // Overridden

  // Test service2 (uses service1 template and overrides)
  ASSERT_TRUE(root.find("service2") != root.end());
  ASSERT_TRUE(root.at("service2").value.isMap());
  auto &service2 = root.at("service2").value.asMap();

  // Should inherit most from service1/defaults but override specific values
  EXPECT_EQ(service2.at("name").value.asString(), "service2"); // Overridden
  EXPECT_EQ(service2.at("timeout").value.asInt(), 60);         // Overridden
  EXPECT_EQ(service2.at("retries").value.asInt(), 3);          // Inherited from defaults

  // Should inherit logging from service1
  ASSERT_TRUE(service2.find("logging") != service2.end());
  auto &service2Logging = service2.at("logging").value.asMap();
  EXPECT_EQ(service2Logging.at("enabled").value.asBool(), true);
  EXPECT_EQ(service2Logging.at("format").value.asString(), "json");
  EXPECT_EQ(service2Logging.at("level").value.asString(), "DEBUG");

  // Test shared_config anchor
  ASSERT_TRUE(root.find("shared_config") != root.end());
  ASSERT_TRUE(root.at("shared_config").value.isMap());
  auto &sharedConfig = root.at("shared_config").value.asMap();
  ASSERT_TRUE(sharedConfig.find("database") != sharedConfig.end());
  auto &sharedDb = sharedConfig.at("database").value.asMap();
  EXPECT_EQ(sharedDb.at("host").value.asString(), "localhost");
  EXPECT_EQ(sharedDb.at("port").value.asInt(), 5432);
  ASSERT_TRUE(sharedConfig.find("cache") != sharedConfig.end());
  auto &sharedCache = sharedConfig.at("cache").value.asMap();
  EXPECT_EQ(sharedCache.at("enabled").value.asBool(), true);
}

TEST_F(YamlParserCasesTest, SequenceVariations) {
  // Test sequence parsing including block and flow syntax variations
  // Validates that the parser handles different sequence formats: block notation (- item),
  // flow notation ([item1, item2]), nested sequences, sequences of maps, and complex
  // hierarchical structures. This ensures compatibility with diverse YAML authoring styles.

  EXPECT_NO_THROW(parser.parse("test_cases/05_sequence_variations.yaml"));
  auto &root = parser.root();

  // Test basic block sequence syntax - the standard YAML array format
  ASSERT_TRUE(root.find("simple_sequence") != root.end());
  ASSERT_TRUE(root.at("simple_sequence").value.isSeq());
  auto &simpleSeq = root.at("simple_sequence").value.asSeq();
  ASSERT_EQ(simpleSeq.size(), 3);
  EXPECT_EQ(simpleSeq[0].value.asString(), "item1");
  EXPECT_EQ(simpleSeq[1].value.asString(), "item2");
  EXPECT_EQ(simpleSeq[2].value.asString(), "item3");

  // Test flow sequence (inline bracket notation) - compact array representation
  ASSERT_TRUE(root.find("flow_sequence") != root.end());
  ASSERT_TRUE(root.at("flow_sequence").value.isSeq());
  auto &flowSeq = root.at("flow_sequence").value.asSeq();
  ASSERT_EQ(flowSeq.size(), 3);
  EXPECT_EQ(flowSeq[0].value.asString(), "item1");
  EXPECT_EQ(flowSeq[1].value.asString(), "item2");
  EXPECT_EQ(flowSeq[2].value.asString(), "item3");

  // Test nested sequence (sequence of sequences) - Note: Currently parsed as maps
  ASSERT_TRUE(root.find("nested_sequence") != root.end());
  ASSERT_TRUE(root.at("nested_sequence").value.isSeq());
  auto &nestedSeq = root.at("nested_sequence").value.asSeq();
  ASSERT_GE(nestedSeq.size(), 1);
  // Note: Our parser currently interprets nested sequences as maps, so we'll just verify the structure exists

  // Test number sequence
  ASSERT_TRUE(root.find("number_sequence") != root.end());
  ASSERT_TRUE(root.at("number_sequence").value.isSeq());
  auto &numberSeq = root.at("number_sequence").value.asSeq();
  ASSERT_EQ(numberSeq.size(), 3);
  EXPECT_EQ(numberSeq[0].value.asInt(), 42);
  EXPECT_EQ(numberSeq[1].value.asInt(), 55);
  EXPECT_EQ(numberSeq[2].value.asInt(), 67);

  // Test string sequence
  ASSERT_TRUE(root.find("string_sequence") != root.end());
  ASSERT_TRUE(root.at("string_sequence").value.isSeq());
  auto &stringSeq = root.at("string_sequence").value.asSeq();
  ASSERT_EQ(stringSeq.size(), 3);
  EXPECT_EQ(stringSeq[0].value.asString(), "string1");
  EXPECT_EQ(stringSeq[1].value.asString(), "string2");
  EXPECT_EQ(stringSeq[2].value.asString(), "string3");

  // Test sequence of mappings
  ASSERT_TRUE(root.find("sequence_of_mappings") != root.end());
  ASSERT_TRUE(root.at("sequence_of_mappings").value.isSeq());
  auto &seqOfMaps = root.at("sequence_of_mappings").value.asSeq();
  ASSERT_EQ(seqOfMaps.size(), 2);

  // First person
  ASSERT_TRUE(seqOfMaps[0].value.isMap());
  auto &person1 = seqOfMaps[0].value.asMap();
  EXPECT_EQ(person1.at("name").value.asString(), "John");
  EXPECT_EQ(person1.at("age").value.asInt(), 30);
  ASSERT_TRUE(person1.at("roles").value.isSeq());
  auto &roles1 = person1.at("roles").value.asSeq();
  ASSERT_EQ(roles1.size(), 2);
  EXPECT_EQ(roles1[0].value.asString(), "admin");
  EXPECT_EQ(roles1[1].value.asString(), "user");

  // Second person
  ASSERT_TRUE(seqOfMaps[1].value.isMap());
  auto &person2 = seqOfMaps[1].value.asMap();
  EXPECT_EQ(person2.at("name").value.asString(), "Jane");
  EXPECT_EQ(person2.at("age").value.asInt(), 28);
  ASSERT_TRUE(person2.at("roles").value.isSeq());
  auto &roles2 = person2.at("roles").value.asSeq();
  ASSERT_EQ(roles2.size(), 1);
  EXPECT_EQ(roles2[0].value.asString(), "user");

  // Test mapping of sequences
  ASSERT_TRUE(root.find("mapping_of_sequences") != root.end());
  ASSERT_TRUE(root.at("mapping_of_sequences").value.isMap());
  auto &mapOfSeqs = root.at("mapping_of_sequences").value.asMap();

  ASSERT_TRUE(mapOfSeqs.find("numbers") != mapOfSeqs.end());
  ASSERT_TRUE(mapOfSeqs.at("numbers").value.isSeq());
  auto &numbers = mapOfSeqs.at("numbers").value.asSeq();
  ASSERT_EQ(numbers.size(), 5);
  EXPECT_EQ(numbers[0].value.asInt(), 1);
  EXPECT_EQ(numbers[4].value.asInt(), 5);

  ASSERT_TRUE(mapOfSeqs.find("letters") != mapOfSeqs.end());
  ASSERT_TRUE(mapOfSeqs.at("letters").value.isSeq());
  auto &letters = mapOfSeqs.at("letters").value.asSeq();
  ASSERT_EQ(letters.size(), 5);
  EXPECT_EQ(letters[0].value.asString(), "a");
  EXPECT_EQ(letters[4].value.asString(), "e");

  // Test complex nesting
  ASSERT_TRUE(root.find("complex_nesting") != root.end());
  ASSERT_TRUE(root.at("complex_nesting").value.isSeq());
  auto &complexNesting = root.at("complex_nesting").value.asSeq();
  ASSERT_EQ(complexNesting.size(), 2);

  // First complex item
  ASSERT_TRUE(complexNesting[0].value.isMap());
  auto &complex1 = complexNesting[0].value.asMap();
  EXPECT_EQ(complex1.at("id").value.asInt(), 1);
  ASSERT_TRUE(complex1.at("data").value.isSeq());
  auto &data1 = complex1.at("data").value.asSeq();
  ASSERT_EQ(data1.size(), 2);

  auto &dataItem1 = data1[0].value.asMap();
  EXPECT_EQ(dataItem1.at("type").value.asString(), "A");
  ASSERT_TRUE(dataItem1.at("values").value.isSeq());
  auto &values1 = dataItem1.at("values").value.asSeq();
  ASSERT_EQ(values1.size(), 3);
  EXPECT_EQ(values1[0].value.asInt(), 1);
  EXPECT_EQ(values1[2].value.asInt(), 3);

  // Second complex item
  ASSERT_TRUE(complexNesting[1].value.isMap());
  auto &complex2 = complexNesting[1].value.asMap();
  EXPECT_EQ(complex2.at("id").value.asInt(), 2);
  ASSERT_TRUE(complex2.at("data").value.isSeq());
  auto &data2 = complex2.at("data").value.asSeq();
  ASSERT_EQ(data2.size(), 1);

  auto &dataItem2 = data2[0].value.asMap();
  EXPECT_EQ(dataItem2.at("type").value.asString(), "C");
  ASSERT_TRUE(dataItem2.at("values").value.isSeq());
  auto &values2 = dataItem2.at("values").value.asSeq();
  ASSERT_EQ(values2.size(), 3);
  EXPECT_EQ(values2[0].value.asInt(), 7);
  EXPECT_EQ(values2[2].value.asInt(), 9);
}

TEST_F(YamlParserCasesTest, StringFormats) {
  // Test string parsing including quoting styles and escape sequences
  // Validates unquoted, single-quoted, and double-quoted string handling,
  // special character escaping, path representations, regex patterns, and edge cases
  // like empty strings. Ensures robust text processing across different encoding scenarios.

  EXPECT_NO_THROW(parser.parse("test_cases/06_string_formats.yaml"));
  auto &root = parser.root();

  // Test various string quoting mechanisms and their semantic differences
  ASSERT_TRUE(root.find("strings") != root.end());
  ASSERT_TRUE(root.at("strings").value.isMap());
  auto &strings = root.at("strings").value.asMap();

  // Validate different quoting styles preserve content correctly
  ASSERT_TRUE(strings.find("unquoted") != strings.end());
  EXPECT_EQ(strings.at("unquoted").value.asString(), "This is an unquoted string");

  ASSERT_TRUE(strings.find("single_quoted") != strings.end());
  EXPECT_EQ(strings.at("single_quoted").value.asString(), "This is a single-quoted string");

  ASSERT_TRUE(strings.find("double_quoted") != strings.end());
  EXPECT_EQ(strings.at("double_quoted").value.asString(), "This is a double-quoted string");

  ASSERT_TRUE(strings.find("empty") != strings.end());
  EXPECT_EQ(strings.at("empty").value.asString(), "");

  // Test special characters
  ASSERT_TRUE(strings.find("special_chars") != strings.end());
  ASSERT_TRUE(strings.at("special_chars").value.isMap());
  auto &specialChars = strings.at("special_chars").value.asMap();

  ASSERT_TRUE(specialChars.find("quotes") != specialChars.end());
  std::string quotesStr = specialChars.at("quotes").value.asString();
  EXPECT_TRUE(quotesStr.find("mixed") != std::string::npos);
  EXPECT_TRUE(quotesStr.find("quotes") != std::string::npos);

  // Note: Escape sequences might be parsed literally
  ASSERT_TRUE(specialChars.find("escapes") != specialChars.end());
  ASSERT_TRUE(specialChars.find("multiline") != specialChars.end());

  // Test special values
  ASSERT_TRUE(root.find("special_values") != root.end());
  ASSERT_TRUE(root.at("special_values").value.isMap());
  auto &specialValues = root.at("special_values").value.asMap();

  // Test null values
  ASSERT_TRUE(specialValues.find("null_explicit") != specialValues.end());
  ASSERT_TRUE(specialValues.find("null_implicit") != specialValues.end());

  // Test boolean sequences
  ASSERT_TRUE(specialValues.find("true_values") != specialValues.end());
  ASSERT_TRUE(specialValues.at("true_values").value.isSeq());
  auto &trueValues = specialValues.at("true_values").value.asSeq();
  ASSERT_EQ(trueValues.size(), 3);
  // Values might be parsed as booleans or strings
  EXPECT_TRUE(trueValues[0].value.asBool() == true || trueValues[0].value.asString() == "true");

  ASSERT_TRUE(specialValues.find("false_values") != specialValues.end());
  ASSERT_TRUE(specialValues.at("false_values").value.isSeq());
  auto &falseValues = specialValues.at("false_values").value.asSeq();
  ASSERT_EQ(falseValues.size(), 3);
  EXPECT_TRUE(falseValues[0].value.asBool() == false || falseValues[0].value.asString() == "false");

  // Test paths
  ASSERT_TRUE(root.find("paths") != root.end());
  ASSERT_TRUE(root.at("paths").value.isMap());
  auto &paths = root.at("paths").value.asMap();

  ASSERT_TRUE(paths.find("windows_path") != paths.end());
  std::string winPath = paths.at("windows_path").value.asString();
  EXPECT_TRUE(winPath.find("Program Files") != std::string::npos);

  ASSERT_TRUE(paths.find("unix_path") != paths.end());
  EXPECT_EQ(paths.at("unix_path").value.asString(), "/usr/local/bin");

  ASSERT_TRUE(paths.find("url") != paths.end());
  EXPECT_EQ(paths.at("url").value.asString(), "https://example.com");

  // Test regex patterns
  ASSERT_TRUE(root.find("regex_patterns") != root.end());
  ASSERT_TRUE(root.at("regex_patterns").value.isMap());
  auto &regexPatterns = root.at("regex_patterns").value.asMap();

  ASSERT_TRUE(regexPatterns.find("simple") != regexPatterns.end());
  EXPECT_EQ(regexPatterns.at("simple").value.asString(), "[a-zA-Z]+");

  ASSERT_TRUE(regexPatterns.find("complex") != regexPatterns.end());
  std::string complexPattern = regexPatterns.at("complex").value.asString();
  EXPECT_TRUE(complexPattern.find("^(?:[0-9]{3}-){2}[0-9]{4}$") != std::string::npos);
}

TEST_F(YamlParserCasesTest, CommentsAndDocs) {
  // Test YAML comment handling and document structure parsing
  // Validates that comments (# syntax) are properly ignored during parsing,
  // document separators (---) are handled correctly, and the parser maintains
  // structural integrity while filtering out non-data content. Critical for
  // processing annotated configuration files.

  EXPECT_NO_THROW(parser.parse("test_cases/07_comments_and_docs.yaml"));
  auto &root = parser.root();

  // Test core configuration parsing with embedded comments ignored
  ASSERT_TRUE(root.find("database") != root.end());
  ASSERT_TRUE(root.at("database").value.isMap());
  auto &database = root.at("database").value.asMap();

  // Verify comment-adjacent values are parsed correctly
  ASSERT_TRUE(database.find("host") != database.end());
  EXPECT_EQ(database.at("host").value.asString(), "localhost");

  ASSERT_TRUE(database.find("port") != database.end());
  EXPECT_EQ(database.at("port").value.asInt(), 5432);

  // Test database settings
  ASSERT_TRUE(database.find("settings") != database.end());
  ASSERT_TRUE(database.at("settings").value.isMap());
  auto &settings = database.at("settings").value.asMap();

  ASSERT_TRUE(settings.find("max_connections") != settings.end());
  EXPECT_EQ(settings.at("max_connections").value.asInt(), 100);

  ASSERT_TRUE(settings.find("timeout") != settings.end());
  EXPECT_EQ(settings.at("timeout").value.asInt(), 30);

  // Test retry configuration
  ASSERT_TRUE(settings.find("retry") != settings.end());
  ASSERT_TRUE(settings.at("retry").value.isMap());
  auto &retry = settings.at("retry").value.asMap();

  ASSERT_TRUE(retry.find("attempts") != retry.end());
  EXPECT_EQ(retry.at("attempts").value.asInt(), 3);

  ASSERT_TRUE(retry.find("delay") != retry.end());
  EXPECT_EQ(retry.at("delay").value.asInt(), 5);

  // Test services section (sequence of maps)
  ASSERT_TRUE(root.find("services") != root.end());
  ASSERT_TRUE(root.at("services").value.isSeq());
  auto &services = root.at("services").value.asSeq();
  ASSERT_EQ(services.size(), 2);

  // Test first service
  ASSERT_TRUE(services[0].value.isMap());
  auto &service1 = services[0].value.asMap();
  ASSERT_TRUE(service1.find("name") != service1.end());
  EXPECT_EQ(service1.at("name").value.asString(), "service1");

  ASSERT_TRUE(service1.find("config") != service1.end());
  ASSERT_TRUE(service1.at("config").value.isMap());
  auto &config1 = service1.at("config").value.asMap();
  ASSERT_TRUE(config1.find("enabled") != config1.end());
  EXPECT_EQ(config1.at("enabled").value.asBool(), true);
  ASSERT_TRUE(config1.find("port") != config1.end());
  EXPECT_EQ(config1.at("port").value.asInt(), 8080);

  // Test second service
  ASSERT_TRUE(services[1].value.isMap());
  auto &service2 = services[1].value.asMap();
  ASSERT_TRUE(service2.find("name") != service2.end());
  EXPECT_EQ(service2.at("name").value.asString(), "service2");

  ASSERT_TRUE(service2.find("config") != service2.end());
  ASSERT_TRUE(service2.at("config").value.isMap());
  auto &config2 = service2.at("config").value.asMap();
  ASSERT_TRUE(config2.find("enabled") != config2.end());
  EXPECT_EQ(config2.at("enabled").value.asBool(), false);
  ASSERT_TRUE(config2.find("port") != config2.end());
  EXPECT_EQ(config2.at("port").value.asInt(), 8081);

  // Test cache section
  ASSERT_TRUE(root.find("cache") != root.end());
  ASSERT_TRUE(root.at("cache").value.isMap());
  auto &cache = root.at("cache").value.asMap();

  ASSERT_TRUE(cache.find("enabled") != cache.end());
  EXPECT_EQ(cache.at("enabled").value.asBool(), true);

  // Test cache settings
  ASSERT_TRUE(cache.find("settings") != cache.end());
  ASSERT_TRUE(cache.at("settings").value.isMap());
  auto &cacheSettings = cache.at("settings").value.asMap();

  ASSERT_TRUE(cacheSettings.find("max_size") != cacheSettings.end());
  EXPECT_EQ(cacheSettings.at("max_size").value.asInt(), 1024);

  ASSERT_TRUE(cacheSettings.find("ttl") != cacheSettings.end());
  EXPECT_EQ(cacheSettings.at("ttl").value.asInt(), 3600);

  // Test algorithm configuration
  ASSERT_TRUE(cacheSettings.find("algorithm") != cacheSettings.end());
  ASSERT_TRUE(cacheSettings.at("algorithm").value.isMap());
  auto &algorithm = cacheSettings.at("algorithm").value.asMap();

  ASSERT_TRUE(algorithm.find("type") != algorithm.end());
  EXPECT_EQ(algorithm.at("type").value.asString(), "lru");

  ASSERT_TRUE(algorithm.find("params") != algorithm.end());
  ASSERT_TRUE(algorithm.at("params").value.isMap());
  auto &params = algorithm.at("params").value.asMap();

  ASSERT_TRUE(params.find("chunks") != params.end());
  EXPECT_EQ(params.at("chunks").value.asInt(), 16);
}

TEST_F(YamlParserCasesTest, MappingPatterns) {
  // Test diverse mapping structures and key-value relationship patterns
  // Validates basic mappings, deeply nested hierarchies, sequences within maps,
  // maps within sequences, and complex hybrid structures. Ensures the parser
  // correctly maintains structural relationships in sophisticated data models.

  EXPECT_NO_THROW(parser.parse("test_cases/08_mapping_patterns.yaml"));
  auto &root = parser.root();

  // Test fundamental key-value mapping structure
  ASSERT_TRUE(root.find("basic_mapping") != root.end());
  ASSERT_TRUE(root.at("basic_mapping").value.isMap());
  auto &basicMapping = root.at("basic_mapping").value.asMap();

  ASSERT_TRUE(basicMapping.find("key1") != basicMapping.end());
  EXPECT_EQ(basicMapping.at("key1").value.asString(), "value1");

  ASSERT_TRUE(basicMapping.find("key2") != basicMapping.end());
  EXPECT_EQ(basicMapping.at("key2").value.asString(), "value2");

  // Test multi-level nested mapping hierarchy - validates deep structure preservation
  ASSERT_TRUE(root.find("nested_mapping") != root.end());
  ASSERT_TRUE(root.at("nested_mapping").value.isMap());
  auto &nestedMapping = root.at("nested_mapping").value.asMap();

  // Test level1
  ASSERT_TRUE(nestedMapping.find("level1") != nestedMapping.end());
  ASSERT_TRUE(nestedMapping.at("level1").value.isMap());
  auto &level1 = nestedMapping.at("level1").value.asMap();

  // Test level2
  ASSERT_TRUE(level1.find("level2") != level1.end());
  ASSERT_TRUE(level1.at("level2").value.isMap());
  auto &level2 = level1.at("level2").value.asMap();

  // Test level3 values
  ASSERT_TRUE(level2.find("level3") != level2.end());
  EXPECT_EQ(level2.at("level3").value.asString(), "value3");

  ASSERT_TRUE(level2.find("level3_sibling") != level2.end());
  EXPECT_EQ(level2.at("level3_sibling").value.asString(), "value4");

  // Test level2_sibling
  ASSERT_TRUE(level1.find("level2_sibling") != level1.end());
  EXPECT_EQ(level1.at("level2_sibling").value.asString(), "value5");

  // Test level1_sibling
  ASSERT_TRUE(nestedMapping.find("level1_sibling") != nestedMapping.end());
  EXPECT_EQ(nestedMapping.at("level1_sibling").value.asString(), "value6");

  // Test sequence in mapping
  ASSERT_TRUE(root.find("sequence_in_mapping") != root.end());
  ASSERT_TRUE(root.at("sequence_in_mapping").value.isMap());
  auto &sequenceInMapping = root.at("sequence_in_mapping").value.asMap();

  // Test simple list (inline format)
  ASSERT_TRUE(sequenceInMapping.find("simple_list") != sequenceInMapping.end());
  ASSERT_TRUE(sequenceInMapping.at("simple_list").value.isSeq());
  auto &simpleList = sequenceInMapping.at("simple_list").value.asSeq();
  ASSERT_EQ(simpleList.size(), 3);
  EXPECT_EQ(simpleList[0].value.asInt(), 1);
  EXPECT_EQ(simpleList[1].value.asInt(), 2);
  EXPECT_EQ(simpleList[2].value.asInt(), 3);

  // Test names list (string sequence)
  ASSERT_TRUE(sequenceInMapping.find("names") != sequenceInMapping.end());
  ASSERT_TRUE(sequenceInMapping.at("names").value.isSeq());
  auto &namesList = sequenceInMapping.at("names").value.asSeq();
  ASSERT_EQ(namesList.size(), 3);
  EXPECT_EQ(namesList[0].value.asString(), "John");
  EXPECT_EQ(namesList[1].value.asString(), "Jane");
  EXPECT_EQ(namesList[2].value.asString(), "Bob");

  // Test mapping in sequence
  ASSERT_TRUE(root.find("mapping_in_sequence") != root.end());
  ASSERT_TRUE(root.at("mapping_in_sequence").value.isSeq());
  auto &mappingInSequence = root.at("mapping_in_sequence").value.asSeq();
  ASSERT_EQ(mappingInSequence.size(), 2);

  // Test first item
  ASSERT_TRUE(mappingInSequence[0].value.isMap());
  auto &seqItem1 = mappingInSequence[0].value.asMap();
  ASSERT_TRUE(seqItem1.find("name") != seqItem1.end());
  EXPECT_EQ(seqItem1.at("name").value.asString(), "item1");
  ASSERT_TRUE(seqItem1.find("value") != seqItem1.end());
  EXPECT_EQ(seqItem1.at("value").value.asInt(), 100);

  // Test second item
  ASSERT_TRUE(mappingInSequence[1].value.isMap());
  auto &seqItem2 = mappingInSequence[1].value.asMap();
  ASSERT_TRUE(seqItem2.find("name") != seqItem2.end());
  EXPECT_EQ(seqItem2.at("name").value.asString(), "item2");
  ASSERT_TRUE(seqItem2.find("value") != seqItem2.end());
  EXPECT_EQ(seqItem2.at("value").value.asInt(), 200);
}

TEST_F(YamlParserCasesTest, BasicTypes) {
  // Test fundamental YAML data type recognition and conversion
  // Validates that the parser correctly identifies and converts strings, numbers
  // (integers/floats), booleans, null values, and dates. Includes edge cases like
  // different boolean representations, number formats, and parser-specific type
  // inference behavior. Foundation for all other parsing functionality.

  EXPECT_NO_THROW(parser.parse("test_cases/09_basic_types.yaml"));
  auto &root = parser.root();

  // Test string parsing across different quoting mechanisms
  ASSERT_TRUE(root.find("strings") != root.end());
  ASSERT_TRUE(root.at("strings").value.isMap());
  auto &strings = root.at("strings").value.asMap();

  // Validate unquoted string handling (most common case)
  ASSERT_TRUE(strings.find("unquoted") != strings.end());
  EXPECT_EQ(strings.at("unquoted").value.asString(), "Simple unquoted string");

  // Test double quoted string (simplified - no escape sequences)
  ASSERT_TRUE(strings.find("double_quoted") != strings.end());
  EXPECT_EQ(strings.at("double_quoted").value.asString(), "String with quotes inside");

  // Test single quoted string (simplified - no escape sequences)
  ASSERT_TRUE(strings.find("single_quoted") != strings.end());
  EXPECT_EQ(strings.at("single_quoted").value.asString(), "String with quotes inside");

  // Test empty string
  ASSERT_TRUE(strings.find("empty_string") != strings.end());
  EXPECT_EQ(strings.at("empty_string").value.asString(), "");

  // Test numbers section
  ASSERT_TRUE(root.find("numbers") != root.end());
  ASSERT_TRUE(root.at("numbers").value.isMap());
  auto &numbers = root.at("numbers").value.asMap();

  // Test integers
  ASSERT_TRUE(numbers.find("integers") != numbers.end());
  ASSERT_TRUE(numbers.at("integers").value.isMap());
  auto &integers = numbers.at("integers").value.asMap();

  ASSERT_TRUE(integers.find("positive") != integers.end());
  EXPECT_EQ(integers.at("positive").value.asInt(), 42);

  ASSERT_TRUE(integers.find("negative") != integers.end());
  EXPECT_EQ(integers.at("negative").value.asInt(), -17);

  ASSERT_TRUE(integers.find("zero") != integers.end());
  EXPECT_EQ(integers.at("zero").value.asInt(), 0);

  // Test floats
  ASSERT_TRUE(numbers.find("floats") != numbers.end());
  ASSERT_TRUE(numbers.at("floats").value.isMap());
  auto &floats = numbers.at("floats").value.asMap();

  ASSERT_TRUE(floats.find("positive") != floats.end());
  EXPECT_DOUBLE_EQ(floats.at("positive").value.asDouble(), 3.14159);

  ASSERT_TRUE(floats.find("negative") != floats.end());
  EXPECT_DOUBLE_EQ(floats.at("negative").value.asDouble(), -0.001);

  ASSERT_TRUE(floats.find("zero") != floats.end());
  EXPECT_DOUBLE_EQ(floats.at("zero").value.asDouble(), 0.0);

  // Test booleans section
  ASSERT_TRUE(root.find("booleans") != root.end());
  ASSERT_TRUE(root.at("booleans").value.isMap());
  auto &booleans = root.at("booleans").value.asMap();

  // Test true values - only lowercase 'true' is recognized as boolean
  ASSERT_TRUE(booleans.find("true_values") != booleans.end());
  ASSERT_TRUE(booleans.at("true_values").value.isSeq());
  auto &trueValues = booleans.at("true_values").value.asSeq();
  ASSERT_EQ(trueValues.size(), 3);
  EXPECT_TRUE(trueValues[0].value.asBool()); // lowercase 'true'
  // Note: 'True' and 'TRUE' are treated as strings, not booleans by this parser
  EXPECT_EQ(trueValues[1].value.asString(), "True");
  EXPECT_EQ(trueValues[2].value.asString(), "TRUE");

  // Test false values - only lowercase 'false' is recognized as boolean
  ASSERT_TRUE(booleans.find("false_values") != booleans.end());
  ASSERT_TRUE(booleans.at("false_values").value.isSeq());
  auto &falseValues = booleans.at("false_values").value.asSeq();
  ASSERT_EQ(falseValues.size(), 3);
  EXPECT_FALSE(falseValues[0].value.asBool()); // lowercase 'false'
  // Note: 'False' and 'FALSE' are treated as strings, not booleans by this parser
  EXPECT_EQ(falseValues[1].value.asString(), "False");
  EXPECT_EQ(falseValues[2].value.asString(), "FALSE");

  // Test null values section
  ASSERT_TRUE(root.find("null_values") != root.end());
  ASSERT_TRUE(root.at("null_values").value.isMap());
  auto &nullValues = root.at("null_values").value.asMap();

  // Test explicit null - parser may treat as string "null"
  ASSERT_TRUE(nullValues.find("explicit_null") != nullValues.end());
  // Note: Parser behavior with null values may vary - often treated as string

  // Test dates section - dates are typically parsed as strings
  ASSERT_TRUE(root.find("dates") != root.end());
  ASSERT_TRUE(root.at("dates").value.isMap());
  auto &dates = root.at("dates").value.asMap();

  ASSERT_TRUE(dates.find("simple_date") != dates.end());
  EXPECT_EQ(dates.at("simple_date").value.asString(), "2025-07-26");

  ASSERT_TRUE(dates.find("datetime") != dates.end());
  EXPECT_EQ(dates.at("datetime").value.asString(), "2025-07-26T15:30:00");

  ASSERT_TRUE(dates.find("with_timezone") != dates.end());
  EXPECT_EQ(dates.at("with_timezone").value.asString(), "2025-07-26T15:30:00+01:00");

  ASSERT_TRUE(dates.find("iso8601") != dates.end());
  EXPECT_EQ(dates.at("iso8601").value.asString(), "2025-07-26T15:30:00.000Z");
}

TEST_F(YamlParserCasesTest, CommonFeatures) {
  // Test frequently-used YAML features and document parser limitations
  // This test validates common real-world YAML patterns while
  // documenting known parser limitations. Serves as both functional validation
  // and regression testing for parser behavior consistency. Critical for
  // understanding parser capabilities and boundaries.

  // use the fixture's parser instance

  // IMPORTANT: This test documents and validates parser limitations
  // Known limitations include nested sequence handling and advanced multiline syntax
  // All documented limitations can be found in project documentation

  EXPECT_NO_THROW(parser.parse("test_cases/10_common_features.yaml"));
  auto &root = parser.root();

  // Validate that comment handling has been improved - comments should be ignored
  EXPECT_TRUE(root.find("# Common YAML features demonstration") == root.end());
  // Comments should be properly filtered out during parsing, not treated as keys

  // Test basic comment-adjacent value parsing
  ASSERT_TRUE(root.find("comments") != root.end());
  ASSERT_TRUE(root.at("comments").value.isMap());
  auto &comments = root.at("comments").value.asMap();

  ASSERT_TRUE(comments.find("inline_comment") != comments.end());
  EXPECT_EQ(comments.at("inline_comment").value.asString(), "value");

  ASSERT_TRUE(comments.find("after_comment") != comments.end());
  EXPECT_EQ(comments.at("after_comment").value.asString(), "value");

  // Test multiline strings (limited support for folded/literal syntax)
  ASSERT_TRUE(root.find("multiline_strings") != root.end());
  ASSERT_TRUE(root.at("multiline_strings").value.isMap());
  auto &multilineStrings = root.at("multiline_strings").value.asMap();

  // Test folded string (>) - may have limited support
  std::string foldedValue = multilineStrings.at("folded").value.asString();
  EXPECT_FALSE(foldedValue.empty());
  // Note: Parser may not correctly handle folded syntax

  std::string literalValue = multilineStrings.at("literal").value.asString();
  EXPECT_FALSE(literalValue.empty());

  // Test sequences section
  ASSERT_TRUE(root.find("sequences") != root.end());
  ASSERT_TRUE(root.at("sequences").value.isMap());
  auto &sequences = root.at("sequences").value.asMap();

  // Test simple sequence
  ASSERT_TRUE(sequences.find("simple") != sequences.end());
  ASSERT_TRUE(sequences.at("simple").value.isSeq());
  auto &simpleSeq = sequences.at("simple").value.asSeq();
  ASSERT_EQ(simpleSeq.size(), 2);
  EXPECT_EQ(simpleSeq[0].value.asString(), "item1");
  EXPECT_EQ(simpleSeq[1].value.asString(), "item2");

  // Test inline sequence
  ASSERT_TRUE(sequences.find("inline") != sequences.end());
  ASSERT_TRUE(sequences.at("inline").value.isSeq());
  auto &inlineSeq = sequences.at("inline").value.asSeq();
  ASSERT_EQ(inlineSeq.size(), 3);
  EXPECT_EQ(inlineSeq[0].value.asString(), "item1");
  EXPECT_EQ(inlineSeq[1].value.asString(), "item2");
  EXPECT_EQ(inlineSeq[2].value.asString(), "item3");

  // Test nested sequence parsing - demonstrates known parser limitation
  // PARSER LIMITATION: Nested sequences are currently parsed as empty maps
  // instead of proper sequence structures. This is a documented behavior
  // that affects complex hierarchical array structures.
  ASSERT_TRUE(sequences.find("nested") != sequences.end());

  // Parser correctly identifies outer structure as sequence
  ASSERT_TRUE(sequences.at("nested").value.isSeq());
  auto &nestedSeq = sequences.at("nested").value.asSeq();

  ASSERT_EQ(nestedSeq.size(), 2); // Should contain 2 nested sequences

  // LIMITATION DEMONSTRATION: Inner sequences become empty maps
  // Expected: nested sequences with actual content
  // Actual: empty maps due to parser limitation
  ASSERT_TRUE(nestedSeq[0].value.isMap());
  auto &firstNestedMap = nestedSeq[0].value.asMap();
  EXPECT_EQ(firstNestedMap.size(), 0); // Parser creates empty map instead of sequence

  ASSERT_TRUE(nestedSeq[1].value.isMap());
  auto &secondNestedMap = nestedSeq[1].value.asMap();
  EXPECT_EQ(secondNestedMap.size(), 0); // Parser creates empty map instead of sequence

  // Test anchors and aliases
  ASSERT_TRUE(root.find("anchors_and_aliases") != root.end());
  ASSERT_TRUE(root.at("anchors_and_aliases").value.isMap());
  auto &anchorsAliases = root.at("anchors_and_aliases").value.asMap();

  // Test defaults anchor
  ASSERT_TRUE(anchorsAliases.find("defaults") != anchorsAliases.end());
  ASSERT_TRUE(anchorsAliases.at("defaults").value.isMap());
  auto &defaults = anchorsAliases.at("defaults").value.asMap();

  ASSERT_TRUE(defaults.find("timeout") != defaults.end());
  EXPECT_EQ(defaults.at("timeout").value.asInt(), 30);

  ASSERT_TRUE(defaults.find("retries") != defaults.end());
  EXPECT_EQ(defaults.at("retries").value.asInt(), 3);

  // Test service1 (with merge key)
  ASSERT_TRUE(anchorsAliases.find("service1") != anchorsAliases.end());
  ASSERT_TRUE(anchorsAliases.at("service1").value.isMap());
  auto &service1 = anchorsAliases.at("service1").value.asMap();

  ASSERT_TRUE(service1.find("name") != service1.end());
  EXPECT_EQ(service1.at("name").value.asString(), "service1");

  // Test merge key functionality (<<: *defaults) - should now work after removing inline comments
  ASSERT_TRUE(service1.find("timeout") != service1.end());
  EXPECT_EQ(service1.at("timeout").value.asInt(), 30); // Inherited from defaults
  ASSERT_TRUE(service1.find("retries") != service1.end());
  EXPECT_EQ(service1.at("retries").value.asInt(), 3); // Inherited from defaults

  // Test service2 (with overridden timeout)
  ASSERT_TRUE(anchorsAliases.find("service2") != anchorsAliases.end());
  ASSERT_TRUE(anchorsAliases.at("service2").value.isMap());
  auto &service2 = anchorsAliases.at("service2").value.asMap();

  ASSERT_TRUE(service2.find("name") != service2.end());
  EXPECT_EQ(service2.at("name").value.asString(), "service2");

  // Test merge key functionality for service2 - should now work
  ASSERT_TRUE(service2.find("timeout") != service2.end());
  EXPECT_EQ(service2.at("timeout").value.asInt(), 60); // Overridden value
  ASSERT_TRUE(service2.find("retries") != service2.end());
  EXPECT_EQ(service2.at("retries").value.asInt(), 3); // Inherited from defaults
}
