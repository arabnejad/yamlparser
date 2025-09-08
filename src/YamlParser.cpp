#include "YamlParser.hpp"
#include "YamlException.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <set>
#include "YamlPrinter.hpp"

#include "YamlHelperFunctions.hpp"

namespace yamlparser {

/**
 * @brief Get the root mapping
 * @return Reference to the root mapping
 * @warning Only valid when isSequenceRoot() returns false
 */
const YamlMap &YamlParser::root() const {
  return m_data;
}

/**
 * @brief Get the root sequence
 * @return Reference to the root sequence
 * @warning Only valid when isSequenceRoot() returns true
 */
const YamlSeq &YamlParser::sequenceRoot() const {
  return m_sequenceData;
}

/**
 * @brief Check if the root element is a sequence
 * @return true if root is a sequence, false if it's a mapping
 */
bool YamlParser::isSequenceRoot() const {
  return m_sequenceRoot;
}

/**
 * @brief Parses a YAML file and loads its contents into the parser
 * @param filename Path to the YAML file to parse
 * @throws FileException if file cannot be opened or read
 * @throws SyntaxException if YAML syntax is invalid
 * @details This function:
 *          1. Opens and reads the specified YAML file
 *          2. Detects if the root element is a sequence or mapping
 *          3. For sequence root: stores in m_sequenceData and sets m_sequenceRoot flag
 *          4. For mapping root: stores in m_data and clears m_sequenceRoot flag
 *          5. Handles empty files gracefully
 */
void YamlParser::parse(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw FileException(filename);
  }

  std::string              line;
  std::vector<std::string> lines;
  while (std::getline(file, line)) {
    lines.push_back(line);
  }

  size_t idx = 0;
  // First pass: detect if the root element is a sequence (starts with '-')
  // Iterate through lines until we find non-empty content
  for (const auto &l : lines) {
    std::string trimmed = trim(l);
    if (trimmed.empty()) // Skip empty or whitespace-only lines
      continue;
    if (!trimmed.empty() && trimmed[0] == '#') // Skip comment lines
      continue;
    if (trimmed[0] == '-') {
      // Found sequence indicator at root level - parse entire sequence
      YamlSeq seq    = parseSeq(lines, idx, 0);
      m_sequenceRoot = true;
      m_sequenceData = seq;
      m_data.clear();
      return;
    } else {
      break;
    }
  }
  // Parse as a mapping (default case)
  idx            = 0;
  m_sequenceRoot = false;
  m_data         = parseMap(lines, idx, 0);
  m_sequenceData.clear();
}

/**
 * @brief Validates the structure of a mapping line and extracts key-value pair
 * @param line The line to validate and parse
 * @param lineNumber Current line number for error reporting
 * @param key Output parameter for the extracted key
 * @param value Output parameter for the extracted value
 * @throws SyntaxException if line structure is invalid
 */
void YamlParser::validateMapStructure(const std::string &line, size_t lineNumber, std::string &key,
                                      std::string &value) {
  auto pos = line.find(":");
  if (pos == std::string::npos) {
    throw SyntaxException("Missing ':' in key-value pair: '" + line + "'", lineNumber + 1);
  }

  key   = trim(line.substr(0, pos));
  value = trim(line.substr(pos + 1));

  if (key.empty()) {
    throw SyntaxException("Empty key in key-value pair", lineNumber + 1);
  }
}

/**
 * @brief Handles syntax errors during map parsing with context information
 * @param error The error message
 * @param context Additional context about the error
 * @param lineNumber The line number where the error occurred
 * @throws SyntaxException with detailed error information
 */
void YamlParser::handleMapSyntaxError(const std::string &error, const std::string &context, size_t lineNumber) {
  std::string detailedError = error;
  if (!context.empty()) {
    detailedError += " (Context: " + context + ")";
  }
  throw SyntaxException(detailedError, lineNumber + 1);
}

/**
 * @brief Parses a single map entry and adds it to the map
 * @param lines Vector of all lines in the YAML content
 * @param idx Current parsing position (modified as parsing progresses)
 * @param indent Current indentation level
 * @param curIndent Indentation of the current line
 * @param line The current line content
 * @param map The map to add the entry to
 * @return true if entry was processed, false if parsing should break
 */
