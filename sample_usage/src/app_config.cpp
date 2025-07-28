#include "YamlParser.hpp"
#include "YamlException.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <string>

using namespace yamlparser;

int main() {
  std::cout << "=== Application Configuration Parser Example ===\n\n";

  try {
    YamlParser parser;
    parser.parse("yaml_files/app_config.yaml");

    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }

    const auto &config = parser.root();

    // Application metadata
    std::cout << "Application Information:\n";
    std::cout << "=======================\n";

    const auto &app = config.at("application").value.asMap();
    std::cout << "Name: " << app.at("name").value.asString() << "\n";
    std::cout << "Version: " << app.at("version").value.asString() << "\n";
    std::cout << "\n";

    // Server configuration
    const auto &server = config.at("server").value.asMap();
    std::cout << "Server Configuration:\n";
    std::cout << "====================\n";

    std::cout << "Host: " << server.at("host").value.asString() << "\n";
    std::cout << "Port: " << server.at("port").value.asInt() << "\n";
    std::cout << "Threads: " << server.at("threads").value.asInt() << "\n";

    std::cout << "\n";

    // Database configuration
    const auto &db = config.at("database").value.asMap();
    std::cout << "Database Configuration:\n";
    std::cout << "======================\n";
    std::cout << "Driver: " << db.at("driver").value.asString() << "\n";
    std::cout << "Host: " << db.at("host").value.asString() << "\n";
    std::cout << "Port: " << db.at("port").value.asInt() << "\n";
    std::cout << "Database: " << db.at("database").value.asString() << "\n";
    std::cout << "Username: " << db.at("username").value.asString() << "\n";
    std::cout << "password: " << db.at("password").value.asString() << "\n";
    std::cout << "\n";

    // Logging configuration
    const auto &logging = config.at("logging").value.asMap();
    std::cout << "Logging Configuration:\n";
    std::cout << "=====================\n";
    std::cout << "Level: " << logging.at("level").value.asString() << "\n";
    std::cout << "Log File: " << logging.at("file").value.asString() << "\n";
    std::cout << "Max File Size: " << logging.at("max_size").value.asString() << "\n";
    std::cout << "\n";

    // Features configuration
    const auto &features = config.at("features").value.asMap();
    std::cout << "Feature Flags:\n";
    std::cout << "=============\n";
    for (const auto &feature : features) {
      std::cout << feature.first << ": " << (feature.second.value.asBool() ? "Enabled" : "Disabled") << "\n";
    }
    std::cout << "\n";

    // Cache configuration
    const auto &cache = config.at("cache").value.asMap();
    std::cout << "Cache Configuration:\n";
    std::cout << "==================\n";
    std::cout << "Type: " << cache.at("type").value.asString() << "\n";
    std::cout << "Host: " << cache.at("host").value.asString() << "\n";
    std::cout << "Port: " << cache.at("port").value.asInt() << "\n";
    std::cout << "TTL: " << cache.at("ttl").value.asInt() << " seconds\n";
    std::cout << "\n";

    std::cout << "This example shows how to extract various configuration settings\n";
    std::cout << "from a real-world application configuration file.\n\n";
    std::cout << "✅ Successfully parsed application configuration!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
