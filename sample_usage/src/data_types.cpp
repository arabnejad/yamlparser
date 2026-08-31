#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <iomanip>

using namespace yamlparser;

int main() {
  std::cout << "=== Data Types Parser Example ===\n\n";

  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/data_types.yaml");
    const auto     &config   = document.asMapping();

    std::cout << "YAML Data Types:\n";
    std::cout << "================\n\n";

    for (const auto &item : config) {
      std::cout << item.first << ": ";
      if (item.second.isString()) {
        std::cout << '"' << item.second.asString() << '"' << " (string)";
      } else if (item.second.isInteger()) {
        std::cout << item.second.asInteger() << " (integer)";
      } else if (item.second.isDouble()) {
        std::cout << std::fixed << std::setprecision(6) << item.second.asDouble() << " (float)";
      } else if (item.second.isBoolean()) {
        std::cout << (item.second.asBoolean() ? "true" : "false") << " (boolean)";
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
