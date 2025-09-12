#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "Testing Merge Key Inline Comment Limitation\n";
  std::cout << "===========================================\n";

  YamlParser parser;

  try {
    parser.parse("sample_yaml/merge_comment_test.yaml");
    std::cout << "Parse result: SUCCESS\n";

    auto &root = parser.root();

    auto serviceIt = root.find("service");
    if (serviceIt != root.end() && serviceIt->second.value.isMap()) {
      auto &serviceMap = serviceIt->second.value.asMap();

      bool hasTimeout = serviceMap.find("timeout") != serviceMap.end();
      bool hasRetries = serviceMap.find("retries") != serviceMap.end();
      bool hasName    = serviceMap.find("name") != serviceMap.end();

      std::cout << "Service has timeout: " << (hasTimeout ? "YES" : "NO") << "\n";
      std::cout << "Service has retries: " << (hasRetries ? "YES" : "NO") << "\n";
      std::cout << "Service has name: " << (hasName ? "YES" : "NO") << "\n";

      if (!hasTimeout || !hasRetries) {
        std::cout << "CONFIRMED: Inline comment breaks merge functionality (limitation exists)\n";
      } else {
        std::cout << "UNEXPECTED: Merge works despite inline comment\n";
      }
    } else {
      std::cout << "ERROR: Could not find service mapping\n";
    }
  } catch (const std::exception &e) {
    std::cout << "Parse result: FAILED\n";
    std::cout << "Error: " << e.what() << "\n";
    std::cout << "CONFIRMED: Merge with inline comment causes parsing failure\n";
  }

  return 0;
}
