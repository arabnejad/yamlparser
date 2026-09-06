#include "YamlParser.hpp"

#include "YamlDocumentAnchorStore.hpp"
#include "YamlFlowParser.hpp"
#include "YamlScalarParser.hpp"

#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace yamlparser {
namespace {

using internal::AliasReference;
using internal::DocumentAnchorStore;
using internal::parsePlainScalarValue;
using internal::parseQuotedScalar;
using internal::SourcePosition;

std::string trimWhitespace(const std::string &text) {
  const std::size_t first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos)
    return "";
  const std::size_t last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1U);
}

SourcePosition positionAfterLeadingWhitespace(const std::string &text, SourcePosition startingPosition) {
  const std::size_t firstContentOffset = text.find_first_not_of(" \t");
  if (firstContentOffset != std::string::npos)
    startingPosition.columnNumber += firstContentOffset;
  return startingPosition;
}

std::size_t indentationWidthOf(const std::string &line) {
  const std::size_t firstContent = line.find_first_not_of(" \t");
  return firstContent == std::string::npos ? line.size() : firstContent;
}

std::string contentWithoutIndentation(const std::string &line) {
  return line.substr(indentationWidthOf(line));
}

bool isSequenceMarker(const std::string &text) {
  return text == "-" ||
         (text.size() > 1U && text.front() == '-' && std::isspace(static_cast<unsigned char>(text[1])) != 0);
}

std::string removeInlineComment(const std::string &text) {
  bool insideSingleQuotes = false;
  bool insideDoubleQuotes = false;
  bool escapedCharacter   = false;

  for (std::size_t index = 0; index < text.size(); ++index) {
    const char character = text[index];
    if (insideDoubleQuotes && escapedCharacter) {
      escapedCharacter = false;
      continue;
    }
    if (insideDoubleQuotes && character == '\\') {
      escapedCharacter = true;
      continue;
    }
    if (character == '\'' && !insideDoubleQuotes) {
      insideSingleQuotes = !insideSingleQuotes;
      continue;
    }
    if (character == '"' && !insideSingleQuotes) {
      insideDoubleQuotes = !insideDoubleQuotes;
      continue;
    }
    const bool startsComment = character == '#' && !insideSingleQuotes && !insideDoubleQuotes &&
                               (index == 0U || std::isspace(static_cast<unsigned char>(text[index - 1U])) != 0);
    if (startsComment)
      return trimWhitespace(text.substr(0, index));
  }
  return trimWhitespace(text);
}

std::size_t findBlockMappingSeparator(const std::string &text) {
  bool insideSingleQuotes = false;
  bool insideDoubleQuotes = false;
  bool escapedCharacter   = false;
  int  collectionDepth    = 0;

  for (std::size_t index = 0; index < text.size(); ++index) {
    const char character = text[index];
    if (insideDoubleQuotes && escapedCharacter) {
      escapedCharacter = false;
      continue;
    }
    if (insideDoubleQuotes && character == '\\') {
      escapedCharacter = true;
      continue;
    }
    if (character == '\'' && !insideDoubleQuotes) {
      insideSingleQuotes = !insideSingleQuotes;
      continue;
    }
    if (character == '"' && !insideSingleQuotes) {
      insideDoubleQuotes = !insideDoubleQuotes;
      continue;
    }
    if (insideSingleQuotes || insideDoubleQuotes)
      continue;
    if (character == '[' || character == '{') {
      ++collectionDepth;
      continue;
    }
    if (character == ']' || character == '}') {
      --collectionDepth;
      continue;
    }
    if (character == ':' && collectionDepth == 0) {
      const bool hasValidSeparator =
          index + 1U == text.size() || std::isspace(static_cast<unsigned char>(text[index + 1U])) != 0;
      if (hasValidSeparator)
        return index;
    }
  }
  return std::string::npos;
}

class DocumentParser {
public:
  explicit DocumentParser(std::vector<std::string> lines) : m_lines(std::move(lines)) {}

