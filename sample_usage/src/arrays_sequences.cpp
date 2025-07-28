#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <vector>

using namespace yamlparser;

int main() {
  std::cout << "=== Arrays and Sequences Parser Example ===\n\n";

  try {
    YamlParser parser;
    parser.parse("yaml_files/arrays_sequences.yaml");
    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }

    const auto &config = parser.root();

    // Process fruits array
    const auto &fruits = config.at("fruits").value.asSeq();

    std::cout << "Fruits List:\n";
    std::cout << "------------\n";
    for (size_t i = 0; i < fruits.size(); ++i) {
      std::cout << (i + 1) << ". " << fruits[i].value.asString() << "\n";
    }
    std::cout << "\n";

    // Process numbers array
    const auto &numbers = config.at("numbers").value.asSeq();

    std::cout << "Numbers: ";
    for (const auto &num : numbers) {
      std::cout << num.value.asInt() << " ";
    }
    std::cout << "\n\n";

    // Process mixed array
    const auto &mixed = config.at("mixed_array").value.asSeq();

    std::cout << "Mixed Array:\n";
    std::cout << "------------\n";
    for (size_t i = 0; i < mixed.size(); ++i) {
      std::cout << "Item " << (i + 1) << ": ";

      if (mixed[i].value.isInt()) {
        std::cout << "Integer: " << mixed[i].value.asInt();
      } else if (mixed[i].value.isDouble()) {
        std::cout << "Double: " << mixed[i].value.asDouble();
      } else if (mixed[i].value.isString()) {
        std::cout << "String: \"" << mixed[i].value.asString() << "\"";
      } else if (mixed[i].value.isBool()) {
        std::cout << "Boolean: " << (mixed[i].value.asBool() ? "true" : "false");
      } else {
        std::cout << "Unknown type";
      }
      std::cout << "\n";
    }
    std::cout << "\n";

    // Process users array (array of objects)
    const auto &users = config.at("users").value.asSeq();
    std::cout << "Users:\n";
    std::cout << "------\n";
    for (size_t i = 0; i < users.size(); ++i) {
      const auto &user = users[i].value.asMap();
      std::cout << "User " << (i + 1) << ":\n";
      std::cout << "  Name: " << user.at("name").value.asString() << "\n";
      std::cout << "  Age: " << user.at("age").value.asInt() << "\n";
      std::cout << "  Active: " << (user.at("active").value.asBool() ? "Yes" : "No") << "\n";
      std::cout << "\n";
    }

    std::cout << "✅ Successfully parsed arrays and sequences!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
