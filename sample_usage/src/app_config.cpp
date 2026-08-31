#include "YamlParser.hpp"
#include "YamlException.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <string>

using namespace yamlparser;

int main() {
  std::cout << "=== Application Configuration Parser Example ===\n\n";

  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/app_config.yaml");
    const auto     &config   = document.asMapping();

    // Application metadata
    std::cout << "Application Information:\n";
    std::cout << "=======================\n";

    const auto &app = config.at("application").asMapping();
    std::cout << "Name: " << app.at("name").asString() << "\n";
    std::cout << "Version: " << app.at("version").asString() << "\n";
    std::cout << "\n";

    // Server configuration
    const auto &server = config.at("server").asMapping();
    std::cout << "Server Configuration:\n";
    std::cout << "====================\n";

    std::cout << "Host: " << server.at("host").asString() << "\n";
    std::cout << "Port: " << server.at("port").asInteger() << "\n";
    std::cout << "Threads: " << server.at("threads").asInteger() << "\n";

    std::cout << "\n";

    // Database configuration
    const auto &db = config.at("database").asMapping();
    std::cout << "Database Configuration:\n";
    std::cout << "======================\n";
    std::cout << "Driver: " << db.at("driver").asString() << "\n";
    std::cout << "Host: " << db.at("host").asString() << "\n";
    std::cout << "Port: " << db.at("port").asInteger() << "\n";
    std::cout << "Database: " << db.at("database").asString() << "\n";
    std::cout << "Username: " << db.at("username").asString() << "\n";
    std::cout << "password: " << db.at("password").asString() << "\n";
    std::cout << "\n";

    // Logging configuration
    const auto &logging = config.at("logging").asMapping();
    std::cout << "Logging Configuration:\n";
    std::cout << "=====================\n";
    std::cout << "Level: " << logging.at("level").asString() << "\n";
    std::cout << "Log File: " << logging.at("file").asString() << "\n";
    std::cout << "Max File Size: " << logging.at("max_size").asString() << "\n";
    std::cout << "\n";

    // Features configuration
    const auto &features = config.at("features").asMapping();
    std::cout << "Feature Flags:\n";
    std::cout << "=============\n";
    for (const auto &feature : features) {
      std::cout << feature.first << ": " << (feature.second.asBoolean() ? "Enabled" : "Disabled") << "\n";
    }
    std::cout << "\n";

    // Cache configuration
    const auto &cache = config.at("cache").asMapping();
    std::cout << "Cache Configuration:\n";
    std::cout << "==================\n";
    std::cout << "Type: " << cache.at("type").asString() << "\n";
    std::cout << "Host: " << cache.at("host").asString() << "\n";
    std::cout << "Port: " << cache.at("port").asInteger() << "\n";
    std::cout << "TTL: " << cache.at("ttl").asInteger() << " seconds\n";
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