  YamlValue parseDocument() {
    skipIgnoredLines();
    if (atEnd())
      return YamlValue(YamlMapping());

    if (trimWhitespace(m_lines[m_nextLineIndex]) == "---") {
      ++m_nextLineIndex;
      skipIgnoredLines();
    }
    if (atEnd())
      return YamlValue(YamlMapping());

    const std::size_t rootIndentation = indentationWidthOf(m_lines[m_nextLineIndex]);
    YamlValue         document        = parseBlock(rootIndentation);

    skipIgnoredLines();
    if (!atEnd() && trimWhitespace(m_lines[m_nextLineIndex]) == "...") {
      ++m_nextLineIndex;
      skipIgnoredLines();
    }
    if (!atEnd())
      throw SyntaxException("Unexpected content after the root value", currentLineNumber());
    return document;
  }

private:
  std::vector<std::string> m_lines;
  std::size_t              m_nextLineIndex = 0U;
  DocumentAnchorStore      m_anchorStore;

  bool atEnd() const noexcept {
    return m_nextLineIndex >= m_lines.size();
  }
  std::size_t currentLineNumber() const noexcept {
    return m_nextLineIndex + 1U;
  }

  bool lineIsIgnored(std::size_t lineIndex) const {
    const std::string content = trimWhitespace(m_lines[lineIndex]);
    return content.empty() || content.front() == '#';
  }

  void skipIgnoredLines() {
    while (!atEnd() && lineIsIgnored(m_nextLineIndex))
      ++m_nextLineIndex;
  }

  YamlValue parseBlock(std::size_t expectedIndentation) {
    skipIgnoredLines();
    if (atEnd())
      return YamlValue();

    const std::size_t actualIndentation = indentationWidthOf(m_lines[m_nextLineIndex]);
    if (actualIndentation != expectedIndentation)
      throw SyntaxException("Unexpected indentation", currentLineNumber());

    const std::string content = removeInlineComment(contentWithoutIndentation(m_lines[m_nextLineIndex]));
    if (isSequenceMarker(content))
      return YamlValue(parseSequence(expectedIndentation));
    if (findBlockMappingSeparator(content) != std::string::npos)
      return YamlValue(parseMapping(expectedIndentation));

    const std::size_t lineNumber = currentLineNumber();
    ++m_nextLineIndex;
    return parseValueOrIndentedBlock(content, expectedIndentation,
                                     SourcePosition{lineNumber, expectedIndentation + 1U});
  }

  YamlMapping parseMapping(std::size_t expectedIndentation) {
    YamlMapping           mapping;
    std::set<std::string> explicitKeys;
    parseRemainingMappingEntries(expectedIndentation, mapping, explicitKeys);
    return mapping;
  }

  YamlMapping parseMappingWithFirstEntry(const std::string &firstEntry, std::size_t mappingIndentation,
                                         SourcePosition firstEntryPosition) {
    YamlMapping           mapping;
    std::set<std::string> explicitKeys;
    parseMappingEntry(firstEntry, mappingIndentation, firstEntryPosition, mapping, explicitKeys);
    parseRemainingMappingEntries(mappingIndentation, mapping, explicitKeys);
    return mapping;
  }

  void parseRemainingMappingEntries(std::size_t expectedIndentation, YamlMapping &mapping,
                                    std::set<std::string> &explicitKeys) {
    while (true) {
      skipIgnoredLines();
      if (atEnd())
        return;

      const std::size_t actualIndentation = indentationWidthOf(m_lines[m_nextLineIndex]);
      if (actualIndentation < expectedIndentation)
        return;
      if (actualIndentation > expectedIndentation)
        throw SyntaxException("Unexpected indentation in mapping", currentLineNumber());

      const std::string entry = removeInlineComment(contentWithoutIndentation(m_lines[m_nextLineIndex]));
      if (isSequenceMarker(entry))
        return;

      const std::size_t lineNumber = currentLineNumber();
      ++m_nextLineIndex;
      parseMappingEntry(entry, expectedIndentation, SourcePosition{lineNumber, expectedIndentation + 1U}, mapping,
                        explicitKeys);
    }
  }

