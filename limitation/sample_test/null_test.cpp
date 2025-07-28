#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "Testing Empty Values and Implicit Nulls Limitation\n";
  std::cout << "==================================================\n";

  YamlParser parser;

  try {
    parser.parse("../sample_yaml/null_test.yaml");
    std::cout << "Parse result: SUCCESS\n";

    auto &root = parser.root();

    std::cout << "\nValue analysis:\n";

    auto implicitIt    = root.find("implicit_null");
    auto explicitIt    = root.find("explicit_null");
    auto emptyStringIt = root.find("explicit_string");
    auto normalIt      = root.find("normal_value");

    if (implicitIt != root.end()) {
      bool isString = implicitIt->second.value.isString();
      std::cout << "implicit_null: ";
      if (isString) {
        std::string value = implicitIt->second.value.asString();
        std::cout << "STRING ('" << value << "', length: " << value.length() << ")\n";
      } else {
        std::cout << "OTHER TYPE\n";
      }
    }

    if (explicitIt != root.end()) {
      bool isString = explicitIt->second.value.isString();
      std::cout << "explicit_null: ";
      if (isString) {
        std::string value = explicitIt->second.value.asString();
        std::cout << "STRING ('" << value << "', length: " << value.length() << ")\n";
      } else {
        std::cout << "OTHER TYPE\n";
      }
    }

    if (emptyStringIt != root.end()) {
      bool isString = emptyStringIt->second.value.isString();
      std::cout << "explicit_string: ";
      if (isString) {
        std::string value = emptyStringIt->second.value.asString();
        std::cout << "STRING ('" << value << "', length: " << value.length() << ")\n";
      } else {
        std::cout << "OTHER TYPE\n";
      }
    }

    if (normalIt != root.end()) {
      bool isString = normalIt->second.value.isString();
      std::cout << "normal_value: ";
      if (isString) {
        std::cout << "STRING ('" << normalIt->second.value.asString() << "')\n";
      } else {
        std::cout << "OTHER TYPE\n";
      }
    }

    bool nullsAreStrings = (implicitIt != root.end() && implicitIt->second.value.isString()) ||
                           (explicitIt != root.end() && explicitIt->second.value.isString());

    if (nullsAreStrings) {
      std::cout << "CONFIRMED: Null values treated as strings (limitation exists)\n";
    } else {
      std::cout << "UNEXPECTED: Null values handled correctly\n";
    }
  } catch (const std::exception &e) {
    std::cout << "Parse result: FAILED\n";
    std::cout << "Error: " << e.what() << "\n";
    std::cout << "CONFIRMED: Null values cause parsing failure\n";
  }

  return 0;
}
