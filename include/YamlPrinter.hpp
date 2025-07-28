#pragma once
#include "YamlElement.hpp"
#include <ostream>

/**
 * @file YamlPrinter.hpp
 * @brief Serialization of YAML data structures to text
 *
 * Provides functionality to:
 * - Convert YAML data structures to formatted text
 * - Support proper indentation and nesting
 * - Handle all YAML data types consistently
 * - Output to any standard stream
 *
 * Usage example:
 * @code
 *   YamlMap map;
 *   // ... populate map ...
 *   YamlPrinter::print(map, std::cout);  // Print to console
 *   YamlPrinter::print(map, outFile);    // Print to file
 * @endcode
 */

namespace yamlparser {

/**
 * @brief Static utility class for YAML serialization
 *
 * Handles converting YAML data structures to properly formatted text:
 * - Maintains consistent indentation (2 spaces per level)
 * - Preserves type information in output
 * - Handles special cases (null values, empty strings)
 * - Supports both mapping and sequence root elements
 */
class YamlPrinter {
public:
  static void print(const YamlMap &map, std::ostream &os, int indent = 0);

  static void print(const YamlSeq &seq, std::ostream &os, int indent = 0);

  static void print(const YamlItem &item, std::ostream &os, int indent = 0);
};

} // namespace yamlparser
