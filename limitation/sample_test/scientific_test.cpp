#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "Testing Scientific Notation Limitation\n";
  std::cout << "======================================\n";

  YamlParser parser;

  try {
    parser.parse("../sample_yaml/scientific_test.yaml");
    std::cout << "Parse result: SUCCESS\n";

    auto &root = parser.root();

    auto scientificIt = root.find("scientific");
    auto largeIt      = root.find("large_scientific");
    auto normalIt     = root.find("normal_decimal");
    auto integerIt    = root.find("integer");

    std::cout << "\nValue types:\n";

    if (scientificIt != root.end()) {
      bool isDouble = scientificIt->second.value.isDouble();
      bool isInt    = scientificIt->second.value.isInt();
      bool isString = scientificIt->second.value.isString();

      std::cout << "scientific (1.23e-4): ";
      if (isDouble) {
        std::cout << "DOUBLE (" << scientificIt->second.value.asDouble() << ")\n";
      } else if (isInt) {
        std::cout << "INT (" << scientificIt->second.value.asInt() << ")\n";
      } else if (isString) {
        std::cout << "STRING ('" << scientificIt->second.value.asString() << "')\n";
      } else {
        std::cout << "OTHER TYPE\n";
      }
    }

    if (largeIt != root.end()) {
      bool isDouble = largeIt->second.value.isDouble();
      bool isString = largeIt->second.value.isString();

      std::cout << "large_scientific: ";
      if (isDouble) {
        std::cout << "DOUBLE (" << largeIt->second.value.asDouble() << ")\n";
      } else if (isString) {
        std::cout << "STRING ('" << largeIt->second.value.asString() << "')\n";
      } else {
        std::cout << "OTHER TYPE\n";
      }
    }

    if (normalIt != root.end()) {
      bool isDouble = normalIt->second.value.isDouble();
      std::cout << "normal_decimal: " << (isDouble ? "DOUBLE" : "OTHER") << "\n";
    }

    if (integerIt != root.end()) {
      bool isInt = integerIt->second.value.isInt();
      std::cout << "integer: " << (isInt ? "INT" : "OTHER") << "\n";
    }

    bool scientificIsString = scientificIt != root.end() && scientificIt->second.value.isString();
    if (scientificIsString) {
      std::cout << "CONFIRMED: Scientific notation parsed as string (limitation exists)\n";
    } else {
      std::cout << "UNEXPECTED: Scientific notation parsed as number\n";
    }
  } catch (const std::exception &e) {
    std::cout << "Parse result: FAILED\n";
    std::cout << "Error: " << e.what() << "\n";
    std::cout << "CONFIRMED: Scientific notation causes parsing failure\n";
  }

  return 0;
}
