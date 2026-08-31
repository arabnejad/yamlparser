#include "YamlParser.hpp"
#include "YamlException.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "=== Basic Configuration Parser Example ===\n\n";

  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/basic_config.yaml");
    const auto     &config   = document.asMapping();

    // Read basic configuration values
    std::cout << "Configuration Details:\n";
    std::cout << "----------------------\n";
    std::cout << "Name: " << config.at("name").asString() << "\n";
    std::cout << "Version: " << config.at("version").asString() << "\n";
    std::cout << "Enabled: " << (config.at("enabled").asBoolean() ? "Yes" : "No") << "\n";
    std::cout << "Port: " << config.at("port").asInteger() << "\n";
    std::cout << "Timeout: " << config.at("timeout").asInteger() << " seconds\n";
    std::cout << "Description: " << config.at("description").asString() << "\n";

    std::cout << "\n✅ Successfully parsed basic configuration!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
