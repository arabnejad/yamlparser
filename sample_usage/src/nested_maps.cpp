#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

void printMapLevel(const std::map<std::string, YamlItem> &map, int level = 0) {
  const std::string indent(level * 2, ' ');
  for (const auto &pair : map) {
    std::cout << indent << pair.first << ": ";
    if (pair.second.value.isString()) {
      std::cout << '"' << pair.second.value.asString() << '"' << std::endl;
    } else if (pair.second.value.isInt()) {
      std::cout << pair.second.value.asInt() << std::endl;
    } else if (pair.second.value.isDouble()) {
      std::cout << pair.second.value.asDouble() << std::endl;
    } else if (pair.second.value.isBool()) {
      std::cout << (pair.second.value.asBool() ? "true" : "false") << std::endl;
    } else if (pair.second.value.isMap()) {
      std::cout << std::endl;
      printMapLevel(pair.second.value.asMap(), level + 1);
    } else if (pair.second.value.isSeq()) {
      std::cout << "[array with " << pair.second.value.asSeq().size() << " elements]" << std::endl;
    } else {
      std::cout << "[unknown type]" << std::endl;
    }
  }
}

int main() {
  try {
    YamlParser parser;
    parser.parse("yaml_files/nested_maps.yaml");
    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }
    const auto &root = parser.root();

    // Print Servers in requested order
    if (root.count("servers") && root.at("servers").value.isMap()) {
      const auto &servers = root.at("servers").value.asMap();
      std::cout << "Servers:" << std::endl;
      std::cout << "--------" << std::endl;
      // Print in order: database, web1, web2
      const char *server_keys[] = {"database", "web1", "web2"};
      for (const auto &key : server_keys) {
        auto it = servers.find(key);
        if (it != servers.end() && it->second.value.isMap()) {
          const auto &srvMap = it->second.value.asMap();
          std::cout << key << ":\n";
          if (srvMap.count("host") && srvMap.at("host").value.isString())
            std::cout << "  Host: " << srvMap.at("host").value.asString() << std::endl;
          if (srvMap.count("port") && srvMap.at("port").value.isInt())
            std::cout << "  Port: " << srvMap.at("port").value.asInt() << std::endl;
          if (srvMap.count("ssl") && srvMap.at("ssl").value.isBool())
            std::cout << "  SSL: " << (srvMap.at("ssl").value.asBool() ? "true" : "false") << std::endl;
        }
      }
      std::cout << std::endl;
    }

    // Print Environments in requested order
    if (root.count("environments") && root.at("environments").value.isMap()) {
      const auto &envs = root.at("environments").value.asMap();
      std::cout << "Environments:" << std::endl;
      std::cout << "------------" << std::endl;
      const char *env_keys[] = {"development", "production"};
      for (const auto &key : env_keys) {
        auto it = envs.find(key);
        if (it != envs.end() && it->second.value.isMap()) {
          const auto &envMap = it->second.value.asMap();
          std::cout << key << ":\n";
          if (envMap.count("api_url") && envMap.at("api_url").value.isString())
            std::cout << "  API URL: " << envMap.at("api_url").value.asString() << std::endl;
          if (envMap.count("debug") && envMap.at("debug").value.isBool())
            std::cout << "  Debug: " << (envMap.at("debug").value.asBool() ? "true" : "false") << std::endl;
          if (envMap.count("log_level") && envMap.at("log_level").value.isString())
            std::cout << "  Log Level: " << envMap.at("log_level").value.asString() << std::endl;
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