bool YamlParser::parseMapEntry(const std::vector<std::string> &lines, size_t &idx, int indent,
                               std::string::size_type curIndent, const std::string &line, YamlMap &map) {
  // Skip empty lines
  if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
    idx++;
    return true;
  }

  // Skip comment lines
  std::string trimmed_line = trim(line);
  if (!trimmed_line.empty() && trimmed_line[0] == '#') {
    idx++;
    return true;
  }

  // Check indentation level
  if (static_cast<int>(curIndent) < indent) {
    return false; // Break from parsing
  }

  std::string processedLine = line.substr(curIndent);

  // Handle sequence lines within a map
  if (processedLine[0] == '-') {
    if (idx > 0) {
      std::string prevLine = lines[idx - 1].substr(lines[idx - 1].find_first_not_of(" \t"));
      auto        prevPos  = prevLine.find(":");
      if (prevPos != std::string::npos) {
        std::string key = trim(prevLine.substr(0, prevPos));
        if (map.find(key) == map.end()) {
          map[key] = YamlItem(YamlElement(parseSeq(lines, idx, static_cast<int>(curIndent))));
        }
      }
    }
    idx++;
    return true;
  }

  // Parse key-value pair
  std::string key, value;
  validateMapStructure(processedLine, idx, key, value);
  // Check for duplicate key
  if (map.find(key) != map.end()) {
    throw SyntaxException("Duplicate mapping key: '" + key + "'", idx + 1);
  }

  // Handle different value types
  if (value.empty() || value == "" || value == "\n" || value == "\r" || value == "\r\n") {
    // Check for nested content
    size_t lookahead = idx + 1;
    if (lookahead < lines.size() && lines[lookahead].find_first_not_of(" \t") > curIndent) {
      std::string nextLine    = lines[lookahead];
      auto        nextIndent  = nextLine.find_first_not_of(" \t");
      std::string nextContent = nextLine.substr(nextIndent);

      idx++;
      if (nextContent[0] == '-') {
        map[key] = YamlItem(YamlElement(parseSeq(lines, idx, static_cast<int>(nextIndent))));
      } else {
        map[key] = YamlItem(YamlElement(parseMap(lines, idx, static_cast<int>(nextIndent))));
      }
    } else {
      // Treat as explicit null (empty string)
      map[key] = YamlItem(YamlElement(std::string("")));
      idx++;
    }
  } else if (isMultilineLiteral(value)) {
    map[key] = parseMultilineLiteral(lines, idx, static_cast<int>(curIndent), value[0]);
  } else if (isAnchor(value)) {
    map[key] = parseAnchor(value, lines, idx, m_anchors, *this);
  } else if (isMergeKey(key, value)) {
    parseMergeKey(value, map, m_anchors);
    idx++;
  } else if (isAlias(value)) {
    map[key] = parseAlias(value, m_anchors);
    idx++;
  } else if (isInlineSeq(value)) {
    map[key] = parseInlineSeq(value);
    idx++;
  } else if (!value.empty() && value.front() == '[' && value.back() != ']') {
    throw SyntaxException("Malformed inline sequence: missing closing bracket");
  } else {
    map[key] = YamlItem(YamlElement(parseScalar(value)));
    idx++;
  }

  return true;
}

/**
 * @brief Parses a YAML mapping (dictionary/object) from a sequence of lines
 * @param lines Vector of all lines in the YAML content
 * @param idx Current parsing position (modified as parsing progresses)
 * @param indent Expected indentation level for this mapping
 * @return YamlMap containing the parsed key-value pairs
 * @details Handles various YAML mapping features:
 *          - Nested mappings and sequences
 *          - Multiline literals (| and >)
 *          - Anchors (&) and aliases (*)
 *          - Merge keys (<<)
 *          - Inline sequences
 *          - Empty/null values
 *          - Proper indentation-based nesting
 */
