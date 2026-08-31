#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "=== Anchors and Merge Keys Parser Example ===\n\n";

  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/anchors_merge.yaml");
    const auto     &config   = document.asMapping();

    // List environments (development, production, staging)
    const char *envs[] = {"development", "production", "staging"};
    for (const char *env : envs) {
      const auto &envMap = config.at(env).asMapping();
      std::cout << "Environment: " << env << "\n";
      std::cout << "-------------\n";
      for (const auto &field : envMap) {
        if (field.second.isString()) {
          std::cout << field.first << ": " << field.second.asString() << "\n";
        } else if (field.second.isInteger()) {
          std::cout << field.first << ": " << field.second.asInteger() << "\n";
        } else if (field.second.isDouble()) {
          std::cout << field.first << ": " << field.second.asDouble() << "\n";
        } else if (field.second.isBoolean()) {
          std::cout << field.first << ": " << (field.second.asBoolean() ? "true" : "false") << "\n";
        }
      }
      std::cout << std::endl;
    }

    // Show default settings
    const auto &defaults = config.at("defaults").asMapping();
    std::cout << "Default Settings (anchor):\n";
    std::cout << "-------------------------\n";
    for (const auto &field : defaults) {
      if (field.second.isString()) {
        std::cout << field.first << ": " << field.second.asString() << "\n";
      } else if (field.second.isInteger()) {
        std::cout << field.first << ": " << field.second.asInteger() << "\n";
      } else if (field.second.isDouble()) {
        std::cout << field.first << ": " << field.second.asDouble() << "\n";
      } else if (field.second.isBoolean()) {
        std::cout << field.first << ": " << (field.second.asBoolean() ? "true" : "false") << "\n";
      }
    }
    std::cout << std::endl;

    std::cout << "✅ Successfully parsed anchors and merge keys!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
