#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

void printMapLevel(const std::map<std::string, YamlValue> &map, int level = 0) {
  const std::string indent(level * 2, ' ');
  for (const auto &pair : map) {
    std::cout << indent << pair.first << ": ";
    if (pair.second.isString()) {
      std::cout << '"' << pair.second.asString() << '"' << std::endl;
    } else if (pair.second.isInteger()) {
      std::cout << pair.second.asInteger() << std::endl;
    } else if (pair.second.isDouble()) {
      std::cout << pair.second.asDouble() << std::endl;
    } else if (pair.second.isBoolean()) {
      std::cout << (pair.second.asBoolean() ? "true" : "false") << std::endl;
    } else if (pair.second.isMapping()) {
      std::cout << std::endl;
      printMapLevel(pair.second.asMapping(), level + 1);
    } else if (pair.second.isSequence()) {
      std::cout << "[array with " << pair.second.asSequence().size() << " elements]" << std::endl;
    } else {
      std::cout << "[unknown type]" << std::endl;
    }
  }
}

int main() {
  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/nested_maps.yaml");
    const auto     &root     = document.asMapping();

    // Print Servers in requested order
    if (root.count("servers") && root.at("servers").isMapping()) {
      const auto &servers = root.at("servers").asMapping();
      std::cout << "Servers:" << std::endl;
      std::cout << "--------" << std::endl;
      // Print in order: database, web1, web2
      const char *server_keys[] = {"database", "web1", "web2"};
      for (const auto &key : server_keys) {
        auto it = servers.find(key);
        if (it != servers.end() && it->second.isMapping()) {
          const auto &srvMap = it->second.asMapping();
          std::cout << key << ":\n";
          if (srvMap.count("host") && srvMap.at("host").isString())
            std::cout << "  Host: " << srvMap.at("host").asString() << std::endl;
          if (srvMap.count("port") && srvMap.at("port").isInteger())
            std::cout << "  Port: " << srvMap.at("port").asInteger() << std::endl;
          if (srvMap.count("ssl") && srvMap.at("ssl").isBoolean())
            std::cout << "  SSL: " << (srvMap.at("ssl").asBoolean() ? "true" : "false") << std::endl;
        }
      }
      std::cout << std::endl;
    }

    // Print Environments in requested order
    if (root.count("environments") && root.at("environments").isMapping()) {
      const auto &envs = root.at("environments").asMapping();
      std::cout << "Environments:" << std::endl;
      std::cout << "------------" << std::endl;
      const char *env_keys[] = {"development", "production"};
      for (const auto &key : env_keys) {
        auto it = envs.find(key);
        if (it != envs.end() && it->second.isMapping()) {
          const auto &envMap = it->second.asMapping();
          std::cout << key << ":\n";
          if (envMap.count("api_url") && envMap.at("api_url").isString())
            std::cout << "  API URL: " << envMap.at("api_url").asString() << std::endl;
          if (envMap.count("debug") && envMap.at("debug").isBoolean())
            std::cout << "  Debug: " << (envMap.at("debug").asBoolean() ? "true" : "false") << std::endl;
          if (envMap.count("log_level") && envMap.at("log_level").isString())
            std::cout << "  Log Level: " << envMap.at("log_level").asString() << std::endl;
        }
      }
      std::cout << std::endl;
    }
    std::cout << "✅ Successfully parsed nested maps!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
