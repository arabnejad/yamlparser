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
  idx++; // move past the line containing '|' or '>'

  auto continues = [&](size_t i) -> bool {
    if (i >= lines.size()) // reach end of file
      return false;
    auto firstNonSpacePos = lines[i].find_first_not_of(" \t");
    // must be a non-empty line AND more indented than the literal introducer
    return firstNonSpacePos != std::string::npos && firstNonSpacePos > static_cast<std::string::size_type>(curIndent);
  };

  if (style == '|') {
    while (continues(idx)) {
      multiline += trim(lines[idx]) + "\n";
      ++idx;
    }
  } else { // style == '>'
    while (continues(idx)) {
      multiline += trim(lines[idx]) + " ";
      ++idx;
    }

    // trim a trailing space added by the last fold
    if (!multiline.empty() && multiline.back() == ' ')
      multiline.pop_back();
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
YamlItem parseAnchor(const std::string &value, const std::vector<std::string> &lines, size_t &idx,
                     std::map<std::string, YamlItem> &anchors, YamlParser &parser) {
  // value starts with '&'
  std::string anchorName = value.substr(1);
  idx++; // move to the first line of the anchored node

  if (idx < lines.size()) {
    auto nextIndentPos = lines[idx].find_first_not_of(" \t");
    if (nextIndentPos != std::string::npos) {
      std::string next = lines[idx].substr(nextIndentPos);
      // if next begins with '-', parse sequence; otherwise, parse map
      YamlItem anchorNode = (!next.empty() && next[0] == '-')
                                ? YamlItem(YamlElement(parser.parseSeq(lines, idx, static_cast<int>(nextIndentPos))))
                                : YamlItem(YamlElement(parser.parseMap(lines, idx, static_cast<int>(nextIndentPos))));
      anchors[anchorName] = anchorNode;
      return anchorNode;
    }
  }

  // Empty anchor value -> treat as empty string scalar
  YamlItem empty(YamlElement(std::string("")));
  anchors[anchorName] = empty;
  return empty;
}

/**
 * @brief Resolves a YAML alias reference to its anchor value
 * @param value The alias reference string (starts with *)
 * @param anchors Map containing all defined anchors
 * @return YamlItem referenced by the alias, or empty item if not found
 * @details If the alias is not found in the anchors map, returns an empty string item
 */
YamlItem parseAlias(const std::string &value, const std::map<std::string, YamlItem> &anchors) {
  // value starts with '*'
  std::string aliasName = value.substr(1);
  auto        it        = anchors.find(aliasName);
  if (it == anchors.end()) {
    throw KeyException("*" + aliasName);
  }
  return it->second;
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

  const std::string        seqContent = value.substr(1, value.size() - 2);
  std::vector<std::string> items;
  std::string              current;
  bool                     inSingleQuote = false;
  bool                     inDoubleQuote = false;
  int                      bracketDepth  = 0; // support nested inline sequences

  for (size_t i = 0; i < seqContent.size(); ++i) {
    char c = seqContent[i];
    if (c == '\'' && !inDoubleQuote) {
      inSingleQuote = !inSingleQuote;
      current += c;
    } else if (c == '\"' && !inSingleQuote) {
      inDoubleQuote = !inDoubleQuote;
      current += c;
    } else if (!inSingleQuote && !inDoubleQuote) {
      if (c == '[') {
        ++bracketDepth;
        current += c;
      } else if (c == ']') {
        --bracketDepth;
        current += c;
      } else if (c == ',' && bracketDepth == 0) {
        items.push_back(trim(current));
        current.clear();
      } else {
        current += c;
      }
    } else {
      current += c;
    }
  }
  if (!current.empty())
    items.push_back(trim(current));

  YamlSeq seq;
  seq.reserve(items.size());
  for (const auto &it : items) {
    // If item itself looks like an inline seq, parse recursively
    if (!it.empty() && it.front() == '[' && it.back() == ']') {
      seq.push_back(parseInlineSeq(it));
    } else {
      seq.push_back(YamlItem(YamlElement(yamlparser::YamlParser::parseScalar(it))));
    }
  }
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
  // value is like: *anchorName
  std::string aliasName = value.substr(1);
  auto        it        = anchors.find(aliasName);
  if (it == anchors.end()) {
    throw KeyException("*" + aliasName);
  }
  if (!it->second.value.isMap()) {
    throw TypeException("Merge target is not a mapping: '*" + aliasName + "'");
  }
  const YamlMap &merged = it->second.value.asMap();
  for (const auto &kv : merged) {
    if (map.find(kv.first) == map.end()) {
      map.insert(kv); // preserve existing keys
    }
  }
}

} // namespace yamlparser