  void parseMappingEntry(const std::string &entry, std::size_t entryIndentation, SourcePosition entryPosition,
                         YamlMapping &mapping, std::set<std::string> &explicitKeys) {
    const std::size_t separatorIndex = findBlockMappingSeparator(entry);
    if (separatorIndex == std::string::npos)
      throw SyntaxException("Expected a 'key: value' mapping entry", entryPosition.lineNumber,
                            entryPosition.columnNumber);

    const std::string rawKey = trimWhitespace(entry.substr(0, separatorIndex));
    if (rawKey.empty())
      throw SyntaxException("Mapping key cannot be empty", entryPosition.lineNumber, entryPosition.columnNumber);

    // Only the plain `<<` spelling is YAML's merge key. Quoted spellings such
    // as `"<<"` are ordinary string keys, even though they decode to the same
    // text.
    const bool        isMergeKey = rawKey == "<<";
    const std::string key =
        (rawKey.front() == '\'' || rawKey.front() == '"') ? parseQuotedScalar(rawKey, entryPosition) : rawKey;
    const std::string    textAfterSeparator = entry.substr(separatorIndex + 1U);
    const SourcePosition valuePosition      = positionAfterLeadingWhitespace(
        textAfterSeparator, SourcePosition{entryPosition.lineNumber, entryPosition.columnNumber + separatorIndex + 1U});
    const std::string valueText = removeInlineComment(textAfterSeparator);

    if (isMergeKey) {
      const std::vector<AliasReference> aliases = parseMergeAliases(valueText, entryIndentation, valuePosition);
      m_anchorStore.mergeMappingsFromAliases(aliases, mapping);
      return;
    }
    if (!explicitKeys.insert(key).second)
      throw SyntaxException("Duplicate mapping key: '" + key + "'", entryPosition.lineNumber,
                            entryPosition.columnNumber);

    mapping[key] = parseValueOrIndentedBlock(valueText, entryIndentation, valuePosition);
  }

  YamlSequence parseSequence(std::size_t expectedIndentation) {
    YamlSequence sequence;
    parseRemainingSequenceItems(expectedIndentation, sequence);
    return sequence;
  }

  YamlSequence parseSequenceWithFirstItem(const std::string &firstItem, std::size_t sequenceIndentation,
                                          SourcePosition firstItemPosition) {
    YamlSequence sequence;
    sequence.push_back(parseSequenceItem(firstItem, sequenceIndentation, firstItemPosition));
    parseRemainingSequenceItems(sequenceIndentation, sequence);
    return sequence;
  }

  void parseRemainingSequenceItems(std::size_t expectedIndentation, YamlSequence &sequence) {
    while (true) {
      skipIgnoredLines();
      if (atEnd())
        return;

      const std::size_t actualIndentation = indentationWidthOf(m_lines[m_nextLineIndex]);
      if (actualIndentation < expectedIndentation)
        return;
      if (actualIndentation > expectedIndentation)
        throw SyntaxException("Unexpected indentation in sequence", currentLineNumber());

      const std::string content = removeInlineComment(contentWithoutIndentation(m_lines[m_nextLineIndex]));
      if (!isSequenceMarker(content))
        return;

      const std::size_t    lineNumber      = currentLineNumber();
      const std::string    textAfterMarker = content.substr(1U);
      const SourcePosition itemPosition =
          positionAfterLeadingWhitespace(textAfterMarker, SourcePosition{lineNumber, expectedIndentation + 2U});
      const std::string itemText = trimWhitespace(textAfterMarker);
      ++m_nextLineIndex;
      sequence.push_back(parseSequenceItem(itemText, expectedIndentation, itemPosition));
    }
  }

  YamlValue parseSequenceItem(const std::string &itemText, std::size_t sequenceIndentation,
                              SourcePosition itemPosition) {
    if (itemText.empty())
      return parseIndentedBlockOrNull(sequenceIndentation);

    if (isSequenceMarker(itemText)) {
      const std::string    textAfterNestedMarker   = itemText.substr(1U);
      const SourcePosition firstNestedItemPosition = positionAfterLeadingWhitespace(
          textAfterNestedMarker, SourcePosition{itemPosition.lineNumber, itemPosition.columnNumber + 1U});
      const std::string firstNestedItem = trimWhitespace(textAfterNestedMarker);
      return YamlValue(parseSequenceWithFirstItem(firstNestedItem, sequenceIndentation + 2U, firstNestedItemPosition));
    }

    if (findBlockMappingSeparator(itemText) != std::string::npos)
      return YamlValue(parseMappingWithFirstEntry(itemText, sequenceIndentation + 2U, itemPosition));

    return parseValueOrIndentedBlock(itemText, sequenceIndentation, itemPosition);
  }

  YamlValue parseValueOrIndentedBlock(const std::string &valueText, std::size_t parentIndentation,
                                      SourcePosition valuePosition) {
    const std::string value = trimWhitespace(valueText);
    if (value.empty())
      return parseIndentedBlockOrNull(parentIndentation);
    if (value.front() == '&')
      return parseAnchoredValue(value, parentIndentation, valuePosition);
    if (value.front() == '*')
      return resolveAlias(value, valuePosition);
    if (value.front() == '|' || value.front() == '>')
      return parseBlockScalar(value, parentIndentation);
    return parseSingleLineValue(value, valuePosition);
  }

