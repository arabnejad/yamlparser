
#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <string>

using namespace yamlparser;

int main() {
  std::cout << "=== Nested Arrays Parser Example ===\n\n";

  try {
    const YamlValue document = YamlParser().parseFile("yaml_files/nested_arrays.yaml");
    const auto     &config   = document.asMapping();

    // Print nested_string_arrays
    const auto &nested_str = config.at("nested_string_arrays").asSequence();
    std::cout << "Nested String Arrays:\n";
    std::cout << "---------------------\n";
    for (size_t i = 0; i < nested_str.size(); ++i) {
      const auto &arr = nested_str[i].asSequence();
      std::cout << "[ ";
      for (size_t j = 0; j < arr.size(); ++j) {
        std::cout << '"' << arr[j].asString() << '"';
        if (j < arr.size() - 1)
          std::cout << ", ";
      }
      std::cout << " ]\n";
    }
    std::cout << "\n";

    // Print matrix
    const auto &matrix = config.at("matrix").asSequence();
    std::cout << "Matrix:\n";
    std::cout << "-------\n";
    for (size_t i = 0; i < matrix.size(); ++i) {
      const auto &row = matrix[i].asSequence();
      std::cout << "Row " << (i + 1) << ": [ ";
      for (size_t j = 0; j < row.size(); ++j) {
        if (row[j].isInteger())
          std::cout << row[j].asInteger();
        else if (row[j].isDouble())
          std::cout << row[j].asDouble();
        else if (row[j].isString())
          std::cout << '"' << row[j].asString() << '"';
        else
          std::cout << "?";
        if (j < row.size() - 1)
          std::cout << ", ";
      }
      std::cout << " ]\n";
    }
    std::cout << "\n";

    // Print coordinates
    const auto &coords = config.at("coordinates").asSequence();
    std::cout << "Coordinates:\n";
    std::cout << "-----------\n";
    for (size_t i = 0; i < coords.size(); ++i) {
      const auto &point = coords[i].asMapping();
      std::cout << "Point " << (i + 1) << ": ";
      std::cout << "x=" << point.at("x").asInteger() << ", ";
      std::cout << "y=" << point.at("y").asInteger();
      std::cout << "\n";
    }
    std::cout << "\n";

    // Print categories and items
    const auto &categories = config.at("categories").asSequence();
    std::cout << "Categories:\n";
    std::cout << "----------\n";
    for (size_t i = 0; i < categories.size(); ++i) {
      const auto &cat = categories[i].asMapping();
      std::cout << "Category: " << cat.at("name").asString() << "\n";
      const auto &items = cat.at("items").asSequence();
      for (size_t j = 0; j < items.size(); ++j) {
        const auto &item = items[j].asMapping();
        std::cout << "  - ";
        std::cout << item.at("name").asString();
        std::cout << " ($" << item.at("price").asDouble() << ")";
        std::cout << "\n";
      }
    }
    std::cout << "\n";

    // Print nested_arrays
    const auto &nested = config.at("nested_arrays").asSequence();
    std::cout << "Nested Arrays:\n";
    std::cout << "--------------\n";
    for (size_t i = 0; i < nested.size(); ++i) {
      const auto &arr = nested[i].asSequence();
      std::cout << "[ ";
      for (size_t j = 0; j < arr.size(); ++j) {
        if (arr[j].isString()) {
          std::cout << '"' << arr[j].asString() << '"';
        } else if (arr[j].isInteger()) {
          std::cout << arr[j].asInteger();
        } else if (arr[j].isDouble()) {
          std::cout << arr[j].asDouble();
        } else {
          std::cout << "?";
        }
        if (j < arr.size() - 1)
          std::cout << ", ";
      }
      std::cout << " ]\n";
    }
    std::cout << "\n";

    std::cout << "✅ Successfully parsed nested arrays and objects!\n";

  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
