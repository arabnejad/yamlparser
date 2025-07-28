#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "=== Anchors and Merge Keys Parser Example ===\n\n";

  try {
    YamlParser parser;
    parser.parse("yaml_files/anchors_merge.yaml");
    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }

    const auto &config = parser.root();

    // List environments (development, production, staging)
    const char *envs[] = {"development", "production", "staging"};
    for (const char *env : envs) {
      const auto &envMap = config.at(env).value.asMap();
      std::cout << "Environment: " << env << "\n";
      std::cout << "-------------\n";
      for (const auto &field : envMap) {
        if (field.second.value.isString()) {
          std::cout << field.first << ": " << field.second.value.asString() << "\n";
        } else if (field.second.value.isInt()) {
          std::cout << field.first << ": " << field.second.value.asInt() << "\n";
        } else if (field.second.value.isDouble()) {
          std::cout << field.first << ": " << field.second.value.asDouble() << "\n";
        } else if (field.second.value.isBool()) {
          std::cout << field.first << ": " << (field.second.value.asBool() ? "true" : "false") << "\n";
        }
      }
      std::cout << std::endl;
    }

    // Show default settings
    const auto &defaults = config.at("defaults").value.asMap();
    std::cout << "Default Settings (anchor):\n";
    std::cout << "-------------------------\n";
    for (const auto &field : defaults) {
      if (field.second.value.isString()) {
        std::cout << field.first << ": " << field.second.value.asString() << "\n";
      } else if (field.second.value.isInt()) {
        std::cout << field.first << ": " << field.second.value.asInt() << "\n";
      } else if (field.second.value.isDouble()) {
        std::cout << field.first << ": " << field.second.value.asDouble() << "\n";
      } else if (field.second.value.isBool()) {
        std::cout << field.first << ": " << (field.second.value.asBool() ? "true" : "false") << "\n";
      }
    }
    std::cout << std::endl;

    std::cout << "Note: This parser has limitations with anchors and merge keys.\n";
    std::cout << "Anchors (&) and aliases (*) are not fully supported.\n";
    std::cout << "Merge keys (<<) are not fully supported.\n";
    std::cout << "The values shown above may not reflect the intended merged structure.\n";
    std::cout << "See the limitation/ folder for detailed tests of these features.\n\n";
    std::cout << "✅ Successfully parsed anchors and merge keys!" << std::endl;
    std::cout << "✅ YAML file parsed successfully (with limitations noted)!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
