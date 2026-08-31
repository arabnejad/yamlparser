// ...existing code...
#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/nested_structures.yaml");
    const auto     &root     = document.asMapping();
    if (!root.count("database") || !root.at("database").isMapping()) {
      std::cerr << "Error: 'database' section missing or not a map\n";
      return 1;
    }
    const auto &db = root.at("database").asMapping();

    // Print all fields in the YAML, matching structure and labels
    std::cout << "Host: " << db.at("host").asString() << std::endl;
    std::cout << "Port: " << db.at("port").asInteger() << std::endl;
    std::cout << "Name: " << db.at("name").asString() << std::endl;

    const auto &pool = db.at("pool").asMapping();
    std::cout << "Pool:" << std::endl;
    std::cout << "  Max Connections: " << pool.at("max_connections").asInteger() << std::endl;
    std::cout << "  Timeout: " << pool.at("timeout").asInteger() << std::endl;
    std::cout << "  Retry Attempts: " << pool.at("retry_attempts").asInteger() << std::endl;

    const auto &creds = db.at("credentials").asMapping();
    std::cout << "Credentials:" << std::endl;
    std::cout << "  Username: " << creds.at("username").asString() << std::endl;
    std::cout << "  Password: " << creds.at("password").asString() << std::endl;
    std::cout << "  SSL Enabled: " << (creds.at("ssl_enabled").asBoolean() ? "true" : "false") << std::endl;

    std::cout << std::endl;
    std::cout << "✅ Successfully parsed nested structures!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