  YamlValue parseIndentedBlockOrNull(std::size_t parentIndentation) {
    skipIgnoredLines();
    if (atEnd() || indentationWidthOf(m_lines[m_nextLineIndex]) <= parentIndentation)
      return YamlValue();
    return parseBlock(indentationWidthOf(m_lines[m_nextLineIndex]));
  }

  YamlValue parseAnchoredValue(const std::string &anchorExpression, std::size_t parentIndentation,
                               SourcePosition expressionPosition) {
    std::size_t anchorNameEndIndex = 1U;
    while (anchorNameEndIndex < anchorExpression.size() &&
           std::isspace(static_cast<unsigned char>(anchorExpression[anchorNameEndIndex])) == 0)
      ++anchorNameEndIndex;

    const std::string anchorName = anchorExpression.substr(1U, anchorNameEndIndex - 1U);
    if (anchorName.empty())
      throw SyntaxException("Anchor name cannot be empty", expressionPosition.lineNumber,
                            expressionPosition.columnNumber);

    const std::string    textAfterName         = anchorExpression.substr(anchorNameEndIndex);
    const SourcePosition anchoredValuePosition = positionAfterLeadingWhitespace(
        textAfterName,
        SourcePosition{expressionPosition.lineNumber, expressionPosition.columnNumber + anchorNameEndIndex});
    const std::string anchoredText  = trimWhitespace(textAfterName);
    YamlValue         anchoredValue = parseValueOrIndentedBlock(anchoredText, parentIndentation, anchoredValuePosition);
    m_anchorStore.defineAnchor(anchorName, anchoredValue);
    return anchoredValue;
  }

  YamlValue resolveAlias(const std::string &aliasExpression, SourcePosition aliasPosition) const {
    const std::string aliasName = trimWhitespace(aliasExpression.substr(1U));
    if (aliasName.empty() || aliasName.find_first_of(" \t") != std::string::npos)
      throw SyntaxException("Invalid alias", aliasPosition.lineNumber, aliasPosition.columnNumber);

    return m_anchorStore.resolveAlias(aliasName);
  }

  // Parses one item from a block merge list.
  // For example, `*defaults` becomes a reference to the `defaults` anchor.
  AliasReference parseBlockMergeAlias(const std::string &itemText, SourcePosition itemPosition) const {
    if (itemText.empty() || itemText.front() != '*')
      throw SyntaxException("Merge list items must be aliases", itemPosition.lineNumber, itemPosition.columnNumber);

    const std::string aliasName = trimWhitespace(itemText.substr(1U));
    if (aliasName.empty() || aliasName.find_first_of(" \t") != std::string::npos)
      throw SyntaxException("Merge list items must contain one alias", itemPosition.lineNumber,
                            itemPosition.columnNumber);
    return AliasReference{aliasName, itemPosition};
  }

  // Parses an indented sequence used as a mapping merge value.
  // For example, two `- *name` lines return two alias references in their
  // source order.
  std::vector<AliasReference> parseBlockMergeAliases(std::size_t parentIndentation, SourcePosition valuePosition) {
    skipIgnoredLines();
    if (atEnd() || indentationWidthOf(m_lines[m_nextLineIndex]) <= parentIndentation)
      throw SyntaxException("Merge value must be an alias or a list of aliases", valuePosition.lineNumber,
                            valuePosition.columnNumber);

    const std::size_t           expectedItemIndentation = indentationWidthOf(m_lines[m_nextLineIndex]);
    std::vector<AliasReference> aliases;

    while (true) {
      skipIgnoredLines();
      if (atEnd())
        break;

      const std::size_t actualItemIndentation = indentationWidthOf(m_lines[m_nextLineIndex]);
      if (actualItemIndentation < expectedItemIndentation)
        break;
      if (actualItemIndentation > expectedItemIndentation)
        throw SyntaxException("Unexpected indentation in merge list", currentLineNumber());

      const std::string itemContent = removeInlineComment(contentWithoutIndentation(m_lines[m_nextLineIndex]));
      if (!isSequenceMarker(itemContent)) {
        if (aliases.empty())
          throw SyntaxException("Merge value must be a list of aliases", currentLineNumber(),
                                actualItemIndentation + 1U);
        break;
      }

      const std::size_t    lineNumber      = currentLineNumber();
      const std::string    textAfterMarker = itemContent.substr(1U);
      const SourcePosition itemPosition =
          positionAfterLeadingWhitespace(textAfterMarker, SourcePosition{lineNumber, actualItemIndentation + 2U});
      const std::string itemText = trimWhitespace(textAfterMarker);
      ++m_nextLineIndex;

      aliases.push_back(parseBlockMergeAlias(itemText, itemPosition));
    }

    return aliases;
  }

