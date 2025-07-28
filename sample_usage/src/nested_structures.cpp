// ...existing code...
#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  try {
    YamlParser parser;
    parser.parse("yaml_files/nested_structures.yaml");
    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }
    const auto &root = parser.root();
    if (!root.count("database") || !root.at("database").value.isMap()) {
      std::cerr << "Error: 'database' section missing or not a map\n";
      return 1;
    }
    const auto &db = root.at("database").value.asMap();

    // Print all fields in the YAML, matching structure and labels
    std::cout << "Host: " << db.at("host").value.asString() << std::endl;
    std::cout << "Port: " << db.at("port").value.asInt() << std::endl;
    std::cout << "Name: " << db.at("name").value.asString() << std::endl;

    const auto &pool = db.at("pool").value.asMap();
    std::cout << "Pool:" << std::endl;
    std::cout << "  Max Connections: " << pool.at("max_connections").value.asInt() << std::endl;
    std::cout << "  Timeout: " << pool.at("timeout").value.asInt() << std::endl;
    std::cout << "  Retry Attempts: " << pool.at("retry_attempts").value.asInt() << std::endl;

    const auto &creds = db.at("credentials").value.asMap();
    std::cout << "Credentials:" << std::endl;
    std::cout << "  Username: " << creds.at("username").value.asString() << std::endl;
    std::cout << "  Password: " << creds.at("password").value.asString() << std::endl;
    std::cout << "  SSL Enabled: " << (creds.at("ssl_enabled").value.asBool() ? "true" : "false") << std::endl;

    std::cout << std::endl;
    std::cout << "✅ Successfully parsed nested structures!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
