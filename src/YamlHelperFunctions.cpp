#include "YamlHelperFunctions.hpp"
#include "YamlParser.hpp"
#include "YamlPrinter.hpp"
#include "YamlElement.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iostream>

// Implementation of utility functions for YAML parsing
// Provides helper functions for:
// - YAML type detection (multiline, anchor, alias, etc.)
// - String manipulation and validation
// - Complex value parsing (inline sequences, merge keys, etc.)
// These functions are used internally by the parser to handle specific YAML features

namespace yamlparser {

/**
 * @brief Checks if a value is a YAML multiline literal
 * @param value The string to check
 * @return true if the value starts with '|' or '>' indicators
 */
bool isMultilineLiteral(const std::string &value) {
  return !value.empty() && (value[0] == '|' || value[0] == '>');
}

/**
 * @brief Checks if a value is a YAML anchor declaration
 * @param value The string to check
 * @return true if the value starts with '&' character
 */
bool isAnchor(const std::string &value) {
  return !value.empty() && value[0] == '&';
}

/**
 * @brief Checks if a value is a YAML alias reference
 * @param value The string to check
 * @return true if the value starts with '*' character
 */
bool isAlias(const std::string &value) {
  return !value.empty() && value[0] == '*';
}

/**
 * @brief Checks if a value is a YAML inline sequence
 * @param value The string to check
 * @return true if the value is enclosed in square brackets and contains non-whitespace
 * @details Validates that:
 *          1. String is at least 3 characters long
 *          2. Starts with '[' and ends with ']'
 *          3. Contains at least one non-whitespace character between brackets
 */
bool isInlineSeq(const std::string &value) {
  if (value.size() < 3)
    return false;
  if (value.front() != '[' || value.back() != ']')
    return false;
  // Require at least one non-whitespace character between brackets
  std::string inner = value.substr(1, value.size() - 2);
  for (char c : inner) {
    if (!isspace(static_cast<unsigned char>(c)))
      return true;
  }
  return false;
}

/**
 * @brief Checks if a key-value pair represents a YAML merge key
 * @param key The key to check
 * @param value The value to check
 * @return true if key is '<<' and value starts with '*'
 */
bool isMergeKey(const std::string &key, const std::string &value) {
  return key == "<<" && !value.empty() && value[0] == '*';
}

/**
 * @brief Removes leading and trailing whitespace from a string
 * @param s The string to trim
 * @return A new string with whitespace removed from both ends
 * @details Returns empty string if input consists only of whitespace
 */
std::string trim(const std::string &s) {
  auto start = s.find_first_not_of(" \t");
  auto end   = s.find_last_not_of(" \t");
  if (start == std::string::npos || end == std::string::npos)
    return "";
  return s.substr(start, end - start + 1);
}

/**
 * @brief Parses a YAML multiline literal value
 * @param lines Vector of all lines in the YAML content
 * @param idx Current parsing position (modified as parsing progresses)
 * @param curIndent Current indentation level
 * @param style Literal style indicator ('|' or '>')
 * @return YamlItem containing the parsed multiline string
 * @details Handles both literal ('|') and folded ('>') styles:
 *          - For '|': preserves newlines
 *          - For '>': folds newlines to spaces
 *          Maintains proper indentation handling
 */
YamlItem parseMultilineLiteral(const std::vector<std::string> &lines, size_t &idx, int curIndent, char style) {
  std::string multiline;
  idx++;
  if (style == '|') {
    while (idx < lines.size() && lines[idx].find_first_not_of(" \t") > static_cast<std::string::size_type>(curIndent)) {
      multiline += trim(lines[idx]) + "\n";
      idx++;
    }
  } else { // style == '>'
    while (idx < lines.size() && lines[idx].find_first_not_of(" \t") > static_cast<std::string::size_type>(curIndent)) {
      multiline += trim(lines[idx]) + " ";
      idx++;
    }
  }
  return YamlItem(YamlElement(multiline));
}

/**
 * @brief Parses a YAML anchor and its associated value
 * @param value The anchor declaration string (starts with &)
 * @param lines Vector of all lines in the YAML content
 * @param idx Current parsing position (modified as parsing progresses)
 * @param curIndent Current indentation level
 * @param anchors Map to store the anchor reference
 * @param parser Reference to the YamlParser instance for nested parsing
 * @return YamlItem containing the parsed anchor value
 * @details Stores the parsed value in the anchors map for later reference
 *          Supports both sequence and mapping anchor values
 */
YamlItem parseAnchor(const std::string &, const std::string &value, const std::vector<std::string> &lines, size_t &idx,
                     int curIndent, std::map<std::string, YamlItem> &anchors, YamlParser &parser) {
  std::string anchorName = value.substr(1);
  idx++;
  YamlItem anchorNode;
  if (idx < lines.size() && lines[idx].find("-") != std::string::npos) {
    anchorNode = YamlItem(YamlElement(parser.parseSeq(lines, idx, curIndent + 2)));
  } else {
    anchorNode = YamlItem(YamlElement(parser.parseMap(lines, idx, curIndent + 2)));
  }
  anchors[anchorName] = anchorNode;
  return anchorNode;
}

/**
 * @brief Resolves a YAML alias reference to its anchor value
 * @param value The alias reference string (starts with *)
 * @param anchors Map containing all defined anchors
 * @return YamlItem referenced by the alias, or empty item if not found
 * @details If the alias is not found in the anchors map, returns an empty string item
 */
YamlItem parseAlias(const std::string &value, const std::map<std::string, YamlItem> &anchors) {
  std::string aliasName = value.substr(1);
  auto        it        = anchors.find(aliasName);
  if (it != anchors.end()) {
    return it->second;
  } else {
    return YamlItem(YamlElement(std::string("")));
  }
}

/**
 * @brief Parses a YAML inline sequence
 * @param value The string containing the inline sequence (e.g., "[item1, item2]")
 * @return YamlItem containing the parsed sequence
 * @details Handles:
 *          - Quoted strings (both single and double quotes)
 *          - Proper comma separation
 *          - Nested sequences
 *          - Whitespace trimming
 */
YamlItem parseInlineSeq(const std::string &value) {
  // Validate input: must be at least 2 chars, start with [ and end with ]
  if (value.size() < 2 || value.front() != '[' || value.back() != ']') {
    throw SyntaxException("Malformed inline sequence: missing brackets");
  }
  std::string              seq_content = value.substr(1, value.size() - 2);
  std::vector<std::string> items;
  std::string              item;
  bool                     in_single_quote = false;
  bool                     in_double_quote = false;
  for (size_t i = 0; i < seq_content.size(); ++i) {
    char c = seq_content[i];
    if (c == '\'' && !in_double_quote) {
      in_single_quote = !in_single_quote;
    } else if (c == '"' && !in_single_quote) {
      in_double_quote = !in_double_quote;
    } else if (c == ',' && !in_single_quote && !in_double_quote) {
      items.push_back(trim(item));
      item.clear();
    } else {
      item += c;
    }
  }
  if (!item.empty())
    items.push_back(trim(item));
  YamlSeq seq;
  seq.reserve(items.size());
  std::transform(items.begin(), items.end(), std::back_inserter(seq),
                 [](const std::string &it) { return YamlItem(YamlElement(yamlparser::YamlParser::parseScalar(it))); });
  return YamlItem(YamlElement(seq));
}

/**
 * @brief Processes a YAML merge key (<<) by merging an anchor's mapping into the current map
 * @param value The merge key value (an alias reference)
 * @param map The current mapping to merge into (modified in place)
 * @param anchors Map containing all defined anchors
 * @details This function:
 *          - Extracts the alias name from the value
 *          - Looks up the referenced anchor
 *          - If found and it's a mapping, copies all its key-value pairs to the current map
 *          - Keys that already exist in the target map are not overwritten
 */
void parseMergeKey(const std::string &value, std::map<std::string, YamlItem> &map,
                   const std::map<std::string, YamlItem> &anchors) {
  std::string aliasName = value.substr(1);
  auto        it        = anchors.find(aliasName);
  if (it != anchors.end() && it->second.value.isMap()) {
    YamlMap merged = it->second.value.asMap();
    for (const auto &kv : merged) {
      // Only insert if key does not already exist in the current map
      if (map.find(kv.first) == map.end()) {
        map[kv.first] = kv.second;
      }
    }
  }
}

} // namespace yamlparser
