#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <vector>

using namespace yamlparser;

int main() {
  std::cout << "=== Arrays and Sequences Parser Example ===\n\n";

  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/arrays_sequences.yaml");
    const auto     &config   = document.asMapping();

    // Process fruits array
    const auto &fruits = config.at("fruits").asSequence();

    std::cout << "Fruits List:\n";
    std::cout << "------------\n";
    for (size_t i = 0; i < fruits.size(); ++i) {
      std::cout << (i + 1) << ". " << fruits[i].asString() << "\n";
    }
    std::cout << "\n";

    // Process numbers array
    const auto &numbers = config.at("numbers").asSequence();

    std::cout << "Numbers: ";
    for (const auto &num : numbers) {
      std::cout << num.asInteger() << " ";
    }
    std::cout << "\n\n";

    // Process mixed array
    const auto &mixed = config.at("mixed_array").asSequence();

    std::cout << "Mixed Array:\n";
    std::cout << "------------\n";
    for (size_t i = 0; i < mixed.size(); ++i) {
      std::cout << "Item " << (i + 1) << ": ";

      if (mixed[i].isInteger()) {
        std::cout << "Integer: " << mixed[i].asInteger();
      } else if (mixed[i].isDouble()) {
        std::cout << "Double: " << mixed[i].asDouble();
      } else if (mixed[i].isString()) {
        std::cout << "String: \"" << mixed[i].asString() << "\"";
      } else if (mixed[i].isBoolean()) {
        std::cout << "Boolean: " << (mixed[i].asBoolean() ? "true" : "false");
      } else {
        std::cout << "Unknown type";
      }
      std::cout << "\n";
    }
    std::cout << "\n";

    // Process users array (array of objects)
    const auto &users = config.at("users").asSequence();
    std::cout << "Users:\n";
    std::cout << "------\n";
    for (size_t i = 0; i < users.size(); ++i) {
      const auto &user = users[i].asMapping();
      std::cout << "User " << (i + 1) << ":\n";
      std::cout << "  Name: " << user.at("name").asString() << "\n";
      std::cout << "  Age: " << user.at("age").asInteger() << "\n";
      std::cout << "  Active: " << (user.at("active").asBoolean() ? "Yes" : "No") << "\n";
      std::cout << "\n";
    }

    std::cout << "✅ Successfully parsed arrays and sequences!\n";
  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
