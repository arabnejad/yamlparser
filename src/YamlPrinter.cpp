#include "YamlPrinter.hpp"
#include <ostream>

// YamlPrinter implementation - Converts YAML data structures to formatted text
// Key features:
// - Recursive traversal of nested structures (maps and sequences)
// - Consistent 2-space indentation for nested levels
// - Special handling of null values and empty strings
// - YAML-compliant formatting for all scalar types
// - Stream-based output for efficiency

namespace yamlparser {

// Return a string of n spaces for indentation
/**
 * @brief Creates an indentation string with specified number of spaces
 * @param n Number of spaces to generate
 * @return A string containing n spaces
 */
std::string makeIndent(int n) {
  return std::string(static_cast<std::string::size_type>(n), ' ');
}

/**
 * @brief Prints a YAML mapping to an output stream
 * @param map The YAML mapping to print
 * @param os The output stream to write to
 * @param indent Current indentation level (number of spaces)
 * @details Handles:
 *          - Key-value pair formatting
 *          - Proper indentation of nested structures
 *          - Special handling of null values
 *          - Empty string conversion to null
 */
void YamlPrinter::print(const YamlMap &map, std::ostream &os, int indent) {
  std::string indentStr(makeIndent(indent));
  for (const auto &kv : map) {
    os << indentStr << kv.first << ": ";
    // Special case: key is literally "null"
    if (kv.first == "null") {
      os << "null" << std::endl;
      continue;
    }
    // Convert NONE type or empty strings to explicit "null"
    if (kv.second.value.type == YamlElement::ElementType::NONE ||
        (kv.second.value.isString() && kv.second.value.asString().empty())) {
      os << "null" << std::endl;
    } else {
      print(kv.second, os, indent + 2);
    }
  }
}

/**
 * @brief Prints a YAML sequence to an output stream
 * @param seq The YAML sequence to print
 * @param os The output stream to write to
 * @param indent Current indentation level (number of spaces)
 * @details Handles:
 *          - Sequence item formatting with '-' prefix
 *          - Proper indentation of nested items
 *          - Recursive printing of complex items
 */
void YamlPrinter::print(const YamlSeq &seq, std::ostream &os, int indent) {
  std::string indentStr(static_cast<std::string::size_type>(indent), ' ');
  for (const auto &item : seq) {
    os << indentStr << "- ";
    print(item, os, indent + 2);
  }
}

/**
 * @brief Prints a YAML item to an output stream
 * @param item The YAML item to print
 * @param os The output stream to write to
 * @param indent Current indentation level (number of spaces)
 * @details Handles all YAML element types:
 *          - Strings (including empty strings as null)
 *          - Numbers (both integer and floating point)
 *          - Booleans
 *          - Null values
 *          - Nested maps and sequences
 */
void YamlPrinter::print(const YamlItem &item, std::ostream &os, int indent) {
  switch (item.value.type) {
  case YamlElement::ElementType::STRING: {
    const std::string &str = *item.value.data.str;
    if (str.empty())
      os << "null" << std::endl;
    else
      os << str << std::endl;
    break;
  }
  case YamlElement::ElementType::DOUBLE:
    os << item.value.data.d << std::endl;
    break;
  case YamlElement::ElementType::INT:
    os << item.value.data.i << std::endl;
    break;
  case YamlElement::ElementType::BOOL:
    os << (item.value.data.b ? "true" : "false") << std::endl;
    break;
  case YamlElement::ElementType::SEQ:
    os << std::endl;
    print(*item.value.data.seq, os, indent + 2);
    break;
  case YamlElement::ElementType::MAP:
    os << std::endl;
    print(*item.value.data.map, os, indent + 2);
    break;
  default:
    os << "null" << std::endl;
    break;
  }
}

} // namespace yamlparser
