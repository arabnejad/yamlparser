#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "Testing Boolean Value Recognition Limitation\n";
  std::cout << "============================================\n";

  YamlParser parser;

  try {
    parser.parse("sample_yaml/boolean_test.yaml");
    std::cout << "Parse result: SUCCESS\n";

    auto &root = parser.root();

    auto bool1It = root.find("bool1");
    auto bool3It = root.find("bool3");
    auto bool5It = root.find("bool5");

    std::cout << "\nValue analysis:\n";

    if (bool1It != root.end()) {
      bool isBool = bool1It->second.value.isBool();
      std::cout << "bool1 (true): " << (isBool ? "BOOLEAN" : "STRING") << "\n";
    }

    if (bool3It != root.end()) {
      bool isBool   = bool3It->second.value.isBool();
      bool isString = bool3It->second.value.isString();
      std::cout << "bool3 (True): " << (isBool ? "BOOLEAN" : isString ? "STRING" : "OTHER") << "\n";
    }

    if (bool5It != root.end()) {
      bool isBool   = bool5It->second.value.isBool();
      bool isString = bool5It->second.value.isString();
      std::cout << "bool5 (TRUE): " << (isBool ? "BOOLEAN" : isString ? "STRING" : "OTHER") << "\n";
    }

    bool mixedCaseAreStrings = (bool3It != root.end() && bool3It->second.value.isString()) ||
                               (bool5It != root.end() && bool5It->second.value.isString());

    if (mixedCaseAreStrings) {
      std::cout << "CONFIRMED: Mixed case booleans treated as strings (limitation exists)\n";
    } else {
      std::cout << "UNEXPECTED: Mixed case booleans work correctly\n";
    }
  } catch (const std::exception &e) {
    std::cout << "Parse result: FAILED\n";
    std::cout << "Error: " << e.what() << "\n";
    std::cout << "CONFIRMED: Boolean parsing causes failure\n";
  }

  return 0;
}
