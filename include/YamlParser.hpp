#pragma once
#include "YamlElement.hpp"
#include "YamlException.hpp"
#include <string>
#include <map>
#include <vector>

namespace yamlparser {

// Forward declarations for friend functions
class YamlParser;
YamlItem parseAnchor(const std::string &, const std::string &value, const std::vector<std::string> &lines, size_t &idx,
                     int curIndent, std::map<std::string, YamlItem> &anchors, YamlParser &parser);
YamlItem parseInlineSeq(const std::string &value);

/**
 * @brief Main YAML parsing class
 *
 * Provides functionality to:
 * - Parse YAML files into a structured representation
 * - Support both sequence and mapping root elements
 * - Access parsed data through type-safe interfaces
 * - Handle YAML anchors and aliases
 * - Provide proper exception-based error handling
 *
 * Usage example:
 * @code
 *   try {
 *     YamlParser parser;
 *     parser.parse("config.yaml");
 *     if (parser.isSequenceRoot()) {
 *       const auto& seq = parser.sequenceRoot();
 *       // Process sequence...
 *     } else {
 *       const auto& map = parser.root();
 *       // Process mapping...
 *     }
 *   } catch (const yamlparser::YamlException& e) {
 *     std::cerr << "YAML Error: " << e.what() << std::endl;
 *   }
 * @endcode
 */
class YamlParser {
  // Grant friend access to helper functions that need internal parsing methods
  friend YamlItem parseAnchor(const std::string &, const std::string &value, const std::vector<std::string> &lines,
                              size_t &idx, int curIndent, std::map<std::string, YamlItem> &anchors, YamlParser &parser);
  friend YamlItem parseInlineSeq(const std::string &value);

public:
  /**
   * @brief Default constructor
   */
  YamlParser() = default;

  void parse(const std::string &filename);

  bool isSequenceRoot() const;

  const YamlSeq &sequenceRoot() const;

  const YamlMap &root() const;

  const YamlItem &get(const std::string &key) const;

private:
  YamlMap parseMap(const std::vector<std::string> &lines, size_t &idx, int indent);

  bool parseMapEntry(const std::vector<std::string> &lines, size_t &idx, int indent, std::string::size_type curIndent,
                     const std::string &line, YamlMap &map);

  void validateMapStructure(const std::string &line, size_t lineNumber, std::string &key, std::string &value);

  void handleMapSyntaxError(const std::string &error, const std::string &context, size_t lineNumber);

  YamlSeq parseSeq(const std::vector<std::string> &lines, size_t &idx, int indent);

  bool parseSeqElement(const std::vector<std::string> &lines, size_t &idx, int indent, std::string::size_type curIndent,
                       const std::string &line, YamlSeq &seq);

  void validateSeqStructure(const std::string &line, size_t lineNumber);

  void handleSeqSyntaxError(const std::string &error, const std::string &context, size_t lineNumber);

  static YamlElement parseScalar(const std::string &value);

  static std::string preprocessScalarValue(const std::string &value);

  static YamlElement tryParsePrimitive(const std::string &cleanValue);

  static YamlElement parseNumericValue(const std::string &value);

  static std::string processQuotedString(const std::string &value);

  /** @brief Flag indicating if root is a sequence (true) or mapping (false) */
  bool m_sequenceRoot = false;

  /** @brief Storage for root sequence when m_sequenceRoot is true */
  YamlSeq m_sequenceData;

  /** @brief Storage for root mapping when m_sequenceRoot is false */
  YamlMap m_data;

  /** @brief Storage for named anchors to support YAML aliases */
  std::map<std::string, YamlItem> m_anchors;
};

} // namespace yamlparser
