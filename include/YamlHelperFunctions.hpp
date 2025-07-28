#pragma once
#include <string>
#include <vector>
#include <map>
#include "YamlParser.hpp"
#include "YamlPrinter.hpp"
#include "YamlElement.hpp"

/**
 * @file YamlHelperFunctions.hpp
 * @brief Collection of utility functions for YAML parsing
 *
 * This file provides helper functions that:
 * 1. Detect YAML syntax elements (multiline, anchor, alias, etc.)
 * 2. Parse complex YAML structures (sequences, anchors, etc.)
 * 3. Handle string manipulation (trimming, validation)
 * 4. Support the main parser implementation
 *
 * These functions are internal to the parser implementation
 * and should not be used directly by library users.
 */

namespace yamlparser {

bool isMultilineLiteral(const std::string &value);

bool isAnchor(const std::string &value);

bool isAlias(const std::string &value);

bool isInlineSeq(const std::string &value);

bool isMergeKey(const std::string &key, const std::string &value);

std::string trim(const std::string &s);

YamlItem parseMultilineLiteral(const std::vector<std::string> &lines, size_t &idx, int curIndent, char style);

YamlItem parseAnchor(const std::string &, const std::string &value, const std::vector<std::string> &lines, size_t &idx,
                     int curIndent, std::map<std::string, YamlItem> &anchors, YamlParser &parser);

YamlItem parseAlias(const std::string &value, const std::map<std::string, YamlItem> &anchors);

YamlItem parseInlineSeq(const std::string &value);

void parseMergeKey(const std::string &value, std::map<std::string, YamlItem> &map,
                   const std::map<std::string, YamlItem> &anchors);

} // namespace yamlparser