YamlMap YamlParser::parseMap(const std::vector<std::string> &lines, size_t &idx, int indent) {
  YamlMap map;
  // Track explicitly defined keys in this mapping block (not merged)
  std::set<std::string> explicitKeys;

  while (idx < lines.size()) {
    std::string            line      = lines[idx];
    std::string::size_type curIndent = line.find_first_not_of(" \t");

    // Custom parseMapEntry logic to allow explicit key tracking
    // (inlined from parseMapEntry for this block)
    // ...existing code...
    // Skip empty lines
    if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
      idx++;
      continue;
    }
    // Skip comment lines
    std::string trimmed_line = trim(line);
    if (!trimmed_line.empty() && trimmed_line[0] == '#') {
      idx++;
      continue;
    }
    // Check indentation level
    if (static_cast<int>(curIndent) < indent) {
      break;
    }
    std::string processedLine = line.substr(curIndent);
    // Handle sequence lines within a map
    if (processedLine[0] == '-') {
      if (idx > 0) {
        std::string prevLine = lines[idx - 1].substr(lines[idx - 1].find_first_not_of(" \t"));
        auto        prevPos  = prevLine.find(":");
        if (prevPos != std::string::npos) {
          std::string key = trim(prevLine.substr(0, prevPos));
          if (map.find(key) == map.end()) {
            map[key] = YamlItem(YamlElement(parseSeq(lines, idx, static_cast<int>(curIndent))));
            explicitKeys.insert(key);
          }
        }
      }
      idx++;
      continue;
    }
    // Parse key-value pair
    std::string key, value;
    validateMapStructure(processedLine, idx, key, value);
    // Check for duplicate key: only error if explicitly defined in this block
    if (explicitKeys.find(key) != explicitKeys.end()) {
      throw SyntaxException("Duplicate mapping key: '" + key + "'", idx + 1);
    }
    // Handle different value types
    if (value.empty() || value == "" || value == "\n" || value == "\r" || value == "\r\n") {
      // Check for nested content
      size_t lookahead = idx + 1;
      if (lookahead < lines.size() && lines[lookahead].find_first_not_of(" \t") > curIndent) {
        std::string nextLine    = lines[lookahead];
        auto        nextIndent  = nextLine.find_first_not_of(" \t");
        std::string nextContent = nextLine.substr(nextIndent);
        idx++;
        if (nextContent[0] == '-') {
          map[key] = YamlItem(YamlElement(parseSeq(lines, idx, static_cast<int>(nextIndent))));
        } else {
          map[key] = YamlItem(YamlElement(parseMap(lines, idx, static_cast<int>(nextIndent))));
        }
      } else {
        // Treat as explicit null (empty string)
        map[key] = YamlItem(YamlElement(std::string("")));
        idx++;
      }
      explicitKeys.insert(key);
    } else if (isMultilineLiteral(value)) {
      map[key] = parseMultilineLiteral(lines, idx, static_cast<int>(curIndent), value[0]);
      explicitKeys.insert(key);
    } else if (isAnchor(value)) {
      map[key] = parseAnchor(value, lines, idx, m_anchors, *this);
      explicitKeys.insert(key);
    } else if (isMergeKey(key, value)) {
      parseMergeKey(value, map, m_anchors);
      idx++;
      // Do not add '<<' to explicitKeys
    } else if (isAlias(value)) {
      map[key] = parseAlias(value, m_anchors);
      idx++;
      explicitKeys.insert(key);
    } else if (isInlineSeq(value)) {
      map[key] = parseInlineSeq(value);
      idx++;
      explicitKeys.insert(key);
    } else if (!value.empty() && value.front() == '[' && value.back() != ']') {
      throw SyntaxException("Malformed inline sequence: missing closing bracket");
    } else {
      map[key] = YamlItem(YamlElement(parseScalar(value)));
      idx++;
      explicitKeys.insert(key);
    }
  }
  return map;
}

/**
 * @brief Validates the structure of a sequence line
 * @param line The line to validate
 * @param lineNumber Current line number for error reporting
 * @throws SyntaxException if line structure is invalid
 */
void YamlParser::validateSeqStructure(const std::string &line, size_t lineNumber) {
  if (line.empty()) {
    throw SyntaxException("Empty sequence line", lineNumber + 1);
  }

  if (line[0] != '-') {
    throw SyntaxException("Invalid sequence format: expected '-' at line start", lineNumber + 1);
  }
}

/**
 * @brief Handles syntax errors during sequence parsing with context information
 * @param error The error message
 * @param context Additional context about the error
 * @param lineNumber The line number where the error occurred
 * @throws SyntaxException with detailed error information
 */
