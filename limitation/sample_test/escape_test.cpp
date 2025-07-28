#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "Testing String Escape Sequences Limitation\n";
  std::cout << "==========================================\n";

  YamlParser parser;

  try {
    parser.parse("../sample_yaml/escape_test.yaml");
    std::cout << "Parse result: SUCCESS\n";

    auto &root = parser.root();

    auto specialIt = root.find("special_chars");
    auto quotedIt  = root.find("quoted_string");

    if (specialIt != root.end() && specialIt->second.value.isString()) {
      std::string specialValue = specialIt->second.value.asString();
      std::cout << "Special chars value: '" << specialValue << "'\n";

      bool hasLiteralEscape =
          specialValue.find("\\t") != std::string::npos || specialValue.find("\\n") != std::string::npos;

      if (hasLiteralEscape) {
        std::cout << "CONFIRMED: Escape sequences kept literal (limitation exists)\n";
      } else {
        std::cout << "UNEXPECTED: Escape sequences processed correctly\n";
      }
    }

    if (quotedIt != root.end() && quotedIt->second.value.isString()) {
      std::string quotedValue = quotedIt->second.value.asString();
      std::cout << "Quoted string value: '" << quotedValue << "'\n";

      bool hasLiteralQuotes = quotedValue.find("\\\"") != std::string::npos;

      if (hasLiteralQuotes) {
        std::cout << "CONFIRMED: Quote escapes kept literal (limitation exists)\n";
      } else {
        std::cout << "Note: Quote escapes may be processed\n";
      }
    }
  } catch (const std::exception &e) {
    std::cout << "Parse result: FAILED\n";
    std::cout << "Error: " << e.what() << "\n";
    std::cout << "CONFIRMED: String escape sequences cause parsing failure\n";
  }

  return 0;
}
