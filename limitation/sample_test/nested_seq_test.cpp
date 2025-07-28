#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>

using namespace yamlparser;

int main() {
  std::cout << "Testing Nested Sequence Limitation\n";
  std::cout << "==================================\n";

  YamlParser parser;

  try {
    parser.parse("../sample_yaml/nested_seq_test.yaml");
    std::cout << "Parse result: SUCCESS\n";

    auto &root = parser.root();

    auto nestedIt = root.find("nested");
    if (nestedIt != root.end() && nestedIt->second.value.isSeq()) {
      auto &nested = nestedIt->second.value.asSeq();

      std::cout << "Nested sequence size: " << nested.size() << "\n";

      if (nested.size() > 0) {
        bool firstIsMap = nested[0].value.isMap();
        bool firstIsSeq = nested[0].value.isSeq();

        std::cout << "First item is map: " << (firstIsMap ? "YES" : "NO") << "\n";
        std::cout << "First item is sequence: " << (firstIsSeq ? "YES" : "NO") << "\n";

        if (firstIsMap && !firstIsSeq) {
          if (firstIsMap) {
            auto &firstMap = nested[0].value.asMap();
            std::cout << "First item map size: " << firstMap.size() << "\n";
          }
          std::cout << "CONFIRMED: Nested sequences become empty maps (limitation exists)\n";
        } else if (firstIsSeq) {
          std::cout << "UNEXPECTED: Nested sequences work correctly\n";
        } else {
          std::cout << "Unexpected item type in nested sequence\n";
        }
      } else {
        std::cout << "ERROR: Nested sequence is empty\n";
      }
    } else {
      std::cout << "ERROR: Could not find nested as sequence\n";
    }
  } catch (const std::exception &e) {
    std::cout << "Parse result: FAILED\n";
    std::cout << "Error: " << e.what() << "\n";
    std::cout << "CONFIRMED: Nested sequences cause parsing failure\n";
  }

  return 0;
}
