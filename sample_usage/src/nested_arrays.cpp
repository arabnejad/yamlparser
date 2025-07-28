
#include "YamlParser.hpp"
#include "YamlException.hpp"
#include <iostream>
#include <string>

using namespace yamlparser;

int main() {
  std::cout << "=== Nested Arrays Parser Example ===\n\n";

  try {
    YamlParser parser;
    parser.parse("yaml_files/nested_arrays.yaml");
    if (parser.isSequenceRoot()) {
      std::cerr << "Error: Expected root to be a map, but got sequence\n";
      return 1;
    }

    const auto &config = parser.root();

    // Print nested_string_arrays
    const auto &nested_str = config.at("nested_string_arrays").value.asSeq();
    std::cout << "Nested String Arrays:\n";
    std::cout << "---------------------\n";
    for (size_t i = 0; i < nested_str.size(); ++i) {
      const auto &arr = nested_str[i].value.asSeq();
      std::cout << "[ ";
      for (size_t j = 0; j < arr.size(); ++j) {
        std::cout << '"' << arr[j].value.asString() << '"';
        if (j < arr.size() - 1)
          std::cout << ", ";
      }
      std::cout << " ]\n";
    }
    std::cout << "\n";

    // Print matrix
    const auto &matrix = config.at("matrix").value.asSeq();
    std::cout << "Matrix:\n";
    std::cout << "-------\n";
    for (size_t i = 0; i < matrix.size(); ++i) {
      const auto &row = matrix[i].value.asSeq();
      std::cout << "Row " << (i + 1) << ": [ ";
      for (size_t j = 0; j < row.size(); ++j) {
        if (row[j].value.isInt())
          std::cout << row[j].value.asInt();
        else if (row[j].value.isDouble())
          std::cout << row[j].value.asDouble();
        else if (row[j].value.isString())
          std::cout << '"' << row[j].value.asString() << '"';
        else
          std::cout << "?";
        if (j < row.size() - 1)
          std::cout << ", ";
      }
      std::cout << " ]\n";
    }
    std::cout << "\n";

    // Print coordinates
    const auto &coords = config.at("coordinates").value.asSeq();
    std::cout << "Coordinates:\n";
    std::cout << "-----------\n";
    for (size_t i = 0; i < coords.size(); ++i) {
      const auto &point = coords[i].value.asMap();
      std::cout << "Point " << (i + 1) << ": ";
      std::cout << "x=" << point.at("x").value.asInt() << ", ";
      std::cout << "y=" << point.at("y").value.asInt();
      std::cout << "\n";
    }
    std::cout << "\n";

    // Print categories and items
    const auto &categories = config.at("categories").value.asSeq();
    std::cout << "Categories:\n";
    std::cout << "----------\n";
    for (size_t i = 0; i < categories.size(); ++i) {
      const auto &cat = categories[i].value.asMap();
      std::cout << "Category: " << cat.at("name").value.asString() << "\n";
      const auto &items = cat.at("items").value.asSeq();
      for (size_t j = 0; j < items.size(); ++j) {
        const auto &item = items[j].value.asMap();
        std::cout << "  - ";
        std::cout << item.at("name").value.asString();
        std::cout << " ($" << item.at("price").value.asDouble() << ")";
        std::cout << "\n";
      }
    }
    std::cout << "\n";

    // Print nested_arrays
    const auto &nested = config.at("nested_arrays").value.asSeq();
    std::cout << "Nested Arrays:\n";
    std::cout << "--------------\n";
    for (size_t i = 0; i < nested.size(); ++i) {
      const auto &arr = nested[i].value.asSeq();
      std::cout << "[ ";
      for (size_t j = 0; j < arr.size(); ++j) {
        if (arr[j].value.isString()) {
          std::cout << '"' << arr[j].value.asString() << '"';
        } else if (arr[j].value.isInt()) {
          std::cout << arr[j].value.asInt();
        } else if (arr[j].value.isDouble()) {
          std::cout << arr[j].value.asDouble();
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