  // Parses either one alias on the same line, a flow-style list such as
  // `[*one, *two]`, or an indented block list. Applying aliases is kept
  // separate so every form uses the same validation and precedence rules.
  std::vector<AliasReference> parseMergeAliases(const std::string &valueText, std::size_t parentIndentation,
                                                SourcePosition valuePosition) {
    const std::string mergeExpression = trimWhitespace(valueText);
    if (mergeExpression.empty())
      return parseBlockMergeAliases(parentIndentation, valuePosition);
    return internal::parseFlowMergeAliases(mergeExpression, valuePosition, m_anchorStore);
  }

  YamlValue parseSingleLineValue(const std::string &value, SourcePosition valuePosition) {
    if (value.empty())
      return YamlValue();
    if (value.front() == '\'' || value.front() == '"')
      return YamlValue(parseQuotedScalar(value, valuePosition));
    if (value.front() == '[' || value.front() == '{')
      return internal::parseFlowCollectionValue(value, valuePosition, m_anchorStore);
    return parsePlainScalarValue(value);
  }

  YamlValue parseBlockScalar(const std::string &header, std::size_t parentIndentation) {
    const char style          = header.front();
    const char chompingMethod = header.size() > 1U ? header[1] : '\0';

    struct ScalarLine {
      std::string text;
      bool        moreIndented;
    };

    std::vector<ScalarLine> scalarLines;
    std::size_t             contentIndentation = std::string::npos;

    while (!atEnd()) {
      const std::string &line = m_lines[m_nextLineIndex];
      if (trimWhitespace(line).empty()) {
        scalarLines.push_back(ScalarLine{"", false});
        ++m_nextLineIndex;
        continue;
      }

      const std::size_t lineIndentation = indentationWidthOf(line);
      if (lineIndentation <= parentIndentation)
        break;
      if (contentIndentation == std::string::npos)
        contentIndentation = lineIndentation;

      const std::size_t textStart = line.size() < contentIndentation ? line.size() : contentIndentation;
      scalarLines.push_back(ScalarLine{line.substr(textStart), lineIndentation > contentIndentation});
      ++m_nextLineIndex;
    }

    std::string result;
    if (style == '|') {
      for (const ScalarLine &line : scalarLines) {
        result += line.text;
        result.push_back('\n');
      }
    } else {
      for (std::size_t index = 0; index < scalarLines.size(); ++index) {
        if (index > 0U) {
          const ScalarLine &previous = scalarLines[index - 1U];
          const ScalarLine &current  = scalarLines[index];
          result.push_back(
              previous.text.empty() || current.text.empty() || previous.moreIndented || current.moreIndented ? '\n'
                                                                                                             : ' ');
        }
        result += scalarLines[index].text;
      }
      if (!scalarLines.empty())
        result.push_back('\n');
    }

    if (chompingMethod == '-') {
      while (!result.empty() && result.back() == '\n')
        result.pop_back();
    } else if (chompingMethod != '+') {
      while (!result.empty() && result.back() == '\n')
        result.pop_back();
      if (!scalarLines.empty())
        result.push_back('\n');
    }
    return YamlValue(result);
  }
};

std::vector<std::string> readLines(std::istream &input) {
  std::vector<std::string> lines;
  std::string              line;
  while (std::getline(input, line))
    lines.push_back(line);
  return lines;
}

} // namespace

YamlValue YamlParser::parseFile(const std::string &filename) const {
  std::ifstream input(filename);
  if (!input.is_open())
    throw FileException(filename);
  return parse(input);
}

YamlValue YamlParser::parse(std::istream &input) const {
  DocumentParser parser(readLines(input));
  return parser.parseDocument();
}

YamlValue YamlParser::parseText(const std::string &yamlText) const {
  std::istringstream input(yamlText);
  return parse(input);
}

} // namespace yamlparser
