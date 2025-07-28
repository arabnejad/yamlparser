#include "YamlParser.hpp"
#include "YamlException.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "=== Basic Configuration Parser Example ===\n\n";

  try {
    YamlParser parser;
    parser.parse("yaml_files/basic_config.yaml");

    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }

    const auto &config = parser.root();

    // Read basic configuration values
    std::cout << "Configuration Details:\n";
    std::cout << "----------------------\n";
    std::cout << "Name: " << config.at("name").value.asString() << "\n";
    std::cout << "Version: " << config.at("version").value.asString() << "\n";
    std::cout << "Enabled: " << (config.at("enabled").value.asBool() ? "Yes" : "No") << "\n";
    std::cout << "Port: " << config.at("port").value.asInt() << "\n";
    std::cout << "Timeout: " << config.at("timeout").value.asInt() << " seconds\n";
    std::cout << "Description: " << config.at("description").value.asString() << "\n";

    std::cout << "\n✅ Successfully parsed basic configuration!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
