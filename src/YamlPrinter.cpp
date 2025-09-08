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

namespace {
bool needsQuoting(const std::string &s) {
  if (s.empty())
    return true; // for empty string, "null" will be print
  if (s.front() == ' ' || s.back() == ' ')
    return true;
  // special leading characters or YAML syntax chars
  if (s.front() == '-' || s.front() == '?' || s.front() == ':')
    return true;
  return s.find_first_of(":#{}[],&*!?|>'\"%@`") != std::string::npos;
}

std::string quoteIfNeeded(const std::string &s) {
  if (!needsQuoting(s))
    return s;
  // simple single-quote strategy: double single-quotes inside
  std::string out;
  out.reserve(s.size() + 2);
  out.push_back('\'');
  for (char c : s) {
    out.push_back(c);
    if (c == '\'')
      out.push_back('\''); // escape by doubling
  }
  out.push_back('\'');
  return out;
}
} // anonymous namespace

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
  std::string indentStr = makeIndent(indent);
  for (const auto &kv : map) {
    os << indentStr << quoteIfNeeded(kv.first) << ": ";
    const YamlElement &v = kv.second.value;
    if (v.isString() && v.asString().empty()) {
      // empty string prints as null
      os << "null" << std::endl;
    } else if (v.type == YamlElement::ElementType::NONE) {
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
  std::string indentStr = makeIndent(indent);
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
  const YamlElement &v = item.value;
  switch (v.type) {
  case YamlElement::ElementType::STRING:
    os << quoteIfNeeded(v.asString()) << std::endl;
    break;
  case YamlElement::ElementType::DOUBLE:
    os << v.asDouble() << std::endl;
    break;
  case YamlElement::ElementType::INT:
    os << v.asInt() << std::endl;
    break;
  case YamlElement::ElementType::BOOL:
    os << (v.asBool() ? "true" : "false") << std::endl;
    break;
  case YamlElement::ElementType::SEQ:
    os << std::endl;
    print(v.asSeq(), os, indent + 2);
    break;
  case YamlElement::ElementType::MAP:
    os << std::endl;
    print(v.asMap(), os, indent + 2);
    break;
  default:
    os << "null" << std::endl;
    break;
  }
}
} // namespace yamlparser