void YamlParser::handleSeqSyntaxError(const std::string &error, const std::string &context, size_t lineNumber) {
  std::string detailedError = error;
  if (!context.empty()) {
    detailedError += " (Context: " + context + ")";
  }
  throw SyntaxException(detailedError, lineNumber + 1);
}

/**
 * @brief Parses a single sequence element and adds it to the sequence
 * @param lines Vector of all lines in the YAML content
 * @param idx Current parsing position (modified as parsing progresses)
 * @param indent Current indentation level
 * @param curIndent Indentation of the current line
 * @param line The current line content
 * @param seq The sequence to add the element to
 * @return true if element was processed, false if parsing should break
 */
bool YamlParser::parseSeqElement(const std::vector<std::string> &lines, size_t &idx, int indent,
                                 std::string::size_type curIndent, const std::string &line, YamlSeq &seq) {
  // Skip empty lines
  if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
    idx++;
    return true;
  }

  // Skip comment lines
  std::string trimmed_line = trim(line);
  if (!trimmed_line.empty() && trimmed_line[0] == '#') {
    idx++;
    return true;
  }

  // Check indentation level
  if (static_cast<int>(curIndent) < indent) {
    return false; // Break from parsing
  }

  std::string processedLine = line.substr(curIndent);

  // Validate sequence structure
  if (processedLine[0] != '-') {
    return false; // Not a sequence line, break parsing
  }

  std::string value = trim(processedLine.substr(1));

  // Check if this is a mapping block (next line is more indented)
  size_t lookahead = idx + 1;
  if (lookahead < lines.size()) {
    std::string nextLine   = lines[lookahead];
    auto        nextIndent = nextLine.find_first_not_of(" \t");

    if (nextIndent != std::string::npos && nextIndent > curIndent) {
      // This is a mapping block. Handle case where sequence item has content
      YamlMap itemMap;

      // If the sequence item has content, parse it as the first key-value pair
      if (!value.empty()) {
        auto pos = value.find(":");
        if (pos != std::string::npos) {
          std::string key = trim(value.substr(0, pos));
          std::string val = trim(value.substr(pos + 1));
          itemMap[key]    = YamlItem(parseScalar(val));
        }
      }

      // Parse the indented lines as additional key-value pairs
      idx++;
      YamlMap indentedMap = parseMap(lines, idx, static_cast<int>(nextIndent));

      // Merge the indented map into the item map
      for (const auto &pair : indentedMap) {
        itemMap[pair.first] = pair.second;
      }

      seq.push_back(YamlItem(YamlElement(itemMap)));
      return true; // parseMap will have updated idx
    }
  }

  // Not a mapping block, parse as scalar or inline sequence if not empty
  if (!value.empty()) {
    if (isInlineSeq(value)) {
      seq.push_back(parseInlineSeq(value));
    } else {
      seq.push_back(YamlItem(parseScalar(value)));
    }
  } else {
    seq.push_back(YamlItem(YamlElement(std::string(""))));
  }
  idx++;
  return true;
}

/**
 * @brief Parses a YAML sequence (array/list) from a sequence of lines
 * @param lines Vector of all lines in the YAML content
 * @param idx Current parsing position (modified as parsing progresses)
 * @param indent Expected indentation level for this sequence
 * @return YamlSeq containing the parsed sequence items
 * @details Handles:
 *          - Simple scalar elements
 *          - Nested mappings as elements
 *          - Empty elements
 *          - Proper indentation-based nesting
 */
YamlSeq YamlParser::parseSeq(const std::vector<std::string> &lines, size_t &idx, int indent) {
  YamlSeq seq;

  while (idx < lines.size()) {
    std::string            line      = lines[idx];
    std::string::size_type curIndent = line.find_first_not_of(" \t");

    if (!parseSeqElement(lines, idx, indent, curIndent, line, seq)) {
      break;
    }
  }
  return seq;
}

/**
 * @brief Parses a YAML scalar value into its appropriate type
 * @param value The string to parse
 * @return YamlElement containing the parsed value
 * @details Handles these scalar types:
 *          - Booleans (true/false)
 *          - Integers (matched by regex ^-?\d+$)
 *          - Floating point numbers (matched by regex ^-?\d*\.\d+$)
 *          - Quoted strings (both single and double quotes)
 *          - Plain strings (anything else)
 *          Also handles:
 *          - Comment removal (strips everything after #)
 *          - Whitespace trimming
 *          - Quote stripping from quoted strings
 */
