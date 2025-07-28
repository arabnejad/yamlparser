#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <iomanip>

using namespace yamlparser;

int main() {
  std::cout << "=== Data Types Parser Example ===\n\n";

  try {
    YamlParser parser;
    parser.parse("yaml_files/data_types.yaml");
    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }

    const auto &config = parser.root();

    std::cout << "YAML Data Types:\n";
    std::cout << "================\n\n";

    for (const auto &item : config) {
      std::cout << item.first << ": ";
      if (item.second.value.isString()) {
        std::cout << '"' << item.second.value.asString() << '"' << " (string)";
      } else if (item.second.value.isInt()) {
        std::cout << item.second.value.asInt() << " (integer)";
      } else if (item.second.value.isDouble()) {
        std::cout << std::fixed << std::setprecision(6) << item.second.value.asDouble() << " (float)";
      } else if (item.second.value.isBool()) {
        std::cout << (item.second.value.asBool() ? "true" : "false") << " (boolean)";
      } else {
        std::cout << "[unknown type]";
      }
      std::cout << "\n";
    }

    std::cout << "\n✅ Successfully analyzed YAML data types!\n";

  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