YamlElement YamlParser::parseScalar(const std::string &value) {
  std::string cleanValue = preprocessScalarValue(value);

  // Try primitive types first (bool, numeric)
  YamlElement primitiveResult = tryParsePrimitive(cleanValue);

  // If it's not a string result, we successfully parsed a primitive
  if (!primitiveResult.isString()) {
    return primitiveResult;
  }

  // Handle quoted/unquoted strings
  return YamlElement(processQuotedString(cleanValue));
}

/**
 * @brief Preprocess scalar value by removing comments and trimming
 * @param value Raw scalar text
 * @return Cleaned scalar text
 */
std::string YamlParser::preprocessScalarValue(const std::string &value) {
  // Trim first; if quoted, do NOT strip comments here.
  std::string s = trim(value);
  if (s.empty())
    return s;
  if (s.front() == '\'' || s.front() == '\"') {
    return s; // leave quoted content intact; quotes handled later
  }
  // Otherwise, strip trailing comment and trim again.
  auto hash = s.find('#');
  if (hash != std::string::npos) {
    s.resize(hash);
  }
  return trim(s);
}

/**
 * @brief Attempt to parse primitive types (bool, numeric)
 * @param cleanValue Preprocessed scalar text
 * @return Parsed primitive element or string if not primitive
 */
YamlElement YamlParser::tryParsePrimitive(const std::string &cleanValue) {
  // Handle boolean values
  if (cleanValue == "true")
    return YamlElement(true);
  if (cleanValue == "false")
    return YamlElement(false);

  // Try numeric parsing
  return parseNumericValue(cleanValue);
}

/**
 * @brief Parse numeric values (int, double)
 * @param value Scalar text to parse as number
 * @return Parsed numeric element or string if not numeric
 */
YamlElement YamlParser::parseNumericValue(const std::string &value) {
  static const std::regex int_re("^-?\\d+$");
  static const std::regex double_re("^-?(?:\\d+\\.\\d*|\\.\\d+|\\d+)(?:[eE][+-]?\\d+)?$");

  // Try integer parsing
  if (std::regex_match(value, int_re)) {
    try {
      return YamlElement(std::stoi(value));
    } catch (const std::out_of_range &) {
      throw ConversionException(value, "integer (value out of range)");
    } catch (const std::invalid_argument &) {
      throw ConversionException(value, "integer (invalid format)");
    }
  }

  // Try double parsing
  if (std::regex_match(value, double_re)) {
    try {
      return YamlElement(std::stod(value));
    } catch (const std::out_of_range &) {
      throw ConversionException(value, "double (value out of range)");
    } catch (const std::invalid_argument &) {
      throw ConversionException(value, "double (invalid format)");
    }
  }

  // Return as string if not numeric
  return YamlElement(value);
}

/**
 * @brief Process quoted strings by removing surrounding quotes
 * @param value String that may have surrounding quotes
 * @return String with quotes removed if present
 */
std::string YamlParser::processQuotedString(const std::string &value) {
  // Strip surrounding quotes if present
  if ((value.size() >= 2) &&
      ((value.front() == '\'' && value.back() == '\'') || (value.front() == '"' && value.back() == '"'))) {
    return value.substr(1, value.size() - 2);
  }
  return value;
}

/**
 * @brief Retrieves a value from the parsed YAML data by its key
 * @param key The key to look up in the root mapping
 * @return Reference to the found YamlItem, or to an empty item if not found
 * @details Only works when the root is a mapping (not a sequence)
 *          Returns a static empty item when:
 *          - The root is a sequence (m_sequenceRoot is true)
 *          - The key is not found in the mapping
 *          The returned reference remains valid as long as the parser exists
 */
const YamlItem &YamlParser::get(const std::string &key) const {
  if (m_sequenceRoot) {
    throw StructureException("Cannot access key '" + key + "' on sequence root");
  }

  auto it = m_data.find(key);
  if (it != m_data.end()) {
    return it->second;
  }

  throw KeyException(key);
}

} // namespace yamlparser
